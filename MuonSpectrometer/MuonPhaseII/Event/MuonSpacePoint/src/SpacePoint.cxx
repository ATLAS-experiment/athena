/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonSpacePoint/SpacePoint.h"
#include "xAODMuonPrepData/UtilFunctions.h"


#include "xAODMuonPrepData/MdtDriftCircle.h"
#include "xAODMuonPrepData/RpcStrip.h"
#include "xAODMuonPrepData/TgcStrip.h"
#include "xAODMuonPrepData/MMCluster.h"
#include "xAODMuonPrepData/sTgcMeasurement.h"

#include "MuonReadoutGeometryR4/MmReadoutElement.h"
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
    using Cov_t = SpacePoint::Cov_t;
    
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
        /// Temporary hack to activate the measures phi flag for micromegas
        if (primMeas->type() == xAOD::UncalibMeasType::MMClusterType) {
            const auto* clust = static_cast<const xAOD::MMCluster*>(primMeas);
            if (clust->readoutElement()->stripLayer(clust->layerHash()).design().hasStereoAngle()) {
                m_secondaryMeas = m_primaryMeas;
            }
        }
    }
    const Amg::Vector3D& SpacePoint::localPosition() const { return m_pos; }
    const Amg::Vector3D& SpacePoint::sensorDirection() const { return m_dir; } 
    const Amg::Vector3D& SpacePoint::toNextSensor() const { return m_toNext; }
    const Amg::Vector3D& SpacePoint::planeNormal() const { return m_normal; }
    double SpacePoint::driftRadius() const { 
        return m_primaryMeas->type() == xAOD::UncalibMeasType::MdtDriftCircleType ?  
               static_cast<const xAOD::MdtDriftCircle*>(m_primaryMeas)->driftRadius() : 0.;
    }
    double SpacePoint::time() const {
        return 0.;
    }
    const Cov_t& SpacePoint::covariance() const { return m_measCovariance; }    
    bool SpacePoint::isStraw() const { return m_primaryMeas->type() == xAOD::UncalibMeasType::MdtDriftCircleType; }
    bool SpacePoint::hasTime() const { return false; }
    bool SpacePoint::measuresLoc0() const { return measuresPhi(); }
    bool SpacePoint::measuresLoc1() const { return measuresEta(); }    
    void SpacePoint::setCovariance(Cov_t&& cov){ m_measCovariance = std::move(cov); }    
    void SpacePoint::setDirection(const Amg::Vector3D& sensorDir,
                                  const Amg::Vector3D& toNextSensor) {
        m_dir = sensorDir;
        m_toNext = toNextSensor;
        m_normal = m_dir.cross(m_toNext).unit();
    }
    void SpacePoint::setPosition(Amg::Vector3D&& pos){ m_pos = std::move(pos); }
            
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
    void SpacePoint::print(std::ostream& ostr) const {

        ostr<<"Uncalibrated SP "<<msSector()->idHelperSvc()->toString(identify());        
        ostr<<" @ "<<Amg::toString(localPosition());
        if (type() == xAOD::UncalibMeasType::MdtDriftCircleType) {
            ostr<<", wire: "<<Amg::toString(sensorDirection())<<", drift R: "<<driftRadius();
        } else {
            ostr<<", (dir/toNext/planeNormal): "<<Amg::toString(sensorDirection())
                <<"/"<<Amg::toString(toNextSensor())<<"/"<<Amg::toString(planeNormal());
        }
        auto boolToStr = [](const bool B) -> std::string {
            return B ? "yay" : "nay";
        };
        if (hasTime()) {
            ostr<<", time: "<<time();
        }
        ostr<<", measures eta/phi/time: "<<boolToStr(measuresEta())
            <<"/"<<boolToStr(measuresPhi())<<"/"<<boolToStr(hasTime());
        ostr<<", covariance (eta/phi/time): ("<<m_measCovariance[Acts::toUnderlying(CovIdx::etaCov)]
             <<", "<<m_measCovariance[Acts::toUnderlying(CovIdx::phiCov)]
             <<", "<<m_measCovariance[Acts::toUnderlying(CovIdx::timeCov)]<<")"; 
    }

}
