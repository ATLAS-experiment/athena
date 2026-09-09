/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonFastRecoHelpers/GlobalPatternFinder.h"
#include "MuonFastRecoHelpers/GlobalPatternFinderDefs.h"

#include "FourMomUtils/P4Helpers.h"
#include "MuonDetDescrUtils/MuonSectorMapping.h"

namespace {
    const Muon::MuonSectorMapping sectorMap{};

    /** @brief Function to convert an angle from radians to degrees */
    double inDeg(double angle) {
        return angle / Gaudi::Units::deg;
    }
}

namespace MuonR4::FastReco {
using namespace Acts::UnitLiterals;

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


std::vector<GlobalPattern> 
GlobalPatternFinder::findPatterns(const ActsTrk::GeometryContext& gctx,
                                  std::span<const SpacePointContainer*> spacepoints) const {
    /** Create the search tree by ordering hits in theta and **expanded** spectrometer sector 
     *  and find patterns in eta. The hit payloads are stored in a vector, and the tree will
     *  contain the index of the hits in the vector. */
    const SearchTreeData treeData {constructTree(gctx, spacepoints)};

    auto visualInfo {m_cfg.visionTool ? std::make_unique<std::vector<PatHitVisual>>() : nullptr};
    PatternStateVec patterns{findPatternsInEta(treeData.tree, visualInfo.get())};

    /** Add phi-only hits to the patterns */
    addPhiOnlyHits(gctx, patterns);
    
    for (const PatternState& pat : patterns) {
        /** Add the successfull pattern to visual info, as we won't touch it again */
        addVisualInfo(pat, PatHitVisual::PatternStatus::eSuccessful, visualInfo.get());
    }

    /** Plot patterns */
    if (visualInfo) {
        m_cfg.visionTool->plotPatternBuckets(Gaudi::Hive::currentContext(), "GlobPatFind_", std::move(*visualInfo));
    }
    return convertToPattern(patterns);
}
GlobalPatternFinder::PatternStateVec 
GlobalPatternFinder::findPatternsInEta(const SearchTree_t& orderedSpacepoints,
                                       std::vector<PatHitVisual>* visualInfo) const {
    constexpr auto thetaIdx {Acts::toUnderlying(SeedCoords::eTheta)};
    constexpr auto sectorIdx {Acts::toUnderlying(SeedCoords::eSector)};

    /** Define candidate hit buffer */
    std::vector<CandidateHit> candidateHits{};
    candidateHits.reserve(100);

    /** Define two PatternState buffers to avoid reallocations */
    PatternStateVec startPatternBuff{}, endPatternBuff{};
    startPatternBuff.reserve(10);
    endPatternBuff.reserve(10);

    /** TODO: Retrieve the beamspot if desired */
    const Amg::Vector3D beamSpot{Amg::Vector3D::Zero()};
    
    PatternStateVec outPatterns{};
    outPatterns.reserve(10);
    /** @brief Helper function to count existing patterns containing a hit
     *  @param hit The hit to check
     *  @param coords The coordinates of the hit
     *  @return The number of existing patterns containing the hit */
    auto countPatterns = [this](const PatternStateVec& patterns,
                                const HitPayload& hit,
                                const SearchTree_t::coordinate_t& coords) -> uint8_t {
        return std::ranges::count_if(patterns, [&](const PatternState& pattern){
            if (std::abs(pattern.patTheta - coords[thetaIdx]) > 2.*m_cfg.thetaSearchWindow ||
                !pattern.expSect.isNeighbour(
                    ExpandedSector{static_cast<std::int8_t>(coords[sectorIdx])})) {
                return false;
            }
            return pattern.isInPattern(hit);
        });
    };
    using enum SeedCoords;
    for (const auto seedingLayer : m_cfg.layerSeedings) {
        /** We try to build a pattern in eta starting from every hit in the three */
        for (const auto& [seedCoords, seedPtr] : orderedSpacepoints) {
            /** Get the seed hit */
            const HitPayload& seed {*seedPtr};
            /** Check the seed is in the current seeding layer, and if seeding from MDT hits is enabled  */
            const LayerIndex seedLayer {toLayerIndex(seed.station)};
            if (seedLayer != seedingLayer || (seed.isStraw && !m_cfg.seedFromMdt)) {
                continue;
            }
            ATH_MSG_VERBOSE(__func__<<"() New seed hit "<<*seed<<", coordinates "<<seedCoords);
            /** check how many existing patterns contain this hit */
            uint8_t nExistingPatterns {countPatterns(outPatterns, seed, seedCoords)};
            if (nExistingPatterns >= m_cfg.maxSeedAttempts) {
                // Try first to resolve overlaps and re-count the number of patterns containing the seed
                outPatterns = resolveOverlaps(outPatterns, visualInfo);
                nExistingPatterns = countPatterns(outPatterns,seed, seedCoords);
                if (nExistingPatterns >= m_cfg.maxSeedAttempts) {
                    ATH_MSG_VERBOSE(__func__<<"() Seed has already been used in "
                        <<static_cast<int>(nExistingPatterns)
                        <<" patterns, which is above the limit - skip this seed.");
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
            candidateHits.clear();
            orderedSpacepoints.rangeSearchMapDiscard(selectRange, [&](const SearchTree_t::coordinate_t& /*coords*/,
                                                                      const HitPayload* hit) {
                candidateHits.emplace_back(hit, hit->station, 0u);
            });
            if (candidateHits.size() < m_cfg.minTriggerLayers + m_cfg.minPrecisionLayers) {
                ATH_MSG_VERBOSE(__func__<<"() Found "<<candidateHits.size()<<" candidate hits, below minimum required - skip seed.");
                continue;
            }
            /** Check that the candidate hits extend at least in two layers */
            if (std::ranges::none_of(candidateHits, [this, seedLayer](const CandidateHit& c){
                    return m_cfg.idHelperSvc->layerIndex(c.sp()->identify()) != seedLayer; }) ) {
                ATH_MSG_VERBOSE(__func__<<"() All candidates in same station layer, and we need at least two - skip seed.");
                continue;
            }
            /** Sort the compatible spacepoints by global logical layer */
            std::ranges::sort(candidateHits, [](const CandidateHit& c1, const CandidateHit& c2){
                LayerOrdering ordering {checkLayerOrdering(*c1, *c2)};
                if (ordering == LayerOrdering::eSameLayer) {
                    /** If the two hits are in the same layer, sort them by local y coordinate. */
                    return c1.sp()->localPosition().y() < c2.sp()->localPosition().y();
                }
                return ordering == LayerOrdering::eLowerLayer;
            });
            /** Assign global layer number. This will avoid re-computing it many times later */
            for (std::size_t i {1}; i < candidateHits.size(); ++i) {
                candidateHits[i].globLayer = candidateHits[i - 1].globLayer + 
                    (checkLayerOrdering(*candidateHits[i - 1], *candidateHits[i]) != LayerOrdering::eSameLayer);
            }
            if (candidateHits.back().globLayer + 1u < (m_cfg.minTriggerLayers + m_cfg.minPrecisionLayers)) {
                ATH_MSG_VERBOSE(__func__<<"() Found "<<candidateHits.size()<<" candidate hits on "
                    <<static_cast<int>(candidateHits.back().globLayer + 1u)
                    <<" layers, below the minimum required - skip this seed.");
                continue;
            }
            if (msgLvl(MSG::VERBOSE)) {
                ATH_MSG_VERBOSE(__func__<<"() Found "<< candidateHits.size()<<" candidate hits: ");
                for (const auto& c : candidateHits) {
                    ATH_MSG_VERBOSE(__func__<<"() \t**"<<c);
                }
            }

            /** Start pattern building from the seed */
            const auto seedItr {std::ranges::find_if(candidateHits,
                [&seed](const CandidateHit& c){ return *c == seed; })};
            assert(seedItr != candidateHits.end());
            const CandidateHit& seedCand {*seedItr};

            PatternState patternSeed{seedCand, static_cast<std::int8_t>(seedCoords[sectorIdx]), &m_cfg, this};
            if (visualInfo) {
                patternSeed.visualInfo = std::make_unique<PatHitVisual>(
                    seed.sp, seedCoords[thetaIdx] - thetaHalfWindow, seedCoords[thetaIdx] + thetaHalfWindow);
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
                    if (testHit.globLayer == seedCand.globLayer) {
                        continue; // skip hits on the same layer as the seed
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
                ATH_MSG_VERBOSE(__func__<<"() Start backward search for pattern "<<detailed(pat));
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
                    addVisualInfo(pat, PatHitVisual::PatternStatus::eFailed, visualInfo);
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
                                         std::vector<PatHitVisual>* visualInfo) const {
    endPatterns.clear();
    ATH_MSG_VERBOSE(__func__<<"() *** Test "<<testHit<<" against " << startPatterns.size() << " active patterns.");

    // Compute the minimum number of missed layer hits among the active patterns, 
    // to use as reference for pruning patterns with too many missed layers. 
    auto missedLayers = [&testHit](const PatternState& pat) -> unsigned {
        return std::abs(pat.lastInsertedHit.globLayer - testHit.globLayer);
    };

    unsigned minMissedLayers {std::numeric_limits<unsigned>::max()};
    std::ranges::for_each(startPatterns, [&missedLayers, &minMissedLayers](const PatternState& pat){
        minMissedLayers = std::min(minMissedLayers, missedLayers(pat));
    });

    const bool shouldPrune {startPatterns.size() > 1 && 
        std::ranges::any_of(startPatterns, [](const PatternState& p){
            return p.nBendingLayers() > 2; })};
    
    for (auto [i, pat] : Acts::enumerate(startPatterns)) {
        if (pat.isOverlap) {
            addVisualInfo(pat, PatHitVisual::PatternStatus::eFailed, visualInfo);
            continue;
        }
        /** Check the pattern has not already missed too many layers compared to other patterns. */
        if (pat.lastInsertedHit.station == testHit.station && 
            missedLayers(pat) > std::max(m_cfg.maxMissLayersInStation, minMissedLayers)) {
            ATH_MSG_VERBOSE(__func__<<"() Pattern " << detailed(pat) << "\nhas missed " << (int)missedLayers(pat) 
                                    << " layer hits, above the max allowed - abort pattern.");
            addVisualInfo(pat, PatHitVisual::PatternStatus::eFailed, visualInfo);
            continue;
        }
        /** Prunes pattern hypotheses within groups sharing the same last-hit layer. This step reduces branching by  
         *  keeping only the best-scoring pattern within each last-hit equivalence group, while preserving all 
         *  patterns when the last-hit layer matches the reference layer (to allow further branching). */
        if (shouldPrune && pat.lastInsertedHit.globLayer != testHit.globLayer &&
            std::ranges::find_if(std::next(startPatterns.begin(), i + 1), startPatterns.end(), [&](PatternState& p){
                if (p.lastInsertedHit != pat.lastInsertedHit || p.isOverlap) return false;

                if (isBetter(pat, p)) {
                    ATH_MSG_VERBOSE("extendPatterns() Pruning: "<<detailed(pat)<<"\nis BETTER than "<<detailed(p));
                    p.isOverlap = true;
                    return false;
                }
                ATH_MSG_VERBOSE("extendPatterns() Pruning: "<<detailed(p)<<"\nis BETTER than "<<detailed(pat));
                return true; }) != startPatterns.end()) {
            addVisualInfo(pat, PatHitVisual::PatternStatus::eFailed, visualInfo);
            continue;
        }
        /** Check angular compatibility of the test hit and the pattern */
        const auto [residual, resSigma, result] {pat.checkLineComp(testHit, beamSpot)};
        switch (result) {
            case LineTestDecision::eAddHit: {
                /** TO DO: Study feasibility of loosening the criteria for low-confidence hits with OR */
                const bool lowConfidenceRes {resSigma > m_cfg.lowConfidenceResSigma && 
                                             residual / resSigma > 2.};
                if (lowConfidenceRes) {
                    ATH_MSG_VERBOSE(__func__<<"() Low-confidence hit: residual pull "<<residual / resSigma);
                    /** If hit is compatible but with poor confidence, we create both a pattern with the hit and a pattern without the hit,
                     *  to keep also the possibility of rejecting this hit in the next iterations. First we make sure that the low-confidence
                     *  pattern is original, i.e. accumulating not seen hits */
                    if (std::ranges::any_of(endPatterns, [&testHit, &pat](const PatternState& p) {
                            return p.lastInsertedHit == testHit && 
                                   (p.prevLayerHit == pat.lastInsertedHit || p.nBendingLayers() > (pat.nBendingLayers() + 1u)); })) {
                        ATH_MSG_VERBOSE(__func__<<"() Forking leads to existing pattern - reject.");
                        break;
                    }
                    /** Add the new pattern to the list of next patterns */
                    endPatterns.push_back(pat);
                    endPatterns.back().addHit(testHit, residual, resSigma);
                    /** Update visual information of the original pattern */
                    if (visualInfo) {
                        pat.visualInfo->discardedHits.push_back(testHit.sp());
                    }
                    break;
                }
                ATH_MSG_VERBOSE(__func__<<"() Hit compatible - add to pattern. Residual pull "<<residual / resSigma);
                pat.addHit(testHit, residual, resSigma);
                break;
            }
            case LineTestDecision::eBranchPattern: {
                /* Check first if the branched pattern already exists*/
                if (std::ranges::any_of(endPatterns, [&testHit, &pat](const PatternState& p) {
                        return p.lastInsertedHit == testHit && p.prevLayerHit == pat.prevLayerHit; })) {
                    ATH_MSG_VERBOSE(__func__<<"() Hit compatible & on same layer of last added hit - branched pattern already exists.");
                    break;
                }
                /** Branch the pattern: we clone it and overwrite the existing hit with the test hit */
                ATH_MSG_VERBOSE(__func__<<"() Hit compatible & on same layer of last added hit - branch pattern.");
                endPatterns.push_back(pat);
                endPatterns.back().overWriteHit(testHit, residual, resSigma);
            
                /** Update visual information */
                if (visualInfo) {
                    pat.visualInfo->discardedHits.push_back(testHit.sp());
                }
                break;
            }
            case LineTestDecision::eRejectHit: {
                ATH_MSG_VERBOSE(__func__<<"() Hit is not compatible with the pattern - reject hit.");
                if (visualInfo) {
                    pat.visualInfo->discardedHits.push_back(testHit.sp());
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
        std::ranges::count_if(pat.hitsPerStation, 
            [this](const auto& hits) { return hits.size() >= m_cfg.minStationLayers; }) < 2) {
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
                                     std::vector<PatHitVisual>* visualInfo) const {
    ATH_MSG_VERBOSE(__func__<<"() Resolving overlaps among "<<toResolve.size()<<" patterns.");
    PatternStateVec outputPatterns{};
    outputPatterns.reserve(toResolve.size());
    /** Check if two patterns overlap in space */
    auto areOverlapping = [this](const PatternState& a, const PatternState& b) {
        /** Check first the geometrical overlap */
        if(!a.expSect.isNeighbour(b.expSect)) {
            return false;
        }
        /** Check the angular difference between the seed hits */
        if (std::abs(a.patTheta - b.patTheta) > 2.*m_cfg.thetaSearchWindow) {
            return false;
        }
        if (a.nPhiLayers > 0 && b.nPhiLayers > 0) {
            if (std::abs(P4Helpers::deltaPhi(a.patPhi, b.patPhi)) > 5.*Gaudi::Units::deg) {
                return false;
            }
        } else if (a.nPhiLayers > 0) {
            if (!sectorMap.insideSector(b.expSect.msSector(),         a.patPhi) || 
                !sectorMap.insideSector(b.expSect.adjacentMsSector(), a.patPhi)) {
                return false;
            }
        } else if (b.nPhiLayers > 0) {
            if (!sectorMap.insideSector(a.expSect.msSector(),         b.patPhi) || 
                !sectorMap.insideSector(a.expSect.adjacentMsSector(), b.patPhi)) {
                return false;
            }
        }
        /** If we reach here, the patterns can overlap geometrically, so check the hit content */
        std::size_t nSharedHits{0}, nSharedStations{0};
        for (std::size_t st{0u}; st < s_nStations; ++st) {
            const auto& hitsA {a.hitsPerStation[st]};
            const auto& hitsB {b.hitsPerStation[st]};
            if (hitsA.empty() || hitsB.empty()) {
                continue;
            }
            const std::size_t nSharedInStation = std::ranges::count_if(hitsA, [&](const CandidateHit& hitA){
                return std::ranges::any_of(hitsB, [&hitA](const CandidateHit& hitB) {
                    return hitA.sp()->primaryMeasurement() == hitB.sp()->primaryMeasurement();
                });
            });
            nSharedHits += nSharedInStation;
            if (nSharedInStation >= m_cfg.minStationLayers) {
                nSharedStations++;
            }
        }
        /** Overlap if more than 50% of the hits of the smaller pattern are shared */
        const std::size_t minHits {std::min(a.nBendingLayers(), b.nBendingLayers())};
        const std::size_t minStations {std::min(a.nStations(/*onlyGoodStations=*/ true), b.nStations(/*onlyGoodStations=*/ true))};
        return nSharedHits >= 0.5 *minHits && nSharedStations >= std::min(2ul, minStations);
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
            addVisualInfo(*it, PatHitVisual::PatternStatus::eOverlap, visualInfo);
            continue;
        }
        for (auto jt = std::next(it); jt != toResolve.end(); ++jt) {
            if (jt->isOverlap || !areOverlapping(*it, *jt)) {
                continue;
            }
            if (isBetterOverlap(*it, *jt)) {
                ATH_MSG_VERBOSE(__func__<<"() Pattern "<<detailed(*it)<<"\nis BETTER than "<<detailed(*jt));
                jt->isOverlap = true;
            } else {
                it->isOverlap = true;
                ATH_MSG_VERBOSE(__func__<<"() Pattern "<<detailed(*jt)<<"\nis BETTER than "<<detailed(*it));
                break;
            }
        }
        if (!it->isOverlap) {
            outputPatterns.push_back( std::move(*it));
        } else {
            // If overlap, add to visual info, as the pattern will be discarded
            addVisualInfo(*it, PatHitVisual::PatternStatus::eOverlap, visualInfo);
        }
    }
    ATH_MSG_VERBOSE(__func__<<"() Patterns surviving overlap removal: "<< outputPatterns.size());
    return outputPatterns;
}
void GlobalPatternFinder::addPhiOnlyHits(const ActsTrk::GeometryContext& gctx,
                                         PatternStateVec& patterns) const {

    auto computePatternLineInStation = [](PatternState& pat, 
                                          const StIndex station) -> bool {
        const std::vector<CandidateHit>& stationHits {
            pat.hitsPerStation[Acts::toUnderlying(station)]};
        if(stationHits.empty()) {
            return false;
        }

        /** We use useBeamspot as a flag to indicate whether the pattern line has been determined successfully */
        pat.useBeamspot = true;

        if (stationHits.size() > 1) {
            // if we have >= 2 eta hits in the station, we use the furthestmost to define the pattern line
            const auto [minIt, maxIt] {std::ranges::minmax_element(stationHits, {},
                [](const CandidateHit& c){ return c.globLayer; })};
            pat.lineAnchorHit = *minIt;
            pat.lastInsertedHit = *maxIt;
            pat.updateLineParameters(Amg::Vector3D::Zero());
        }
        if (pat.useBeamspot) {
            // if we have only one eta hit or the layer separation is too small, to find the second hit 
            // we use the functionality of anchor hit
            pat.moveLineAnchorHit(stationHits.front());
            pat.lastInsertedHit = *std::ranges::max_element(stationHits, {},
                [&pat](const CandidateHit& c){ 
                    return (pat.projToPhiPlane(*c) - pat.projToPhiPlane(*pat.lineAnchorHit)).mag(); }); 
            pat.updateLineParameters(Amg::Vector3D::Zero());
        }
        return !pat.useBeamspot;
    };

    PatternStateVec survivingPatterns{};
    survivingPatterns.reserve(patterns.size());
    for (PatternState& pat : patterns) {
        /** We look for phi-only hits in the buckets associated with the pattern */
        ATH_MSG_VERBOSE(__func__<<"() Search for phi-only hits for pattern: " << brief(pat));

        std::optional<StIndex> patterLineStation{std::nullopt};

        auto projOntoPhiPlane = [&pat](const Amg::Vector3D& pos) -> Amg::Vector3D {
            return pos - pos.dot(pat.bendPlaneNorm) * pat.bendPlaneNorm;
        };

        for (const SpacePointBucket* bucket : pat.getParentBuckets()) {

            const Amg::Transform3D& localToGlobal {bucket->msSector()->localToGlobalTransform(gctx)};
            const StIndex station {m_cfg.idHelperSvc->stationIndex(bucket->front()->identify())};
            
            for (const auto& hit : *bucket) {
                // We are looking for phi-only hits
                if (hit->measuresEta()){
                    continue;
                }
                ATH_MSG_VERBOSE(__func__<<"() *** Test phi-only hit "<<*hit);

                /** Reject hits from a layer that already contains a phi hit */
                const uint8_t layNum = m_spSorter.sectorLayerNum(*hit);
                const std::vector<CandidateHit>& stationHits {
                    pat.hitsPerStation[Acts::toUnderlying(station)]};
                assert(!stationHits.empty());
                if (std::ranges::any_of(stationHits, [&](const CandidateHit& h){
                        return h->measuresPhi && 
                               hit->msSector() == h.sp()->msSector() && 
                               layNum == h->locLayer;
                        }) ||
                    std::ranges::any_of(pat.phiOnlyHits, [&](const HitPayload& h){
                        return station == h.station && 
                               hit->msSector() == h->msSector() && 
                               layNum == h.locLayer; 
                        })) {
                    ATH_MSG_VERBOSE(__func__<<"() The pattern already has a phi hit in the same layer - skip hit.");
                    continue;
                }
                /** Build the quantities needed for the phi compatibility test. */
                HitPayload newHit {hit.get(), bucket, localToGlobal, layNum, station};

                if (!pat.isPhiCompatible(newHit)) {
                    ATH_MSG_VERBOSE(__func__<<"() Phi-only hit not compatible");
                    continue;
                }

                if (!patterLineStation || *patterLineStation != station) {
                    if (!computePatternLineInStation(pat, station)) {
                        ATH_MSG_VERBOSE(__func__<<"() Invalid projection model for station "<<station<<" - skip hit.");
                        continue;
                    }
                    patterLineStation = station;
                }

                const double stripHalfLength {
                    std::sqrt(hit->covariance()[Acts::toUnderlying(SpacePoint::CovIdx::etaCov)])};
                const Amg::Vector3D stripLow {
                    projOntoPhiPlane(newHit.position - stripHalfLength * newHit.sensorDir)};
                const Amg::Vector3D stripHigh {
                    projOntoPhiPlane(newHit.position + stripHalfLength * newHit.sensorDir)};
                const Amg::Vector3D stripDirOnPlane {(stripHigh - stripLow).unit()};
                
                const double stripIntersect {Acts::detail::LineHelper::lineIntersect<3>(
                    pat.linePos, pat.lineDir, stripLow, stripDirOnPlane).pathLength()};
                const double stripProjLength {(stripHigh - stripLow).mag()};

                ATH_MSG_VERBOSE(__func__<<"() Intersect distance from lower strip edge: "
                    <<stripIntersect<<", proj strip length: "<< (stripHigh - stripLow).mag());
                
                constexpr double margin {10 * Gaudi::Units::mm};
                if (stripIntersect < -margin || stripIntersect > (stripProjLength + margin)) {
                    ATH_MSG_VERBOSE(__func__<<"() The pattern falls outside the test hit strip in eta - skip hit.");
                    continue;
                }
                pat.phiOnlyHits.push_back(std::move(newHit));
                pat.nPhiLayers++;
                pat.updatePatternPhi();
            }
        }
        if (pat.nPhiLayers < m_cfg.minPhiLayers) {
            ATH_MSG_VERBOSE(__func__<<"() Pattern "<<detailed(pat)
                <<" has only "<<static_cast<int>(pat.nPhiLayers)
                <<" phi layers, below the minimum required - reject this pattern.");
            continue;
        }
        survivingPatterns.push_back(std::move(pat));
    }
    std::swap(patterns, survivingPatterns);
}
GlobalPattern GlobalPatternFinder::convertToPattern(const PatternState& cache) const {
    GlobalPattern::HitCollection hitPerStation{};
    std::vector<const SpacePointBucket*> parentBuckets{};
    /** Add eta hits */
    for (uint8_t st{0u}; st < s_nStations; ++st) {
        const auto& hits {cache.hitsPerStation[st]};
        if (hits.empty()) continue;

        auto& outHits {hitPerStation[static_cast<StIndex>(st)]};
        outHits.reserve(hits.size());
        
        std::ranges::for_each(hits, [&outHits, &parentBuckets](const CandidateHit& h){
            outHits.push_back(h.sp());
            if (std::ranges::find(parentBuckets, h->bucket) == parentBuckets.end()) {
                parentBuckets.push_back(h->bucket);
            }
        });
    }

    /** Add phi-only hits */
    for (const HitPayload& hit : cache.phiOnlyHits) {
        hitPerStation[static_cast<StIndex>(hit.station)].push_back(hit.sp);
    }
    GlobalPattern pattern{std::move(hitPerStation), std::move(parentBuckets)};
    pattern.setTheta(cache.patTheta);
    pattern.setPhi(cache.patPhi);
    // Set the pattern sector(s) and theta.
    pattern.setSector(cache.expSect.sector());
    // Set pattern quality information.
    pattern.setNPrecisionLayers(cache.nPrecisionLayers);
    pattern.setNTriggerLayers(cache.nTriggerLayers);
    pattern.setNPhiLayers(cache.nPhiLayers);
    pattern.setMeanNormResidual2(cache.getMeanResidual2());
    return pattern;
}

std::vector<GlobalPattern>
GlobalPatternFinder::convertToPattern(const PatternStateVec& cache) const {
    std::vector<GlobalPattern> patterns{};
    patterns.reserve(cache.size());
    std::transform(cache.begin(), cache.end(), std::back_inserter(patterns), 
        [this](const PatternState& cacheEntry) {
            return convertToPattern(cacheEntry);
    });
    return patterns;
}

GlobalPatternFinder::SearchTreeData 
GlobalPatternFinder::constructTree(const ActsTrk::GeometryContext& gctx,
                                   std::span<const SpacePointContainer*> spacepoints) const {
    
    std::vector<HitPayload> hitPayloads{};
    using SectorProjector = ExpandedSector::SectorProjector;
    using enum SectorProjector;
    /** First estimate the number of hits */
    size_t totalHits = 0;
    for (const SpacePointContainer* spc : spacepoints) {
        for (const SpacePointBucket* bucket : *spc) {
            totalHits += bucket->size();
        }
    }
    hitPayloads.reserve(totalHits);

    for (const SpacePointContainer* spc : spacepoints) {
        for (const SpacePointBucket* bucket : *spc) {
            
            const Amg::Transform3D& localToGlobal {bucket->msSector()->localToGlobalTransform(gctx)};
            const StIndex station {m_cfg.idHelperSvc->stationIndex(bucket->front()->identify())};

            for (const auto& hit : *bucket) {
                // Ignore only-phi hits and MDT hits if desired
                if (!hit->measuresEta() || (!m_cfg.useMdtHits && hit->isStraw())) {
                    continue;
                }
                const uint8_t layNum = m_spSorter.sectorLayerNum(*hit);
         
                hitPayloads.emplace_back(hit.get(), bucket, localToGlobal, layNum, station);
  
                if (msgLvl(MSG::VERBOSE)) {
                    const HitPayload& newHit {hitPayloads.back()};
                    std::ostringstream oss{};
                    oss<<__func__<<"() Building hit from "<<*hit<<std::endl<<"PhiCov: "<<newHit.phiCov
                        <<", Pos: "<<Amg::toString(newHit.position)<<", SensorDir: "<<Amg::toString(newHit.sensorDir);
                    if (newHit.isStraw) {
                        oss<<", discCov: "<<newHit.secondaryMeasDir.x();
                    } else {
                        oss<<"orthogonalStrips: "<<!newHit.nonOrthogonalStrips
                           <<", secondaryMeasDir: "<<Amg::toString(newHit.secondaryMeasDir);
                    }
                    ATH_MSG_VERBOSE(oss.str());
                }
            }
        }
    }

    SearchTree_t::vector_t treeData{};
    treeData.reserve(3 * hitPayloads.size());

    for (const HitPayload& hit : hitPayloads) {
        ATH_MSG_VERBOSE(__func__<<"() Spacepoint: " << *hit);
        const Amg::Vector3D& pos {hit.position};
        const ExpandedSector hitExpSector {pos.phi()};
        
        /** Try to duplicate the hit in the neighboring sectors if it is close to the sector border.  
         *  This ensures that we can find patterns crossing the sector borders. */ 
        for (const SectorProjector proj : {leftOverlap, center, rightOverlap}) {
            /// Check whether the hit belongs to the left or right sector as well
            const ExpandedSector expSect {static_cast<uint8_t>(hit->msSector()->sector()), 
                                          proj};
            if (proj != SectorProjector::center && hit.measuresPhi && expSect != hitExpSector) {
                ATH_MSG_VERBOSE("addHitToTree() Hit with "<<hitExpSector<<" is not compatible with "<<expSect);
                continue;
            }

            /* Project the hit onto the plane along the sector radial direction. 
             * This allows to remove the bias of hit displacement in phi direction */
            const Amg::Vector3D planeNormal {expSect.normalDir()};
            const double projR {hit.measuresPhi
                ? pos.perp() 
                : (pos - pos.dot(planeNormal) * planeNormal).perp()};
            
            std::array<double, 2> coords{};
            coords[Acts::toUnderlying(SeedCoords::eTheta)] = atan2(projR, pos.z());
            coords[Acts::toUnderlying(SeedCoords::eSector)] = expSect.sector();

            ATH_MSG_VERBOSE("addHitToTree() Add hit: Z: " << pos.z() << ", R: " << pos.perp() 
                <<", ProjR: "<<projR<< ", Phi: "<< inDeg(pos.phi()) 
                <<", SectorPhi: "<< inDeg(expSect.phi())<<" and coordinates "<<coords<<" to search tree");
            treeData.emplace_back(std::move(coords), &hit);
        }  
    }
    ATH_MSG_VERBOSE(__func__<<"() Create a new tree with "<<treeData.size()
        <<" entries and "<<hitPayloads.size()<<" hits.");
    return SearchTreeData{std::move(hitPayloads), SearchTree_t{std::move(treeData)}};
}
bool GlobalPatternFinder::isBetter(const PatternState& a, const PatternState& b) {
    const double resA {a.getMeanResidual2()};
    const double resB {b.getMeanResidual2()};
    const double resDiff {
        std::abs(resA - resB) / std::max(resA, resB)
    };
    const int nLayerDiff {a.nBendingLayers() - b.nBendingLayers()};
    const int nPrecLayDiff {a.nPrecisionLayers - b.nPrecisionLayers};

    /** For patterns that differ by 1–2 layers, don't sacrifice fit quality  
     *  unless the extra layers are genuinely comparable. For a ≥3-layer difference,  
     *  the multiplicity advantage is strong enough to dominate. */
    if ((nLayerDiff == 0 && nPrecLayDiff == 0) ||
        (std::abs(nLayerDiff) < 3 && resDiff > 0.1)) {
        return resA < resB;
    }
    if (nLayerDiff == 0) {
        return nPrecLayDiff > 0;
    }
    return nLayerDiff > 0;
}
GlobalPatternFinder::LayerOrdering
GlobalPatternFinder::checkLayerOrdering(const HitPayload& hit1,
                                        const HitPayload& hit2) {
    using enum LayerOrdering;
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
        }
        return getLayerOrdering(hit1.locLayer < hit2.locLayer);
    }
    StIndex st1 {hit1.station};
    StIndex st2 {hit2.station};
    /** Hits in the same station and different sectors. We can have this case for hits 
     *  in the overlap region of two adjacent sectors. */
    if (st1 == st2) {
        const double delta {isBarrel(st1) 
            ? hit1.position.perp() - hit2.position.perp() 
            : std::abs(hit1.position.z()) - std::abs(hit2.position.z())}; 
        if (std::abs(delta) <= Acts::s_epsilon) {
            return eSameLayer;
        }
        return getLayerOrdering(delta < 0.);
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
            return getLayerOrdering(hit1.position.perp() < hit2.position.perp());
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
void GlobalPatternFinder::addVisualInfo(const PatternState& cache,
                                        PatHitVisual::PatternStatus status,
                                        std::vector<PatHitVisual>* visualInfo) const {
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
}