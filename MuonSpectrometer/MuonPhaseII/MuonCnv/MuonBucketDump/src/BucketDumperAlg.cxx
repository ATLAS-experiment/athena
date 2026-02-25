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
#include "MuonInferenceInterfaces/GraphData.h"
#include <fstream>
#include <unordered_map>
#include <AthenaKernel/RNGWrapper.h>
#include "CLHEP/Random/RandFlat.h"

namespace {
    struct LocalSegSorter{
        bool operator()(const xAOD::MuonSegment* a, const xAOD::MuonSegment* b) const {
             if(a == b) {
                return false;
             }
             if (a->chamberIndex() != b->chamberIndex()) {
                return a->chamberIndex() < b->chamberIndex();
             }
             if (a->sector() != b->sector()) {
                return a->sector() < b->sector();
             }
             if (a->etaIndex() != b->etaIndex()) {
                return a->etaIndex() < b->etaIndex();
             }
            using namespace MuonR4::SegmentFit;
            auto locParsA = localSegmentPars(*a);
            auto locParsB = localSegmentPars(*b);
            return  locParsA < locParsB;
        }
    };
}

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
        ATH_CHECK(m_geoCtxKey.initialize());
        m_tree.addBranch(std::make_shared<MuonVal::EventHashBranch>(m_tree.tree()));
        ATH_CHECK(m_visionTool.retrieve(EnableTool{!m_visionTool.empty()}));        
        if (m_visionTool.empty()) {
            m_tree.disableBranch(m_spoint_trueLabel.name());
        }
        
        // Setup ML bucket score branches
        if (m_doMLBucketScore) {
            ATH_CHECK(m_inferenceTool.retrieve());
            ATH_MSG_INFO("ML bucket scoring enabled with tool: " << m_inferenceTool.name());
        }
        
        // Disable ML score branches if not enabled
        if (!m_doMLBucketScore) {
            m_tree.disableBranch(m_bucket_ml_score_class0.name());
            m_tree.disableBranch(m_bucket_ml_score_class1.name());
            m_tree.disableBranch(m_bucket_ml_score_class2.name());
        }
        
        ATH_CHECK(m_tree.init(this));
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
                                    keyNum < m_inSegmentKeys.size() ?  m_inSegmentKeys[keyNum] : emptyKey));
        }
        return StatusCode::SUCCESS;
    }
    StatusCode BucketDumperAlg::dumpContainer(const EventContext& ctx,
                                              const SG::ReadHandleKey<SpacePointContainer>& spacePointKey,
                                              const SG::ReadHandleKey<xAOD::MuonSegmentContainer>& segmentKey) {
        
        // Define the segment mapping type
        using SegmentsPerBucket_t = std::unordered_map<const SpacePointBucket*,
                                                       std::set<const xAOD::MuonSegment*, LocalSegSorter>>;
        SegmentsPerBucket_t segmentMap{};
        
        const xAOD::MuonSegmentContainer* readSegment{nullptr};
        ATH_CHECK(SG::get(readSegment, segmentKey, ctx));
        if (readSegment) {
            for (const xAOD::MuonSegment* segment : *readSegment) {
                segmentMap[detailedSegment(*segment)->parent()->parentBucket()].insert(segment);
            }
        }

        const ActsTrk::GeometryContext* gctx{nullptr};
        ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));

        const SpacePointContainer* spContainer{nullptr};
        ATH_CHECK(SG::get(spContainer, spacePointKey, ctx));

        // Compute ML bucket scores if enabled - only for MuonSpacePoints container
        // The tool is configured to read "MuonSpacePoints" from StoreGate, so we only run it for that container
        std::unordered_map<const SpacePointBucket*, std::vector<float>> bucketScores;
        const std::string& containerName = spacePointKey.key();
        if (m_doMLBucketScore && containerName == "MuonSpacePoints") {
            ATH_MSG_DEBUG("Computing ML bucket scores for container: " << containerName);
            ATH_CHECK(computeAllBucketScores(ctx, spContainer, bucketScores));
            ATH_MSG_DEBUG("ML scoring completed, got scores for " << bucketScores.size() << " buckets");
        } else if (m_doMLBucketScore && containerName != "MuonSpacePoints") {
            ATH_MSG_DEBUG("Skipping ML scoring for container: " << containerName  
                         << " (only MuonSpacePoints is supported)");
        }

        CLHEP::HepRandomEngine* rndEngine = getRandomEngine(ctx);

        const SpacePointPerLayerSorter layerSorter{};

        for(const SpacePointBucket* bucket : *spContainer) {
            /// Filter random noise
            if (!m_isMC && segmentMap[bucket].empty() && m_fracToKeep < 1. &&
                CLHEP::RandFlat::shoot(rndEngine,0.,1.) > m_fracToKeep) {
                ATH_MSG_VERBOSE("Skipping bucket without segment");
                continue;
            }
            
            if (m_doMLBucketFilter && !bucketScores.empty()) {
                auto scoreIt = bucketScores.find(bucket);
                if (scoreIt != bucketScores.end() && scoreIt->second.size() >= 3) {
                    const auto& logits = scoreIt->second;
                    int predictedClass = 0;
                    float maxLogit = logits[0];
                    if (logits[1] > maxLogit) { maxLogit = logits[1]; predictedClass = 1; }
                    if (logits[2] > maxLogit) { maxLogit = logits[2]; predictedClass = 2; }
                    if (predictedClass == 0) continue;
                }
            }
            
            /// Bucket identifier
            m_bucket_sector     = bucket->msSector()->sector();
            m_bucket_chamberIdx = static_cast<uint8_t>(bucket->msSector()->chamberIndex());
            m_bucket_side = bucket->msSector()->side();
            //// Bucket dimension
            m_bucket_min      = bucket->coveredMin();
            m_bucket_max      = bucket->coveredMax();

            /// Global bucket position
            const Amg::Vector3D bucketPos = bucket->msSector()->localToGlobalTransform(*gctx) * 
                                            (0.5*(bucket->coveredMin() + bucket->coveredMax()) * Amg::Vector3D::UnitY());
            m_bucket_posX = bucketPos.x();
            m_bucket_posY = bucketPos.y();
            m_bucket_posZ = bucketPos.z();

            /// Flag whether the bucket contains at least one good hit
            m_bucket_truthHit = std::ranges::any_of(*bucket,[this](const SpacePointBucket::value_type & sp){
                return m_visionTool->isLabeled(*sp);
            });

            /// Number of reconstructed segments in the bucket
            m_bucket_segments   = segmentMap[bucket].size();



            std::unordered_map<const SpacePoint*, std::vector<int16_t>> spacePointToSegment{};
            std::set<const xAOD::MuonSegment*, LocalSegSorter> truthSegments{};
            /// Associate the Space points to the reconstructed segments
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
                    m_segmentLocX += pars[Acts::toUnderlying(ParamDefs::x0)];
                    m_segmentLocY += pars[Acts::toUnderlying(ParamDefs::y0)];
                    m_segmentLocTheta += pars[Acts::toUnderlying(ParamDefs::theta)];
                    m_segmentLocPhi += pars[Acts::toUnderlying(ParamDefs::phi)];

                    /** For the moment this only works on MC */
                    unsigned truthLink = -1;
                    if (const xAOD::TruthParticle* truthPart = getTruthMatchedParticle(*segment)) {
                         truthLink = truthPart->index();
                    }
                    /** Truth segment parameters */
                    if (const xAOD::MuonSegment* truthSeg = getMatchedTruthSegment(*segment)) {
                        truthSegments.insert(truthSeg);
                    }
                    m_segmentTruthIdx+=truthLink;
                    m_segmentPos.push_back(segment->position());
                    m_segmentDir.push_back(segment->direction());
                    m_segment_chiSquared.push_back(segment->chiSquared());
                    m_segment_numberDoF.push_back(segment->numberDoF());
                }
            }

            std::vector<unsigned int> layNumbers{};
            std::unordered_map<const SpacePoint*, std::vector<const xAOD::MuonSegment*>> spToTrueSeg{};
            if (m_isMC) {
                using SegLinkVec_t = std::vector<ElementLink<xAOD::MuonSegmentContainer>>;
                static const SG::ConstAccessor<SegLinkVec_t> segAcc{"truthSegmentLinks"};
                for (const auto& sp : *bucket){
                    for (const auto& link : segAcc(*sp->primaryMeasurement())) {
                        spToTrueSeg[sp.get()].push_back(*link);
                        truthSegments.insert(*link);
                    }
                }
            }
            
            for(const SpacePointBucket::value_type& sp : *bucket) {
                /// Calculate the layer of the space point
                const unsigned int layNum = layerSorter.sectorLayerNum(*sp);
                if (std::find(layNumbers.begin(), layNumbers.end(), layNum) == layNumbers.end()) {
                    layNumbers.push_back(layNum);
                }
                const unsigned layer = layNumbers.size()-1;
    
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
                auto& trueSegLinks = m_spoint_trueSeg[m_spoint_trueSeg.size()];
                for (const xAOD::MuonSegment* matchedSeg : spToTrueSeg[sp.get()]) {
                    trueSegLinks.push_back(std::distance(truthSegments.begin(), truthSegments.find(matchedSeg)));
                }
                m_spoint_nSegments.push_back(segIdxs.size());
                
                m_bucket_spacePoints = bucket->size();
                m_spoint_localPosition.push_back(sp->localPosition());
                using CovIdx = SpacePoint::CovIdx;
                m_spoint_covX.push_back(sp->covariance()[Acts::toUnderlying(CovIdx::phiCov)]);
                m_spoint_covY.push_back(sp->covariance()[Acts::toUnderlying(CovIdx::etaCov)]);
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

                Amg::Vector3D globalPos = sp->msSector()->localToGlobalTransform(*gctx) * sp->localPosition();
                m_spoint_globalPosition.push_back( globalPos );
            }

            for (const xAOD::MuonSegment* truthSeg: truthSegments) {
                using namespace SegmentFit;
                const auto truthPars = localSegmentPars(*truthSeg);
                m_truthSegLocX     += truthPars[Acts::toUnderlying(ParamDefs::x0)];
                m_truthSegLocY     += truthPars[Acts::toUnderlying(ParamDefs::y0)];
                m_truthSegLocTheta += truthPars[Acts::toUnderlying(ParamDefs::theta)];
                m_truthSegLocPhi   += truthPars[Acts::toUnderlying(ParamDefs::phi)];
            }

            m_bucket_layers = layNumbers.size();

            // Add ML bucket filter scores if available
            // Only add scores if we actually computed them for this container
            if (m_doMLBucketScore && !bucketScores.empty()) {
                auto scoreIt = bucketScores.find(bucket);
                if (scoreIt != bucketScores.end() && scoreIt->second.size() >= 3) {
                    m_bucket_ml_score_class0.push_back(scoreIt->second[0]);
                    m_bucket_ml_score_class1.push_back(scoreIt->second[1]);
                    m_bucket_ml_score_class2.push_back(scoreIt->second[2]);
                } else {
                    // Empty buckets are skipped during inference, so missing scores is expected
                    // Only warn if a non-empty bucket is missing scores (indicates a real problem)
                    if (bucket && !bucket->empty()) {
                        ATH_MSG_WARNING("Non-empty bucket from scored container not found in ML scores map");
                    }
                    m_bucket_ml_score_class0.push_back(0.f);
                    m_bucket_ml_score_class1.push_back(0.f);
                    m_bucket_ml_score_class2.push_back(0.f);
                }
            } else if (m_doMLBucketScore) {
                // No scores computed for this container (e.g., NswSpacePoints) - fill with zeros
                m_bucket_ml_score_class0.push_back(0.f);
                m_bucket_ml_score_class1.push_back(0.f);
                m_bucket_ml_score_class2.push_back(0.f);
            }

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

    StatusCode BucketDumperAlg::computeAllBucketScores(const EventContext& ctx,
                                                       const SpacePointContainer* spContainer,
                                                       std::unordered_map<const SpacePointBucket*, std::vector<float>>& bucketScoreMap) const {
        bucketScoreMap.clear();
        
        ATH_MSG_DEBUG("computeAllBucketScores: doMLBucketScore=" << m_doMLBucketScore 
                      << ", tool empty=" << m_inferenceTool.empty() 
                      << ", spContainer=" << (spContainer ? "valid" : "null"));
        
        if (!m_doMLBucketScore || m_inferenceTool.empty() || !spContainer) {
            return StatusCode::SUCCESS;
        }
        
        // Count non-empty buckets
        size_t nonEmptyBuckets = 0;
        for (const MuonR4::SpacePointBucket* bucket : *spContainer) {
            if (bucket && !bucket->empty()) {
                ++nonEmptyBuckets;
            }
        }
        
        // If no non-empty buckets, skip inference
        if (nonEmptyBuckets == 0) {
            ATH_MSG_DEBUG("Container has no non-empty buckets, skipping ML scoring");
            return StatusCode::SUCCESS;
        }

        // Run the inference tool
        ATH_MSG_DEBUG("Calling runGraphInference on tool: " << m_inferenceTool.name());
        MuonML::GraphRawData graphData{};
        ATH_CHECK(m_inferenceTool->runGraphInference(ctx, graphData));
        ATH_MSG_DEBUG("runGraphInference completed successfully");

        // Extract logits from the inference output
        if (!graphData.graph || graphData.graph->dataTensor.size() <= 2) {
            ATH_MSG_ERROR("Missing output logits tensor at index 2");
            return StatusCode::FAILURE;
        }
        
        const Ort::Value& outTensor = graphData.graph->dataTensor[2];
        const auto& info = outTensor.GetTensorTypeAndShapeInfo();
        std::vector<int64_t> outShape = info.GetShape();
        
        if (outShape.size() != 2 || outShape[1] != 3) {
            ATH_MSG_ERROR("Unexpected ONNX output tensor shape");
            return StatusCode::FAILURE;
        }
        
        const float* logitsPtr = outTensor.GetTensorData<float>();
        if (!logitsPtr) {
            ATH_MSG_ERROR("Failed to get logits data pointer");
            return StatusCode::FAILURE;
        }
        
        // Map logits to buckets - only considering non-empty buckets
        size_t totalNumPredictions = static_cast<size_t>(outShape[0]);
        size_t predIdx = 0;
        
        for (const MuonR4::SpacePointBucket* bucket : *spContainer) {
            if (!bucket || bucket->empty()) {
                continue;  // Skip empty buckets
            }
            
            if (predIdx >= totalNumPredictions) {
                ATH_MSG_ERROR("More non-empty buckets than predictions from model");
                return StatusCode::FAILURE;
            }
            
            std::vector<float> scores(3);
            scores[0] = logitsPtr[3 * predIdx + 0];
            scores[1] = logitsPtr[3 * predIdx + 1];
            scores[2] = logitsPtr[3 * predIdx + 2];
            
            bucketScoreMap[bucket] = scores;
            ++predIdx;
        }
        
        if (predIdx != totalNumPredictions) {
            ATH_MSG_ERROR("Number of non-empty buckets (" << predIdx << ") does not match predictions (" 
                          << totalNumPredictions << ")");
            return StatusCode::FAILURE;
        }
        
        ATH_MSG_INFO("Successfully computed ML bucket scores for " << bucketScoreMap.size() << " buckets");
        return StatusCode::SUCCESS;
    }

}




