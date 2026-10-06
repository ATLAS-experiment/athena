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
        if (std::abs(rotAngles.alpha - 180._degree)< Acts::s_epsilon){
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
        for (std::size_t patch = 1ul; patch < m_constituents.size(); ++ patch) {
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
        Amg::Vector3D& botCorner{expandedCorners[toIndex(side, Level::bottom)]};
        Amg::Vector3D& topCorner{expandedCorners[toIndex(side, Level::top)]};
        ///
        botCorner = makeVertex(m_constituents.at(0), side, Level::bottom);
        topCorner = makeVertex(m_constituents.at(highPatch), side, Level::top);
        const Amg::Vector3D edge = Amg::projectDirOntoPlane((botCorner - topCorner).unit(), 
                                                            Amg::Vector3D::UnitZ());
        /** Ensure that the corners are on the top and bottom of the box */
        for (Level lvl : { Level::bottom, Level::top}) {
            const std::uint8_t i = toIndex(side,lvl);
            auto iSect = intersectPlane(expandedCorners.at(i), edge,
                                        Amg::Vector3D::UnitY(),
                                        Acts::copySign(halfY, lvl));
            expandedCorners.at(i)= iSect.position();
        }
        ACTS_DEBUG(__func__<<"() "<<__LINE__<<" - Identified "<<Amg::toString(botCorner)
                 <<", "<<Amg::toString(topCorner)<<" as vertices. Edge: "<<Amg::toString(edge));
        /** Now we just move them outwards */
        for (const TrapezoidPatch& patch : m_constituents) {
            const double distBot = distanceToTrapezoid(botCorner, edge, 
                                        makeVertex(patch, side, Level::bottom), side);
            const double distTop = distanceToTrapezoid(topCorner, edge, 
                                        makeVertex(patch, side, Level::top), side);
            ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" - Bottom (top) distance "<<distBot
                              <<" ("<<distTop<<") to "<<patch);
            /** The point is outside the trapzoid */
            if (std::max(distBot, distTop) > 0.) {
                const double d = std::max(distBot, distTop) * Acts::toUnderlying(side);
                botCorner.x() += d;
                topCorner.x() += d; 
                ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" - Shift the trapezoid.");
            }
        }
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
    
}
#endif
