/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonReadoutGeometryR4/TrapezoidBoundsExpander.h"

#ifndef SIMULATIONBASE
///
#include "Acts/Geometry/TrapezoidVolumeBounds.hpp"
#include "Acts/Geometry/CuboidVolumeBounds.hpp"
#include "Acts/Definitions/Tolerance.hpp"
#include "Acts/Surfaces/detail/PlanarHelper.hpp" 
#include "Acts/Definitions/Units.hpp"
#include "MuonReadoutGeometryR4/MuonDetectorDefs.h"

using namespace Acts::PlanarHelper;
using namespace Acts::UnitLiterals;

namespace MuonGMR4 {
std::string toString(const std::vector<TrapezoidBoundsExpander::TrapezoidPatch>& patches) {
    std::stringstream sstr{};
    for (const auto& patch : patches) {
        sstr<<"  **** "<<patch<<std::endl;
    }
    return sstr.str();
}


TrapezoidBoundsExpander::TrapezoidPatch::TrapezoidPatch(const Acts::Transform3& trf,
                                        std::shared_ptr<const Acts::VolumeBounds> volBounds):
    toCenter{trf}, bounds{std::move(volBounds)} {}
void TrapezoidBoundsExpander::TrapezoidPatch::print(std::ostream& ostr) const {
   ostr<<"BL: "<<Amg::toString(makeVertex(*this, Side::left, Level::bottom))
       <<", TL: "<<Amg::toString(makeVertex(*this, Side::left, Level::top))
       <<", BR: "<<Amg::toString(makeVertex(*this, Side::right, Level::bottom))
       <<", TR: "<<Amg::toString(makeVertex(*this, Side::right, Level::top))
       <<", bounds: "<<(*bounds)
       <<", shift: "<<Amg::toString(toCenter);
}
double TrapezoidBoundsExpander::TrapezoidPatch::area() const {
    return 2.*MuonGMR4::halfY(*bounds)*(MuonGMR4::halfXhighY(*bounds) + MuonGMR4::halfXhighY(*bounds));
}
Amg::Vector3D TrapezoidBoundsExpander::makeVertex(const TrapezoidPatch& trapezoid,
                                                  const Side side, const Level lvl) {
    const double halfX = lvl == Level::top ? MuonGMR4::halfXhighY(*trapezoid.bounds)
                                           : MuonGMR4::halfXlowY(*trapezoid.bounds);
    return trapezoid.toCenter * Amg::Vector3D{Acts::copySign(halfX, side),
                                              Acts::copySign(MuonGMR4::halfY(*trapezoid.bounds), lvl), 0.};
}

void TrapezoidBoundsExpander::expand(const Acts::GeometryContext& tgContext,
                                     const Acts::Volume& volume){
    ACTS_DEBUG(__func__<<"() "<<__LINE__<<" - Bounds "<<volume.volumeBounds());
    reCenter(cornerPoints(tgContext, volume));
    m_constituents.emplace_back(volume.localToGlobalTransform(tgContext), 
                                volume.volumeBoundsPtr());
}

std::uint8_t TrapezoidBoundsExpander::toIndex(const Side side, const Level lvl) {
    return (side == Side::left) + 2*(lvl == Level::top); 
}

double TrapezoidBoundsExpander::distanceToTrapezoid(const Amg::Vector3D& vertex,
                                                    const Amg::Vector3D& edge,
                                                    const Amg::Vector3D& testMe,
                                                    const Side side) {
    const Amg::Vector3D closest = intersectPlane(vertex, edge, 
                                                 Amg::Vector3D::UnitY(), testMe).position();
   return (testMe.x() - closest.x()) *Acts::toUnderlying(side);                        
}

void TrapezoidBoundsExpander::setMargin(const double margin) {
    ACTS_DEBUG(__func__<<"() "<<__LINE__<<" - Assign "<<margin<<" to all vertices");
    m_margin.fill(margin);
}

void TrapezoidBoundsExpander::setMargin(const double marginXlow, const double marginXhigh,
                                        const double marginY, const double marginZ) {
    m_margin[0] = marginXlow;
    m_margin[1] = marginXhigh;
    m_margin[2] = marginY;
    m_margin[3] = marginZ;
    ACTS_DEBUG(__func__<<"() "<<__LINE__<<" - Assign "<<marginXlow<<"/"<<marginXhigh
        <<" to  x-edges , "<<marginY<<" to the y-edge, "<<marginZ<<" to the z-edge as extra margin");
}

std::shared_ptr<Acts::VolumeBounds> 
    TrapezoidBoundsExpander::makeBounds(Acts::VolumeBoundFactory& factory) {
    if (m_finalBounds) {
        return m_finalBounds;
    }
    const auto boxMin = minima();
    const auto boxMax = maxima();
  
    const double halfZ = 0.5*(boxMax[Amg::z] - boxMin[Amg::z]);
    const double halfY = 0.5*(boxMax[Amg::y] - boxMin[Amg::y]);

    if (std::ranges::all_of(m_constituents, [](const TrapezoidPatch& patch) {
        return patch.bounds->type() == Acts::VolumeBounds::BoundsType::eCuboid;
        })) {
        ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" Only cuboids have been passed. Construct a cuboid");
        m_finalBounds = factory.makeBounds<Acts::CuboidVolumeBounds>(0.5*(boxMax[Amg::x] - boxMin[Amg::x]+ 
                                                                     std::max(m_margin[0], m_margin[1])), 
                                                                     halfY + 0.5*m_margin[2], 
                                                                     halfZ + 0.5* m_margin[3]);
        m_constituents.clear();
        return m_finalBounds;
    }
    /** shift first the constituents by the centering transform */
    for (TrapezoidPatch &trapezoid : m_constituents) {
        trapezoid.toCenter = toVertexCenter() * trapezoid.toCenter;
        ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" - Trapezoid after shift: "<<trapezoid);
        /// Hack to cope with the RPCs which may be rotated by 180 degrees around the x or z-axis in cases,
        /// they're upside down.
        GeoTrf::CoordEulerAngles rotAngles = GeoTrf::getCoordRotationAngles(trapezoid.toCenter);
        if (std::abs(rotAngles.gamma - 180._degree)< Acts::s_epsilon) {
            trapezoid.toCenter = trapezoid.toCenter *Amg::getRotateZ3D(180._degree);
            ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" - Flip trapezoid around Z: "
                    <<Amg::toString(trapezoid.toCenter));
        }
        if (std::abs(rotAngles.alpha - 180._degree)< Acts::s_epsilon) {
            trapezoid.toCenter = trapezoid.toCenter *Amg::getRotateX3D(180._degree);
            ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" - Flip trapezoid around X: "
                    <<Amg::toString(trapezoid.toCenter));
        }
    }

    std::array<Amg::Vector3D, 4> expandedCorners{Acts::filledArray<Amg::Vector3D, 4>(Amg::Vector3D::Zero())};
    for (const Side side : {Side::left, Side::right}) {
        std::ranges::sort(m_constituents, [side](const auto& a, const auto& b){
            const Amg::Vector3D botA = makeVertex(a, side, Level::bottom);
            const Amg::Vector3D botB = makeVertex(b, side, Level::bottom);
            if (std::abs(botA.y() - botB.y()) > Acts::s_onSurfaceTolerance) {
                return botA.y() < botB.y();
            }
            return (botA.x()  - botB.x()) * Acts::toUnderlying(side) > 0.;
        });
        std::size_t highPatch{0ul};
        /** Find the first envelope which is a trapezoid (Needed to keep the 
          * opening angle low in EIL4/5 */
        std::optional<std::size_t> lowPatch{};
        for (std::size_t patch = 0ul; patch < m_constituents.size(); ++ patch) {
            if (!lowPatch.has_value() && m_constituents.at(patch).bounds->type() == Acts::VolumeBounds::eTrapezoid) {
                lowPatch = patch;
            }
            const Amg::Vector3D botA = makeVertex(m_constituents.at(highPatch),
                                                  side, Level::bottom);
            const Amg::Vector3D botB = makeVertex(m_constituents.at(patch),
                                                  side, Level::bottom);
            ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" - Test: "<<patch<<" position: "
                    <<Amg::toString(botB)<<" --- best: "<<highPatch<<" position: "
                    <<Amg::toString(botA)<<", distance: "<<(botB.x()  - botA.x())<<", side: "<<side);
            if ((botB.x()  - botA.x()) * Acts::toUnderlying(side) > 0.) {
                highPatch = patch;
            }
        }
        ACTS_DEBUG(__func__<<"() "<<__LINE__<<" - Expand corners at "<<side
                 <<" side. Resorted trapezoids:\n"<<toString(m_constituents)
                 <<"\n   ---- Selected upper trapezoid:\n"
                 <<m_constituents.at(highPatch));
      
        findTrapezoidEdge(expandedCorners[toIndex(side, Level::bottom)],
                          expandedCorners[toIndex(side, Level::top)], side, 
                          lowPatch.value_or(0), highPatch);
    }
    const double halfXhighY = std::max(std::abs(expandedCorners[toIndex(Side::left, Level::top)].x()),
                                       std::abs(expandedCorners[toIndex(Side::right, Level::top)].x())); 
    const double halfXlowY = std::max(std::abs(expandedCorners[toIndex(Side::left, Level::bottom)].x()),
                                      std::abs(expandedCorners[toIndex(Side::right, Level::bottom)].x())); 
    if (std::abs(halfXhighY - halfXlowY) > Acts::s_onSurfaceTolerance) {
        m_finalBounds = factory.makeBounds<Acts::TrapezoidVolumeBounds>(halfXlowY + 0.5*m_margin[0], 
                                                                        halfXhighY + 0.5*m_margin[1],
                                                                        halfY + 0.5*m_margin[2], 
                                                                        halfZ + 0.5* m_margin[3]);
    } else {
        m_finalBounds = factory.makeBounds<Acts::CuboidVolumeBounds>(halfXlowY + 0.5*std::max(m_margin[0], m_margin[1]), 
                                                                     halfY + 0.5*m_margin[2], 
                                                                     halfZ + 0.5* m_margin[3]);
    }
    ACTS_DEBUG(__func__<<"() "<<__LINE__<<" - Created new bounds "<<(*m_finalBounds));
    m_constituents.clear();
    return m_finalBounds;
}
void TrapezoidBoundsExpander::findTrapezoidEdge(Amg::Vector3D& botCorner,
                                                Amg::Vector3D& topCorner,
                                                const Side side,
                                                const std::size_t lowPatch,
                                                const std::size_t highPatch) const {
    const double halfY = 0.5*(maxima()[Amg::y] - minima()[Amg::y]);
    botCorner = makeVertex(m_constituents.at(lowPatch), side, Level::bottom);
    topCorner = makeVertex(m_constituents.at(highPatch), side, Level::top);
    Amg::Vector3D edge = Amg::projectDirOntoPlane((botCorner - topCorner).unit(), 
                                                      Amg::Vector3D::UnitZ());
    botCorner = intersectPlane(botCorner, edge, Amg::Vector3D::UnitY(), -halfY).position();
    topCorner = intersectPlane(topCorner, edge, Amg::Vector3D::UnitY(), halfY).position();
    
    double steepM{0.}; 
    /** @brief Update the slope of the tilted trapezoid edge to find the steepest one */
    auto updateSlope = [&]( const Amg::Vector3D& v) {
         if (v.y() <= botCorner.y()) {
            return;
         }
         const double m = (v.x() - botCorner.x()) / (v.y() - botCorner.y());
         if(std::abs(m)> std::abs(steepM)) {
            steepM = m;
         }
    };
    updateSlope(topCorner);
    ACTS_DEBUG(__func__<<"() "<<__LINE__<<" - Identified "<<Amg::toString(botCorner)
                <<", "<<Amg::toString(topCorner)<<" as vertices. Edge: "<<Amg::toString(edge));
    Amg::Vector3D farestVtx{botCorner};
    double area{0.};
    for (const TrapezoidPatch& patch : m_constituents) {
        area += patch.area();
        for (auto  lvl : {Level::bottom, Level::top}) {
            const Amg::Vector3D vtx = makeVertex(patch, side, lvl);
            const double d = distanceToTrapezoid(botCorner, edge, vtx, side);
            ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" - Vertex at "<<lvl
                        <<" edge "<<Amg::toString(vtx)<<" distance: "<<d);
            /** If the point is outwards update the outer vertex and the slope */
            if (d > 0. &&  d > distanceToTrapezoid(botCorner, edge, farestVtx, side)) {
                updateSlope(vtx);
                farestVtx = vtx;
                ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" - Shift the trapezoid.");
            }
        }
    }
    ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" - The farest "<<Amg::toString(farestVtx)<<" with distance "
        <<distanceToTrapezoid(botCorner, edge, farestVtx, side)<<", steepest m: "<<steepM
        <<", start M: "<<(topCorner.x() - botCorner.x()) / (topCorner.y() - botCorner.y()));

    edge = Amg::Vector3D{steepM, 1., 0.}.unit();
    botCorner = intersectPlane(farestVtx, edge, Amg::Vector3D::UnitY(), -halfY).position();
    topCorner = intersectPlane(farestVtx, edge, Amg::Vector3D::UnitY(), halfY).position();
    /// Try to decrease
    const double envArea = (std::abs(botCorner.x()) + std::abs(topCorner.x()))* 2.*halfY;
    ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" - Update corners to "<<Amg::toString(botCorner)
        <<" (bottom) and "<<Amg::toString(topCorner)<<" (top) New area: "<<envArea
    <<", old area: "<<area<<", diff: "<<(area/ envArea));
}
    
}
#endif
