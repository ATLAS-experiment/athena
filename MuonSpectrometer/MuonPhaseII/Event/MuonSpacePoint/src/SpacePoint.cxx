/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonSpacePoint/SpacePoint.h"
#include "xAODMuonPrepData/UtilFunctions.h"

#include "MuonReadoutGeometryR4/MdtReadoutElement.h"
#include "MuonReadoutGeometryR4/RpcReadoutElement.h"
#include "MuonReadoutGeometryR4/TgcReadoutElement.h"
#include "MuonReadoutGeometryR4/MmReadoutElement.h"
#include "MuonReadoutGeometryR4/sTgcReadoutElement.h"

#include "xAODMuonPrepData/MdtDriftCircle.h"
#include "xAODMuonPrepData/RpcStrip.h"
#include "xAODMuonPrepData/TgcStrip.h"
#include "xAODMuonPrepData/MMCluster.h"
#include "xAODMuonPrepData/sTgcMeasurement.h"
#include "EventPrimitives/EventPrimitivesHelpers.h"
#include <memory>

namespace {
    /**  @brief Helper function to overwrite the existing prd multiplicity counts */
    inline void updateInstanceCounts(std::shared_ptr<const unsigned>& currCounter,
                                     std::shared_ptr<unsigned>&& newCounter) {
        if (!newCounter) {
            return;
        }
        ++(*newCounter);
        currCounter = std::move(newCounter);
    }
}

namespace MuonR4{
    SpacePoint::SpacePoint(const xAOD::UncalibratedMeasurement* primMeas,
                           const xAOD::UncalibratedMeasurement* secondMeas) : 
        m_primaryMeas{primMeas},
        m_secondaryMeas{secondMeas} {

        /// In case of 2D measurements like sTgc-pads or BI-RPC strips we can directly take the covariance
        /// from the measurement itself. To indicate that the space point measures both, eta & phi coordinate
        /// set the secondary measurement to be the primary one
        if (primMeas->numDimensions() == 2) {
            m_secondaryMeas = m_primaryMeas;
        }                            
    }
            
    const xAOD::UncalibratedMeasurement* SpacePoint::primaryMeasurement() const {
       return m_primaryMeas;
    }
    const xAOD::UncalibratedMeasurement* SpacePoint::secondaryMeasurement() const {
       return m_secondaryMeas;
    }
    const MuonGMR4::Chamber* SpacePoint::chamber() const {
        return m_chamber;
    }
    const MuonGMR4::SpectrometerSector* SpacePoint::msSector() const {
        return m_msSector;
    }
    const Amg::Vector3D& SpacePoint::positionInChamber() const {
        return m_pos;
    }
    const Amg::Vector3D& SpacePoint::directionInChamber() const {
        return m_dir;
    } 
    const Amg::Vector3D& SpacePoint::normalInChamber() const {
        return m_normal;
    }
    Amg::Vector3D SpacePoint::planeNormal() const {
        return directionInChamber().cross(normalInChamber()).unit();
    }
    xAOD::UncalibMeasType SpacePoint::type() const {
        return primaryMeasurement()->type();
    }
    bool SpacePoint::measuresPhi() const {
        return secondaryMeasurement() || !m_measEta;
    }
    bool SpacePoint::measuresEta() const {
        return secondaryMeasurement() || m_measEta;
    }
    const Identifier& SpacePoint::identify() const {
        return xAOD::identify(m_primaryMeas);
    }
    double SpacePoint::driftRadius() const { 
        return m_primaryMeas->type() == xAOD::UncalibMeasType::MdtDriftCircleType ?  
               static_cast<const xAOD::MdtDriftCircle*>(m_primaryMeas)->driftRadius() : 0.;
    }
    Amg::Vector2D SpacePoint::uncertainty() const {
        return Amg::Vector2D{ Amg::error(m_measCovariance,0),Amg::error(m_measCovariance,1)  };
    }
    const AmgSymMatrix(2)&  SpacePoint::covariance() const {
        return m_measCovariance;
    }
    void SpacePoint::setInstanceCounts(std::shared_ptr<unsigned int> etaCounts, 
                                       std::shared_ptr<unsigned int> phiCounts) {
        updateInstanceCounts(m_etaInstances, std::move(etaCounts));
        updateInstanceCounts(m_phiInstances, std::move(phiCounts));
    }
    unsigned int SpacePoint::nEtaInstanceCounts() const { return (*m_etaInstances); }
    unsigned int SpacePoint::nPhiInstanceCounts() const { return (*m_phiInstances); }
    unsigned int SpacePoint::dimension() const { 
        return (secondaryMeasurement() != nullptr) + 1;
    }

}
