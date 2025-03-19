/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonHoughTransformTester.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "MuonTesterTree/EventInfoBranch.h"
#include "MuonReadoutGeometryR4/SpectrometerSector.h"
#include "MuonPatternHelpers/SegmentFitHelperFunctions.h"
#include "MuonPatternEvent/MuonHoughDefs.h"
#include "xAODMuonPrepData/UtilFunctions.h"
#include "xAODMuonPrepData/sTgcMeasurement.h"
#include "MuonPatternHelpers/HoughHelperFunctions.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "Acts/Utilities/Enumerate.hpp"
#include "GaudiKernel/PhysicalConstants.h"

 #include "AthContainers/ConstDataVector.h"


namespace {
    constexpr double c_inv = 1. /Gaudi::Units::c_light;
}


namespace MuonValR4 {
    using namespace MuonR4;
    using namespace MuonVal;
    using ObjectMatching = MuonHoughTransformTester::ObjectMatching;
    using simHitSet = std::unordered_set<const xAOD::MuonSimHit*>;
    unsigned int countMatched(const simHitSet& truthHits,
                              const simHitSet& recoHits) {
        unsigned int matched{0};
        for (const xAOD::MuonSimHit* reco : recoHits) {
            matched += truthHits.count(reco);
        }
        return matched;
    }
    unsigned int countMatched(const xAOD::MuonSegment* truthSeg,
                              const MuonR4::SegmentSeed* seed) {
        return truthSeg ? countMatched(getMatchingSimHits(*truthSeg), getMatchingSimHits(*seed)) : 0;
    }
    unsigned int countMatched(const xAOD::MuonSegment* truthSeg,
                              const MuonR4::Segment* segment) {
        return truthSeg ? countMatched(getMatchingSimHits(*truthSeg), getMatchingSimHits(*segment)) : 0;
    }
    /** @brief Define a spacepoint as precision hit if it's a Mdt or NSW eta hit */
    template <class SpType>
    bool isPrecHit(const SpType& sp) {
        return sp.type() == xAOD::UncalibMeasType::MdtDriftCircleType ||
               sp.type() == xAOD::UncalibMeasType::MMClusterType ||
               (sp.type() == xAOD::UncalibMeasType::sTgcStripType && sp.measuresEta());
    }

    StatusCode MuonHoughTransformTester::initialize() {
        ATH_CHECK(m_geoCtxKey.initialize());
        
        {
            int infoOpts = 0;
            if (m_isMC) infoOpts = EventInfoBranch::isMC;
            m_tree.addBranch(std::make_unique<EventInfoBranch>(m_tree, infoOpts));  
        }
        ATH_CHECK(m_truthSegmentKey.initialize(!m_truthSegmentKey.empty()));
        /// The collection of readHandle keys should be either 1 or 2
        ATH_CHECK(m_inSegmentKeys.initialize());
        ATH_CHECK(m_inHoughSegmentSeedKeys.initialize());
        ATH_CHECK(m_spKey.initialize(m_writeSpacePoints));
        if (m_writeSpacePoints) {
            m_spTester = std::make_unique<SpacePointTesterModule>(m_tree, m_spKey.key(), msgLevel());
            m_tree.addBranch(m_spTester);
        } else {
            m_tree.disableBranch(m_spMatchedToPattern.name());
            m_tree.disableBranch(m_spMatchedToSegment.name());
        }
        ATH_CHECK(m_tree.init(this)); 
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(detStore()->retrieve(m_detMgr));

        ATH_CHECK(m_visionTool.retrieve(EnableTool{!m_visionTool.empty()}));
        ATH_MSG_DEBUG("Succesfully initialised");
        return StatusCode::SUCCESS;
    }


    unsigned int MuonHoughTransformTester::countOnSameSide(const ActsGeometryContext& gctx,
                                                           const xAOD::MuonSegment& truthSeg,
                                                           const MuonR4::Segment& recoSeg) const{
        unsigned int same{0};
        const auto [truPos, truDir] = SegmentFit::makeLine(SegmentFit::localSegmentPars(truthSeg));
        const auto [recoPos, recoDir] = SegmentFit::makeLine(SegmentFit::localSegmentPars(gctx, recoSeg));
        const std::vector<int> truthSigns = SegmentFitHelpers::driftSigns(truPos, truDir, recoSeg.measurements(), msgStream());
        const std::vector<int> recoSigns = SegmentFitHelpers::driftSigns(recoPos, recoDir, recoSeg.measurements(), msgStream());
        for (unsigned int s = 0 ; s < truthSigns.size(); ++s) {
            same += (truthSigns[s] != 0) && truthSigns[s] == recoSigns[s];
        }
        return same;
    }
    std::vector<ObjectMatching> 
            MuonHoughTransformTester::matchWithTruth(const ActsGeometryContext& gctx,
                                                     const xAOD::MuonSegmentContainer* truthSegments,
                                                     const SegmentSeedContainer* seedContainer,
                                                     const SegmentContainer* segmentContainer) const {
        std::vector<ObjectMatching> allAssociations{};
        std::unordered_set<const SegmentSeed*> usedSeeds{};
        std::unordered_set<const Segment*> usedSegs{};

        /// Step 1: Truth matched seeds and segments, and non-reconstructed truth. 
        /// Go from truth to the matched reco objects and fill one common tree entry
        if (m_isMC) {
            // collect the sim hits that contributed to our segments and seeds.
            std::vector<simHitSet> truthHitsVec{}, seedSimHitVec{}, segmentSimHitVec{};
            for (const SegmentSeed* seed: *seedContainer) {
                seedSimHitVec.emplace_back(getMatchingSimHits(*seed));
            }
            for (const Segment* segment: *segmentContainer){
                segmentSimHitVec.emplace_back(getMatchingSimHits(*segment));
            }

            // Now look at the truth segments, collecting their sim hits.
            // Compare these to the sim hits on our reco objects 
            for (const xAOD::MuonSegment* truth: *truthSegments) {
                const simHitSet& truthHits{truthHitsVec.emplace_back(getMatchingSimHits(*truth))};
                ObjectMatching & matchedWithTruth = allAssociations.emplace_back(); 
                matchedWithTruth.truthSegment = truth;
                matchedWithTruth.chamber = m_detMgr->getSectorEnvelope((*truthHits.begin())->identify());

                std::vector<std::pair<const SegmentSeed*, unsigned>> matchedSeeds{};
                
                // Find seeds sharing at least one simHit with our truth segment 
                // can't wait for views::enumerate
                int seedIdx{-1};
                for (const SegmentSeed* seed : *seedContainer) {
                    ++seedIdx;
                    if (seed->msSector() != matchedWithTruth.chamber) {
                        continue;
                    }
                    const simHitSet& seedHits{seedSimHitVec[seedIdx]};
                    unsigned int matchedHits = countMatched(truthHits, seedHits);
                    if (!matchedHits) {
                        continue;
                    }
                    matchedSeeds.emplace_back(std::make_pair(seed, matchedHits));
                }
                // Find segments sharing at least one simHit with our truth segment 
                std::vector<std::pair<const Segment*, unsigned>> matchedSegs{};
                int segmentIdx{-1};
                for (const Segment* segment  : *segmentContainer) {
                    ++segmentIdx; 
                    if (segment->msSector() != matchedWithTruth.chamber) {
                        continue;
                    }
                    const simHitSet& segmentHits{segmentSimHitVec[segmentIdx]};
                    unsigned int matchedHits = countMatched(truthHits, segmentHits);
                    if (!matchedHits) {
                        continue;
                    }
                    matchedSegs.emplace_back(std::make_pair(segment,matchedHits));
                }

                // sort by quality of match 

                // for segments (by hit count and same-side hits) 
                std::ranges::sort(matchedSegs,
                        [this, &truth, &gctx](const std::pair<const Segment*, unsigned>& segA, 
                                              const std::pair<const Segment*, unsigned>& segB){
                    if (segA.second != segB.second) return segA.second > segB.second;
                    return countOnSameSide(gctx, *truth, *segA.first) > countOnSameSide(gctx, *truth, *segB.first);
                });
                // and for seeds (by raw hit count)
                std::ranges::sort(matchedSeeds, [](const std::pair<const SegmentSeed*, unsigned>& seedA, 
                                                const std::pair<const SegmentSeed*, unsigned>& seedB) {
                    return seedA.second > seedB.second;
                });


                // now we can populate our association object 

                // first, we handle the segments and any seeds connected with them
                for (const auto& [matched, nMatchedHits] : matchedSegs) {
                    // add segment to the list of all segments
                    matchedWithTruth.matchedSegments.push_back(matched);
                    // and update our book-keeping to record that this segment and its seed have already been written 
                    usedSeeds.insert(matched->parent());
                    usedSegs.insert(matched);
                }
        
                // now, we add the seeds
                for (const auto& [seed , nHits] : matchedSeeds) {
                    // add seed to the list of all seeds
                    ATH_MSG_VERBOSE("Seed with "<<nHits);
                    matchedWithTruth.matchedSeeds.push_back(seed);
                    matchedWithTruth.matchedSeedFoundSegment.push_back(usedSeeds.count(seed)); 
                    usedSeeds.insert(seed); 
                }
            } // end of loop over truth segments
        }

        /// Now we have processed all truth segments, as well as all reco objects that share sim hits with them. 
        /// We still need to collect seeds and segments that are not matched to any truth. 
        /// This happens in all data events, or through fake hits / segments. 

        // start with segments, and also collect "their" seeds in a common entry
        for (const Segment* seg: *segmentContainer) {
            // skip segments that were previously seen and written in the truth loop
            if (usedSegs.count(seg)) {
                continue;
            }
            ObjectMatching & match = allAssociations.emplace_back();
            match.chamber = seg->msSector();
            match.matchedSegments = {seg}; 
            match.matchedSeeds = {seg->parent()};
            // this seed has been written as well - do not write it in the following loop 
            usedSeeds.insert(seg->parent());
        }
        for (const SegmentSeed* seed: *seedContainer) {
            // skip seeds that are on segments or seen in the truth loop 
            if (usedSeeds.count(seed)) {
                continue;
            }
            ObjectMatching & match = allAssociations.emplace_back(); 
            match.chamber = seed->msSector();
            match.matchedSeeds = {seed}; 
        }
        return allAssociations;
    }

   template <class ContainerType>
        StatusCode MuonHoughTransformTester::retrieveContainer(const EventContext& ctx, 
                                                               const SG::ReadHandleKey<ContainerType>& key,
                                                               const ContainerType*& contToPush) const {
            contToPush = nullptr;
            if (key.empty()) {
                ATH_MSG_VERBOSE("No key has been parsed for object "<< typeid(ContainerType).name());
                return StatusCode::SUCCESS;
            }
            SG::ReadHandle readHandle{key, ctx};
            if (!readHandle.isPresent()) {
                ATH_MSG_FATAL("Failed to load "<<key.fullKey());
                return StatusCode::FAILURE;
            }
            contToPush = readHandle.cptr();
            return StatusCode::SUCCESS;
        }

    StatusCode MuonHoughTransformTester::finalize() {
        ATH_CHECK(m_tree.write());
        return StatusCode::SUCCESS;
    }
    StatusCode MuonHoughTransformTester::execute()  {
        
        const EventContext & ctx = Gaudi::Hive::currentContext();
        const ActsGeometryContext* gctxPtr{nullptr};
        ATH_CHECK(retrieveContainer(ctx, m_geoCtxKey, gctxPtr));
        const ActsGeometryContext& gctx{*gctxPtr};


        ConstDataVector<MuonR4::SegmentSeedContainer> segmentSeeds{SG::VIEW_ELEMENTS};
        ConstDataVector<MuonR4::SegmentContainer> segments{SG::VIEW_ELEMENTS};

        for (const SG::ReadHandleKey<SegmentSeedContainer>& key : m_inHoughSegmentSeedKeys) {
            const SegmentSeedContainer* readSegmentSeeds{nullptr};
            ATH_CHECK(retrieveContainer(ctx,key, readSegmentSeeds));
            segmentSeeds.insert(segmentSeeds.end(),readSegmentSeeds->begin(), readSegmentSeeds->end());
        }
        for (const SG::ReadHandleKey<SegmentContainer>& key : m_inSegmentKeys) {
            const SegmentContainer* readSegments{nullptr};
            ATH_CHECK(retrieveContainer(ctx,key, readSegments));
            segments.insert(segments.end(),readSegments->begin(), readSegments->end());
        }
        const xAOD::MuonSegmentContainer* readTruthSegments{nullptr};
        ATH_CHECK(retrieveContainer(ctx, m_truthSegmentKey, readTruthSegments));
            
        ATH_MSG_DEBUG("Succesfully retrieved input collections. Seeds: "<<segmentSeeds.size()
                    <<", segments: "<<segments.size() <<", truth segments: "<<(readTruthSegments? readTruthSegments->size() : -1)<<".");
        std::vector<ObjectMatching> objects = matchWithTruth(gctx, readTruthSegments, segmentSeeds.asDataVector(), 
                                                             segments.asDataVector());
        for (const ObjectMatching& obj : objects) {
            fillChamberInfo(obj.chamber);
            fillTruthInfo(gctx, obj.truthSegment);
            fillSeedInfo(obj);
            fillSegmentInfo(gctx, obj);
            ATH_CHECK(m_tree.fill(ctx));
        }
        return StatusCode::SUCCESS;
    }
    void MuonHoughTransformTester::fillChamberInfo(const MuonGMR4::SpectrometerSector* msSector){
        m_out_chamberIndex = msSector->chamberIndex();
        m_out_stationSide = msSector->side();
        m_out_stationPhi = msSector->stationPhi();
    }                
    void MuonHoughTransformTester:: fillTruthInfo(const ActsGeometryContext& gctx,
                                                  const xAOD::MuonSegment* segment) {
        if (!segment) return; 
        m_out_hasTruth = true; 

        const Amg::Vector3D segDir{segment->direction()};
        static const SG::Accessor<float> acc_pt{"pt"};
        static const SG::Accessor<float> acc_charge{"charge"};
        // eta is interpreted as the eta-location 
        m_out_gen_Eta = segDir.eta();
        m_out_gen_Phi = segDir.phi();
        m_out_gen_Pt  = acc_pt(*segment);
        m_out_gen_Q = acc_charge(*segment);

        const auto [chamberPos, chamberDir] = SegmentFit::makeLine(SegmentFit::localSegmentPars(*segment));
        m_out_gen_nHits = segment->nPrecisionHits()+segment->nPhiLayers() + segment->nTrigEtaLayers(); 
       
        m_out_gen_nMDTHits = (segment->technology() == Muon::MuonStationIndex::MDT ? segment->nPrecisionHits() : 0); 
        m_out_gen_nNswHits = (segment->technology() != Muon::MuonStationIndex::MDT ? segment->nPrecisionHits() : 0); 
        m_out_gen_nTGCHits = (segment->chamberIndex() > Muon::MuonStationIndex::ChIndex::BEE ? segment->nPhiLayers() + segment->nTrigEtaLayers() : 0);
        m_out_gen_nRPCHits = (segment->chamberIndex() <= Muon::MuonStationIndex::ChIndex::BEE ? segment->nPhiLayers() + segment->nTrigEtaLayers() : 0);

        m_out_gen_tantheta = houghTanTheta(chamberDir); 
        m_out_gen_tanphi   = houghTanPhi(chamberDir);
        m_out_gen_y0 = chamberPos.y(); 
        m_out_gen_x0 = chamberPos.x(); 
        m_out_gen_time = segment->t0();

        double minYhit = std::numeric_limits<double>::max();
        double maxYhit = -1 * std::numeric_limits<double>::max();
        for (const xAOD::MuonSimHit* hit : getMatchingSimHits(*segment)){
            const Identifier hitId = hit->identify();
            const MuonGMR4::MuonReadoutElement* RE = m_detMgr->getReadoutElement(hitId); 
            const IdentifierHash hash{m_idHelperSvc->isMdt(hitId) ? RE->measurementHash(hitId)
                                                                  : RE->layerHash(hitId) };
            const Amg::Transform3D localToChamber = RE->msSector()->globalToLocalTrans(gctx) * RE->localToGlobalTrans(gctx, hash);
            const Amg::Vector3D chamberPos = localToChamber * xAOD::toEigen(hit->localPosition()); 
            minYhit = std::min(chamberPos.y(), minYhit); 
            maxYhit = std::max(chamberPos.y(), maxYhit); 
        }
        m_out_gen_minYhit = minYhit;
        m_out_gen_maxYhit = maxYhit;

        ATH_MSG_DEBUG("A true max on chamber index "<<m_out_chamberIndex.getVariable()<<" side "<<m_out_stationSide.getVariable()<<" phi "<<m_out_stationPhi.getVariable()<<" with "
                    <<m_out_gen_nMDTHits.getVariable()<<" MDT and "<<m_out_gen_nRPCHits.getVariable()+m_out_gen_nTGCHits.getVariable()<< " trigger hits is at "
                    <<m_out_gen_tantheta.getVariable()<<" and "<<m_out_gen_y0.getVariable()); 
    }
    void MuonHoughTransformTester::fillBucketInfo(const SpacePointBucket& bucket) {
        m_out_bucketEnd = bucket.coveredMax();
        m_out_bucketStart = bucket.coveredMin();
        m_out_nSpacePoints = bucket.size();
        m_out_nPrecSpacePoints = std::ranges::count_if(bucket, [](const SpacePointBucket::value_type& sp){
            return isPrecHit(*sp);
        });
        m_out_nPhiSpacePoints = std::ranges::count_if(bucket, [](const SpacePointBucket::value_type& sp){
            return sp->measuresPhi();
        });
        if (!m_visionTool.isEnabled()){
            return;
        }
        m_out_nTrueSpacePoints = std::ranges::count_if(bucket,[this](const SpacePointBucket::value_type& sp){
            return m_visionTool->isLabeled(*sp);
        });
        m_out_nTruePrecSpacePoints = std::ranges::count_if(bucket,[this](const SpacePointBucket::value_type& sp){
            return isPrecHit(*sp) && m_visionTool->isLabeled(*sp);
        });
        m_out_nTruePhiSpacePoints = std::ranges::count_if(bucket,[this](const SpacePointBucket::value_type& sp){
            return sp->measuresPhi() && m_visionTool->isLabeled(*sp);
        });
    }
    void MuonHoughTransformTester::fillSeedInfo(const ObjectMatching& obj) {

        m_out_seed_n = obj.matchedSeeds.size();
        for (const auto& [iseed, seed] : Acts::enumerate(obj.matchedSeeds)){
            if (iseed ==0) {
                fillBucketInfo(*seed->parentBucket());
            }
            double minYhit = m_out_bucketEnd.getVariable();
            double maxYhit = m_out_bucketStart.getVariable();
            for (const SpacePoint* hit : seed->getHitsInMax()){
                minYhit = std::min(hit->positionInChamber().y(),minYhit); 
                maxYhit = std::max(hit->positionInChamber().y(),maxYhit); 
            }
            m_out_seed_minYhit.push_back(minYhit);
            m_out_seed_maxYhit.push_back(maxYhit);

            m_out_seed_hasPhiExtension.push_back(seed->hasPhiExtension()); 
            m_out_seed_nMatchedHits.push_back(countMatched(obj.truthSegment, seed));
            m_out_seed_y0.push_back(seed->interceptY());
            m_out_seed_tantheta.push_back(seed->tanTheta());
            if (seed->hasPhiExtension()){
                m_out_seed_x0.push_back(seed->interceptX());
                m_out_seed_tanphi.push_back(seed->tanPhi());
            } else{
                m_out_seed_x0.push_back(-999);
                m_out_seed_tanphi.push_back(-999);
            }
         
            m_out_seed_nHits.push_back(seed->getHitsInMax().size());
            unsigned nMdtSeed{0}, nRpcSeed{0}, nTgcSeed{0}, nMmSeed{0}, nsTgcSeed{0}; 
            unsigned nPrecHits{0}, nEtaHits{0}, nPhiHits{0}, nTrueHits{0}, nTruePrecHits{0}, nTrueEtaHits{0}, nTruePhiHits{0};
            std::vector<unsigned char> matched{};
            for (const HoughHitType & houghSP: seed->getHitsInMax()){                
                if (m_writeSpacePoints){
                    unsigned treeIdx = m_spTester->push_back(*houghSP);
                    if (treeIdx >= matched.size()){
                        matched.resize(treeIdx +1);
                    }
                    matched[treeIdx] = true;
                }
                nPrecHits += isPrecHit(*houghSP);
                nPhiHits  += houghSP->measuresPhi();
                nEtaHits  += houghSP->measuresEta();

                if (m_visionTool.isEnabled()) {
                    nTrueHits += m_visionTool->isLabeled(*houghSP);
                    nTruePrecHits += m_visionTool->isLabeled(*houghSP) && isPrecHit(*houghSP);
                    nTruePhiHits += m_visionTool->isLabeled(*houghSP) && houghSP->measuresPhi();
                    nTrueEtaHits += m_visionTool->isLabeled(*houghSP) && houghSP->measuresEta();
                }
                switch (houghSP->type()) {
                    case xAOD::UncalibMeasType::MdtDriftCircleType: 
                        ++nMdtSeed;
                        break;
                    case xAOD::UncalibMeasType::RpcStripType:
                        nRpcSeed+=houghSP->measuresEta();
                        nRpcSeed+=houghSP->measuresPhi();
                        break;
                    case xAOD::UncalibMeasType::TgcStripType:
                        nTgcSeed+=houghSP->measuresEta();
                        nTgcSeed+=houghSP->measuresPhi();
                        break;
                    case xAOD::UncalibMeasType::sTgcStripType:
                        nsTgcSeed += houghSP->measuresEta();
                        nsTgcSeed += houghSP->measuresPhi();
                        break;
                    case xAOD::UncalibMeasType::MMClusterType:
                        ++nMmSeed;
                        break;
                    default:
                        ATH_MSG_WARNING("Technology "<<houghSP->identify()  <<" not yet implemented");                        
                }                    
            }
            m_out_seed_nMdt.push_back(nMdtSeed);
            m_out_seed_nRpc.push_back(nRpcSeed);
            m_out_seed_nTgc.push_back(nTgcSeed);
            m_out_seed_nsTgc.push_back(nsTgcSeed);
            m_out_seed_nMm.push_back(nMmSeed);

            m_out_seed_nPrecHits.push_back(nPrecHits);
            m_out_seed_nEtaHits.push_back(nEtaHits); 
            m_out_seed_nPhiHits.push_back(nPhiHits);

            m_out_seed_nTrueHits.push_back(nTrueHits);
            m_out_seed_nTruePrecHits.push_back(nTruePrecHits);            
            m_out_seed_nTrueEtaHits.push_back(nTrueEtaHits);
            m_out_seed_nTruePhiHits.push_back(nTruePhiHits);

            m_out_seed_ledToSegment.push_back(obj.matchedSeedFoundSegment.at(iseed));
            if (m_writeSpacePoints) {
                m_spMatchedToPattern[iseed] = std::move(matched);
            } 
        }
    }
    
    void MuonHoughTransformTester::fillSegmentInfo(const ActsGeometryContext& gctx,
                                                   const ObjectMatching& obj){
        using namespace SegmentFit;

        m_out_segment_n = obj.matchedSegments.size(); 
        for (auto & segment : obj.matchedSegments){
            m_out_segment_hasPhi.push_back(std::ranges::find_if(segment->measurements(), [](const auto& meas){  return meas->measuresPhi();}) 
                                !=segment->measurements().end());
            m_out_segment_fitIter.push_back(segment->nFitIterations());
            m_out_segment_truthMatchedHits.push_back(countMatched(obj.truthSegment, segment));
            m_out_segment_chi2.push_back(segment->chi2());
            m_out_segment_nDoF.push_back(segment->nDoF());
            m_out_segment_hasTimeFit.push_back(segment->hasTimeFit());

            m_out_segment_err_x0.push_back(segment->covariance()(toInt(ParamDefs::x0), toInt(ParamDefs::x0)));
            m_out_segment_err_y0.push_back(segment->covariance()(toInt(ParamDefs::y0), toInt(ParamDefs::y0)));
            m_out_segment_err_tantheta.push_back(segment->covariance()(toInt(ParamDefs::theta), toInt(ParamDefs::theta)));
            m_out_segment_err_tanphi.push_back(segment->covariance()(toInt(ParamDefs::phi), toInt(ParamDefs::phi)));
            m_out_segment_err_time.push_back(segment->covariance()(toInt(ParamDefs::time), toInt(ParamDefs::time)));
            const auto [locPos, locDir] = makeLine(localSegmentPars(gctx, *segment));
            m_out_segment_tanphi.push_back(houghTanPhi(locDir));
            m_out_segment_tantheta.push_back(houghTanTheta(locDir));
            m_out_segment_y0.push_back(locPos.y());
            m_out_segment_x0.push_back(locPos.x());
            m_out_segment_time.push_back(segment->segementT0() + segment->position().mag() * c_inv);

            unsigned nMdtHits{0}, nRpcEtaHits{0}, nRpcPhiHits{0}, nTgcEtaHits{0}, nTgcPhiHits{0},
                     nMmEtaHits{0}, nMmStereoHits{0}, nStgcStripHits{0},nStgcWireHits{0}, nStgcPadHits{0};
            unsigned nTrueHits{0}, nTruePrecHits{0}, nTrueEtaHits{0}, nTruePhiHits{0};
            
            double minYhit = std::numeric_limits<double>::max();
            double maxYhit = -1 * std::numeric_limits<double>::max();

            std::vector<unsigned char> matched;
            for (const auto & meas : segment->measurements()){
                // skip dummy measurement from beam spot constraint
                if (meas->type() == xAOD::UncalibMeasType::Other) continue;
                minYhit = std::min(meas->positionInChamber().y(),minYhit); 
                maxYhit = std::max(meas->positionInChamber().y(),maxYhit);
                if (m_writeSpacePoints) {
                    unsigned treeIdx = m_spTester->push_back(*meas->spacePoint());
                    if (treeIdx >= matched.size()){
                        matched.resize(treeIdx +1);
                    }
                    matched[treeIdx] = true;
                }
                if (m_visionTool.isEnabled()) {
                    nTrueHits += m_visionTool->isLabeled(*meas->spacePoint());
                    nTruePrecHits += isPrecHit(*meas) && m_visionTool->isLabeled(*meas->spacePoint());
                    nTrueEtaHits += meas->measuresEta() && m_visionTool->isLabeled(*meas->spacePoint());
                    nTruePhiHits += meas->measuresPhi() && m_visionTool->isLabeled(*meas->spacePoint());
                }
                switch (meas->type()) {
                    case xAOD::UncalibMeasType::MdtDriftCircleType:
                        ++nMdtHits;
                        break;
                    case xAOD::UncalibMeasType::RpcStripType:
                        nRpcEtaHits += meas->measuresEta();
                        nRpcPhiHits += meas->measuresPhi();
                        break;
                    case xAOD::UncalibMeasType::TgcStripType:
                        nTgcEtaHits += meas->measuresEta();
                        nTgcPhiHits += meas->measuresPhi();
                        break;
                    case xAOD::UncalibMeasType::MMClusterType:{
                        const MmIdHelper& idHelper{m_idHelperSvc->mmIdHelper()};
                        nMmEtaHits += !idHelper.isStereo(meas->spacePoint()->identify());
                        nMmStereoHits += !idHelper.isStereo(meas->spacePoint()->identify());
                        break;
                    } case xAOD::UncalibMeasType::sTgcStripType: {
                        const auto* prd = static_cast<const xAOD::sTgcMeasurement*>(meas->spacePoint()->primaryMeasurement());
                        nStgcStripHits += prd->channelType() == sTgcIdHelper::sTgcChannelTypes::Strip;
                        nStgcWireHits += prd->channelType() == sTgcIdHelper::sTgcChannelTypes::Wire;
                        nStgcPadHits += prd->channelType() == sTgcIdHelper::sTgcChannelTypes::Pad;
                        prd = dynamic_cast<const xAOD::sTgcMeasurement*>(meas->spacePoint()->secondaryMeasurement());
                        if (prd) {
                            nStgcWireHits += prd->channelType() == sTgcIdHelper::sTgcChannelTypes::Wire;
                            nStgcPadHits += prd->channelType() == sTgcIdHelper::sTgcChannelTypes::Pad;                            
                        }
                        break;
                    } default:
                        break;
                }
            }
            m_out_segment_nMdtHits.push_back(nMdtHits);
            m_out_segment_nRpcEtaHits.push_back(nRpcEtaHits);
            m_out_segment_nRpcPhiHits.push_back(nRpcPhiHits);
            m_out_segment_nTgcEtaHits.push_back(nTgcEtaHits);
            m_out_segment_nTgcPhiHits.push_back(nTgcPhiHits);

            m_out_segment_nMmEtaHits.push_back(nMmEtaHits);
            m_out_segment_nMmStereoHits.push_back(nMmStereoHits);
            m_out_segment_nsTgcStripHits.push_back(nStgcStripHits);
            m_out_segment_nsTgcWireHits.push_back(nStgcWireHits);
            m_out_segment_nsTgcPadpHits.push_back(nStgcPadHits);
        

	    m_out_segment_nTrueHits.push_back(nTrueHits);
            m_out_segment_nTruePrecHits.push_back(nTruePrecHits);
            m_out_segment_nTruePhiHits.push_back(nTruePhiHits);
            m_out_segment_nTrueEtaHits.push_back(nTrueEtaHits);

            m_out_segment_minYhit.push_back(minYhit);
            m_out_segment_maxYhit.push_back(maxYhit);
            if (m_writeSpacePoints) {
                m_spMatchedToSegment.push_back(std::move(matched));
            }
        }
    }
}  // namespace MuonValR4
