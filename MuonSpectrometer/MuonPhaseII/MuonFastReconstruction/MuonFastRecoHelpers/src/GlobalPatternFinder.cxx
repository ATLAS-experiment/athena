/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonFastRecoHelpers/GlobalPatternFinder.h"

#include "xAODMuonPrepData/MdtDriftCircle.h"
#include "MuonDetDescrUtils/MuonSectorMapping.h"

#include "CxxUtils/phihelper.h"

/// Macro printing verbose messages
#define PRINT_VERBOSE( xmsg )                                      \
    do {                                                           \
        if( logger->msgLvl( MSG::VERBOSE ) ) {                     \
            logger->msg( MSG::VERBOSE ) << xmsg << endmsg;         \
        }                                                          \
   } while( 0 )
namespace {
    const Muon::MuonSectorMapping sectorMap{};

    double inDegrees(double angle) {
        return angle / Gaudi::Units::deg;
    }
}

namespace MuonR4::FastReco {
GlobalPatternFinder::GlobalPatternFinder(const std::string& name, Config&& config) :
    AthMessaging{name},
    m_cfg{config} {
        static_assert(std::is_move_assignable_v<PatternState>);
        static_assert(std::is_move_constructible_v<PatternState>);
        static_assert(std::is_copy_assignable_v<PatternState>);
        static_assert(std::is_copy_constructible_v<PatternState>);
        static_assert(std::is_nothrow_move_constructible_v<PatternState>);
        static_assert(std::is_nothrow_move_assignable_v<PatternState>);
};


GlobalPatternFinder::PatternVec 
GlobalPatternFinder::findPatterns(const ActsTrk::GeometryContext& gctx,
                                  const SpacePointContainerVec& spacepoints,
                                  BucketPerContainer& outBuckets) const {
    /** Create the search tree by ordering hits in theta and **expanded** spectrometer sector 
     *  and find patterns in eta. The tree should outlive the pattern finding process */
    const SearchTree_t orderedSpacepoints {constructTree(gctx, spacepoints)};
    auto visualInfo {m_cfg.visionTool ? std::make_unique<PatternHitVisualInfoVec>() : nullptr};
    PatternStateVec patterns{findPatternsInEta(orderedSpacepoints, visualInfo.get())};

    /** Add phi-only hits to the patterns */
    addPhiOnlyHits(gctx, patterns);

    
    for (const PatternState& pat : patterns) {
        /** Add the successfull pattern to visual info, as we won't touch it again */
        addVisualInfo(pat, PatternHitVisualInfo::PatternStatus::eSuccessful, visualInfo.get());

        /** Fill the output buckets */
        for (const std::vector<CandidateHit>&  hits : pat.hitsPerStation) {
            for (const auto& hit : hits) {
                if (outBuckets.find(hit->container) == outBuckets.end()) {
                    throw std::runtime_error("The space point container associated to the pattern is not present in the output bucket map.");
                }
                auto& outBucketVec = outBuckets[hit->container];
                if (std::ranges::find(outBucketVec, hit->bucket) == outBucketVec.end()) {
                    outBucketVec.push_back(hit->bucket);
                }
            }
        }
    }
    /** Plot patterns */
    if (visualInfo) {
        m_cfg.visionTool->plotPatternBuckets(Gaudi::Hive::currentContext(), "GlobPatFind_", std::move(*visualInfo));
    }
    return convertToPattern(patterns);
}
GlobalPatternFinder::PatternStateVec 
GlobalPatternFinder::findPatternsInEta(const SearchTree_t& orderedSpacepoints,
                                       PatternHitVisualInfoVec* visualInfo) const {
    constexpr auto thetaIdx {Acts::toUnderlying(SeedCoords::eTheta)};
    constexpr auto sectorIdx {Acts::toUnderlying(SeedCoords::eSector)};

    /** Define two PatternState buffers to avoid reallocations */
    PatternStateVec startPatternBuff{};
    startPatternBuff.reserve(20);
    PatternStateVec endPatternBuff{};
    endPatternBuff.reserve(20);

    /** TODO: Retrieve the beamspot if desired */
    const Amg::Vector3D beamSpot{Amg::Vector3D::Zero()};
    
    PatternStateVec outPatterns{};
    outPatterns.reserve(40);
    /** @brief Helper function to count existing patterns containing a hit
     *  @param hit The hit to check
     *  @param coords The coordinates of the hit
     *  @return The number of existing patterns containing the hit */
    auto countPatterns = [&outPatterns, this](const HitPayload& hit, 
                                              const SearchTree_t::coordinate_t& coords) -> uint8_t {
        return std::ranges::count_if(outPatterns, [&](const PatternState& pattern){
            if (std::abs(pattern.theta - coords[thetaIdx]) > 2.*m_cfg.thetaSearchWindow ||
                !pattern.expSect.isNeighbour(ExpandedSector{static_cast<std::int8_t>(coords[sectorIdx])})) {
                return false;
            }
            return pattern.isInPattern(hit);
        });
    };
    using enum SeedCoords;
    for (const auto seedingLayer : m_cfg.layerSeedings) {
        /** We try to build a pattern in eta starting from every hit in the three */
        for (const auto& [seedCoords, seed] : orderedSpacepoints) {
            /** Check the seed is in the current seeding layer, and if seeding from MDT hits is enabled  */
            const LayerIndex seedLayer {toLayerIndex(seed.station)};
            if (seedLayer != seedingLayer || (seed.isStraw && !m_cfg.seedFromMdt)) {
                continue;
            }
            ATH_MSG_VERBOSE(__func__<<"() New seed hit "<<*seed<<", coordinates "<<seedCoords);
            /** check how many existing patterns contain this hit */
            uint8_t nExistingPatterns {countPatterns(seed, seedCoords)};
            if (nExistingPatterns >= m_cfg.maxSeedAttempts) {
                // Try first to resolve overlaps and re-count the number of patterns containing the seed
                outPatterns = resolveOverlaps(outPatterns, visualInfo);
                nExistingPatterns = countPatterns(seed, seedCoords);
                if (nExistingPatterns > m_cfg.maxSeedAttempts) {
                    ATH_MSG_VERBOSE(__func__<<"() Seed has already been used in "<<nExistingPatterns<<" patterns, which is above the limit - skip this seed.");
                    continue;
                }   
            }
            /** Define the search range. */
            SearchTree_t::range_t selectRange{};
            /** Search hits with same **expanded** sector */
            selectRange[sectorIdx].shrink(seedCoords[sectorIdx] - 0.1, seedCoords[sectorIdx] + 0.1);
            /** Define theta window size. While a middle-layer seed already constrains the track direction more tightly
             *  (the line must connect to hits on both sides), inner- and outer-layer seeds are more loosely constraining, 
             *  and we need a larger theta window with double size to achieve the same angular acceptance as inner/outer seeds */ 
            const double thetaHalfWindow {(seedLayer == LayerIndex::Inner || seedLayer == LayerIndex::Outer) 
                                    ? m_cfg.thetaSearchWindow : 0.5*m_cfg.thetaSearchWindow};
            selectRange[thetaIdx].shrink(seedCoords[thetaIdx] - thetaHalfWindow, seedCoords[thetaIdx] + thetaHalfWindow);
            /** Search for compatible spacepoints with the seed and check if there are enough to build a pattern */
            std::vector<CandidateHit> candidateHits{};
            orderedSpacepoints.rangeSearchMapDiscard(selectRange, [&candidateHits](const SearchTree_t::coordinate_t& /*coords*/,
                                                                                        const HitPayload& hit){
                candidateHits.emplace_back(&hit, hit.R, hit.Z, hit.station, 0u, hit.sector, hit.isStraw);
            });
            if (candidateHits.size() < m_cfg.minTriggerLayers + m_cfg.minPrecisionLayers) {
                ATH_MSG_VERBOSE(__func__<<"() Found "<<candidateHits.size()<<" candidate hits, below the minimum required - skip this seed.");
                continue;
            }
            /** Check that the candidate hits extend at least in two layers */
            if (std::ranges::none_of(candidateHits, [this, seedLayer](const CandidateHit& c){
                    return m_cfg.idHelperSvc->layerIndex(c.sp()->identify()) != seedLayer; }) ) {
                ATH_MSG_VERBOSE(__func__<<"() All candidates are in the same station layer, and we need at least two - skip this seed.");
                continue;
            }
            /** Sort the compatible spacepoints by global logical layer */
            std::ranges::sort(candidateHits, [](const CandidateHit& c1, const CandidateHit& c2){
                LayerOrdering ordering {checkLayerOrdering(*c1, *c2)};
                if (ordering == eSameLayer) {
                    /** If the two hits are in the same layer, sort them by local y coordinate. */
                    return c1.sp()->localPosition().y() < c2.sp()->localPosition().y();
                }
                return ordering == eLowerLayer;
            });
            /** Assign global layer number. This will avoid re-computing it many times later */
            for (std::size_t i {1}; i < candidateHits.size(); ++i) {
                candidateHits[i].globLayer = candidateHits[i - 1].globLayer + 
                    (checkLayerOrdering(*candidateHits[i - 1], *candidateHits[i]) != eSameLayer);
            }
            if (candidateHits.back().globLayer + 1u < (m_cfg.minTriggerLayers + m_cfg.minPrecisionLayers)) {
                ATH_MSG_VERBOSE(__func__<<"() Found "<<candidateHits.size()<<" candidate hits on "<<candidateHits.back().globLayer + 1
                                        <<" layers, below the minimum required - skip this seed.");
                continue;
            }
            if (msgLvl(MSG::VERBOSE)) {
                ATH_MSG_VERBOSE(__func__<<"() Found "<< candidateHits.size()<<" candidate hits: ");
                for (const auto& c : candidateHits) {
                    ATH_MSG_VERBOSE(__func__<<"() \t**"<<**c<<", glob Z/R/phi: "<<c.Z<<" / "<<c.R<<" / "<<inDegrees(c->phi) 
                        << ", st/layer: " << stName(c.station) << " / "<< (int)c.globLayer);
                }
            }

            /** Start pattern building from the seed */
            const auto seedItr {std::ranges::find_if(candidateHits,
                [&seed](const CandidateHit& c){ return *c == seed; })};
            assert(seedItr != candidateHits.end());
            const CandidateHit& seedCand {*seedItr};

            PatternState patternSeed{seedCand, static_cast<std::int8_t>(seedCoords[sectorIdx]), 
                seedCoords[thetaIdx], &m_cfg, this};
            if (visualInfo) {
                patternSeed.visualInfo = std::make_unique<PatternHitVisualInfo>(
                    seed.hit, seedCoords[thetaIdx] - thetaHalfWindow, seedCoords[thetaIdx] + thetaHalfWindow);
            }
            
            /** @brief Helper function to extend a given pattern with a set of hits. We can have pattern
             *         branching when a pattern is compatible with multiple hits on the same layer,
             *         increasing the number of active patterns. For each new hit, extendPatterns will try
             *         to extend every active pattern and remove the ones not meeting continuation criteria. 
             *  @param begin The iterator pointing to the first hit to process. 
             *  @param end The iterator pointing to the end of the hit range.
             *  @param toExtend The pattern to extend.
             *  @return The vector of resulting patterns. */
            auto processHitRange = [&](const auto begin, 
                                                 const auto end, 
                                                 PatternState&& toExtend) -> PatternStateVec {
                startPatternBuff.clear();
                startPatternBuff.push_back(std::move(toExtend));
                
                for (auto testItr = begin; testItr != end; ++testItr) {
                    const CandidateHit& testHit {*testItr};
                    if (testHit.globLayer == seedCand.globLayer && !seedCand.isStraw) {
                        continue; // skip hits on the same layer as the seed, if straw
                    }
                    extendPatterns(startPatternBuff, endPatternBuff, testHit, beamSpot, visualInfo);
                    // Swap the buffers for the next iteration
                    std::swap(startPatternBuff, endPatternBuff);
                }
                return startPatternBuff.size() > 1 
                    ? resolveOverlaps(startPatternBuff, visualInfo) 
                    : PatternStateVec{std::move(startPatternBuff.back())};
            };

            /** First search for compatible hits from the seed layer onwards */
            PatternStateVec forwardExtended {processHitRange(std::next(seedItr), candidateHits.end(), std::move(patternSeed))};

            /** When inverting the search direction, update last inserted hit and line anchor */
            ATH_MSG_VERBOSE(__func__<<"() Finished forward search, found "<<forwardExtended.size()<<" forward patterns, start backward search.");
            PatternStateVec backwardExtended{};
            backwardExtended.reserve(2*forwardExtended.size());
            
            for (PatternState& pat : forwardExtended) {
                pat.moveLineAnchorHit(seedCand);
                pat.lastInsertedHit = seedCand;

                /** Then try to proceed toward the innermost layer */
                std::ranges::move(
                    processHitRange(std::reverse_iterator(seedItr), candidateHits.rend(), std::move(pat)), 
                    std::back_inserter(backwardExtended)
                );
            }
            /** Ensure there are no overlaps */
            if (backwardExtended.size() > 1) {
                backwardExtended = resolveOverlaps(backwardExtended, visualInfo);
            }

            for (PatternState& pat : backwardExtended) {
                pat.meanNormResidual2 /= pat.nBendingLayers();
                if (!passPatternCuts(pat)) {
                    addVisualInfo(pat, PatternHitVisualInfo::PatternStatus::eFailed, visualInfo);
                    continue;
                }
                ATH_MSG_VERBOSE(__func__<<"() Add new pattern "<<detailed(pat));
                pat.isFinalized = true;
                outPatterns.push_back(std::move(pat));
            }
        }
    }
    ATH_MSG_VERBOSE(__func__<<"() Found in total "<<outPatterns.size()<<" patterns in eta before overlap removal");
    return resolveOverlaps(outPatterns, visualInfo);
}
void GlobalPatternFinder::extendPatterns(PatternStateVec& startPatterns,
                                         PatternStateVec& endPatterns,
                                         const CandidateHit& testHit,
                                         const Amg::Vector3D& beamSpot,
                                         PatternHitVisualInfoVec* visualInfo) const {
    endPatterns.clear();
    ATH_MSG_VERBOSE("processHitRange() *** Test "<<**testHit<<" R/Z/globLayer: "<<testHit.R<<", "<<testHit.Z
        <<", "<<static_cast<int>(testHit.globLayer)<<" against " << startPatterns.size() << " active patterns.");

    // Compute the minimum number of missed layer hits among the active patterns, 
    // to use as reference for pruning patterns with too many missed layers. 
    std::vector<unsigned> missedLayersVec{};
    missedLayersVec.reserve(startPatterns.size());
    std::ranges::transform(startPatterns, std::back_inserter(missedLayersVec), [&testHit](const PatternState& pat){
        return std::abs(pat.lastInsertedHit.globLayer - testHit.globLayer);
    });
    const unsigned minMissedLayers {std::ranges::min(missedLayersVec)};

    const bool shouldPrune {startPatterns.size() > 1 && 
        std::ranges::any_of(startPatterns, [](const PatternState& p){
            return p.nBendingLayers() > 2; })};
    
    for (auto [i, pat] : Acts::enumerate(startPatterns)) {
        if (pat.isOverlap) {
            continue;
        }
        // Check the pattern has not already missed too many layers compared to other patterns.
        if (pat.lastInsertedHit.station == testHit.station && 
            missedLayersVec[i] > std::max(m_cfg.maxMissLayersInStation, minMissedLayers)) {
            ATH_MSG_VERBOSE(__func__<<"() Pattern " << detailed(pat) << "\nhas missed " << (int)missedLayersVec[i] 
                                    << " layer hits, above the max allowed - abort pattern.");
            addVisualInfo(pat, PatternHitVisualInfo::PatternStatus::eFailed, visualInfo);
            continue;
        }
        /** Prunes pattern hypotheses within groups sharing the same last-hit layer. This step reduces branching by  
         *  keeping only the best-scoring pattern within each last-hit equivalence group, while preserving all 
         *  patterns when the last-hit layer matches the reference layer (to allow further branching). */
        if (shouldPrune && pat.lastInsertedHit.globLayer != testHit.globLayer &&
            std::ranges::find_if(std::next(startPatterns.begin(), i + 1), startPatterns.end(), [&](PatternState& p){
                if (p.lastInsertedHit != pat.lastInsertedHit || p.isOverlap) return false;

                if (isBetter(pat, p)) {
                    ATH_MSG_VERBOSE("extendPatterns() Pruning: REJECT " << detailed(p));
                    p.isOverlap = true;
                    return false;
                }
                ATH_MSG_VERBOSE("extendPatterns() Pruning: REJECT " << detailed(pat));
                return true; }) != startPatterns.end()) {
            continue;
        }
        /** Check angular compatibility of the test hit and the pattern */
        const auto [result, residual, accWindow] {pat.checkLineComp(testHit, beamSpot)};
        switch (result) {
            case LineTestDecision::eAddHit: {
                if (accWindow > 4.*m_cfg.baseRWindow && residual > m_cfg.baseRWindow) {
                    /** If hit is compatible but with poor confidence, we create both a pattern with the hit and a pattern without the hit,
                     *  to keep also the possibility of rejecting this hit in the next iterations. First we make sure that the low-confidence
                     *  pattern is original, i.e. accumulating not seen hits */
                    if (std::ranges::any_of(endPatterns, [&testHit, &pat](const PatternState& p) {
                            return p.isInLastLayer(testHit) && 
                                   (p.prevLayerHit == pat.lastInsertedHit || p.nBendingLayers() > (pat.nBendingLayers() + 1u)); })) {
                        ATH_MSG_VERBOSE(__func__<<"() Low-confidence hit: forking leads to existing pattern - reject.");
                        break;
                    }
                    ATH_MSG_VERBOSE(__func__<<"() Low-confidence hit: forking leads to new pattern - fork.");
                    /** Add the new pattern to the list of next patterns */
                    endPatterns.push_back(pat);
                    endPatterns.back().addHit(testHit, residual, accWindow);
                    /** Update visual information of the original pattern */
                    if (visualInfo) {
                        pat.visualInfo->discardedHits.push_back(testHit.sp());
                    }
                    break;
                }
                ATH_MSG_VERBOSE(__func__<<"() Hit compatible - add to pattern.");
                pat.addHit(testHit, residual, accWindow);
                break;
            }
            case LineTestDecision::eBranchPattern: {
                /* Check first if the branched pattern already exists*/
                if (std::ranges::any_of(endPatterns, [&testHit, &pat](const PatternState& p) {
                        return p.isInLastLayer(testHit) && p.prevLayerHit == pat.prevLayerHit; })) {
                    ATH_MSG_VERBOSE(__func__<<"() Hit compatible & on same layer of last added hit - branched pattern already exists.");
                    break;
                }
                /** Branch the pattern: we clone it and overwrite the existing hit with the test hit */
                ATH_MSG_VERBOSE(__func__<<"() Hit compatible & on same layer of last added hit - branch pattern.");
                endPatterns.push_back(pat);
                endPatterns.back().overWriteHit(testHit, residual, accWindow);
            
                /** Update visual information of the original pattern */
                if (visualInfo) {
                    pat.visualInfo->replacedHits.push_back(testHit.sp());
                    endPatterns.back().visualInfo->replacedHits.push_back(pat.lastInsertedHit.sp());
                }
                ATH_MSG_VERBOSE("New pattern: " << brief(endPatterns.back()));
                break;
            }
            case LineTestDecision::eRejectHit: {
                ATH_MSG_VERBOSE(__func__<<"() Hit is not compatible with the pattern - reject hit.");
                if (visualInfo) {
                    pat.visualInfo->discardedHits.push_back(testHit.sp());
                }
                break;
            }
            case LineTestDecision::eConsecutiveMdt: {
                /** Check the MDT cluster size on the last layer is not above the maximum, otherwise we break the cluster */ 
                if (const uint8_t nMdtLastLayer{pat.nMDTLastLayer()}; nMdtLastLayer >= 3) {
                    /** We break the residual at the second-to-last hit. So we compute the new cluster residual */
                    LineTestRes newClusterRes {pat.computeLineResidual(pat.lastInsertedHit)};
                    if (newClusterRes.residual > newClusterRes.accWindow) {
                        ATH_MSG_VERBOSE(__func__<<"() Hit is the #"<<nMdtLastLayer+1
                            <<" consecutive MDT hit: new cluster is not compatible - reject.");
                        break;
                    }
                    ATH_MSG_VERBOSE(__func__<<"() Hit is the #"<<nMdtLastLayer + 1
                        <<" consecutive MDT hit: new cluster is compatible - create new pattern with new cluster.");
                    endPatterns.push_back(pat);
                    /** We remove all hits on last layer, except the last hit. Then we add the new hit. */
                    endPatterns.back().overWriteHit(pat.lastInsertedHit, newClusterRes.residual, newClusterRes.accWindow);
                    endPatterns.back().addHit(testHit, residual, accWindow);
            
                    /** Update visual information of the original pattern */
                    if (visualInfo) {
                        pat.visualInfo->discardedHits.push_back(testHit.sp());
                        endPatterns.back().visualInfo->replacedHits.push_back(pat.lastInsertedHit.sp());
                    }
                    ATH_MSG_VERBOSE("New pattern: " << brief(endPatterns.back()));
                    break;
                }
                ATH_MSG_VERBOSE(__func__<<"() Consecutive MDT hits on same layer - accept. " << brief(pat));
                pat.addHit(testHit, residual, accWindow);
                break;
            }
            case LineTestDecision::eOverwriteLastHit: {
                ATH_MSG_VERBOSE(__func__<<"() Hit compatible & on same layer of last added hit - overwrite last hit.");
                pat.overWriteHit(testHit, residual, accWindow);

                if (visualInfo) {
                    pat.visualInfo->replacedHits.push_back(pat.lastInsertedHit.sp());
                }
                break;
            }
        }
        endPatterns.push_back(std::move(pat));
    }
    startPatterns.clear();
};
bool GlobalPatternFinder::passPatternCuts(const PatternState& pat) const {
    /** Check that the pattern meets the minimum requirements for trigger and precision layers */
    if (pat.nTriggerLayers < m_cfg.minTriggerLayers || 
        pat.nPrecisionLayers < m_cfg.minPrecisionLayers ||
        std::ranges::count_if(pat.nMeasurementLayers, 
            [this](const uint8_t nLayers) { return nLayers >= m_cfg.minStationLayers; }) < 2) {
        ATH_MSG_VERBOSE(__func__<<"() Pattern " << detailed(pat) << "\ndoes not meet minimum layer requirements - reject.");
        return false;
    }
    /** Check requirement on the residual */
    if (pat.meanNormResidual2 > m_cfg.meanNormRes2Cut) {
        ATH_MSG_VERBOSE(__func__<<"() Pattern " << detailed(pat) << "\ndoes not meet the mean norm residual2 cut - reject.");
        return false;
    }
    return true;
}
GlobalPatternFinder::PatternStateVec
GlobalPatternFinder::resolveOverlaps(PatternStateVec& toResolve,
                                     PatternHitVisualInfoVec* visualInfo) const {
    PatternStateVec outputPatterns{};
    outputPatterns.reserve(toResolve.size());
    /** Check if two patterns overlap in space */
    auto areOverlapping = [this](const PatternState& a, const PatternState& b) {
        /** Check first the geometrical overlap */
        if(!a.expSect.isNeighbour(b.expSect)) {
            return false;
        }
        if (std::abs(a.theta - b.theta) > 2.*m_cfg.thetaSearchWindow) {
            return false;
        }
        if (a.nPhiLayers > 0 && b.nPhiLayers > 0) {
            if (std::abs(CxxUtils::deltaPhi(a.phi, b.phi)) > 2.*m_cfg.phiTolerance) {
                return false;
            }
        } else if (a.nPhiLayers > 0) {
            if (!sectorMap.insideSector(b.expSect.msSector(),         a.phi) || 
                !sectorMap.insideSector(b.expSect.adjacentMsSector(), a.phi)) {
                return false;
            }
        } else if (b.nPhiLayers > 0) {
            if (!sectorMap.insideSector(a.expSect.msSector(),         b.phi) || 
                !sectorMap.insideSector(a.expSect.adjacentMsSector(), b.phi)) {
                return false;
            }
        }
        /** If we reach here, the patterns can overlap geometrically, so check the hit content */
        int nSharedHits{0};
        for (std::size_t st{0u}; st < s_nStations; ++st) {
            const auto& hitsA {a.hitsPerStation[st]};
            const auto& hitsB {b.hitsPerStation[st]};
            if (hitsA.empty() || hitsB.empty()) {
                continue;
            }
            nSharedHits += std::ranges::count_if(hitsA, [&](const CandidateHit& hitA){
                return std::ranges::any_of(hitsB, [&hitA](const CandidateHit& hitB) {
                    return hitA.sp()->primaryMeasurement() == hitB.sp()->primaryMeasurement();
                });
            });
        }
        /** Overlap if more than 50% of the hits of the smaller pattern are shared */
        const int minHits {std::min(a.nBendingHits(), b.nBendingHits())};
        return nSharedHits >= 0.5 *minHits;
    };
    /** Determine best pattern */
    auto isBetterOverlap = [](const PatternState& a, const PatternState& b) {
        const int nGoodStationDiff {a.nStations(/*onlyGoodStations=*/ true) - b.nStations(/*onlyGoodStations=*/ true)};
        if (nGoodStationDiff != 0) {
            return nGoodStationDiff > 0;
        }
        return isBetter(a,b);
    };

    for (auto it = toResolve.begin(); it != toResolve.end(); ++it) {
        if (it->isOverlap) {
            // If already marked as overlap, add to visual info, and discard the pattern
            addVisualInfo(*it, PatternHitVisualInfo::PatternStatus::eOverlap, visualInfo);
            continue;
        }
        for (auto jt = std::next(it); jt != toResolve.end(); ++jt) {
            if (jt->isOverlap || !areOverlapping(*it, *jt)) {
                continue;
            }
            if (isBetterOverlap(*it, *jt)) {
                ATH_MSG_VERBOSE(__func__<<"() Pattern "<<detailed(*it)<<"\nis BETTER than\n"<<detailed(*jt));
                jt->isOverlap = true;
            } else {
                it->isOverlap = true;
                ATH_MSG_VERBOSE(__func__<<"() Pattern "<<detailed(*jt)<<"\nis BETTER than\n"<<detailed(*it));
                break;
            }
        }
        if (!it->isOverlap) {
            outputPatterns.push_back( std::move(*it));
        } else {
            // If overlap, add to visual info, as the pattern will be discarded
            addVisualInfo(*it, PatternHitVisualInfo::PatternStatus::eOverlap, visualInfo);
        }
    }
    ATH_MSG_VERBOSE(__func__<<"() Patterns surviving overlap removal: "<< outputPatterns.size());
    return outputPatterns;
}
void GlobalPatternFinder::addPhiOnlyHits(const ActsTrk::GeometryContext& gctx,
                                         PatternStateVec& patterns) const {
    constexpr auto covIdxEta {Acts::toUnderlying(SpacePoint::CovIdx::etaCov)};
    /** @brief Struct to model the projection of the pattern line onto the phi strip in a certain station
     *  @param refLay: Reference layer coordinate (e.g. R or Z) increasing with the layer number
     *  @param refStrip: Reference strip coordinate (normal to the layer coordinate, along the strip direction)
     *  @param invSlope: Inverse slope, defined as deltaStrip / deltaLay */
    struct PhiStripProjectionModel {
        StIndex station{};
        double refLay{0.};
        double refStrip{0.};
        double invSlope{0.};
        bool isValid{false};

        double project(const Amg::Vector3D& pos) const {
            const double layCoord   = isBarrel(station) ? pos.perp() : pos.z();
            return refStrip + (layCoord - refLay) * invSlope;
        }
        double residual(const Amg::Vector3D& pos) const {
            const double stripCoord = isBarrel(station) ? pos.z() : pos.perp();
            return std::abs(project(pos) - stripCoord);
        }
    };
    auto makeProjectionModel = [this](PatternState& pat, const StIndex station) {
        PhiStripProjectionModel result{};
        result.station = station;
        const auto& hits {pat.hitsPerStation[Acts::toUnderlying(station)]};
        if (hits.empty()) return result;

        // Define the coordinate across the layers, i.e. orthogonal to the strip
        const auto layCoord = [&result](const HitPayload& hit) {
            return isBarrel(result.station) ? hit.R : hit.Z;
        };
        // Define the coordinate along the strip, i.e. orthogonal to the measureent layers
        const auto stripCoord = [&result](const HitPayload& hit) {
            return isBarrel(result.station) ? hit.Z : hit.R;
        };

        const HitPayload* sp1 {nullptr};
        const HitPayload* sp2 {nullptr};
        if (pat.nMeasurementLayers[Acts::toUnderlying(station)] > 1) {
            // if we have >= 2 eta hits in the station, we use the furthestmost to define the pattern line
            const auto [minIt, maxIt] = std::ranges::minmax_element(hits, {},
                [](const CandidateHit& c){ return c.globLayer; });
            sp1 = minIt->hit;
            sp2 = maxIt->hit;
        }
        if (!sp1 || !sp2 || std::abs(layCoord(*sp2) - layCoord(*sp1)) < m_cfg.minLayerSeparation) {
            // if we have only one eta hit, to find the second hit we use the we use the functionality of anchor hit
            pat.moveLineAnchorHit(hits.front());

            sp1 = hits.front().hit;
            sp2 = pat.lineAnchorHit.hit;
        }
        if (!sp1 || !sp2 ) return result;

        result.refLay = layCoord(*sp1);
        result.refStrip = stripCoord(*sp1);
        result.invSlope = (stripCoord(*sp2) - stripCoord(*sp1)) / (layCoord(*sp2) - layCoord(*sp1));
        result.isValid = true;
        return result;
    };

    PatternStateVec survivingPatterns{};
    survivingPatterns.reserve(patterns.size());
    for (PatternState& pat : patterns) {
        pat.finalizePatternEta();
        /** We look for phi-only hits in the buckets associated with the pattern */
        ATH_MSG_VERBOSE(__func__<<"() Search for phi-only hits for pattern: " << brief(pat));
        // Projection model of pattern line onto a given phi strip
        std::optional<PhiStripProjectionModel> patProjOnStrip{};
        for (const SpacePointBucket* bucket : pat.getParentBuckets()) {
            const Amg::Transform3D& localToGlobal {bucket->msSector()->localToGlobalTransform(gctx)};
            const StIndex station {m_cfg.idHelperSvc->stationIndex(bucket->front()->identify())};
            // If the projection model is not valid, we will use the pattern theta. We cache the local Y in glob frame
            const Amg::Vector3D locY {localToGlobal.linear() * Amg::Vector3D::UnitY()};
            
            for (const auto& hit : *bucket) {
                // We are looking for phi-only hits
                if (hit->measuresEta()){
                    continue;
                }
                ATH_MSG_VERBOSE(__func__<<"() *** Test phi-only hit "<<*hit);
                // Check phi compatibility
                const Amg::Vector3D locPosTest {hit->localPosition()};
                const Amg::Vector3D globPosTest {localToGlobal * locPosTest};
                const double globPhi {globPosTest.phi()};
                if (!pat.isPhiCompatible(globPhi)) {
                    ATH_MSG_VERBOSE(__func__<<"() Phi-only hit not compatible");
                    continue;
                }
                // Check there are not other phi hits in the same layer
                const uint8_t layNum = m_spSorter.sectorLayerNum(*hit);
                const uint8_t stIdx = Acts::toUnderlying(station);
                assert(!pat.hitsPerStation[stIdx].empty());
                if (std::ranges::any_of(pat.hitsPerStation[stIdx], [&](const CandidateHit& h){
                        return h.sp()->measuresPhi() && hit->msSector() == h.sp()->msSector() && layNum == h->locLayer; }) ||
                    std::ranges::any_of(pat.phiOnlyHits, [&](const HitPayload& h){
                        return station == h.station && hit->msSector() == h->msSector() && layNum == h.locLayer; })) {
                    ATH_MSG_VERBOSE(__func__<<"() The pattern already has a phi hit in the same layer - skip this test hit.");
                    continue;
                }
                // Check eta compatibility. Try first to use the pattern line projection model
                bool isEtaCompatible {false};
                const double sigmaEta {std::sqrt(hit->covariance()[covIdxEta])};
                if (!patProjOnStrip.has_value() || patProjOnStrip->station != station) {
                    patProjOnStrip = makeProjectionModel(pat, station);
                }
                if(patProjOnStrip->isValid) {
                    isEtaCompatible = patProjOnStrip->residual(globPosTest) <= 1.1*sigmaEta;
                    ATH_MSG_VERBOSE(__func__<<"() Distance pattern line from strip center: "<<patProjOnStrip->residual(globPosTest)
                                            <<", strip half-length: "<<sigmaEta<<", isCompatible: "<<isEtaCompatible);
                } else {
                    // Check pattern theta against global theta at the boundaries of the phi strip
                    double thetaMin {(globPosTest - sigmaEta * locY).theta()};
                    double thetaMax {(globPosTest + sigmaEta * locY).theta()};
                    if (thetaMax < thetaMin) {
                        std::swap(thetaMin, thetaMax);
                    }
                    isEtaCompatible = std::max(pat.theta - thetaMax, thetaMin - pat.theta) < 0.5 * m_cfg.thetaSearchWindow;
                    ATH_MSG_VERBOSE(__func__<<"() Pattern theta "<<inDegrees(pat.theta)
                                            <<", strip theta window: ["<<inDegrees(thetaMin)<<", "<<inDegrees(thetaMax)<<"]");
                    
                }
                if (!isEtaCompatible) {
                    ATH_MSG_VERBOSE(__func__<<"() The pattern falls outside the test hit strip in eta - skip this test hit.");
                    continue;
                }
                // Create the hit payload and add the hit to the pattern. Save only relevant quantities for phi-only hits.
                ATH_MSG_VERBOSE(__func__<<"() Phi-only hit compatible - add it to the pattern.");
                pat.phiOnlyHits.emplace_back(hit.get(), /*bucket*/nullptr, /*container*/nullptr, /*R*/0.f, /*Z*/0.f, 
                    globPhi, station, layNum, /*sector*/0u, /*isPrecision*/false, /*isStraw*/false);
                if (pat.nPhiLayers == 0) pat.phi = globPhi;
                pat.nPhiLayers++;
            }
        }
        if (pat.nPhiLayers < m_cfg.minPhiLayers) {
            ATH_MSG_VERBOSE(__func__<<"() Pattern "<< detailed(pat)<<" has only "<<pat.nPhiLayers
                                    <<" phi layers, below the minimum required - reject this pattern.");
            continue;
        }
        pat.finalizePatternPhi();
        survivingPatterns.push_back(std::move(pat));
    }
    std::swap(patterns, survivingPatterns);
}
GlobalPattern GlobalPatternFinder::convertToPattern(const PatternState& cache) const {
    GlobalPattern::HitCollection hitPerStation{};
    /** Add eta hits */
    for (uint8_t st{0u}; st < s_nStations; ++st) {
        const auto& hits {cache.hitsPerStation[st]};
        if (hits.empty()) continue;

        auto& out {hitPerStation[static_cast<StIndex>(st)]};
        out.reserve(hits.size());
        std::ranges::transform(hits, std::back_inserter(out), 
            [](const CandidateHit& h){ return h.sp();});
    }
    /** Add phi-only hits */
    for (const HitPayload& hit : cache.phiOnlyHits) {
        auto& out {hitPerStation[static_cast<StIndex>(hit.station)]};
        out.push_back(hit.sp());
    }
    GlobalPattern pattern{std::move(hitPerStation)};
    pattern.setTheta(cache.theta);
    pattern.setPhi(cache.phi);
    // Set the pattern sector(s) and theta.
    pattern.setSector(cache.expSect.sector());
    // Set pattern quality information.
    pattern.setNPrecisionLayers(cache.nPrecisionLayers);
    pattern.setNTriggerLayers(cache.nTriggerLayers);
    pattern.setNPhiLayers(cache.nPhiLayers);
    pattern.setMeanNormResidual2(cache.getMeanResidual2());
    return pattern;
}

GlobalPatternFinder::PatternVec
GlobalPatternFinder::convertToPattern(const PatternStateVec& cache) const {
    PatternVec patterns{};
    patterns.reserve(cache.size());
    std::transform(cache.begin(), cache.end(), std::back_inserter(patterns), 
        [this](const PatternState& cacheEntry) {
            return convertToPattern(cacheEntry);
    });
    return patterns;
}

GlobalPatternFinder::SearchTree_t 
GlobalPatternFinder::constructTree(const ActsTrk::GeometryContext& gctx,
                                   const SpacePointContainerVec& spacepoints) const {
    SearchTree_t::vector_t rawData{};
    using SectorProjector = ExpandedSector::SectorProjector;
    // Before the loops: estimate the total number of hits 
    size_t totalHits = 0;
    for (const SpacePointContainer* spc : spacepoints) {
        for (const SpacePointBucket* bucket : *spc) {
            totalHits += bucket->size();
        }
    }
    // We can have up to 3 entries per hit (when the hit does not measure phi).
    rawData.reserve(3 * totalHits);

    for (const SpacePointContainer* spc : spacepoints) {
        ATH_MSG_VERBOSE(__func__<<"() Processing "<<spc->size()<<" space point buckets...");
        for (const SpacePointBucket* bucket : *spc) {
            ATH_MSG_VERBOSE(__func__<<"() Processing " << bucket->size() << " spacepoints...");
            const Amg::Transform3D& localToGlobal {bucket->msSector()->localToGlobalTransform(gctx)};
            const StIndex bucketStation {m_cfg.idHelperSvc->stationIndex(bucket->front()->identify())};
            const uint8_t sector = bucket->msSector()->sector();

            for (const auto& hit : *bucket) {
                // Ignore only-phi hits and MDT hits if desired
                const bool isStraw {hit->isStraw()};
                if (!hit->measuresEta() || (!m_cfg.useMdtHits && isStraw)) {
                    continue;
                }
                ATH_MSG_VERBOSE(__func__<<"() Spacepoint: " << *hit);
                const Amg::Vector3D globalPos {localToGlobal * hit->localPosition()};
                const double globalPhi {globalPos.phi()};
                const ExpandedSector hitExpSector {globalPhi};
                const uint8_t localLayer = m_spSorter.sectorLayerNum(*hit);
                const bool isPrecision {MuonR4::isPrecisionHit(*hit)};

                /* Determine the projection direction, which is the direction normal to the radial direction */
                const Amg::Vector3D projDir {ExpandedSector{sector, SectorProjector::center}.normalDir()};

                /** Try to duplicate the hit in the neighboring sectors if it is close to the sector border. This ensures 
                 *  that we can find patterns crossing the sector borders. */ 
                for (const auto proj : {SectorProjector::leftOverlap, SectorProjector::center, SectorProjector::rightOverlap}) {
                    /// Check whether the hit belongs to the left or right sector as well
                    const ExpandedSector expSect {sector, proj};
                    if (proj != SectorProjector::center && hit->measuresPhi() && expSect != hitExpSector) {
                        ATH_MSG_VERBOSE(__func__<<"() Hit with "<<hitExpSector<<" is not compatible with "<<expSect);
                        continue;
                    }

                    /* Project the hit onto the plane along the sector radial direction. 
                     * This allows to remove the bias of hit displacement in phi direction */
                    const double projR {(globalPos - globalPos.dot(projDir) * projDir).perp()};
                    
                    std::array<double, 2> coords{};
                    coords[Acts::toUnderlying(SeedCoords::eTheta)] = atan2(projR, globalPos.z());
                    coords[Acts::toUnderlying(SeedCoords::eSector)] = expSect.sector();

                    ATH_MSG_VERBOSE(__func__<<"() Add hit: Z: " << globalPos.z() << ", R: " << globalPos.perp() << ", ProjR: " << projR
                                            << ", Phi: "<< inDegrees(globalPhi) << ", SectorPhi: " << inDegrees(expSect.phi()) << " and coordinates "<<coords<<" to the search tree");
                    rawData.emplace_back(std::move(coords), HitPayload{hit.get(), bucket, spc, 
                        static_cast<float>(projR), static_cast<float>(globalPos.z()), static_cast<float>(globalPhi), 
                        bucketStation, localLayer, sector, isPrecision, isStraw});
                }  
            }
        }
    }
    ATH_MSG_VERBOSE(__func__<<"() Create a new tree with "<<rawData.size()<<" entries. ");
    return SearchTree_t{std::move(rawData)};
}
GlobalPatternFinder::PatternState::PatternState(const CandidateHit& seed,
                                                const std::int8_t expSector,
                                                const double seedTheta,
                                                const Config* cfg,
                                                const AthMessaging* logger)
        : cfg{cfg},
          logger{logger},
          lastInsertedHit{seed},
          prevLayerHit{seed},
          lineAnchorHit{seed},
          theta{seedTheta},
          expSect{ExpandedSector{expSector}} {
          
    /** Update the hit counts in bending direction */
    nMeasurementLayers[Acts::toUnderlying(seed.station)]++;
    if (seed->isPrecision) nPrecisionLayers++;
    else nTriggerLayers++;
    /** Update the phi of the pattern */
    if (seed->sp()->measuresPhi()) {
        phi = seed->phi;
        nPhiLayers++;
    }
    /** Add now the new hit */
    hitsPerStation[Acts::toUnderlying(seed.station)].push_back(seed);
    needLineUpdate = true;
}
GlobalPatternFinder::LineTestRes 
GlobalPatternFinder::PatternState::checkLineComp(const CandidateHit& testHit,
                                                 const Amg::Vector3D& beamSpot) {
    // We test hits in the same **expanded** sector, so we need just to compare hit's phi with the pattern's phi, if both available
    if (nPhiLayers) {
        const double maxPhiDiff {testHit.sp()->measuresPhi() ? 
            cfg->phiTolerance : sectorMap.sectorWidth(testHit.sector)};
        if (std::abs(CxxUtils::deltaPhi(phi, static_cast<double>(testHit->phi))) > maxPhiDiff) {
            PRINT_VERBOSE(__func__<<"() The pattern with phi = "<<inDegrees(phi)
                <<" is not compatible with the test hit with phi "<<inDegrees(testHit->phi) << " - reject.");
            return LineTestRes{};
        }
    }
    
    /** @brief Helper function to make the result
     *  @param decision The decision for the test result if the residual is within the acceptance window 
     *  @return The test result */
    auto makeResult = [&testHit, this](const LineTestDecision decision) -> LineTestRes {
        LineTestRes res{computeLineResidual(testHit)};
        if (res.residual < res.accWindow) {
            res.result = decision;
        }
        if (visualInfo) {
            visualInfo->hitLineInfo[testHit.sp()] = std::make_pair(lineSlope, res.accWindow);
        }
        return res;
    };

    /*************** Test hit is on a new layer — draw line from line anchor to lastHit */
    if(testHit.globLayer != lastInsertedHit.globLayer) {
        updateLineParameters(beamSpot);
        return makeResult(LineTestDecision::eAddHit);
    }
    /*************** Test hit is on the same layer as the last inserted hit ***************/
    /** Sanity check: Test hit coincides with the last inserted hit */
    if (testHit == lastInsertedHit) {
        PRINT_VERBOSE(__func__<<"() Test hit is the same as last inserted hit - reject.");
        return LineTestRes{};
    }
    /** We add the test hit to the pattern if they are consecutive MDT hits */
    if (areConsecutiveMdt(testHit, lastInsertedHit)) {
        return LineTestRes{LineTestDecision::eConsecutiveMdt, -1., -1.};
    }
    /** If they are not consecutive MDT hits && all insterted hits are on the same layer */
    if (lineAnchorHit.globLayer == lastInsertedHit.globLayer) {
        PRINT_VERBOSE(__func__<<"() Test hit on same layer as seed with no prior hits, but not consecutive MDT hits - reject.");
        return LineTestRes{};
    }
    /** If the primary measurement is the same and both measure phi, we keep the most compatibble in phi */
    if (testHit.sp()->primaryMeasurement() == lastInsertedHit.sp()->primaryMeasurement()) { 
        if (testHit.sp()->measuresPhi() && !lastInsertedHit.sp()->measuresPhi()) {
            return makeResult(LineTestDecision::eOverwriteLastHit);
        }
        if (!testHit.sp()->measuresPhi() && lastInsertedHit.sp()->measuresPhi()) {
            return LineTestRes{};
        }
        if (nPhiLayers && std::abs(CxxUtils::deltaPhi(phi, static_cast<double>(testHit->phi))) < 
                        std::abs(CxxUtils::deltaPhi(phi, static_cast<double>(lastInsertedHit->phi)))) {
            return makeResult(LineTestDecision::eOverwriteLastHit);
        }
        return LineTestRes{};
    }
    /** sTGCs: we can have trigger hits (Pad) and precision hits (strip) on the same layer */
    if (testHit->isPrecision != lastInsertedHit->isPrecision) {
        if (lastInsertedHit->isPrecision) {
            /** Test hit is a trigger hit and last inserted is precision, keep the precision hit */
            PRINT_VERBOSE(__func__<<"() Test hit is a trigger hit and last inserted hit is precision on the same layer - keep the precision hit.");
            return LineTestRes{};
        }
        /** Test hit is a precision hit and last inserted is trigger, if compatible we overwrite the last inserted hit */
        PRINT_VERBOSE(__func__<<"() Test hit is a precision hit and last inserted hit is trigger on the same layer - check residual...");
        return makeResult(LineTestDecision::eOverwriteLastHit);
    }
    return makeResult(LineTestDecision::eBranchPattern);
}
void GlobalPatternFinder::PatternState::moveLineAnchorHit(const CandidateHit& refHit) {
    // Treat first the special case where we have only one station
    if (nStations(/*onlyGoodStations=*/ false) < 2) {
        // If we call this method with only one station, it means that we inverted the hit search direction without
        // finding any hit in other stations beside the initial one. So the anchor is the last added hit.
        lineAnchorHit = lastInsertedHit;
        return;
    }
    // Find first the closest station to the reference station among the pattern stations
    const auto& closestStIt = std::ranges::min_element(hitsPerStation, std::ranges::less{},
        [&refHit](const auto& hits){
            if (hits.empty() || hits.front().station == refHit.station) {
                return std::numeric_limits<int>::max();
            }
            return std::abs(hits.front().globLayer - refHit.globLayer);
        });

    // Then find the closest hit in that station to the reference hit
    const auto& hits {*closestStIt};
    auto it {std::ranges::min_element(hits, std::ranges::less{},
        [&refHit](const CandidateHit& hit){
            return std::abs(hit.globLayer - refHit.globLayer); })};
    
    // Find how many hits in the same layer we have, to be able to set the line anchor at the central hit
    uint8_t nSameLayer {1u};
    for (auto jt = std::next(it); jt != hits.end() && jt->globLayer == it->globLayer; ++jt) {
        ++nSameLayer;
    }
    lineAnchorHit = *std::next(it, (nSameLayer - 1u) / 2u);
}
void GlobalPatternFinder::PatternState::updateLineParameters(const Amg::Vector3D& beamSpot) {
    if (!needLineUpdate) {
        return;
    }
    const StIndex lastSt {lastInsertedHit.station};
    auto& hitsLastSt {hitsPerStation[Acts::toUnderlying(lastSt)]};
    // Check whether we have to use the beamspot instead of the last pattern hit to draw the line with the line anchor.
    const bool new_useBeamspot {lastSt == lineAnchorHit.station && 
            (isBarrel(lastSt) ? std::abs(lastInsertedHit.R - lineAnchorHit.R)
                              : std::abs(lastInsertedHit.Z - lineAnchorHit.Z)) <= cfg->minLayerSeparation};

    const double new_anchorR {new_useBeamspot ? beamSpot.perp() : lineAnchorHit.R};
    const double new_anchorZ {new_useBeamspot ? beamSpot.z()    : lineAnchorHit.Z};

    /* Determine the (average) coordinates of the last added hit(s). If it is a straw, find  
     * the consecutive (MDT) hits on the same layer and return use average position
     * for next computations — gives a more central reference for the line direction */
    double refR {0.}, refZ {0.};
    uint8_t nSameLayer {0u};
    for (const auto& hit : hitsLastSt) {
        if (hit.globLayer != lastInsertedHit.globLayer) continue;
        refR += hit.R;
        refZ += hit.Z;
        ++nSameLayer;
    }
    refR /= nSameLayer;
    refZ /= nSameLayer;

    const double new_dZ_slope {refZ - new_anchorZ};
    if (std::abs(new_dZ_slope) < 1.) {
        PRINT_VERBOSE("updateLineParameters() Couldn't update the line parameters.");
        return;
    }
    const double dR_slope {refR - new_anchorR};
    PRINT_VERBOSE("updateLineParameters() Update line parameters dZ/slope: "<<dZ_slope<<", " << lineSlope
            << " --> " << new_dZ_slope << ", " << dR_slope / new_dZ_slope);
    
    lineSlope = dR_slope / new_dZ_slope;
    dZ_slope = new_dZ_slope;
    anchorR = new_anchorR;
    anchorZ = new_anchorZ;
    useBeamspot = new_useBeamspot;
    // If we have only one hit and it is a straw, store the LR correction factor
    LR_factor = (nSameLayer == 1u && lastInsertedHit.isStraw) ? 
        lastInsertedHit.sp()->driftRadius()/ Acts::fastHypot(dZ_slope, dR_slope) : -1.;
    needLineUpdate = false;
}
GlobalPatternFinder::LineTestRes 
GlobalPatternFinder::PatternState::computeLineResidual(const CandidateHit& testHit) const {
    LineTestRes res{};

    // Compute the residual
    const double dZ_res {testHit.Z - anchorZ};
    res.residual = (testHit.R - anchorR) - lineSlope * dZ_res;

    /** The dynamic acceptance window is defined using the error propagation law for the residual.
    *  We have two contributions: the line extrapolation distance (propagation factor) and the 
    *  uncertainty on the line slope (geometrical factor) */
    const double geoFactor {1.+ Acts::square(lineSlope)};
    const double alpha {dZ_res / dZ_slope};  /** Alpha parameter determining the propagation/ line extrapolation magnitude */
    const double propFactor {1. - alpha + Acts::square(alpha)};
    res.accWindow = cfg->baseRWindow * std::sqrt(2*geoFactor * propFactor);
    /** Loosen the window when using the beamspot as reference, as the line slope is less well defined */
    if (useBeamspot) res.accWindow *= 1.5;
    if (testHit->station != lastInsertedHit.station ||
        (testHit->station != prevLayerHit.station && testHit.globLayer == lastInsertedHit.globLayer)) {
        res.accWindow *= 2.;
    }

    /** Apply LR correction if needed */
    if (LR_factor > 0.) {
        const double LRcorrection {dZ_res * geoFactor * LR_factor};
        PRINT_VERBOSE(__func__<<"() signed residual: " << res.residual << " -> Apply LR correction: " << LRcorrection);
        res.residual = std::min(std::abs(res.residual-LRcorrection), std::abs(res.residual+LRcorrection));
    } else {
        res.residual = std::abs(res.residual);
    }

    PRINT_VERBOSE(__func__<<"() "<< brief(*this) << "\nUse beamspot: " << useBeamspot << ", Slope: " << lineSlope 
                                    << ", Residual: " << res.residual << ", Window: " << res.accWindow << ", Geometrical Factor: " 
                                    << std::sqrt(geoFactor) << ", Propagation Factor: " << std::sqrt(propFactor));
    return res;
}
bool GlobalPatternFinder::PatternState::isPhiCompatible(const double testPhi) const {
    /** We check that the test hit is compatible with the pattern phi, if available, which is given by the first
     *  phi measurement in the pattern. If the pattern doesn't have a phi yet, we check that the test hit is in 
     *  the same pattern sector(s) */
    if (nPhiLayers) {
        if (std::abs(CxxUtils::deltaPhi(phi, testPhi)) > cfg->phiTolerance) {
            PRINT_VERBOSE(__func__<<"() The pattern with phi = "<<inDegrees(phi)
                <<" is not compatible with the test hit with phi "<<inDegrees(testPhi));
            return false;
        }
    } else {
        const unsigned sector1 {expSect.msSector()};
        const unsigned sector2 {expSect.adjacentMsSector()};
        const bool isCompatible {sector1 == sector2 
            ? sectorMap.insideSector(sector1, testPhi)
            : sectorMap.insideSector(sector1, testPhi) && sectorMap.insideSector(sector2, testPhi)};
        if (!isCompatible) {
            PRINT_VERBOSE(__func__<<"() The test hit with phi = "<<inDegrees(testPhi)
                <<" is not inside the pattern sectors: "<<sector1<<" and "<<sector2);
            return false;
        }            
    }
    return true;
}
void GlobalPatternFinder::PatternState::addHit(const CandidateHit& hit,
                                               const double residual,
                                               const double acceptWindow) {
    
    if (hit.globLayer != lastInsertedHit.globLayer) {
        /** Update the pointers to previous layer hit */
        prevLayerHit = lastInsertedHit;

        /** Update the hit counts in bending direction */
        nMeasurementLayers[Acts::toUnderlying(hit.station)]++;
        if (hit->isPrecision) nPrecisionLayers++;
        else nTriggerLayers++;
        /** Update the phi of the pattern if there are no phi hits yet */
        if (hit.sp()->measuresPhi()) {
            if (nPhiLayers == 0) phi = hit->phi;
            nPhiLayers++;
        }
    }
    /** Add now the new hit */
    hitsPerStation[Acts::toUnderlying(hit.station)].push_back(hit);
    lastInsertedHit = hit;

    /** Update the residual. Since we can also add consecutive MDT hits without updating the residual, we need to check the accept window */
    if (acceptWindow > 0.) {
        meanNormResidual2 += Acts::square(residual / acceptWindow);
        lastAcceptWindow = acceptWindow;
        lastResidual = residual;
    }
    // If the new compatible hit is in a different station, update the line anchor
    if (hit.station != lastInsertedHit.station) {
        moveLineAnchorHit(hit);
    }
    needLineUpdate = true;
}
void GlobalPatternFinder::PatternState::overWriteHit(const CandidateHit& newHit,
                                                     const double newResidual,
                                                     const double newAcceptWindow) {
    const StIndex st {newHit.station};
    if (st != lastInsertedHit.station || lastInsertedHit.globLayer != newHit.globLayer) {
        throw std::runtime_error(std::format(
            "Trying to overwrite a hit in station/layer {}/{} with another one from station/layer {}/{}", 
                stName(lastInsertedHit.station), lastInsertedHit.globLayer, stName(st), newHit.globLayer));
    }
    /* We expect to overwrite hits of the same type (precision/trigger), since we only branch when we have 
     * compatible hits in the same layer, except for sTGC hits, where we have pad and strips in the same layer */
    if (lastInsertedHit->isPrecision != newHit->isPrecision) {
        if (/*lastInsertedHit->isPrecision ||*/ newHit.sp()->type() != xAOD::UncalibMeasType::sTgcStripType) {
            std::stringstream ss {};
            ss << "Trying to overwrite a hit with incompatible type\n";
            ss << "Old hit: " << **lastInsertedHit << ", isPrecision: " << lastInsertedHit->isPrecision << ", measuresEta: " << lastInsertedHit.sp()->measuresEta() << "\n";
            ss << "New hit: " << **newHit << ", isPrecision: " << newHit->isPrecision << ", measuresEta: " << newHit.sp()->measuresEta();
            throw std::runtime_error(ss.str());
        }
        nTriggerLayers--;
        nPrecisionLayers++;
    }
    /** Update the phi counts */
    if (lastInsertedHit.sp()->measuresPhi()) nPhiLayers--;
    if (newHit.sp()->measuresPhi()) {
        if (nPhiLayers == 0) phi = newHit->phi;
        nPhiLayers++;
    }
    /** Update the residual */
    meanNormResidual2 += Acts::square(newResidual / newAcceptWindow) - Acts::square(lastResidual / lastAcceptWindow);
    lastAcceptWindow = newAcceptWindow;
    lastResidual = newResidual;

    auto& stHits {hitsPerStation[Acts::toUnderlying(st)]};
    /* Remove ALL hits in the same layer */
    while (!stHits.empty()) {
        if (stHits.back().globLayer != lastInsertedHit.globLayer) {
            break;
        }
        stHits.pop_back();
    }
    /** Add now the new hit */
    hitsPerStation[Acts::toUnderlying(st)].push_back(newHit);
    lastInsertedHit = newHit;
    needLineUpdate = true;
}
bool GlobalPatternFinder::PatternState::isInPattern(const HitPayload& hit) const {
    const auto& hits {hitsPerStation[Acts::toUnderlying(hit.station)]};
    return std::ranges::find_if(hits, 
        [&hit](const CandidateHit& c){ return *c == hit; }) != hits.end();                                           
}
void GlobalPatternFinder::PatternState::finalizePatternEta() {
    /** This method is called at the end of pattern building in eta, so we don't have phi-only hits yet */
    theta = 0.; 
    for (const std::vector<CandidateHit>&  hits : hitsPerStation) {
        for (const auto& hit : hits) {
            theta += atan2(hit.R, hit.Z);
        }
    }
    theta /= nBendingHits();
}
void GlobalPatternFinder::PatternState::finalizePatternPhi() {
    if (!nPhiLayers) {
        /** If there are no phi hits, we just use the central phi of the sector/overlap region */
        phi = sectorMap.sectorOverlapPhi(expSect.msSector(), expSect.adjacentMsSector());
        return;
    }
    double deltaPhiAcc {0.};
    std::optional<double> centralPhi {};
    auto processPhiHit = [&deltaPhiAcc, &centralPhi](const HitPayload& hit){
        if (!hit->measuresPhi()) {
            return;
        }
        if (!centralPhi) {
            centralPhi = hit.phi;
        }
        deltaPhiAcc += CxxUtils::deltaPhi(static_cast<double>(hit.phi), *centralPhi);
    };
    for (const std::vector<CandidateHit>&  hits : hitsPerStation) {
        for (const auto& hit : hits) {
            processPhiHit(*hit);
        }
    }
    for (const HitPayload& hit : phiOnlyHits) {
        processPhiHit(hit);
    }
    phi = CxxUtils::wrapToPi(centralPhi.value_or(0.) + deltaPhiAcc / nPhiLayers);
}
uint8_t GlobalPatternFinder::PatternState::nStations(const bool onlyGoodStations) const {
    uint8_t nStations {0u};
    for (uint8_t st{0u}; st < s_nStations; ++st) {
        if (!hitsPerStation[st].empty() && (!onlyGoodStations || nMeasurementLayers[st] >= cfg->minStationLayers)) {
            nStations++;
        }
    }
    return nStations;
}
uint8_t GlobalPatternFinder::PatternState::nBendingHits() const {
    return std::accumulate(hitsPerStation.begin(), hitsPerStation.end(), uint8_t{0u}, 
        [](uint8_t acc, const auto& hits){
            return acc + hits.size(); });
}
uint8_t GlobalPatternFinder::PatternState::nBendingLayers() const {
    return nPrecisionLayers + nTriggerLayers;
}
double GlobalPatternFinder::PatternState::getMeanResidual2() const {
    if (isFinalized) {
        return meanNormResidual2;
    }
    return meanNormResidual2 / nBendingLayers();
}
uint8_t GlobalPatternFinder::PatternState::nMDTLastLayer() const {
    const auto& stationHits {hitsPerStation[Acts::toUnderlying(lastInsertedHit.station)]};
    uint8_t nMdtLastayer {0u};
    for (auto it = stationHits.rbegin(); it != stationHits.rend(); ++it) {
        if (it->globLayer != lastInsertedHit.globLayer) {
            break;
        }
        nMdtLastayer++;
    }
    return nMdtLastayer;
}
std::vector<const SpacePointBucket*> GlobalPatternFinder::PatternState::getParentBuckets() const {
    std::vector<const SpacePointBucket*> buckets{};
    for (const std::vector<CandidateHit>&  hits : hitsPerStation) {
        for (const auto& hit : hits) {
            if (std::ranges::find(buckets, hit->bucket) == buckets.end()) {
                buckets.push_back(hit->bucket);
            }
        }
    }
    return buckets;
}
bool GlobalPatternFinder::PatternState::isInLastLayer(const CandidateHit& hit) const {
    if (lastInsertedHit.globLayer != hit.globLayer) {
        return false;
    }
    if (hit.isStraw) {
        const auto& stationHits {hitsPerStation[Acts::toUnderlying(hit.station)]};
        for (auto it = stationHits.rbegin(); it != stationHits.rend(); ++it) {
            if (it->globLayer != hit.globLayer) {
                return false;
            }
            if (*it == hit) {
                return true;
            }
        }
        return false;
    }
    return lastInsertedHit == hit;
}
bool GlobalPatternFinder::isBetter(const PatternState& a, const PatternState& b) {
    const int nLayerDiff {a.nBendingLayers() - b.nBendingLayers()};
    if (std::abs(nLayerDiff) >= 3) {
        return nLayerDiff > 0;
    }
    return a.getMeanResidual2() < b.getMeanResidual2();
}
GlobalPatternFinder::LayerOrdering
GlobalPatternFinder::checkLayerOrdering(const HitPayload& hit1,
                                        const HitPayload& hit2) {
    auto getLayerOrdering = [](const bool isLayer1Lower) {
        return isLayer1Lower ? eLowerLayer : eHigherLayer;
    };
    if (hit1 == hit2) {
        return eSameLayer;
    }
    /** Hits in the same spectrometer sector */
    if (hit1->msSector() == hit2->msSector()) {
        if (hit1.locLayer == hit2.locLayer) {
            return eSameLayer;
        } else {
            return getLayerOrdering(hit1.locLayer < hit2.locLayer);
        }
    }
    StIndex st1 {hit1.station};
    StIndex st2 {hit2.station};
    /** Hits in the same station and different sectors. We can have this case for hits in the overlap region of two adjacent sectors. */
    if (st1 == st2) {
        return getLayerOrdering(hit1->msSector()->barrel() ? hit1.R < hit2.R : std::abs(hit1.Z) < std::abs(hit2.Z));
    }
    LayerIndex layer1 {toLayerIndex(st1)};
    LayerIndex layer2 {toLayerIndex(st2)};
    if (layer1 == layer2) {
        /** Hit in different stations but same station layer. Expected to happen only for Inner and Middle*/
        if (layer1 == LayerIndex::Middle) {
            /** If both hits are in the middle layer, the one in the barrel comes first */
            return getLayerOrdering(st1 == StIndex::BM);
        } 
        if (layer1 == LayerIndex::Inner) {
            /** If both hits are in the inner layer, we use the global R, since in large sector BI comes first, while in small sector EI comes first. */
            return getLayerOrdering(hit1.R < hit2.R);
        }
        throw std::runtime_error("Unexpected to have two pattern-compatible hits one in BO and the other in EO.");
    }
    if (layer1 == LayerIndex::Inner || layer2 == LayerIndex::Inner) {
        /** If we have one hit in Inner layer for sure it comes first */
        return getLayerOrdering(layer1 == LayerIndex::Inner);
    }
    if (layer1 == LayerIndex::Outer || layer2 == LayerIndex::Outer) {
        /** If we have one hit in Outer layer for sure it comes last */
        return getLayerOrdering(layer2 == LayerIndex::Outer);
    }
    if (layer1 == LayerIndex::BarrelExtended || layer2 == LayerIndex::BarrelExtended) {
        /** If we have one hit in BarrelExtended and the other in the Middle layer, the former comes first */
        return getLayerOrdering(layer1 == LayerIndex::BarrelExtended);
    }
    /** If we have one hit in Extended (EE) layer and the other in the Middle layer, it depends if the latter is endcap or barrel */
    if (layer1 == LayerIndex::Extended) {
        return getLayerOrdering(st2 == StIndex::EM);
    }
    return getLayerOrdering(st1 == StIndex::BM);
}
bool GlobalPatternFinder::areConsecutiveMdt(const CandidateHit& hit1, 
                                            const CandidateHit& hit2) {
        if (!hit1.isStraw || !hit2.isStraw) {
            return false;
        }
        const uint16_t tubeNum1 {static_cast<const xAOD::MdtDriftCircle*>(hit1.sp()->primaryMeasurement())->driftTube()};
        const uint16_t tubeNum2 {static_cast<const xAOD::MdtDriftCircle*>(hit2.sp()->primaryMeasurement())->driftTube()};
        /** To understand: in principle we should not have two MDT hits in the same tube... but this can happen 
         *  now when running run4 digitization. Therefore, for now we don't throw when hit1==hit2 but we just reject
         *  one of the two... to understand for future improvements */
        if(tubeNum1 == tubeNum2) {
            return false;
        }; 
        return std::abs(tubeNum1-tubeNum2) < 2;
    }
void GlobalPatternFinder::addVisualInfo(const PatternState& cache,
                                        PatternHitVisualInfo::PatternStatus status,
                                        PatternHitVisualInfoVec* visualInfo) const {
    if (!visualInfo) {
        return;
    }
    // First save the buckets
    std::vector<const SpacePointBucket*> buckets{cache.getParentBuckets()};

    GlobalPattern pattern {convertToPattern(cache)};
    // Check whether the visual info about this pattern is already in the container
    if (auto it =std::ranges::find_if(*visualInfo, [&pattern](const auto& v){ 
        return v.patternCopy && *v.patternCopy == pattern; }); it != visualInfo->end()) {
        it->status = status; // Update the status if the pattern is already in the container
        return;
    }
    visualInfo->push_back(*cache.visualInfo);

    std::ranges::copy(buckets, std::back_inserter(visualInfo->back().parentBuckets));

    visualInfo->back().patternCopy = std::make_unique<GlobalPattern>(std::move(pattern));
    visualInfo->back().status = status;
}
bool GlobalPatternFinder::HitPayload::operator==(const HitPayload& other) const {
    return hit == other.hit;
}
void GlobalPatternFinder::PatternState::print(std::ostream& ostr, bool detailed) const {
    ostr<<"Pattern state, Exp Sector: "<<(int)expSect.sector()<<", Theta: "<<inDegrees(theta) << ", Phi: "<<inDegrees(phi);
    ostr<<", nPrec: "<<(int)nPrecisionLayers<<", nEtaNonPrec: "<<(int)nTriggerLayers<<", nPhi: "<<(int)nPhiLayers;
    ostr<<", mean norma res sq: "<<getMeanResidual2();
    ostr<<", Hit per station: \n";
    for (uint8_t st{0u}; st < s_nStations; ++st) {
        const auto& hits {hitsPerStation[st]};
        if (hits.empty()) continue;

        ostr<<"  Station "<<Muon::MuonStationIndex::stName(static_cast<StIndex>(st))<<" has "<<hits.size()<<" hits ";
        if (detailed) {
            ostr<<"\n";
            for (const auto& hit : hits) {
                ostr<<"    "<<**hit<<", R: "<<hit.R<<", Z: "<<hit.Z<<", Phi: "<<inDegrees(hit->phi)<<", loc/glob lay: "<<(int)hit->locLayer<<"/"<<(int)hit.globLayer<<"\n";
            }
        }
    }
    if (!detailed) {
        ostr <<"\n    Last hit: "<<**lastInsertedHit<<"\n    prevLayerHit: "<<**prevLayerHit << "\n    lineAnchorHit: "<<**lineAnchorHit;
    }
}
GlobalPatternFinder::PatternPrintView
GlobalPatternFinder::brief(const PatternState& p) {
    return {p, /*detailed=*/false};
}
GlobalPatternFinder::PatternPrintView
GlobalPatternFinder::detailed(const PatternState& p) {
    return {p, /*detailed=*/true};
}
std::ostream& operator<<(std::ostream& os, const GlobalPatternFinder::PatternPrintView& v) {
    v.pat.print(os, v.detailed);
    return os;
}
}