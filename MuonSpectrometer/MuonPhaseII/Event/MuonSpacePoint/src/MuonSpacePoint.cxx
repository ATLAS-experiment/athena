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
        
         if (primaryMeas->type() == xAOD::UncalibMeasType::MdtDriftCircleType) {
            m_driftR = primaryMeas->localPosition<1>()[0];
         }
        m_measUncerts[0] = std::sqrt(primaryMeas->localCovariance<1>()[0]);
        if (secondaryMeas) {
            const Amg::Vector3D pos1{xAOD::positionInChamber(gctx, primaryMeas)};
            const Amg::Vector3D pos2{xAOD::positionInChamber(gctx, secondaryMeas)};

            const Amg::Vector3D dir1{xAOD::chDirectionInChamber(gctx, primaryMeas)};
            const Amg::Vector3D dir2{xAOD::chDirectionInChamber(gctx, secondaryMeas)};
            /// In cases of a secondary measurement this collapses to a primary measurement
            m_pos = pos1 + Amg::intersect<3>(pos2,dir2, pos1, dir1).value_or(0) * dir1;
            m_measUncerts[1] = std::sqrt(secondaryMeas->localCovariance<1>()[0]);
        } else {
            m_pos = xAOD::positionInChamber(gctx, primaryMeas);
            if (primaryMeas->type() == xAOD::UncalibMeasType::MdtDriftCircleType) {
                const xAOD::MdtDriftCircle* dc = static_cast<const xAOD::MdtDriftCircle*>(primaryMeas);
                m_measUncerts[1] = 0.5* dc->readoutElement()->activeTubeLength(dc->measurementHash());
            } else if (primaryMeas->type() == xAOD::UncalibMeasType::RpcStripType) {
                const xAOD::RpcStrip* strip = static_cast<const xAOD::RpcStrip*>(primaryMeas);
                m_measUncerts[1] = strip->measuresPhi() ? 0.5* strip->readoutElement()->stripPhiLength():
                                                          0.5* strip->readoutElement()->stripEtaLength();
            } else if (primaryMeas->type() == xAOD::UncalibMeasType::TgcStripType) {
                const xAOD::TgcStrip* strip = static_cast<const xAOD::TgcStrip*>(primaryMeas);
                if (strip->measuresPhi()) {
                    m_measUncerts[1] = strip->readoutElement()->stripLayout(strip->gasGap()).stripLength(strip->channelNumber());
                } else {
                    m_measUncerts[1] = strip->readoutElement()->wireGangLayout(strip->gasGap()).stripLength(strip->channelNumber());
                }
            }
        }
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
