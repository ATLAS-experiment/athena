/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonReadoutGeometryR4/CylinderBoundsExpander.h"
#include "MuonReadoutGeometryR4/MuonDetectorDefs.h"

#ifndef SIMULATIONBASE

#include "ActsGeometryInterfaces/GeometryContext.h"
#include "Acts/Geometry/CylinderVolumeBounds.hpp"
#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Surfaces/RectangleBounds.hpp"
#include "Acts/Surfaces/TrapezoidBounds.hpp"
#include "Acts/Surfaces/DiamondBounds.hpp"

std::array<double, 2> rectangle(const Acts::RegularSurface& surface) {
    switch(surface.bounds().type()) {
        using enum Acts::SurfaceBounds::BoundsType;
        case eRectangle:{        
            using enum Acts::RectangleBounds::BoundValues;
            const auto& bounds = static_cast<const Acts::RectangleBounds&>(surface.bounds());
            return {bounds.get(eMaxX), bounds.get(eMaxY)};
        } case eTrapezoid: {
            using enum Acts::TrapezoidBounds::BoundValues;
            const auto& bounds = static_cast<const Acts::TrapezoidBounds&>(surface.bounds());
            return {std::max(bounds.get(eHalfLengthXnegY),bounds.get(eHalfLengthXposY)),
                            bounds.get(eHalfLengthY)};
        } case eDiamond: {
            using enum Acts::DiamondBounds::BoundValues;
            const auto& bounds = static_cast<const Acts::DiamondBounds&>(surface.bounds());
            return {std::max(bounds.get(eHalfLengthXnegY),bounds.get(eHalfLengthXposY)),
                    std::max(bounds.get(eHalfLengthYneg), bounds.get(eHalfLengthYneg))};
        } default: break;
    }
    return Acts::filledArray<double, 2>(0.);
}

Amg::Vector2D planeDirFromTrf(const Acts::Transform3& trf, unsigned k){
    const Amg::Vector2D d = trf.linear().col(k).block<2,1>(0,0);
    return d.perp2() > Acts::s_onSurfaceTolerance ? d.unit() : Amg::Vector2D::Zero();
}

namespace MuonGMR4{
     CylinderBoundsExpander::CylinderBoundsExpander(std::unique_ptr<const Acts::Logger> loggerObj):
            BoundsExpander{Acts::Transform3::Identity(), std::move(loggerObj)} {}
    void CylinderBoundsExpander::setMargin(const double extraR, const double extraZ) {
        m_extraR = extraR;
        m_extraZ = extraZ;
    }
    void CylinderBoundsExpander::expand(const Acts::GeometryContext& tgContext, 
                                        const Acts::Volume& volume) {
        const Acts::Transform3& volTrf{volume.localToGlobalTransform(tgContext)};
        ACTS_DEBUG(__func__<<"() "<<__LINE__<<" - Expand bounds to fit "<<Amg::toString(volTrf)
                                  <<", "<<volume.volumeBounds());
        /** Radial distance 
         *  r^{2} = <X_{0} + a * D_{0} + b * D_{1}, X_{0} + a * D_{0} + b * D_{1}>
         *        =  X_{0}^{2} + 2 *a * <X_{0}, D_{0}> +  2*b<X_{0}, D_{1}>
         *                      + 2*a*b<D_{0}, D_{1}>  + a^{2} + b^{2}
         *  --> 0 =  <X_{0}, D_{0}> + b*<D_{0}, D_{1}> + a
         *  --> 0 =  <X_{0}, D_{1}> + a*<D_{0}, D_{1}> + b
         * 
        */
        switch(volume.volumeBounds().type()) {
            using enum Acts::VolumeBounds::BoundsType;
            case eCuboid:
            case eTrapezoid:
            case eDiamond: {
                /** Define the bounding box */
                for (const auto& oriented : volume.volumeBounds().orientedSurfaces(volTrf)) {
                    const Acts::Transform3& trf{oriented.surface->localToGlobalTransform(tgContext)};
                    ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" - Oriented surface at @"
                            <<Amg::toString(trf.translation())<<", normal: "
                            <<Amg::toString(trf.linear().col(2))<<", "<<oriented.surface->bounds());
                    const Amg::Vector2D p = trf.translation().block<2,1>(0,0);
                    const auto [hX, hY] = rectangle(*oriented.surface);
                    const Amg::Vector2D d0 = planeDirFromTrf(trf, 0);
                    const Amg::Vector2D d1 = planeDirFromTrf(trf, 1);
                    const double sX = std::clamp(p.dot(d0), -hX, hX);
                    const double sY = std::clamp(p.dot(d1), -hY, hY);
                    ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" - Rectangle hX:"<<hX<<", hY: "<<hY
                                  <<", p: "<<Amg::toString(p)<<", d0: "<<Amg::toString(d0)<<", sX: "<<sX
                                  <<", d1: "<<Amg::toString(d1)<<", sY: "<<sY);
                    const Amg::Vector2D minR = p - sX*d0  - sY * d1;
                    const Amg::Vector2D maxR = p + hX*d0  + hY * d1;
                    ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" - minR: "<<Amg::toString(minR)
                                                          <<", maxR: "<<Amg::toString(maxR));
                    m_rMin = std::min(m_rMin, minR.mag());
                    m_rMax = std::max(m_rMax, maxR.mag());
                    ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" - Update rMin: "<<m_rMin<<", rMax: "<<m_rMax);
                    for(const Amg::Vector3D& vertex: cornerPoints(tgContext, *oriented.surface)){
                        m_zMin = std::min(m_zMin, vertex.z());
                        m_zMax = std::max(m_zMax, vertex.z());
                    }
                }
                break;
            } default: {
                for(const Amg::Vector3D& vertex: cornerPoints(tgContext, volume)) {
                    const double r = vertex.perp();
                    m_rMax = std::max(m_rMax, r);
                    m_rMin = std::min(m_rMin, r);
                    m_zMax = std::max(m_zMax, vertex.z());
                    m_zMin = std::min(m_zMin, vertex.z());
                }
                break;
            }
        }
        ACTS_DEBUG(__func__<<"() "<<__LINE__<<"- New boundaries are: ["<<m_rMin<<";"<<m_rMax<<"]"
                <<", z: ["<<m_zMax<<";"<<m_zMin<<"]");
    }
    std::shared_ptr<Acts::VolumeBounds> 
        CylinderBoundsExpander::makeBounds(Acts::VolumeBoundFactory& factory) {
        reCenter(std::array{Amg::Vector3D{0.,0., m_zMax},
                            Amg::Vector3D{0.,0., m_zMin}});
        auto bounds = factory.makeBounds<Acts::CylinderVolumeBounds>(std::max(m_rMin - m_extraR, 0.), 
                                                               m_rMax + m_extraR, 
                                                              0.5*(m_zMax - m_zMin) + m_extraZ);
        ACTS_DEBUG(__func__<<"() "<<__LINE__<<" - created new bounds "<<(bounds));
        return bounds;
    }
    double  CylinderBoundsExpander::rMin() const { return m_rMin; }
    double& CylinderBoundsExpander::rMin() { return m_rMin; }
    double  CylinderBoundsExpander::rMax() const { return m_rMax; }
    double& CylinderBoundsExpander::rMax() { return m_rMax; }
    double  CylinderBoundsExpander::zMin() const { return m_zMin; }
    double& CylinderBoundsExpander::zMin() { return m_zMin; }
    double  CylinderBoundsExpander::zMax() const { return m_zMax; }
    double& CylinderBoundsExpander::zMax() { return m_zMax; }
}
#endif