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

        using SegmentsPerBucket_t = std::unordered_map <const SpacePointBucket*, 
                                                       std::set<const xAOD::MuonSegment*, LocalSegSorter>>;

        SegmentsPerBucket_t segmentMap{};
        
        const xAOD::MuonSegmentContainer* readSegment{nullptr};
        ATH_CHECK(SG::get(readSegment, segmentKey, ctx));
        if (readSegment) {
            for (const xAOD::MuonSegment* segment : *readSegment) {
                segmentMap[detailedSegment(*segment)->parent()->parentBucket()].insert(segment);
            }
        }

        const ActsGeometryContext* gctx{nullptr};
        ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));

        const SpacePointContainer* spContainer{nullptr};
        ATH_CHECK(SG::get(spContainer, spacePointKey, ctx));

        CLHEP::HepRandomEngine* rndEngine = getRandomEngine(ctx);

        const SpacePointPerLayerSorter layerSorter{};

        for(const SpacePointBucket* bucket : *spContainer) {
            /// Filter random noise
            if (!m_isMC && segmentMap[bucket].empty() && m_fracToKeep < 1. &&
                CLHEP::RandFlat::shoot(rndEngine,0.,1.) > m_fracToKeep) {
                ATH_MSG_VERBOSE("Skipping bucket without segment");
                continue;
            }
            /// Bucket identifier
            m_bucket_sector     = bucket->msSector()->sector();
            m_bucket_chamberIdx = static_cast<uint8_t>(bucket->msSector()->chamberIndex());
            m_bucket_side = bucket->msSector()->side();
            //// Bucket dimension
            m_bucket_min      = bucket->coveredMin();
            m_bucket_max      = bucket->coveredMax();

            /// Global bucket position
            const Amg::Vector3D bucketPos = bucket->msSector()->localToGlobalTrans(*gctx) * 
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

                Amg::Vector3D globalPos = sp->msSector()->localToGlobalTrans(*gctx) * sp->localPosition();
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
