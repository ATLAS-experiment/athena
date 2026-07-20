/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonFastRecoHelpers/GlobalPatternFinder.h"

#include <xAODMuonPrepData/MdtDriftCircle.h>
#include "MuonDetDescrUtils/MuonSectorMapping.h"

#include "FourMomUtils/P4Helpers.h"

/// Macro printing verbose messages
#define PRINT_VERBOSE( xmsg )                                      \
    do {                                                           \
        if( logger->msgLvl( MSG::VERBOSE ) ) {                     \
            logger->msg( MSG::VERBOSE ) << xmsg << endmsg;         \
        }                                                          \
   } while( 0 ) 
namespace {
    const Muon::MuonSectorMapping sectorMap{};

    double inDeg(double angle) {
        return angle / Gaudi::Units::deg;
    }
    /* Function to extract the distance between two points in the direction 
     * perpendicular to measurement layers */
    double layerDistance(Muon::MuonStationIndex::StIndex station, 
                         const Amg::Vector3D& pos1, 
                         const Amg::Vector3D& pos2) {
        return isBarrel(station) ? std::abs(pos2.perp() - pos1.perp())
                                 : std::abs(pos2.z() - pos1.z());
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
    auto countPatterns = [this](const PatternStateVec& patterns,
                                const HitPayload& hit,
                                const SearchTree_t::coordinate_t& coords) -> uint8_t {
        return std::ranges::count_if(patterns, [&](const PatternState& pattern){
            const double patSeedTheta {pattern.seedHit->position.theta()};
            if (std::abs(patSeedTheta - coords[thetaIdx]) > 2.*m_cfg.thetaSearchWindow ||
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
            uint8_t nExistingPatterns {countPatterns(outPatterns, seed, seedCoords)};
            if (nExistingPatterns >= m_cfg.maxSeedAttempts) {
                // Try first to resolve overlaps and re-count the number of patterns containing the seed
                outPatterns = resolveOverlaps(outPatterns, visualInfo);
                nExistingPatterns = countPatterns(outPatterns,seed, seedCoords);
                if (nExistingPatterns >= m_cfg.maxSeedAttempts) {
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
                candidateHits.emplace_back(&hit, hit.station, 0u, hit.sector, hit.isStraw);
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
                ATH_MSG_VERBOSE(__func__<<"() Found "<<candidateHits.size()<<" candidate hits on "<<candidateHits.back().globLayer + 1u
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
    ATH_MSG_VERBOSE(__func__<<"() *** Test "<<testHit<<" against " << startPatterns.size() << " active patterns.");

    // Compute the minimum number of missed layer hits among the active patterns, 
    // to use as reference for pruning patterns with too many missed layers. 
    std::vector<unsigned> missedLayersVec{};
    missedLayersVec.reserve(startPatterns.size());
    std::ranges::transform(startPatterns, std::back_inserter(missedLayersVec), 
        [&testHit](const PatternState& pat){
            return std::abs(pat.lastInsertedHit.globLayer - testHit.globLayer);
    });
    const unsigned minMissedLayers {std::ranges::min(missedLayersVec)};

    const bool shouldPrune {startPatterns.size() > 1 && 
        std::ranges::any_of(startPatterns, [](const PatternState& p){
            return p.nBendingLayers() > 2; })};
    
    for (auto [i, pat] : Acts::enumerate(startPatterns)) {
        if (pat.isOverlap) {
            addVisualInfo(pat, PatternHitVisualInfo::PatternStatus::eFailed, visualInfo);
            continue;
        }
        /** Check the pattern has not already missed too many layers compared to other patterns. */
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
                    ATH_MSG_VERBOSE("extendPatterns() Pruning: "<<detailed(pat)<<"\nis BETTER than "<<detailed(p));
                    p.isOverlap = true;
                    return false;
                }
                ATH_MSG_VERBOSE("extendPatterns() Pruning: "<<detailed(p)<<"\nis BETTER than "<<detailed(pat));
                return true; }) != startPatterns.end()) {
            addVisualInfo(pat, PatternHitVisualInfo::PatternStatus::eFailed, visualInfo);
            continue;
        }
        /** Check angular compatibility of the test hit and the pattern */
        const auto [result, residual, accWindow] {pat.checkLineComp(testHit, beamSpot)};
        switch (result) {
            case LineTestDecision::eAddHit: {
                if (accWindow > 4.*m_cfg.baseResidualSigma && residual > m_cfg.baseResidualSigma) {
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
                    ATH_MSG_VERBOSE("New pattern: " << brief(endPatterns.back()));
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
            
                /** Update visual information */
                if (visualInfo) {
                    pat.visualInfo->discardedHits.push_back(testHit.sp());
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
                ATH_MSG_VERBOSE(__func__<<"() Compatible MDT hits on same layer - accept.");
                /* For consecutive MDT hits we don't add their residuals to not penalize patterns with many such hits */
                pat.addHit(testHit, -1., -1.);
                break;
            }
            case LineTestDecision::eOverwriteLastHit: {
                ATH_MSG_VERBOSE(__func__<<"() Hit compatible & on same layer of last added hit - overwrite last hit.");
                pat.overWriteHit(testHit, residual, accWindow);
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
        const double deltaThetaSeed {a.seedHit->position.theta() - b.seedHit->position.theta()};
        if (std::abs(deltaThetaSeed) > 2.*m_cfg.thetaSearchWindow) {
            return false;
        }
        
        if (a.nPhiLayers > 0 && b.nPhiLayers > 0) {
            if (std::abs(P4Helpers::deltaPhi(a.patPhi, b.patPhi)) > 2.*m_cfg.phiTolerance) {
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
     *  @param station: Station index of the projection model
     *  @param patPosition: Pattern line position in its phi plane
     *  @param patDirection: Pattern line direction in its phi plane
     *  @param isValid: Whether the projection is valid */
    struct PhiStripProjectionModel {
        StIndex station{};
        Amg::Vector3D patPosition{Amg::Vector3D::Zero()};
        Amg::Vector3D patDirection{Amg::Vector3D::Zero()};
        bool isValid{false};

        double residual(const Amg::Vector3D& stripPos, const Amg::Vector3D& stripDir) const {
            return Acts::detail::LineHelper::lineIntersect<3>(
                patPosition, patDirection, stripPos, stripDir).pathLength();
        }
    };
    auto makeProjectionModel = [this](PatternState& pat, const StIndex station) {
        PhiStripProjectionModel result{};
        result.station = station;
        const std::vector<CandidateHit>& stationHits {
            pat.hitsPerStation[Acts::toUnderlying(station)]};
        if (stationHits.empty()) return result;

        const HitPayload* sp1 {nullptr};
        const HitPayload* sp2 {nullptr};
        if (pat.nMeasurementLayers[Acts::toUnderlying(station)] > 1) {
            // if we have >= 2 eta hits in the station, we use the furthestmost to define the pattern line
            const auto [minIt, maxIt] {std::ranges::minmax_element(stationHits, {},
                [](const CandidateHit& c){ return c.globLayer; })};
            sp1 = minIt->hit;
            sp2 = maxIt->hit;
        }
        if (!sp1 || !sp2 || 
            layerDistance(station, pat.projToPhiPlane(*sp1), pat.projToPhiPlane(*sp2)) < m_cfg.minHitDistance4Line) {
            // if we have only one eta hit or the layer separation is too small, to find the second hit 
            // we use the functionality of anchor hit
            pat.moveLineAnchorHit(stationHits.front());
            sp1 = stationHits.front().hit;
            sp2 = pat.lineAnchorHit.hit;
        }
        if (!sp1 || !sp2 ) return result;

        Amg::Vector3D pos1 {pat.projToPhiPlane(*sp1)};
        Amg::Vector3D pos2 {pat.projToPhiPlane(*sp2)};
        
        result.patPosition = pos1;
        result.patDirection = (pos2 - pos1).unit();
        result.isValid = true;
        return result;
    };

    PatternStateVec survivingPatterns{};
    survivingPatterns.reserve(patterns.size());
    for (PatternState& pat : patterns) {
        /** We look for phi-only hits in the buckets associated with the pattern */
        ATH_MSG_VERBOSE(__func__<<"() Search for phi-only hits for pattern: " << brief(pat));

        // Projection model of pattern line onto a given phi strip
        std::optional<PhiStripProjectionModel> patProjOnStrip{};
        bool stopSearch {false};
        for (const SpacePointBucket* bucket : pat.getParentBuckets()) {
            if (stopSearch) break;

            const Amg::Transform3D& localToGlobal {bucket->msSector()->localToGlobalTransform(gctx)};
            const StIndex station {m_cfg.idHelperSvc->stationIndex(bucket->front()->identify())};
            // If the projection model is not valid, we will use the pattern theta. We cache the local Y in glob frame
            const Amg::Vector3D locY {localToGlobal.linear() * Amg::Vector3D::UnitY()};
            
            for (const auto& hit : *bucket) {
                if (pat.nPhiLayers >= m_cfg.minPhiLayers) {
                    stopSearch = true;
                    break;
                }
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
                const std::vector<CandidateHit>& stationHits {
                    pat.hitsPerStation[Acts::toUnderlying(station)]};
                assert(!stationHits.empty());
                if (std::ranges::any_of(stationHits, [&](const CandidateHit& h){
                        return h.sp()->measuresPhi() && hit->msSector() == h.sp()->msSector() && layNum == h->locLayer; }) ||
                    std::ranges::any_of(pat.phiOnlyHits, [&](const HitPayload& h){
                        return station == h.station && hit->msSector() == h->msSector() && layNum == h.locLayer; })) {
                    ATH_MSG_VERBOSE(__func__<<"() The pattern already has a phi hit in the same layer - skip hit.");
                    continue;
                }
                // Check eta compatibility.
                if (!patProjOnStrip.has_value() || patProjOnStrip->station != station) {
                    patProjOnStrip = makeProjectionModel(pat, station);
                }
                if (!patProjOnStrip->isValid) {
                    ATH_MSG_VERBOSE(__func__<<"() Invalid projection model for station "<<station<<" - skip hit.");
                    continue;
                }
                const Amg::Vector3D stripDir {localToGlobal.linear() * hit->sensorDirection()};
                const double stripHalfLength {std::sqrt(hit->covariance()[covIdxEta])};
                ATH_MSG_VERBOSE(__func__<<"() Distance pattern line from strip center: "
                    <<patProjOnStrip->residual(globPosTest, stripDir)<<", strip half-length: "<<stripHalfLength);

                if (patProjOnStrip->residual(globPosTest, stripDir) > 1.1*stripHalfLength) {
                    ATH_MSG_VERBOSE(__func__<<"() The pattern falls outside the test hit strip in eta - skip hit.");
                    continue;
                }
                // Create the hit payload and add the hit to the pattern. Save only relevant quantities for phi-only hits.
                ATH_MSG_VERBOSE(__func__<<"() Phi-only hit compatible - add it to the pattern.");
                pat.phiOnlyHits.emplace_back(hit.get(), /*bucket*/nullptr, /*container*/nullptr, globPosTest, Amg::Vector3D::Zero(),
                    station, layNum, /*sector*/0u, /*isPrecision*/false, /*isStraw*/false);

                if (pat.nPhiLayers == 0) pat.updatePatternPhi(globPhi);
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
    GlobalPattern::BucketCollection bucketPerStation{};
    /** Add eta hits */
    for (uint8_t st{0u}; st < s_nStations; ++st) {
        const auto& hits {cache.hitsPerStation[st]};
        if (hits.empty()) continue;

        auto& outHits {hitPerStation[static_cast<StIndex>(st)]};
        outHits.reserve(hits.size());
        auto& outBuckets {bucketPerStation[static_cast<StIndex>(st)]};
        
        std::ranges::for_each(hits, [&outHits, &outBuckets](const CandidateHit& h){
            outHits.push_back(h.sp());
            if (std::ranges::find(outBuckets, h->bucket) == outBuckets.end()) {
                outBuckets.push_back(h->bucket);
            }
        });
    }
    /** Add phi-only hits */
    for (const HitPayload& hit : cache.phiOnlyHits) {
        hitPerStation[static_cast<StIndex>(hit.station)].push_back(hit.sp());
    }
    GlobalPattern pattern{std::move(hitPerStation), std::move(bucketPerStation)};
    pattern.setTheta(cache.seedHit->position.theta());
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
    using enum SectorProjector;
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
            const Acts::SquareMatrix<3> rotation {localToGlobal.linear()};
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
                const Amg::Vector3D globWireDir {rotation * hit->sensorDirection()};
                const ExpandedSector hitExpSector {globalPos.phi()};

                /** Try to duplicate the hit in the neighboring sectors if it is close to the sector border. This ensures 
                 *  that we can find patterns crossing the sector borders. */ 
                for (const SectorProjector proj : {leftOverlap, center, rightOverlap}) {
                    /// Check whether the hit belongs to the left or right sector as well
                    const ExpandedSector expSect {sector, proj};
                    if (proj != SectorProjector::center && hit->measuresPhi() && expSect != hitExpSector) {
                        ATH_MSG_VERBOSE(__func__<<"() Hit with "<<hitExpSector<<" is not compatible with "<<expSect);
                        continue;
                    }

                    /* Project the hit onto the plane along the sector radial direction. 
                     * This allows to remove the bias of hit displacement in phi direction */
                    const Amg::Vector3D planeNormal {expSect.normalDir()};
                    const double projR {hit->measuresPhi() ? globalPos.perp() : (globalPos - globalPos.dot(planeNormal) * planeNormal).perp()};
                    
                    std::array<double, 2> coords{};
                    coords[Acts::toUnderlying(SeedCoords::eTheta)] = atan2(projR, globalPos.z());
                    coords[Acts::toUnderlying(SeedCoords::eSector)] = expSect.sector();

                    ATH_MSG_VERBOSE(__func__<<"() Add hit: Z: " << globalPos.z() << ", R: " << globalPos.perp() 
                        <<", ProjR: " << globalPos.perp()<< ", Phi: "<< inDeg(globalPos.phi()) 
                        <<", SectorPhi: "<< inDeg(expSect.phi())<<" and coordinates "<<coords<<" to search tree");
                    rawData.emplace_back(std::move(coords), HitPayload{hit.get(), bucket, spc, globalPos, globWireDir, bucketStation,
                        static_cast<uint8_t>(m_spSorter.sectorLayerNum(*hit)), sector, isPrecisionHit(*hit), isStraw});
                }  
            }
        }
    }
    ATH_MSG_VERBOSE(__func__<<"() Create a new tree with "<<rawData.size()<<" entries. ");
    return SearchTree_t{std::move(rawData)};
}
GlobalPatternFinder::PatternState::PatternState(const CandidateHit& seed,
                                                const std::int8_t expSector,          
                                                const Config* cfg,
                                                const AthMessaging* logger)
        : cfg{cfg},
          logger{logger},
          lastInsertedHit{seed},
          prevLayerHit{seed},
          lineAnchorHit{seed},
          seedHit{seed},
          expSect{ExpandedSector{expSector}} {
          
    /** Update the hit counts in bending direction */
    nMeasurementLayers[Acts::toUnderlying(seed.station)]++;
    if (seed->isPrecision) nPrecisionLayers++;
    else nTriggerLayers++;
    if (seed->sp()->measuresPhi()) nPhiLayers++;

    updatePatternPhi(seed->position.phi());

    /** Add now the new hit */
    hitsPerStation[Acts::toUnderlying(seed.station)].push_back(seed);
    needLineUpdate = true;
}
GlobalPatternFinder::LineTestRes 
GlobalPatternFinder::PatternState::checkLineComp(const CandidateHit& testHit,
                                                 const Amg::Vector3D& beamSpot) {
    // We test hits in same **expanded** sector, so we need just to compare hit's phi with pattern's phi, if available
    const double phiHit {testHit->position.phi()};
    if (nPhiLayers) {
        const double maxPhiDiff {testHit.sp()->measuresPhi() ? 
            cfg->phiTolerance : sectorMap.sectorWidth(testHit.sector)};
        if (std::abs(P4Helpers::deltaPhi(patPhi, phiHit)) > maxPhiDiff) {
            PRINT_VERBOSE(__func__<<"() The pattern with phi = "<<inDeg(patPhi)
                <<" is not compatible with the test hit with phi "<<inDeg(phiHit) << " - reject.");
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
            visualInfo->hitLineInfo[testHit.sp()] =
                std::make_pair(std::tan(lineDir.theta()), res.accWindow);
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
        return makeResult(LineTestDecision::eConsecutiveMdt);
    }
    /** If they are not consecutive MDT hits && all insterted hits are on the same layer */
    if (lineAnchorHit.globLayer == lastInsertedHit.globLayer) {
        PRINT_VERBOSE(__func__<<"() Test hit on same layer as seed with no prior hits, but not consecutive MDT hits - reject.");
        return LineTestRes{};
    }
    /** If the primary measurement is the same and both measure phi, we keep the most compatible in phi */
    if (testHit.sp()->primaryMeasurement() == lastInsertedHit.sp()->primaryMeasurement()) { 
        if (testHit.sp()->measuresPhi() && !lastInsertedHit.sp()->measuresPhi()) {
            return makeResult(LineTestDecision::eOverwriteLastHit);
        }
        if (!testHit.sp()->measuresPhi() && lastInsertedHit.sp()->measuresPhi()) {
            return LineTestRes{};
        }
        if (nPhiLayers && std::abs(P4Helpers::deltaPhi(patPhi, phiHit)) < 
                          std::abs(P4Helpers::deltaPhi(patPhi, lastInsertedHit->position.phi()))) {
            return makeResult(LineTestDecision::eOverwriteLastHit);
        }
        return LineTestRes{};
    }
    /** sTGCs: we can have trigger hits (Pad) and precision hits (strip) on the same layer */
    if (testHit->isPrecision != lastInsertedHit->isPrecision) {
        if (lastInsertedHit->isPrecision) {
            /** Test hit is a trigger hit and last inserted is precision, keep the precision hit */
            PRINT_VERBOSE(__func__<<"() Test hit is trigger hit and last inserted hit is precision, on the same layer - keep precision hit.");
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
    Amg::Vector3D pos1 {projToPhiPlane(*lineAnchorHit)};
    Amg::Vector3D pos2 {projToPhiPlane(*lastInsertedHit)};
    
    // Check whether we have to use the beamspot instead of the last pattern hit to draw the line with the line anchor.
    useBeamspot = (lastSt == lineAnchorHit.station) && 
        (pos1 - pos2).mag() < cfg->minHitDistance4Line;

    if (useBeamspot) {
        pos1 = beamSpot;
    }

    /* Determine the (average) coordinates of the last added hit(s). If it is a straw, find  
     * the consecutive (MDT) hits on the same layer and return use average position
     * for next computations — gives a more central reference for the line direction */
    std::vector<CandidateHit>& hitsLastSt {hitsPerStation[Acts::toUnderlying(lastSt)]};
    uint8_t nSameLayer {1u};
    for (const CandidateHit& hit : hitsLastSt) {
        if (hit == lastInsertedHit || 
            hit.globLayer != lastInsertedHit.globLayer) {
            continue;
        }
        pos2 += projToPhiPlane(*hit);
        ++nSameLayer;
    }
    if (nSameLayer > 1u) {
        pos2 = pos2 / nSameLayer;
    }

    const Amg::Vector3D d {pos2 - pos1};
    linePos = pos1;
    leverArm = d.mag();
    lineDir = d / leverArm;
    needLineUpdate = false;

    PRINT_VERBOSE(__func__<<"() Update line parameters --> Pos: "<<Amg::toString(linePos)
        <<" R/Z: "<<linePos.perp()<<"/"<<linePos.z()<<", Dir: " << Amg::toString(lineDir) 
        <<", slope: "<<std::tan(lineDir.theta())<<", LeverArm: " << leverArm);
}
GlobalPatternFinder::LineTestRes 
GlobalPatternFinder::PatternState::computeLineResidual(const CandidateHit& testHit) const {
    LineTestRes res{};

    // Compute the residual
    const Amg::Vector3D pos {projToPhiPlane(*testHit)};
    const Amg::Vector3D K {pos - linePos};
    res.residual = (K.cross(lineDir)).mag();

    /** Dynamic acceptance window derived from the propagation of the measurement variance to the residual.
     *  In this simple model, the measurement variance is assumed isotropic. The residual variance increases  
     *  with the normalized extrapolation distance alpha = s / L, where s is the projection of the hit
     *  onto the pattern direction and L is the pattern lever arm. */
    const double alpha {K.dot(lineDir) / leverArm};
    const double varianceScale {2. * (1. - alpha + Acts::square(alpha))};
    res.accWindow = cfg->baseResidualSigma * std::sqrt(varianceScale);
    /** Loosen the window when using the beamspot as reference, as the line slope is less well defined, 
     *  and when we are looking for hits in a new station. */
    if (useBeamspot || testHit->station != lastInsertedHit.station ||
        (testHit->station != prevLayerHit.station && testHit.globLayer == lastInsertedHit.globLayer)) {
        res.accWindow *= 2.;
    }
    PRINT_VERBOSE(__func__<<"() "<< brief(*this)<<"\nUse beamspot: "<<useBeamspot<<", Slope: "
        <<std::tan(lineDir.theta())<<", Residual: "<<res.residual<<", Window: "<<res.accWindow
        <<", alpha: "<<alpha<<", Scale Factor: "<<std::sqrt(varianceScale));
    return res;
}
Amg::Vector3D GlobalPatternFinder::PatternState::projToPhiPlane(const HitPayload& hit) const {
    const Amg::Vector3D& toProject {hit.position};
    if (hit->measuresPhi()) {
        const double R {toProject.perp()};
        return Amg::Vector3D{R * std::cos(patPhi), R * std::sin(patPhi), toProject.z()};
    }
    return Acts::PlanarHelper::intersectPlane(toProject, hit.sensorDir,
        bendPlaneNorm, Amg::Vector3D::Zero()).position();        
}
bool GlobalPatternFinder::PatternState::isPhiCompatible(const double testPhi) const {
    /** We check that the test hit is compatible with the pattern phi, if available, which is given by the first
     *  phi measurement in the pattern. If the pattern doesn't have a phi yet, we check that the test hit is in 
     *  the same pattern sector(s) */
    if (nPhiLayers) {
        if (std::abs(P4Helpers::deltaPhi(patPhi, testPhi)) > cfg->phiTolerance) {
            PRINT_VERBOSE(__func__<<"() The pattern with phi = "<<inDeg(patPhi)
                <<" is not compatible with the test hit with phi "<<inDeg(testPhi));
            return false;
        }
    } else {
        const unsigned sector1 {expSect.msSector()};
        const unsigned sector2 {expSect.adjacentMsSector()};
        const bool isCompatible {sector1 == sector2 
            ? sectorMap.insideSector(sector1, testPhi)
            : sectorMap.insideSector(sector1, testPhi) && sectorMap.insideSector(sector2, testPhi)};
        if (!isCompatible) {
            PRINT_VERBOSE(__func__<<"() The test hit with phi = "<<inDeg(testPhi)
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
            if (nPhiLayers == 0) {\
                updatePatternPhi(hit->position.phi());
            }
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
        if (newHit.sp()->type() != xAOD::UncalibMeasType::sTgcStripType) {
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
        if (nPhiLayers == 0) {
            updatePatternPhi(newHit->position.phi());
        }
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
        if (visualInfo) {
            visualInfo->replacedHits.push_back(stHits.back().sp());
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
void GlobalPatternFinder::PatternState::finalizePatternPhi() {
    if (!nPhiLayers) {
        /** If there are no phi hits, we just use the central phi of the sector/overlap region */
        patPhi = sectorMap.sectorOverlapPhi(expSect.msSector(), expSect.adjacentMsSector());
        return;
    }
    double deltaPhiAcc {0.};
    std::optional<double> centralPhi {};
    auto processPhiHit = [&deltaPhiAcc, &centralPhi](const HitPayload& hit){
        if (!hit->measuresPhi()) {
            return;
        }
        const double hitPhi {hit.position.phi()};
        if (!centralPhi) {
            centralPhi = hitPhi;
        }
        deltaPhiAcc += P4Helpers::deltaPhi(hitPhi, *centralPhi);
    };
    for (const std::vector<CandidateHit>&  hits : hitsPerStation) {
        for (const auto& hit : hits) {
            processPhiHit(*hit);
        }
    }
    for (const HitPayload& hit : phiOnlyHits) {
        processPhiHit(hit);
    }
    patPhi = P4Helpers::deltaPhi(centralPhi.value_or(0.) + deltaPhiAcc / nPhiLayers, 0.);
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
    if (hit.isStraw && lastInsertedHit.isStraw) {
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
void GlobalPatternFinder::PatternState::updatePatternPhi(const double newPhi) {
    patPhi = newPhi;
    bendPlaneNorm = Acts::makeDirectionFromPhiTheta(newPhi + 90._degree, 90._degree);
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
        return getLayerOrdering(isBarrel(st1) 
            ? hit1.position.perp() < hit2.position.perp() 
            : std::abs(hit1.position.z()) < std::abs(hit2.position.z()));
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
void GlobalPatternFinder::CandidateHit::print(std::ostream& ostr) const {
    ostr<<**hit<<", glob Z/R/phi: "<<hit->position.z()<<" / "<<hit->position.perp()<<" / "
        <<inDeg(hit->position.phi())<< ", st: " << station <<", loc/glob lay: "
        <<static_cast<int>(hit->locLayer)<<"/"<<static_cast<int>(globLayer);
}
void GlobalPatternFinder::PatternState::print(std::ostream& ostr, bool detailed) const {
    ostr<<"PatternState Exp Sector: "<<static_cast<int>(expSect.sector())
    <<", Theta: "<<inDeg(seedHit->position.theta()) << ", Phi: "<<inDeg(patPhi);
    ostr<<", nPrec: "<<(int)nPrecisionLayers<<", nEtaNonPrec: "<<(int)nTriggerLayers<<", nPhi: "<<(int)nPhiLayers;
    ostr<<", mean norma res sq: "<<getMeanResidual2();
    ostr<<", Hit per station: \n";
    for (uint8_t st{0u}; st < s_nStations; ++st) {
        const auto& hits {hitsPerStation[st]};
        if (hits.empty()) continue;

        ostr<<"  Station "<<static_cast<StIndex>(st)<<" has "<<hits.size()<<" hits ";
        if (detailed) {
            ostr<<"\n";
            for (const auto& hit : hits) {
                ostr<<"    "<<hit<<"\n";
            }
        }
    }
    if (!detailed) {
        ostr <<"\n    Last hit: "<<lastInsertedHit<<"\n    prevLayerHit: "
            <<prevLayerHit << "\n    lineAnchorHit: "<<lineAnchorHit;
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