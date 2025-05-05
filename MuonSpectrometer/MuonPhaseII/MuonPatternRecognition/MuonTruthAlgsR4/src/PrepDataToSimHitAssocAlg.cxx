/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "PrepDataToSimHitAssocAlg.h"

#include "StoreGate/WriteDecorHandle.h"
#include "xAODMuonPrepData/UtilFunctions.h"
#include "xAODMuonPrepData/RpcMeasurement.h"
#include "xAODMuonPrepData/TgcStrip.h"
#include "xAODMuonPrepData/sTgcMeasurement.h"
#include "xAODMuonPrepData/MMCluster.h"
#include "xAODMuonViews/ChamberViewer.h"
namespace MuonR4{
    StatusCode PrepDataToSimHitAssocAlg::initialize() {
        ATH_CHECK(m_simHitsKey.initialize());
        ATH_CHECK(m_prdHitKey.initialize());
        ATH_CHECK(m_decorKey.initialize());
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_geoCtxKey.initialize());
        return StatusCode::SUCCESS;
    }    
    StatusCode PrepDataToSimHitAssocAlg::execute(const EventContext & ctx) const {
        const ActsGeometryContext* gctx{nullptr};
        const xAOD::MuonSimHitContainer* simHits{nullptr};
        const xAOD::UncalibratedMeasurementContainer* measurements{nullptr};
        ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));
        ATH_CHECK(SG::get(simHits, m_simHitsKey, ctx));
        ATH_CHECK(SG::get(measurements, m_prdHitKey, ctx));
        
        
        SG::WriteDecorHandle<xAOD::UncalibratedMeasurementContainer, LinkType> decorHandle{m_decorKey, ctx};
        /** Loop over the measurements */
        for (const xAOD::UncalibratedMeasurement* measurement : *measurements){
            /** Define the place holder for the closest simHit */
            const xAOD::MuonSimHit* bestSimHit{nullptr};

            switch (measurement->type()) {
                /** Drift circles can be directly matched via Identifier */
                case xAOD::UncalibMeasType::MdtDriftCircleType: {
                    const Identifier prdId{xAOD::identify(measurement)};
                    xAOD::MuonSimHitContainer::const_iterator mdt_matching = 
                        std::ranges::find_if(*simHits,[&prdId](const xAOD::MuonSimHit* hit){
                            return hit->identify() == prdId;
                        });
                    if (mdt_matching != simHits->end()){
                        bestSimHit =(*mdt_matching);
                    }
                    break;
                }
                /** The other detectors we need to find the closest hit in the gasGap */
                case xAOD::UncalibMeasType::RpcStripType:
                case xAOD::UncalibMeasType::TgcStripType:
                case xAOD::UncalibMeasType::MMClusterType:
                case xAOD::UncalibMeasType::sTgcStripType: {

                    const Identifier prdId{xAOD::identify(measurement)};
                    const MuonGMR4::MuonReadoutElement* readOutEle = xAOD::muonReadoutElement(measurement);
                    const Amg::Transform3D& locToGlob{readOutEle->localToGlobalTrans(*gctx, readOutEle->layerHash(prdId))};
                    
                    const Identifier gasGapId = m_idHelperSvc->gasGapId(prdId);
                    /** Calculate the local position */                    
                    Amg::Vector3D locPos{Amg::Vector3D::Zero()};
                    if (measurement->numDimensions() == 1) {
                        locPos = measurement->localPosition<1>().x() * Amg::Vector3D::UnitX();
                    } else {
                        locPos.block<2,1>(0,0) = xAOD::toEigen(measurement->localPosition<2>());
                    }
                    double closestDistance{m_PullCutOff};

                    /** Fetch a range of candidate hits */
                    xAOD::ChamberViewer chambViewer{*simHits, m_idHelperSvc.get(), 
                                                    xAOD::ChamberView::Mode::DetElement};

                    if (!chambViewer.loadView(gasGapId)) {
                        break;
                    }
                    for ( const xAOD::MuonSimHit* simHit : chambViewer) {
                        if (gasGapId != m_idHelperSvc->gasGapId(simHit->identify())) {
                            continue; 
                        }
                        const IdentifierHash simLayHash{readOutEle->layerHash(simHit->identify())};
                        const Amg::Transform3D globToLoc{readOutEle->globalToLocalTrans(*gctx, simLayHash) *locToGlob};
                        /** If the prepdata is expressed in the phi view, it's automatically rotated into the eta view */
                        const Amg::Vector3D prdPos = globToLoc * locPos;
                        /** 2D space points closest eucledian disance -> otherwise closest local x */
                        double dist{0.};
                        if (measurement->numDimensions() == 1) {
                            dist = std::abs(prdPos.x() - simHit->localPosition().x()) 
                                 / std::sqrt(measurement->localCovariance<1>()(0,0));
                        } else{
                            const Amg::Vector2D diff = (prdPos - xAOD::toEigen(simHit->localPosition())).block<2,1>(0,0);
                            dist = std::sqrt(diff.dot(xAOD::toEigen(measurement->localCovariance<2>()).inverse() * diff));
                        }
                        if (dist < closestDistance) {
                            closestDistance = dist;
                            bestSimHit = simHit;
                        }
                    }                    
                    break;
                } default: {
                    ATH_MSG_FATAL("Non muon measurement is parsed");
                    return StatusCode::FAILURE;
                }
            }
            if (!bestSimHit) {
                continue;
            }
            decorHandle(*measurement) = LinkType{*simHits, bestSimHit->index()};
        }
        return StatusCode::SUCCESS;
    }
}
