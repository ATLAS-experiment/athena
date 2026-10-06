/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonReadoutGeometryR4/CylinderBoundsExpander.h"

#ifndef SIMULATIONBASE

#include "ActsGeometryInterfaces/GeometryContext.h"
#include "Acts/Geometry/CylinderVolumeBounds.hpp"
namespace MuonGMR4{
     CylinderBoundsExpander::CylinderBoundsExpander(std::unique_ptr<const Acts::Logger> loggerObj):
            BoundsExpander{Acts::Transform3::Identity(), std::move(loggerObj)} {}
    void CylinderBoundsExpander::expand(const Acts::GeometryContext& tgContext, 
                                        const Acts::Volume& volume) {

        for(const Amg::Vector3D& vertex: cornerPoints(tgContext, volume)) {
            const double r = vertex.perp();
            m_rMax = std::max(m_rMax, r);
            m_rMin = std::min(m_rMin, r);
            m_zMax = std::max(m_zMax, vertex.z());
            m_zMin = std::min(m_zMin, vertex.z());
        }
        ACTS_VERBOSE(__func__<<"() "<<__LINE__<<" New boundaries are: ["<<m_rMin<<";"<<m_rMax<<"]"
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