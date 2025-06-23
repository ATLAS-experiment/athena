/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "BucketDumperAlg.h"

#include "StoreGate/ReadHandle.h"
#include "MuonTesterTree/EventHashBranch.h"
#include "MuonSpacePoint/SpacePointPerLayerSorter.h"
#include "xAODMuonPrepData/UtilFunctions.h"
#include "xAODMuonPrepData/MdtDriftCircle.h"
#include "MuonTrackEvent/TrackingHelpers.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "MuonPatternEvent/SegmentFitterEventData.h"
#include <fstream>
#include <TString.h>
#include <AthenaKernel/RNGWrapper.h>
#include "CLHEP/Random/RandFlat.h"

namespace MuonR4{
    StatusCode BucketDumperAlg::initialize() {
        ATH_CHECK(m_spacePointKeys.initialize());
        ATH_CHECK(m_inSegmentKeys.initialize());
        if (m_isMC) {
            for (const auto& key : m_inSegmentKeys) {
                m_truthDecorKeys.emplace_back(key, "truthParticleLink");
            }
        }
        ATH_CHECK(m_truthDecorKeys.initialize());
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_geoCtxKey.initialize());
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
        SG::ReadHandleKey<xAOD::MuonSegmentContainer> emptyKey{};
        ATH_CHECK(emptyKey.initialize(SG::AllowEmpty));
        for (unsigned  keyNum = 0 ; keyNum < m_spacePointKeys.size(); ++keyNum) {
            ATH_CHECK(dumpContainer(ctx, m_spacePointKeys[keyNum], 
                                    keyNum < m_inSegmentKeys.size() ? m_inSegmentKeys[keyNum] : emptyKey ));
        }
        return StatusCode::SUCCESS;
    }
    StatusCode BucketDumperAlg::dumpContainer(const EventContext& ctx,
                                              const SG::ReadHandleKey<SpacePointContainer>& spacePointKey,
                                              const SG::ReadHandleKey<xAOD::MuonSegmentContainer>& segmentKey) {

        std::unordered_map <const SpacePointBucket*, 
                            std::vector<const xAOD::MuonSegment*>> segmentMap;  // MuonR4Segment 
        
        const xAOD::MuonSegmentContainer* readSegment{nullptr};
        ATH_CHECK(SG::get(readSegment, segmentKey, ctx));
        if (readSegment) {
            for (const xAOD::MuonSegment* segment : *readSegment) {
                segmentMap[detailedSegment(*segment)->parent()->parentBucket()].push_back(segment);
            }
        }

        const ActsGeometryContext* gctx{nullptr};
        ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));

        const SpacePointContainer* spContainer{nullptr};
        ATH_CHECK(SG::get(spContainer, spacePointKey, ctx));

        CLHEP::HepRandomEngine* rndEngine = getRandomEngine(ctx);

        const SpacePointPerLayerSorter layerSorter{m_idHelperSvc.get()};

        for(const SpacePointBucket* bucket : *spContainer) {

            if (!m_isMC && segmentMap[bucket].empty() && m_fracToKeep < 1. &&
                CLHEP::RandFlat::shoot(rndEngine,0.,1.) > m_fracToKeep) {
                ATH_MSG_VERBOSE("Skipping bucket without segment");
                continue;
            }            

            m_bucket_min      = bucket->coveredMin();
            m_bucket_max      = bucket->coveredMax();
            m_bucket_truthHit = std::ranges::any_of(*bucket,[this](const SpacePointBucket::value_type & sp){
                return m_visionTool->isLabeled(*sp);
            });
            const Amg::Vector3D bucketPos = bucket->msSector()->localToGlobalTrans(*gctx) * 
                                            (0.5*(bucket->coveredMin() + bucket->coveredMax()) * Amg::Vector3D::UnitY());

            m_bucket_posX = bucketPos.x();
            m_bucket_posY = bucketPos.y();
            m_bucket_posZ = bucketPos.z();
            m_bucket_segments   = segmentMap[bucket].size();
            m_bucket_sector     = bucket->msSector()->sector();
            m_bucket_chamberIdx = static_cast<uint8_t>(bucket->msSector()->chamberIndex());

            std::unordered_map<const SpacePoint*, std::vector<int16_t>> spacePointToSegment;
            
            auto match_itr = segmentMap.find(bucket);

            if (match_itr != segmentMap.end()) {
                for (const xAOD::MuonSegment* segment : match_itr->second) {
                    for (const auto& meas : detailedSegment(*segment)->measurements()) {
                        if (meas->fitState() == CalibratedSpacePoint::State::Valid) {
                            spacePointToSegment[meas->spacePoint()].push_back(segment->index());
                        }
                    }
                    using namespace SegmentFit;
                    const auto pars = localSegmentPars(*segment);
                    m_segmentLocX += pars[toInt(ParamDefs::x0)];
                    m_segmentLocY += pars[toInt(ParamDefs::y0)];
                    m_segmentLocTheta += pars[toInt(ParamDefs::theta)];
                    m_segmentLocPhi += pars[toInt(ParamDefs::phi)];

                    /** For the moment this only works on MC */
                    unsigned truthLink = -1;
                    if (const xAOD::TruthParticle* truthPart = getTruthMatchedParticle(*segment)) {
                         truthLink = truthPart->index();
                    }
                    m_segmentTruthIdx+=truthLink;

                    m_segmentPos.push_back(segment->position());
                    m_segmentDir.push_back(segment->direction());
                    m_segment_chiSquared.push_back(segment->chiSquared());
                    m_segment_numberDoF.push_back(segment->numberDoF());
                }
            }

            std::unordered_map<Identifier, unsigned> layNumbers{};

            for(const SpacePointBucket::value_type& sp : *bucket) {
                const unsigned layer{layNumbers.insert(
                                        std::make_pair(layerSorter.detectorLayerId(sp->identify()), 
                                                       layNumbers.size())).first->second};
    
                const Identifier& id = sp->identify();

                if (sp->type() == xAOD::UncalibMeasType::MdtDriftCircleType) {
                    const auto* dc = static_cast<const xAOD::MdtDriftCircle*>(sp->primaryMeasurement());
                    if (dc->status() != Muon::MdtDriftCircleStatus::MdtStatusDriftTime){
                        continue;
                    }
                    m_spoint_isMdt.push_back(true);
                    m_spoint_isStrip.push_back(false);
                    m_spoint_adc.push_back(dc->adc());
                    m_spoint_tdc.push_back(dc->tdc());
                } else {
                    // check the technology to fill channel, adc and tdc... pushing back 0 for now
                    m_spoint_adc.push_back(0); 
                    m_spoint_tdc.push_back(0);
                    m_spoint_isMdt.push_back(false);
                    m_spoint_isStrip.push_back(true);
                }

                m_spoint_id.push_back(id);


                const std::vector<int16_t>& segIdxs = spacePointToSegment[sp.get()];

                m_spoint_mat[m_spoint_mat.size()] = segIdxs;
                m_spoint_nSegments.push_back(segIdxs.size());
                
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
                if (m_visionTool.isEnabled()) {
                    m_spoint_trueLabel.push_back(m_visionTool->isLabeled(*sp));
                }

                Amg::Vector3D globalPos = sp->msSector()->localToGlobalTrans(*gctx) * sp->positionInChamber();
                m_spoint_globalPosition.push_back( globalPos );
            }

            m_bucket_layers = layNumbers.size();

            if (!m_tree.fill(ctx)) {
                return StatusCode::FAILURE; 
            }
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
