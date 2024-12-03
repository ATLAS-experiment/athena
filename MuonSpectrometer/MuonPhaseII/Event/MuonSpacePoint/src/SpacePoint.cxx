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
    SpacePoint::SpacePoint(const ActsGeometryContext& gctx,
                           const xAOD::UncalibratedMeasurement* primaryMeas,
                           const xAOD::UncalibratedMeasurement* secondaryMeas):
        m_primaryMeas{primaryMeas},
        m_secondaryMeas{secondaryMeas},
        m_dir{xAOD::channelDirInChamber(gctx, primaryMeas)},
        m_normal{xAOD::channelNormalInChamber(gctx, primaryMeas)} {
        
        AmgSymMatrix(2) Jac{AmgSymMatrix(2)::Identity()}, uvcov {AmgSymMatrix(2)::Identity()};
        

        if (primaryMeas->numDimensions() == 1) {
            uvcov(0,0) = primaryMeas->localCovariance<1>()[0];
        }
        Jac.col(0)  = m_normal.block<2,1>(0,0).unit();
        if (secondaryMeas) {
            /// Position of the measurements expressed in the sector frame
            const Amg::Vector3D pos1{xAOD::positionInChamber(gctx, primaryMeas)};
            const Amg::Vector3D pos2{xAOD::positionInChamber(gctx, secondaryMeas)};
            /// Direction along which the measurement strips point to
            const Amg::Vector3D dir2{xAOD::channelDirInChamber(gctx, secondaryMeas)};
            /// Intersect the two channels to define the space point
            m_pos = pos1 + Amg::intersect<3>(pos2,dir2, pos1, m_dir).value_or(0) * m_dir;
            Jac.col(1)  = xAOD::channelNormalInChamber(gctx, secondaryMeas).block<2,1>(0,0).unit();
            uvcov(1,1) = secondaryMeas->localCovariance<1>()[0]; 
        } else { 
            m_pos = xAOD::positionInChamber(gctx, primaryMeas);
            Jac.col(1) = m_dir.block<2,1>(0,0).unit();
            if (primaryMeas->type() == xAOD::UncalibMeasType::MdtDriftCircleType) {
                const xAOD::MdtDriftCircle* dc = static_cast<const xAOD::MdtDriftCircle*>(primaryMeas);
                uvcov(1,1) = 0.5* dc->readoutElement()->activeTubeLength(dc->measurementHash());
            } else if (primaryMeas->type() == xAOD::UncalibMeasType::RpcStripType) {
                if (primaryMeas->numDimensions() == 1) {
                    const xAOD::RpcStrip* strip = static_cast<const xAOD::RpcStrip*>(primaryMeas);
                    uvcov(1,1) = strip->measuresPhi() ? 0.5* strip->readoutElement()->stripPhiLength():
                                                        0.5* strip->readoutElement()->stripEtaLength();
                }
            } else if (primaryMeas->type() == xAOD::UncalibMeasType::TgcStripType) {
                const xAOD::TgcStrip* strip = static_cast<const xAOD::TgcStrip*>(primaryMeas);
                if (strip->measuresPhi()) {
                    uvcov(1,1) = 0.5 * strip->readoutElement()->stripLayout(strip->gasGap()).stripLength(strip->channelNumber());
                } else {
                    uvcov(1,1) = 0.5 * strip->readoutElement()->wireGangLayout(strip->gasGap()).stripLength(strip->channelNumber());
                }
            } else if (primaryMeas->type() == xAOD::UncalibMeasType::MMClusterType) {
                const xAOD::MMCluster* clust = static_cast<const xAOD::MMCluster*>(primaryMeas);
                uvcov(1,1) = 0.5 * clust->readoutElement()->stripLayer(clust->measurementHash()).design().stripLength(clust->channelNumber());
            } else if (primaryMeas->type() == xAOD::UncalibMeasType::sTgcStripType) {
                const xAOD::sTgcMeasurement* meas = static_cast<const xAOD::sTgcMeasurement*>(primaryMeas);
                switch (meas->channelType()) {
                    case sTgcIdHelper::sTgcChannelTypes::Strip:
                        uvcov(1,1) = 0.5 *  meas->readoutElement()->stripDesign(meas->measurementHash()).stripLength(meas->channelNumber());
                        break;
                    case sTgcIdHelper::sTgcChannelTypes::Wire:
                        uvcov(1,1) = 0.5 *  meas->readoutElement()->wireDesign(meas->measurementHash()).stripLength(meas->channelNumber());
                        break;
                    /// Do nothing for the pads
                    case sTgcIdHelper::sTgcChannelTypes::Pad:
                        break;
                }
            }
            uvcov(1,1) = std::pow(uvcov(1,1), 2);
        }

        /// In case of 2D measurements like sTgc-pads or BI-RPC strips we can directly take the covariance
        /// from the measurement itself. To indicate that the space point measures both, eta & phi coordinate
        /// set the secondary measurement to be the primary one
        if (primaryMeas->numDimensions() == 2) {
            uvcov = xAOD::toEigen(primaryMeas->localCovariance<2>());
            m_secondaryMeas = m_primaryMeas;
        }
        m_measCovariance = Jac.inverse() * uvcov * Jac;
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
