/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "BucketDumperAlg.h"

#include "StoreGate/ReadHandle.h"
#include "MuonTesterTree/EventHashBranch.h"
#include "MuonSpacePoint/SpacePointPerLayerSplitter.h"
#include "xAODMuonPrepData/UtilFunctions.h"
#include "xAODMuonPrepData/MdtDriftCircle.h"
#include <fstream>
#include <TString.h>
#include <AthenaKernel/RNGWrapper.h>
#include "CLHEP/Random/RandFlat.h"

namespace MuonR4{
    StatusCode BucketDumperAlg::initialize() {
        ATH_CHECK(m_readKey.initialize());
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_geoCtxKey.initialize());
        ATH_CHECK(m_inSegmentKey.initialize(!m_inSegmentKey.empty()));
        m_tree.addBranch(std::make_shared<MuonVal::EventHashBranch>(m_tree.tree()));
        ATH_CHECK(m_visionTool.retrieve(EnableTool{!m_visionTool.empty()}));        
        if (m_visionTool.empty()) {
            m_tree.disableBranch(m_spoint_trueLabel.name());
        }
        ATH_CHECK(m_tree.init(this));
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_MSG_DEBUG("Successfully initialized");

        return StatusCode::SUCCESS;
    }

    StatusCode BucketDumperAlg::finalize() {
        ATH_CHECK(m_tree.write());
        return StatusCode::SUCCESS;
    }
    
    StatusCode BucketDumperAlg::execute(){
        const EventContext& ctx{Gaudi::Hive::currentContext()};

        std::unordered_map <const SpacePointBucket*, std::vector<const MuonR4::Segment*>> segmentMap;  // MuonR4Segment 
        
        const SegmentContainer* readSegment{nullptr};
        ATH_CHECK(SG::get(readSegment, m_inSegmentKey, ctx));
        for (const MuonR4::Segment* segment : *readSegment) {
            segmentMap[segment->parent()->parentBucket()].push_back(segment);
        }

        const ActsGeometryContext* gctx{nullptr};
        ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));

        const SpacePointContainer* spContainer{nullptr};
        ATH_CHECK(SG::get(spContainer, m_readKey, ctx));

        CLHEP::HepRandomEngine* rndEngine = getRandomEngine(ctx);

        for(const SpacePointBucket* bucket : *spContainer) {

            if (!m_isMC && segmentMap[bucket].size() && m_fracToKeep < 1. &&
                CLHEP::RandFlat::shoot(rndEngine,0.,1.) > m_fracToKeep) {
                ATH_MSG_VERBOSE("Skipping bucket without segment");
                continue;
            }            

            m_bucket_min      = bucket->coveredMin();
            m_bucket_max      = bucket->coveredMax();
            m_bucket_segments = segmentMap[bucket].size();

            std::unordered_map<const SpacePoint*, std::vector<int16_t>> spacePointToSegment;
            
            auto match_itr = segmentMap.find(bucket);

            if (match_itr != segmentMap.end()) {
                unsigned int segIdx{0};
                for (const MuonR4::Segment* segment : match_itr->second) {
                    for (const auto& meas : segment->measurements()) {
                        if (meas->fitState() == CalibratedSpacePoint::State::Valid) {
                            spacePointToSegment[meas->spacePoint()].push_back(segIdx);
                        }
                    }
                    ++segIdx;
                }
                
                for ( const MuonR4::Segment* segment: match_itr->second) {
                    m_segmentPos.push_back(segment->position());
                    m_segmentDir.push_back(segment->direction());
                    m_segment_chiSquared.push_back(segment->chi2());
                    m_segment_numberDoF.push_back(segment->nDoF());
                }
            }

            SpacePointPerLayerSplitter splitter{*bucket};
            unsigned int layer{0};

            for (const auto& hitsInLay : splitter.mdtHits()) {
                for (const auto sp : hitsInLay){

                    const auto* dc = static_cast<const xAOD::MdtDriftCircle*>(sp->primaryMeasurement());
                    if (dc->status() != Muon::MdtDriftCircleStatus::MdtStatusDriftTime){
                        continue;
                    }

                    m_spoint_id.push_back(sp->identify());

                    Identifier id = sp->identify();
                    unsigned int mdt_ML = m_idHelperSvc->mdtIdHelper().multilayer(id);
                    unsigned int mdt_TL = m_idHelperSvc->mdtIdHelper().tubeLayer(id);
                    unsigned int mdt_nL = m_idHelperSvc->mdtIdHelper().tubeLayerMax(id);
                    m_spoint_mdtLayer   = mdt_ML * (mdt_nL-1) + mdt_TL;
                    m_spoint_mdtTube    = m_idHelperSvc->mdtIdHelper().tube(id);

                    const std::vector<int16_t>& segIdxs = spacePointToSegment[sp];

                    m_spoint_mat[m_spoint_mat.size()] = segIdxs;
                    m_spoint_nSegments.push_back(segIdxs.size());
                
                    m_bucket_spacePoints = bucket->size();
                    m_spoint_localPosition.push_back(sp->positionInChamber());
                    m_spoint_adc.push_back(dc->adc());
                    m_spoint_tdc.push_back(dc->tdc());
                    m_spoint_covX.push_back(sp->covariance()(Amg::x, Amg::x));
                    m_spoint_covY.push_back(sp->covariance()(Amg::y, Amg::y));
                    m_spoint_covXY.push_back(sp->covariance()(Amg::x, Amg::y));
                    m_spoint_covYX.push_back(sp->covariance()(Amg::y, Amg::x));
                    m_spoint_driftR.push_back(sp->driftRadius());
                    m_spoint_measuresEta.push_back(sp->measuresEta());
                    m_spoint_measuresPhi.push_back(sp->measuresPhi());
                    m_spoint_nEtaInstances.push_back(sp->nEtaInstanceCounts());
                    m_spoint_nPhiInstances.push_back(sp->nPhiInstanceCounts());
                    m_spoint_dimension.push_back(sp->dimension());
                    m_spoint_layer.push_back(layer);
                    m_spoint_isMdt.push_back(true);
                    m_spoint_isStrip.push_back(false);
                    if (m_visionTool.isEnabled()) {
                        m_spoint_trueLabel.push_back(m_visionTool->isLabeled(*sp));
                    }

                    Amg::Vector3D globalPos = sp->msSector()->localToGlobalTrans(*gctx) * sp->positionInChamber();
                    m_spoint_globalPosition.push_back( globalPos );

                }
                ++layer;

            }

            for (const auto& hitsInLay : splitter.stripHits()) {

                for (const auto sp : hitsInLay){

                    const std::vector<int16_t>& segIdxs = spacePointToSegment[sp];

                    m_spoint_mat[m_spoint_mat.size()] = segIdxs;
                    m_spoint_nSegments.push_back(segIdxs.size());

                    m_spoint_id.push_back(sp->identify());
                    m_bucket_spacePoints = bucket->size();
                    m_spoint_localPosition.push_back(sp->positionInChamber());

                    m_spoint_covX.push_back(sp->covariance()(Amg::x, Amg::x));
                    m_spoint_covY.push_back(sp->covariance()(Amg::y, Amg::y));
                    m_spoint_covXY.push_back(sp->covariance()(Amg::x, Amg::y));
                    m_spoint_covYX.push_back(sp->covariance()(Amg::y, Amg::x));
                    m_spoint_driftR.push_back(sp->driftRadius());
                    m_spoint_measuresEta.push_back(sp->measuresEta());
                    m_spoint_measuresPhi.push_back(sp->measuresPhi());
                    m_spoint_nEtaInstances.push_back(sp->nEtaInstanceCounts());
                    m_spoint_nPhiInstances.push_back(sp->nPhiInstanceCounts());
                    m_spoint_dimension.push_back(sp->dimension());
                    m_spoint_layer.push_back(layer);
                    m_spoint_isStrip.push_back(true);
                    m_spoint_isMdt.push_back(false);
                    if (m_visionTool.isEnabled()) {
                        m_spoint_trueLabel.push_back(m_visionTool->isLabeled(*sp));
                    }

                    Amg::Vector3D globalPos = sp->msSector()->localToGlobalTrans(*gctx) * sp->positionInChamber();
                    m_spoint_globalPosition.push_back( globalPos );

                    // check the technology to fill channel, adc and tdc... pushing back 0 for now
                    m_spoint_adc.push_back(0); 
                    m_spoint_tdc.push_back(0);
                }
                ++layer;
            }

            m_bucket_layers = layer;

            if (!m_tree.fill(ctx)) return StatusCode::FAILURE; 

        }

        return StatusCode::SUCCESS;

    }

    CLHEP::HepRandomEngine* BucketDumperAlg::getRandomEngine(const EventContext&ctx) const {
        ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this, m_streamName);
        std::string rngName = m_streamName;
        rngWrapper->setSeed(rngName, ctx);
        return rngWrapper->getEngine(ctx);
    }

}
