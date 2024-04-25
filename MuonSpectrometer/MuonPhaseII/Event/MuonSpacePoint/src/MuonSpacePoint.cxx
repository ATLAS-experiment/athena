/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonSpacePoint/MuonSpacePoint.h"
#include "xAODMuonPrepData/UtilFunctions.h"

#include "xAODMuonPrepData/MdtDriftCircle.h"
#include "xAODMuonPrepData/RpcStrip.h"
#include "xAODMuonPrepData/TgcStrip.h"


namespace MuonR4{
    MuonSpacePoint::MuonSpacePoint(const ActsGeometryContext& gctx,
                                   const xAOD::UncalibratedMeasurement* primaryMeas,
                                   const xAOD::UncalibratedMeasurement* secondaryMeas):
        m_primaryMeas{primaryMeas},
        m_secondaryMeas{secondaryMeas} {        
        AmgSymMatrix(2) Jac{AmgSymMatrix(2)::Identity()}, uvcov {AmgSymMatrix(2)::Identity()};
        
        if (primaryMeas->type() == xAOD::UncalibMeasType::MdtDriftCircleType) {
            m_driftR = primaryMeas->localPosition<1>()[0];
         }
        uvcov(0,0) = primaryMeas->localCovariance<1>()[0];
        Jac.col(0)  = xAOD::channelNormalInChamber(gctx, primaryMeas).block<2,1>(0,0);
        if (secondaryMeas) {
            /// Position of the measurements expressed in the chamber frame
            const Amg::Vector3D pos1{xAOD::positionInChamber(gctx, primaryMeas)};
            const Amg::Vector3D pos2{xAOD::positionInChamber(gctx, secondaryMeas)};
            /// Direction along which the measurement strips point to
            const Amg::Vector3D dir1{xAOD::channelDirInChamber(gctx, primaryMeas)};
            const Amg::Vector3D dir2{xAOD::channelDirInChamber(gctx, secondaryMeas)};
            /// Intersect the two channels to define the space point
            m_pos = pos1 + Amg::intersect<3>(pos2,dir2, pos1, dir1).value_or(0) * dir1;
            Jac.col(1)  = xAOD::channelNormalInChamber(gctx, secondaryMeas).block<2,1>(0,0);             
            uvcov(1,1) = secondaryMeas->localCovariance<1>()[0]; 
        } else { 
            m_pos = xAOD::positionInChamber(gctx, primaryMeas);
            Jac.col(1) = xAOD::channelDirInChamber(gctx, primaryMeas).block<2,1>(0,0);
            if (primaryMeas->type() == xAOD::UncalibMeasType::MdtDriftCircleType) {
                const xAOD::MdtDriftCircle* dc = static_cast<const xAOD::MdtDriftCircle*>(primaryMeas);
                uvcov(1,1) = 0.5* dc->readoutElement()->activeTubeLength(dc->measurementHash());
            } else if (primaryMeas->type() == xAOD::UncalibMeasType::RpcStripType) {
                const xAOD::RpcStrip* strip = static_cast<const xAOD::RpcStrip*>(primaryMeas);
                uvcov(1,1) = strip->measuresPhi() ? 0.5* strip->readoutElement()->stripPhiLength():
                                                    0.5* strip->readoutElement()->stripEtaLength();
            } else if (primaryMeas->type() == xAOD::UncalibMeasType::TgcStripType) {
                const xAOD::TgcStrip* strip = static_cast<const xAOD::TgcStrip*>(primaryMeas);
                if (strip->measuresPhi()) {
                    uvcov(1,1) = 0.5 * strip->readoutElement()->stripLayout(strip->gasGap()).stripLength(strip->channelNumber());
                } else {
                    uvcov(1,1) = 0.5 * strip->readoutElement()->wireGangLayout(strip->gasGap()).stripLength(strip->channelNumber());
                }
            }
            uvcov(1,1) = std::pow(uvcov(1,1), 2);
        }
        Jac = Jac.inverse().eval();
        AmgSymMatrix(2) cov = Jac * uvcov * Jac.transpose();
        m_measUncerts =  Amg::Vector2D(std::sqrt(cov(0,0)), std::sqrt(cov(1,1)));
    }
            
    const xAOD::UncalibratedMeasurement* MuonSpacePoint::primaryMeasurement() const {
       return m_primaryMeas;
    }
    const xAOD::UncalibratedMeasurement* MuonSpacePoint::secondaryMeasurement() const {
       return m_secondaryMeas;
    }
    const MuonGMR4::MuonChamber* MuonSpacePoint::muonChamber() const {
        return m_chamber;
    }
    const Amg::Vector3D& MuonSpacePoint::positionInChamber() const {
        return m_pos;
    }
    bool MuonSpacePoint::measuresPhi() const {
        return secondaryMeasurement() ||  muonChamber()->idHelperSvc()->measuresPhi(identify());
    }
    bool MuonSpacePoint::measuresEta() const {
        return secondaryMeasurement() ||  !muonChamber()->idHelperSvc()->measuresPhi(identify());
    }
    const Identifier& MuonSpacePoint::identify() const {
        return m_id;
    }
    double MuonSpacePoint::driftRadius() const { 
        return m_driftR; 
    }
    const Amg::Vector2D& MuonSpacePoint::uncertainty() const {
        return m_measUncerts;
    }

}
