/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonHoughTransformTester.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "MuonTesterTree/EventInfoBranch.h"
#include "MuonReadoutGeometryR4/SpectrometerSector.h"
#include "MuonPatternHelpers/SegmentFitHelperFunctions.h"
#include "MuonPatternEvent/MuonHoughDefs.h"
#include "xAODMuonPrepData/UtilFunctions.h"
#include "MuonPatternHelpers/HoughHelperFunctions.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "GaudiKernel/PhysicalConstants.h"


namespace MuonValR4 {
    using namespace MuonR4;
    using simHitSet = std::unordered_set<const xAOD::MuonSimHit*>;
    unsigned int countMatched(const simHitSet& truthHits,
                              const simHitSet& recoHits) {
        unsigned int matched{0};
        for (const xAOD::MuonSimHit* reco : recoHits) {
            matched += truthHits.count(reco);
        }
        return matched;
    }

    StatusCode MuonHoughTransformTester::initialize() {
        ATH_CHECK(m_geoCtxKey.initialize());
        ATH_CHECK(m_inHoughSegmentSeedKey.initialize());
        ATH_CHECK(m_truthSegmentKey.initialize(!m_truthSegmentKey.empty()));
        ATH_CHECK(m_inSegmentKey.initialize(!m_inSegmentKey.empty()));
        ATH_CHECK(m_tree.initialize(this)); 
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(detStore()->retrieve(m_r4DetMgr));
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
    std::vector<MuonValR4::ObjectMatching> 
            MuonHoughTransformTester::matchWithTruth(const ActsGeometryContext& gctx,
                                                     const xAOD::MuonSegmentContainer* truthSegments,
                                                     const SegmentSeedContainer* seedContainer,
                                                     const SegmentContainer* segmentContainer) const {
        std::vector<ObjectMatching> allAssociations{};
        std::unordered_set<const SegmentSeed*> usedSeeds{};
        std::unordered_set<const Segment*> usedSegs{};

        /// Step 1: Truth matched seeds and segments, and non-reconstructed truth. 
        /// Go from truth to the matched reco objects and fill one common tree entry
        if (truthSegments) {

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
                matchedWithTruth.chamber = m_r4DetMgr->getSectorEnvelope((*truthHits.begin())->identify());

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
                int segmentIdx{-1};
                std::vector<std::pair<const Segment*, unsigned>> matchedSegs{};
                for (const Segment* segment : *segmentContainer) {
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
                    matchedWithTruth.matchedSegHits.push_back(nMatchedHits); 
                    // and update our book-keeping to record that this segment and its seed have already been written 
                    usedSeeds.insert(matched->parent());
                    usedSegs.insert(matched);
                }
        
                // now, we add the seeds
                for (const auto& [seed , nHits] : matchedSeeds) {
                    // add seed to the list of all seeds
                    matchedWithTruth.matchedSeeds.push_back(seed);
                    matchedWithTruth.matchedSeedHits.push_back(nHits);
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
            ATH_CHECK(readHandle.isPresent());
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

        // retrieve the two input collections
        
        const SegmentSeedContainer* readSegmentSeeds{nullptr};
        ATH_CHECK(retrieveContainer(ctx, m_inHoughSegmentSeedKey, readSegmentSeeds));
        
        const SegmentContainer* readMuonSegments{nullptr};
        ATH_CHECK(retrieveContainer(ctx, m_inSegmentKey, readMuonSegments));
        
        const xAOD::MuonSegmentContainer* readTruthSegments{nullptr};
        ATH_CHECK(retrieveContainer(ctx, m_truthSegmentKey, readTruthSegments));
        

        ATH_MSG_DEBUG("Succesfully retrieved input collections. Seeds: "<<(readSegmentSeeds ? readSegmentSeeds->size() : -1)
                <<", segments: "<<(readMuonSegments ? readMuonSegments->size() : -1)
                <<", truth segments: "<<(readTruthSegments? readTruthSegments->size() : -1)<<".");

        std::vector<ObjectMatching> objects = matchWithTruth(gctx, readTruthSegments, readSegmentSeeds, readMuonSegments);
        for (const ObjectMatching& obj : objects) {
            m_tree.fillChamberInfo(obj.chamber);
            m_tree.fillTruthInfo(obj.truthSegment, m_r4DetMgr, gctx);
            m_tree.fillSeedInfo(obj);
            m_tree.fillSegmentInfo(gctx, obj);
            ATH_CHECK(m_tree.fill(ctx));
        }

        return StatusCode::SUCCESS;
    }
}  // namespace MuonValR4
