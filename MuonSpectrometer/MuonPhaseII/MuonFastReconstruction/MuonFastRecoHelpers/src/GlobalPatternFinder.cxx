/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonFastRecoHelpers/GlobalPatternFinder.h"

#include "xAODMuonPrepData/MdtDriftCircle.h"
#include "MuonDetDescrUtils/MuonSectorMapping.h"
#include "MuonTrackFindingTools/MsTrackSeeder.h"

#include "Acts/Utilities/Helpers.hpp"

#include "CxxUtils/phihelper.h"

namespace {
    const Muon::MuonSectorMapping sectorMap{};

    /** @brief Helper function to check whether two hits are consecutive MDT measurements */
    bool areConsecutiveMdt(const MuonR4::SpacePoint& hit1, const MuonR4::SpacePoint& hit2) {
        if (!hit1.isStraw() || !hit2.isStraw()) {
            return false;
        }
        const uint16_t tubeNum1 {static_cast<const xAOD::MdtDriftCircle*>(hit1.primaryMeasurement())->driftTube()};
        const uint16_t tubeNum2 {static_cast<const xAOD::MdtDriftCircle*>(hit2.primaryMeasurement())->driftTube()};
        /** To understand: in principle we should not have two MDT hits in the same tube... but this can happen 
         *  now when running run4 digitization. Therefore, for now we don't throw when hit1==hit2 but we just reject
         *  one of the two... to understand for future improvements */
        if(tubeNum1 == tubeNum2) {
            return false;
        }; 
        return std::abs(tubeNum1-tubeNum2) < 2;
    }
    double inDegrees(double angle) {
        return angle / Gaudi::Units::deg;
    }
    using PatternHitVisualInfo = MuonValR4::IFastRecoVisualizationTool::PatternHitVisualInfo;
    using PatternHitVisualInfoVec = std::vector<PatternHitVisualInfo>;
    /** @brief Helper function meant to be used during pattern building. We can use the overlap removal during the pattern
      *        building to remove overlapping pattern candidates. While we want to add to VisualInfo the pattern candidates
      *        marked as overlaps and discarded, we want to remove at this stage the visual Info for pattern surviving the 
      *        overlap removal, since their visual info will be added later during the final overlap removal of finalized patterns */
    void resetVisualToOverlap(PatternHitVisualInfoVec* visualInfo) {
        if (!visualInfo) return;
        const auto [first, last] = std::ranges::remove_if(*visualInfo, [](const PatternHitVisualInfo& v){ 
            return v.status == PatternHitVisualInfo::PatternStatus::eSuccessful; });
        
        visualInfo->erase(first,last);
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
    /** Create the search tree by ordering hits in theta and **expanded** spectrometer sector and find patterns in eta */
    auto visualInfo {m_cfg.visionTool ? std::make_unique<PatternHitVisualInfoVec>() : nullptr};
    PatternStateVec patterns{findPatternsInEta(constructTree(gctx, spacepoints), visualInfo.get())};

    /** Add phi-only hits to the patterns */
    addPhiOnlyHits(gctx, patterns);

    /** Fill the output buckets & visual info */
    for (const PatternState& pat : patterns) {
        /** Add the successfull pattern to visual info, as we won't touch it again */
        addVisualInfo(pat, PatternHitVisualInfo::PatternStatus::eSuccessful, visualInfo.get());

        for (const auto& [spc, bucketVec] : pat.bucketsPerContainer) {
            if (outBuckets.find(spc) == outBuckets.end()) {
                throw std::runtime_error("The space point container associated to the pattern is not present in the output bucket map.");
            }
            for (const SpacePointBucket* bucket : bucketVec) {
                auto& outBucketVec = outBuckets[spc];
                if (std::ranges::find(outBucketVec, bucket) == outBucketVec.end()) {
                    outBucketVec.push_back(bucket);
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
    PatternStateVec patterns{};
    using enum SeedCoords;
    for (const auto seedingLayer : m_cfg.layerSeedings) {
        ATH_MSG_VERBOSE(__func__<<"() Start pattern search in eta with seeding layer "<< layerName(seedingLayer));
        /** We try to build a pattern in eta starting from every hit in the three */
        for (const auto& [seedCoords, seed] : orderedSpacepoints) {
            /** Check the seed is in the current seeding layer, and if seeding from MDT hits is enabled  */
            const LayerIndex seedLayer {toLayerIndex(seed.station)};
            if (seedLayer != seedingLayer || (seed->isStraw() && !m_cfg.seedFromMdt)) {
                continue;
            }
            ATH_MSG_VERBOSE(__func__<<"() New seed hit "<<*seed<<", coordinates "<<seedCoords);
            /** check how many existing (and overlapping) patterns contain this hit */
            auto nExistingPatterns {std::ranges::count_if(patterns, [&seed, &seedCoords, this](const PatternState& pattern){
                if (std::abs(pattern.theta - seedCoords[thetaIdx]) > m_cfg.thetaSearchWindow ||
                    pattern.sectorCoord != static_cast<int>(seedCoords[sectorIdx])) {
                    return false;
                }
                return pattern.isInPattern(seed);
            })};
            if (nExistingPatterns >= m_cfg.maxSeedAttempts) {
                ATH_MSG_VERBOSE(__func__<<"() Seed has already been used in "<<nExistingPatterns<<" patterns, which is above the limit - skip this seed.");
                continue;
            }
            /** Define the search range. */    
            SearchTree_t::range_t selectRange{};
            /** Search hits with same **expanded** sector */
            selectRange[sectorIdx].shrink(seedCoords[sectorIdx] - 0.1, seedCoords[sectorIdx] + 0.1);
            /** Define theta window size. While a middle-layer seed already constrains the track direction more tightly
             *  (the line must connect to hits on both sides), inner- and outer-layer seeds are more loosely constraining, 
             *  and we need a larger theta window with double size to achieve the same angular acceptance as inner/outer seeds */ 
            const double thetaWindow {(seedLayer == LayerIndex::Inner || seedLayer == LayerIndex::Outer) 
                                    ? m_cfg.thetaSearchWindow : 0.5*m_cfg.thetaSearchWindow};
            selectRange[thetaIdx].shrink(seedCoords[thetaIdx] - thetaWindow, seedCoords[thetaIdx] + thetaWindow);
            /** Search for compatible spacepoints with the seed and check if there are enough to build a pattern */
            std::vector<TreeNode> candidateHits = orderedSpacepoints.rangeSearchWithKey(selectRange);
            if (candidateHits.size() < m_cfg.minBendingTriggerHits + m_cfg.minBendingPrecisionHits) {
                ATH_MSG_VERBOSE(__func__<<"() Found "<<candidateHits.size()<<" candidate hits, below the minimum required - skip this seed.");
                continue;
            }
            /** Check that the candidate hits extend at least in two layers */
            if (std::ranges::none_of(candidateHits, [this, seedLayer](const TreeNode& c){
                    return m_cfg.idHelperSvc->layerIndex(c.second->identify()) != seedLayer; }) ) {
                ATH_MSG_VERBOSE(__func__<<"() All candidates are in the same station layer, and we need at least two - skip this seed.");
                continue;
            }
            /** Sort the compatible spacepoints by global logical layer */
            std::ranges::sort(candidateHits, [](const TreeNode& c1, const TreeNode& c2){
                const HitPayload& hit1 {c1.second};
                const HitPayload& hit2 {c2.second};
                LayerOrdering ordering {checkLayerOrdering(hit1, hit2)};
                if (ordering == eSameLayer) {
                    /** If one hit is precision and one trigger, put first the trigger one. Relevant for STGCs */
                    if (hit1.isPrecision != hit2.isPrecision) {
                        return hit2.isPrecision;
                    }
                    /** If the two hits are in the same layer, sort them by local y coordinate. */
                    return hit1->localPosition().y() < hit2->localPosition().y();
                }
                return ordering == eLowerLayer;
            });
            if (msgLvl(MSG::VERBOSE)) {
                ATH_MSG_VERBOSE(__func__<<"() Found "<< candidateHits.size()<<" candidate hits: ");
                for (const auto& [coords, hit] : candidateHits) {
                    ATH_MSG_VERBOSE(__func__<<"() \t**"<<*hit<<", coords: "<<coords << ", glob Z/R/phi: "<<hit.Z<<" / "<<hit.R<<" / "<<inDegrees(hit.phi) 
                        << ", st/layer: " << stName(hit.station) << " / "<< hit.layerNum);
                }
            }
            const auto seedItr {std::ranges::find_if(candidateHits,
                [&seed](const TreeNode& c){ return c.second == seed; })};
            assert(seedItr != candidateHits.end());
            /** Initialize a pattern from the seed hit */
            std::vector<PatternState> activePatterns{};
            activePatterns.emplace_back(seed, static_cast<int>(seedCoords[sectorIdx]), seedCoords[thetaIdx]);
            if (visualInfo) {
                activePatterns.back().visualInfo = std::make_unique<PatternHitVisualInfo>(
                    seed.hit, seedCoords[thetaIdx] - thetaWindow, seedCoords[thetaIdx] + thetaWindow);
            }

            const HitPayload* prevCandidate {&seed};
            /** @brief Helper function to process a new candidate hit. During pattern building, we can have 
             *         pattern branching when the initial pattern is compatible with multiple hits in the same 
             *         layer. These patterns will be stored in activePatterns. For each new hit, extendPatterns  
             *         will try to extend every active pattern and remove the ones not meeting continuation criteria. 
             *  @param testPair The new candidate hit to process. */
            auto processNewHit = [&](const TreeNode& testPair){
                const HitPayload& test {testPair.second};
                ATH_MSG_VERBOSE("processNewHit() *** Test "<<*test<<" against " << activePatterns.size() << " active patterns.");
                extendPatterns(activePatterns, test, seed, *prevCandidate, visualInfo);
                prevCandidate = &test;
            };
            /** @brief Helper function ensuring we have at the end one pattern for seed hit. If multiple 
             *         survived at the end, we keep the best. */
            auto ensureOnePattern = [&activePatterns, &visualInfo, this]() {
                if (activePatterns.size() > 1) {
                    ATH_MSG_VERBOSE("ensureOnePattern() Found "<<activePatterns.size()<<" patterns in current search stage - keep the best one.");
                    activePatterns = resolveOverlaps(std::move(activePatterns), visualInfo);
                    resetVisualToOverlap(visualInfo);
                    assert(activePatterns.size() == 1);
                }
            };

            /** First search for compatible hits from the seed layer onwards */
            ATH_MSG_VERBOSE(__func__<<"() Search between " << candidateHits.size() <<" candidates for compatible hits...");
            for (auto it = std::next(seedItr); it != candidateHits.end(); ++it) {
                processNewHit(*it);
            }
            ensureOnePattern();
            /** Then try to proceed toward the innermost layer */
            for (auto it = std::reverse_iterator(seedItr); it != candidateHits.rend(); ++it) {
                processNewHit(*it);
            }
            ensureOnePattern();
            PatternState& newPattern {activePatterns.back()};
            /** Check that the pattern meets the minimum requirements for trigger and precision hits */
            if (newPattern.nBendingTriggerHits < m_cfg.minBendingTriggerHits || 
                newPattern.nPrecisionHits < m_cfg.minBendingPrecisionHits ||
                std::ranges::count_if(newPattern.hitsPerStation, [](const auto& st) { return st.second.size()>= 3; }) < 2) {
                ATH_MSG_VERBOSE(__func__<<"() Pattern " << newPattern << "\ndoes not meet minimum hit requirements - reject.");
                addVisualInfo(newPattern, PatternHitVisualInfo::PatternStatus::eFailed, visualInfo);
                continue;
            }
            newPattern.meanNormResidual2 /= (newPattern.nPrecisionHits + newPattern.nBendingTriggerHits);
            if (newPattern.meanNormResidual2 > m_cfg.meanNormRes2Cut) {
                ATH_MSG_VERBOSE(__func__<<"() Pattern " << newPattern << "\ndoes not meet the mean norm residual2 cut - reject.");
                addVisualInfo(newPattern, PatternHitVisualInfo::PatternStatus::eFailed, visualInfo);
                continue;
            }
            newPattern.isFinalized = true;
            ATH_MSG_VERBOSE(__func__<<"() Add new pattern "<<newPattern);
            patterns.push_back(std::move(newPattern));
        }
    }
    ATH_MSG_VERBOSE("Found in total "<<patterns.size()<<" patterns in eta before overlap removal");
    return resolveOverlaps(std::move(patterns), visualInfo);
}
void GlobalPatternFinder::extendPatterns(PatternStateVec& activePatterns,
                                         const HitPayload& test,
                                         const HitPayload& seed,
                                         const HitPayload& prevCandidate,
                                         PatternHitVisualInfoVec* visualInfo) const {
    // Filter patterns: deduplicate groups sharing the same last hit, but only when the next 
    // test hit is on a different layer — if it's on the same layer we might branch again and need both
    if (activePatterns.size() > 1) {
        // Sort pointers to avoid moving pattern states during deduplication
        std::vector<PatternState*> patPtrs(activePatterns.size());
        std::ranges::transform(activePatterns, patPtrs.begin(), [](PatternState& p){ return &p; });
        // Sort patteres by last inserted hit to find groups of patterns sharing the same last hit
        std::ranges::sort(patPtrs, {}, [](const PatternState* p){ return p->lastInsertedHit; });

        PatternStateVec deduplicated{};
        deduplicated.reserve(activePatterns.size());
        for (auto it = patPtrs.begin(); it != patPtrs.end(); ) {
            // Find the group of patterns sharing the same last hit
            const SpacePoint* groupLastHit {(*it)->lastInsertedHit};
            auto groupEnd = std::ranges::find_if(it, patPtrs.end(), [&](const PatternState* p) {
                return p->lastInsertedHit != groupLastHit;
            });
            if (checkLayerOrdering(test, (*it)->getNthLastHit(1u)) != eSameLayer) {
                // If last hit of the group is on a different layer than the test hit, we deduplicate and pick the best pattern in the group
                deduplicated.push_back(std::move(**std::ranges::max_element(it, groupEnd, 
                    [this](const PatternState* a, const PatternState* b){ return isBetter(*b, *a); })));
            } else {
                // If the last hit of the group is on the same layer as the test hit, we keep all patterns in the group, as we might branch again with the next hits.
                std::ranges::transform(it, groupEnd, std::back_inserter(deduplicated), 
                    [](PatternState* p){ return std::move(*p); });
            }
            it = groupEnd;
        }
        std::swap(activePatterns, deduplicated);
    }
    PatternStateVec nextPatterns{};
    nextPatterns.reserve(activePatterns.size()* 2);

    const unsigned refMissedLayerHits {std::ranges::min_element(activePatterns,
            [](const auto& a, const auto& b) { return a.nMissedLayerHits < b.nMissedLayerHits; }
        )->nMissedLayerHits};
    
    for (PatternState& pattern : activePatterns) {

        if (pattern.nMissedLayerHits >= m_cfg.maxMissedLayerHits && pattern.nMissedLayerHits > refMissedLayerHits) {
            ATH_MSG_VERBOSE(__func__<<"() Pattern " << pattern << " has missed " << pattern.nMissedLayerHits << " hits in different layers, above the maximum allowed - abort this pattern.");
            addVisualInfo(pattern, PatternHitVisualInfo::PatternStatus::eFailed, visualInfo);
            continue;
        }

        /** Check angular compatibility of the test hit and the pattern */
        const auto [result, residual, accWindow] {checkLineCompatibility(seed, test, pattern)};
        switch (result) {
            case CompatibilityResult::eAddHit: {
                /** Add the test hit to the pattern */
                pattern.addHit(test, residual, accWindow);
                ATH_MSG_VERBOSE(__func__<<"() Hit is compatible, add it to the pattern. Updated pattern: " << pattern);
                break;
            }
            case CompatibilityResult::eBranchPattern: {
                /* Check first if the branched pattern already exists*/
                if (std::ranges::any_of(nextPatterns, [&test, &pattern](const PatternState& p) {
                        return p.lastInsertedHit == test.hit && p.prevLayerHit == pattern.prevLayerHit; })) {
                    ATH_MSG_VERBOSE(__func__<<"() Hit is compatible & on same layer of last added hit, but branched pattern already exists");
                    break;
                }
                /** Branch the pattern: we clone it and overwrite the existing hit with the test hit */
                PatternState newPattern {pattern};
                const HitPayload& lastPatHit {pattern.getNthLastHit(1u)};
                newPattern.overWriteHit(lastPatHit, test, residual, accWindow);
                ATH_MSG_VERBOSE(__func__<<"() Hit is compatible & on same layer of last added hit - new branched pattern: " << newPattern);

                /** Update visual information of the original pattern */
                if (newPattern.visualInfo && pattern.visualInfo) {
                    pattern.visualInfo->replacedHits.push_back(test.hit);
                    newPattern.visualInfo->replacedHits.push_back(lastPatHit.hit);
                }
                /** Add the new pattern to the list of next patterns */
                nextPatterns.push_back(std::move(newPattern));
                break;
            }
            case CompatibilityResult::eRejectHit: {
                ATH_MSG_VERBOSE(__func__<<"() Hit is not compatible with the pattern.");
                if (pattern.visualInfo) {
                    pattern.visualInfo->discardedHits.push_back(test.hit);
                }
                if (checkLayerOrdering(test, prevCandidate) != eSameLayer){
                    pattern.nMissedLayerHits++;
                }
                break;
            }
        }
        nextPatterns.push_back(std::move(pattern));
    }
    std::swap(activePatterns, nextPatterns);
};
GlobalPatternFinder::LineCompatibilityResult 
GlobalPatternFinder::checkLineCompatibility(const HitPayload& seed,
                                            const HitPayload& test,
                                            const PatternState& pat) const {
    // We test hits in the same **expanded** sector, so we need just to compare hit's phi with the pattern's phi, if both available
    if (test->measuresPhi() && pat.nPhiHits && 
        std::abs(CxxUtils::deltaPhi(pat.phi, test.phi)) > m_cfg.phiTolerance) {
        ATH_MSG_VERBOSE(__func__<<"() The pattern with phi = "<<inDegrees(pat.phi)
            <<" is not compatible with the test hit with phi "<<inDegrees(test.phi));
        return LineCompatibilityResult{};
    }
    /** Safety check that the test hit is not on the same layer as the seed */
    if (checkLayerOrdering(test, seed) == eSameLayer) {
        ATH_MSG_VERBOSE(__func__<<"() Test hit is on the same layer as the seed - reject.");
        return LineCompatibilityResult{};
    }
    
    /** Here we can retrieve the beamspot if desired */
    const Amg::Vector3D beamSpot{Amg::Vector3D::Zero()};

    /** @brief Helper function to make the result
     *  @param patHitIdx: Pattern hit index to use to compute the pattern line
     *  @param isNewLayer: Flag indicating if the test hit is in a new layer
     *  @return: Line compatibility result */
    auto makeResult = [&](const uint8_t patHitIdx,
                          const bool isNewLayer) -> LineCompatibilityResult {
        LineCompatibilityResult res {computeResidual(seed, test, pat, patHitIdx, beamSpot)};
        if (res.residual < res.accWindow) {
            res.result = isNewLayer ? CompatibilityResult::eAddHit : CompatibilityResult::eBranchPattern;
        }
        return res;
    };
    
    /** Fetch the last inserted hit */
    const HitPayload& lastPatHit {pat.getNthLastHit(1u)};
    const int nMeas {pat.nBendingTriggerHits + pat.nPrecisionHits};

    /*************** Test hit is on a new layer — draw line from seed to lastHit */
    if(checkLayerOrdering(test, lastPatHit) != eSameLayer) {
        return makeResult(1u, /*isNewLayer*/ true);
    }
    /*************** Test hit is on the same layer as the last inserted hit ***************/
    /** Test hit coincides with the last inserted hit */
    if (test == lastPatHit) {
        ATH_MSG_VERBOSE(__func__<<"() Test hit is the same as last inserted hit - rejecting.");
        return LineCompatibilityResult{};
    }
    /** We add the test hit to the pattern if they are consecutive MDT hits */
    if (areConsecutiveMdt(*test, *lastPatHit)) {
        ATH_MSG_VERBOSE(__func__<<"() Consecutive MDT hits on the same layer - accepting.");
        return LineCompatibilityResult{CompatibilityResult::eAddHit, pat.lastResidual, pat.lastAcceptWindow};
    }
    /** sTGCs: if the test hit is a trigger hit (Pad) and the last inserted is precision (strip), we keep the precision hit */
    if (!test.isPrecision && lastPatHit.isPrecision) {
        ATH_MSG_VERBOSE(__func__<<"() Test hit is a trigger hit and last inserted hit is precision on the same layer - keep the precision hit.");
        return LineCompatibilityResult{};
    }
    /** If they are not consecutive MDT hits && we have no inserted hits yet (last hit = seed), we give priority to the seed and reject the test hit */
    if (nMeas == 1) {
        ATH_MSG_VERBOSE(__func__<<"() Test hit on same layer as seed with no prior hits, and they are not consecutive MDT hits - rejecting.");
        return LineCompatibilityResult{};
    }
    /** Find first previously inserted hit on a different layer from test, starting counting from the second-to-last hit */
    uint8_t n {2u};
    while (n < nMeas &&
           checkLayerOrdering(test, pat.getNthLastHit(n)) == eSameLayer) {
        ++n;
    }
    return makeResult(n, /*isNewLayer*/ false);
}
GlobalPatternFinder::LineCompatibilityResult 
GlobalPatternFinder::computeResidual(const HitPayload& seed,
                                     const HitPayload& test,
                                     const PatternState& pat,
                                     const uint8_t patHitIdx,
                                     const Amg::Vector3D& beamSpot) const {
    const HitPayload& patHit {pat.getNthLastHit(patHitIdx)};
    const int nMeas {pat.nBendingTriggerHits + pat.nPrecisionHits};

    ATH_MSG_VERBOSE(__func__<<"() Start residual check given pat hit: " << *patHit << ", st: " << stName(patHit.station));
    // Check whether we have to use the beamspot instead of the last pattern hit to draw the line with the seed.
    const double patDeltaZ {std::abs(patHit.Z - seed.Z)};
    const bool useBeamspot {patHit == seed || patDeltaZ < Acts::s_epsilon || 
        (patHit.station == seed.station && (patHit->msSector()->barrel() ? std::abs(patHit.R - seed.R) : patDeltaZ) <= m_cfg.minLayerSeparation)};
    
    if (useBeamspot && patHitIdx + 1 < nMeas) {
        ATH_MSG_VERBOSE(__func__<<"() Distance seed to pat hit is very small - try with next pat hit as reference.");
        return computeResidual(seed, test, pat, patHitIdx + 1u, beamSpot);
    }

    /* If the pat hit is a straw, find the consecutive (MDT) hits on the same layer and return use middle one
       for next computations — gives a more central reference for the line direction */
    const HitPayload& centralPatHit { patHit->isStraw() ? [&]() -> const HitPayload& {
        uint8_t nSameLayer{1u};
        while (patHitIdx + nSameLayer < nMeas &&
               checkLayerOrdering(patHit, pat.getNthLastHit(patHitIdx + nSameLayer)) == eSameLayer) {
            ++nSameLayer;
        }
        return nSameLayer > 2u ? pat.getNthLastHit(patHitIdx + (nSameLayer - 1u) / 2u) : patHit;
    }() : patHit};

    ATH_MSG_VERBOSE(__func__<<"() Distance seed to pat hit - deltaR_pat= " << (centralPatHit.R - seed.R) << ", deltaZ_pat= " << (centralPatHit.Z - seed.Z) << ". Use beamspot: " << useBeamspot);
    LineCompatibilityResult res {};

    // Determine the coordinates to use as reference for the line computation
    const double refR {useBeamspot ? beamSpot.perp() : centralPatHit.R};
    const double refZ {useBeamspot ? beamSpot.z()    : centralPatHit.Z};

    // Compute the line slope. 
    const double dZ_slope {refZ - seed.Z};
    const double dR_slope {refR - seed.R};
    assert(dZ_slope != 0);
    const double lineSlope {dR_slope / dZ_slope};
    ATH_MSG_VERBOSE(__func__<<"() Slope computation - deltaR_slope= " << dR_slope << ", deltaZ_slope= " << dZ_slope << ", slope= " << lineSlope);

    // Compute the residual
    const double dZ_res {test.Z - seed.Z};
    res.residual = (test.R - seed.R) - lineSlope * dZ_res;
    ATH_MSG_VERBOSE(__func__<<"() Residual computation - deltaR_res: " << (test.R - seed.R) << ", deltaZ_res: " << dZ_res << ", signed residual: " << res.residual);

    /** The dynamic acceptance window is defined using the error propagation law for the residual.
        *  We have two contributions: the line extrapolation distance and the uncertainty on the line slope */
    const double geometricalFactor {1.+ Acts::square(lineSlope)};
    const double alpha {dZ_res / dZ_slope};  /** Alpha parameter determining the propagation/ line extrapolation magnitude */
    const double propagationFactor {1. - alpha + Acts::square(alpha)};
    res.accWindow = m_cfg.baseRWindow * std::sqrt(2*geometricalFactor * propagationFactor);
    if (useBeamspot) res.accWindow *= 1.5; /** Loosen the window when using the beamspot as reference, as the line slope is less well defined */
    ATH_MSG_VERBOSE(__func__<<"() Window computation - Geometrical Factor: " << std::sqrt(geometricalFactor) << " alpha: " << alpha
                    << ", Propagation Factor: " << std::sqrt(propagationFactor) << ", Computed Window: " << res.accWindow);
        
    /** Apply LR correction if the pat hit is a straw */
    if (!useBeamspot && centralPatHit->isStraw()) {
        const double LRcorrection {dZ_res * geometricalFactor * centralPatHit->driftRadius()/ Acts::fastHypot(dZ_slope, dR_slope)};
        res.residual = std::min(std::abs(res.residual-LRcorrection), std::abs(res.residual+LRcorrection));
        ATH_MSG_VERBOSE(__func__<<"() Apply LR correction: " << LRcorrection << ", corrected residual: " << res.residual);
    } else {
        res.residual = std::abs(res.residual);
    }
    if (pat.visualInfo) {
        pat.visualInfo->hitLineInfo[test.hit] = std::make_pair(lineSlope, res.accWindow);
    }
    return res;
}
bool GlobalPatternFinder::isBetter(const PatternState& a, 
                                   const PatternState& b) const {
    /** Function computing the score for a pattern */
    auto score = [this](const PatternState& p) {
        const long nStations {std::ranges::count_if(p.hitsPerStation, [](const auto& pair){ return pair.second.size() >= 4u; })};
        const double etaHitCount  {1.*p.nBendingTriggerHits + m_cfg.precisionWeight * p.nPrecisionHits};
        const double etaHitScore  {std::tanh(etaHitCount / (std::max(nStations, 1L) * m_cfg.hitScoreSaturation))};
        const double stationScore {std::tanh(1.*nStations / 2.)};
        double residualRatio      {p.meanNormResidual2 / (1.5*m_cfg.meanNormRes2Cut)};
        if (!p.isFinalized) residualRatio /= (p.nBendingTriggerHits + p.nPrecisionHits);
        const double resPenalty {m_cfg.residualPenalty * residualRatio  / (1. + residualRatio)};
        const double etaScore {stationScore * etaHitScore * (1. - resPenalty)};
        const double phiScore {std::tanh(1.*p.nPhiHits / m_cfg.phiBonusSaturation)};
        ATH_MSG_VERBOSE("score() stationScore: "<<stationScore<<", etaHitScore: "<<etaHitScore<<", resPenalty: "<<resPenalty<<", etaScore: "<<etaScore<<", phiScore: "<<phiScore);
        return 0.9 * etaScore + 0.1 * phiScore;
    };
    const double scoreA {score(a)};
    const double scoreB {score(b)};
    ATH_MSG_VERBOSE(__func__<<"() Pattern " << a << " with score " << scoreA << " is " <<
        (scoreA > scoreB ? "BETTER" : "WORSE") << " than " << b << " with score " << scoreB);
    return scoreA > scoreB;                              
}
GlobalPatternFinder::PatternStateVec
GlobalPatternFinder::resolveOverlaps(PatternStateVec&& toResolve,
                                     PatternHitVisualInfoVec* visualInfo) const {
    PatternStateVec outputPatterns{};
    outputPatterns.reserve(toResolve.size());
    /** Check if two patterns overlap in space */
    auto areOverlapping = [this](const PatternState& a, const PatternState& b) {
        /** Check first the geometrical overlap */
        if(std::abs(a.sectorCoord - b.sectorCoord) > 1) {
            return false;
        }
        if (a.nPhiHits > 0 && b.nPhiHits > 0) {
            if (std::abs(CxxUtils::deltaPhi(a.phi, b.phi)) > 2.*m_cfg.phiTolerance) return false;
        } else if (a.nPhiHits > 0) {
            if (!sectorMap.insideSector(b.sector1, a.phi) || !sectorMap.insideSector(b.sector2, a.phi)) {
                return false;
            }
        } else if (b.nPhiHits > 0) {
            if (!sectorMap.insideSector(a.sector1, b.phi) || !sectorMap.insideSector(a.sector2, b.phi)) {
                return false;
            }
        }
        if (std::abs(a.theta - b.theta) > 2.*m_cfg.thetaSearchWindow) {
            return false;
        }
        /** If we reach here, the patterns can overlap geometrically, so check the hit content */
        int nSharedHits{0};
        for (const auto& [stA, hitsA] : a.hitsPerStation) {
            auto itB = b.hitsPerStation.find(stA);
            if (itB == b.hitsPerStation.end()) {
                continue;
            }
            nSharedHits += std::ranges::count_if(hitsA, [&](const HitPayload& hitA){
                return std::ranges::find(itB->second, hitA) != itB->second.end();
            });
        }
        /** Overlap if more than half the hits of the smaller pattern are shared */
        const int minHits {std::min(a.nBendingTriggerHits + a.nPrecisionHits, b.nBendingTriggerHits + b.nPrecisionHits)};
        return nSharedHits > minHits / 2;
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
            if (isBetter(*it, *jt)) {
                jt->isOverlap = true;
            } else {
                it->isOverlap = true;
                break;
            }
        }
        if (!it->isOverlap) {
            it->finalizePatternEta();
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
        bool isBarrel{false};
        bool isValid{false};

        double project(const Amg::Vector3D& pos) const {
            const double layCoord   = isBarrel ? pos.perp() : pos.z();
            return refStrip + (layCoord - refLay) * invSlope;
        }
        double residual(const Amg::Vector3D& pos) const {
            const double stripCoord = isBarrel ? pos.z() : pos.perp();
            return std::abs(project(pos) - stripCoord);
        }
    };
    auto makeProjectionModel = [this](const PatternState& pat, const StIndex station) {
        PhiStripProjectionModel result{};
        result.station = station;
        const std::vector<HitPayload>& hits {pat.hitsPerStation.at(station)};
        if (hits.empty()) return result;
        const bool isBarrel {hits.front().hit->msSector()->barrel()};
        // Define the coordinate across the layers, i.e. orthogonal to the strip
        const auto layCoord = [isBarrel](const HitPayload& hit) {
            return (isBarrel) ? hit.R : hit.Z;
        };
        // Define the coordinate along the strip, i.e. orthogonal to the measureent layers
        const auto stripCoord = [isBarrel](const HitPayload& hit) {
            return (isBarrel) ? hit.Z : hit.R;
        };

        const HitPayload* sp1 {nullptr};
        const HitPayload* sp2 {nullptr};
        if (hits.size() > 1) {
            // if we have two eta hits in the station, we use the furthestmost to define the pattern line
            for (const HitPayload& hit : hits) {
                if (!hit->measuresEta()) continue;
                if (!sp1 || layCoord(hit) < layCoord(*sp1)) {
                    sp1 = &hit;
                }
                if (!sp2 || layCoord(hit) > layCoord(*sp2)) {
                    sp2 = &hit;
                }
            }
        } else {
            // if we have only one eta hit, we use it and look for the closest eta hit in the closest station to define the pattern line
            sp1 = &hits.front();
            auto layDistanceFromSp1 = [&layCoord,&sp1](const HitPayload& hit) {
                return std::abs(layCoord(hit) - layCoord(*sp1));
            };
            const std::vector<HitPayload>& closestSt {std::ranges::min_element(pat.hitsPerStation, 
                [&layDistanceFromSp1, &station](const auto& st1, const auto& st2){ 
                    if (st1.first == station) return false;
                    if (st2.first == station) return true;
                    return layDistanceFromSp1(st1.second.front()) < layDistanceFromSp1(st2.second.front()); 
            })->second};
            for (const HitPayload& hit : closestSt) {
                if (!hit->measuresEta()) continue;
                if (!sp2 || layDistanceFromSp1(hit) < layDistanceFromSp1(*sp2)) {
                    sp2 = &hit;
                }
            }
        }
        if (!sp1 || !sp2 ) return result;
        const double deltaLay  {layCoord(*sp2) - layCoord(*sp1)};
        if (std::abs(deltaLay) < m_cfg.minLayerSeparation) return result;

        result.refLay = layCoord(*sp1);
        result.refStrip = stripCoord(*sp1);
        result.invSlope = (stripCoord(*sp2) - stripCoord(*sp1)) / deltaLay;
        result.isBarrel = isBarrel;
        result.isValid = true;
        return result;
    };

    PatternStateVec survivingPatterns{};
    survivingPatterns.reserve(patterns.size());
    for (PatternState& pat : patterns) {
        /** We look for phi-only hits in the buckets associated with the pattern */
        ATH_MSG_VERBOSE(__func__<<"() Search for phi-only hits for pattern: " << pat);
        // Projection model of pattern line onto a given phi strip
        std::optional<PhiStripProjectionModel> patProjOnStrip{};
        for (const auto& [spc, bucketVec] : pat.bucketsPerContainer) {
            for (const SpacePointBucket* bucket : bucketVec) {
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
                    if (!isPhiCompatible(globPhi, pat)) {
                        ATH_MSG_VERBOSE(__func__<<"() Phi-only hit not compatible");
                        continue;
                    }
                    // Check there are not other phi hits in the same layer
                    const uint8_t layNum = m_spSorter.sectorLayerNum(*hit);
                    if (auto it {pat.hitsPerStation.find(station)}; it != pat.hitsPerStation.end()) {
                        if (std::ranges::any_of(it->second, [&](const HitPayload& h){
                                return h->measuresPhi() && hit->msSector() == h->msSector() && layNum == h.layerNum; })) {
                            ATH_MSG_VERBOSE(__func__<<"() The pattern already has a phi hit in the same layer - skip this test hit.");
                            continue;
                        }
                    }
                    // Check eta compatibility. Try first to use the pattern line projection model
                    bool isEtaCompatible {false};
                    const double sigmaEta {std::sqrt(hit->covariance()[covIdxEta])};
                    if (!patProjOnStrip.has_value() || patProjOnStrip->station != station) {
                        patProjOnStrip = makeProjectionModel(pat, station);
                    }
                    if(patProjOnStrip->isValid) {
                        isEtaCompatible = patProjOnStrip->residual(globPosTest) <= 1.1*sigmaEta;
                        ATH_MSG_VERBOSE(__func__<<"() Distance pattern line from strip center: "<<patProjOnStrip->residual(globPosTest)<<", strip half-length: "<<sigmaEta<<", isCompatible: "<<isEtaCompatible);
                    } else {
                        // Check pattern theta against global theta at the boundaries of the phi strip
                        double thetaMin {(globPosTest - sigmaEta * locY).theta()};
                        double thetaMax {(globPosTest + sigmaEta * locY).theta()};
                        if (thetaMax < thetaMin) {
                            std::swap(thetaMin, thetaMax);
                        }
                        isEtaCompatible = std::max(pat.theta - thetaMax, thetaMin - pat.theta) < 0.5 * m_cfg.thetaSearchWindow;
                        ATH_MSG_VERBOSE(__func__<<"() Pattern theta "<<inDegrees(pat.theta)<<", strip theta window: ["<<inDegrees(thetaMin)<<", "<<inDegrees(thetaMax)<<"]");
                        
                    }
                    if (!isEtaCompatible) {
                        ATH_MSG_VERBOSE(__func__<<"() The pattern falls outside the test hit strip in eta - skip this test hit.");
                        continue;
                    }
                    // Create the hit payload and add the hit to the pattern
                    ATH_MSG_VERBOSE(__func__<<"() Phi-only hit compatible - add it to the pattern.");
                    pat.addHit(HitPayload{hit.get(), station, layNum, globPhi}, 0., 0.);
                }
            }
        }
        if (pat.nPhiHits < m_cfg.minPhiHits) {
            ATH_MSG_VERBOSE(__func__<<"() Pattern "<< pat<<" has only "<<pat.nPhiHits<<" phi hits, below the minimum required - reject this pattern.");
            continue;
        }
        pat.finalizePatternPhi();
        survivingPatterns.push_back(std::move(pat));
    }
    std::swap(patterns, survivingPatterns);
}
bool GlobalPatternFinder::isPhiCompatible(const double testPhi,
                                          const PatternState& pattern) const {
    /** We check that the test hit is compatible with the pattern phi, if available, which is given by the first
     *  phi measurement in the pattern. If the pattern doesn't have a phi yet, we check that the test hit is in 
     *  the same pattern sector(s) */
    if (pattern.nPhiHits) {
        if (std::abs(CxxUtils::deltaPhi(pattern.phi, testPhi)) > m_cfg.phiTolerance) {
            ATH_MSG_VERBOSE(__func__<<"() The pattern with phi = "<<inDegrees(pattern.phi)<<" is not compatible with the test hit with phi "<<inDegrees(testPhi));
            return false;
        }
    } else {
        const bool isCompatible {pattern.sector1 == pattern.sector2 ? sectorMap.insideSector(pattern.sector1, testPhi) : 
                sectorMap.insideSector(pattern.sector1, testPhi) && sectorMap.insideSector(pattern.sector2, testPhi)};
        if (!isCompatible) {
            ATH_MSG_VERBOSE(__func__<<"() The test hit with phi = "<<inDegrees(testPhi)<<" is not inside the pattern sectors: "<<pattern.sector1<<" and "<<pattern.sector2);
            return false;
        }            
    }
    return true;
}
GlobalPattern GlobalPatternFinder::convertToPattern(const PatternState& cache) const {
    GlobalPattern::HitCollection hitPerStation{};
    for (const auto& [station, hits] : cache.hitsPerStation) {
        auto& out = hitPerStation[station];
        out.reserve(hits.size());
        std::ranges::transform(hits, std::back_inserter(out), [](const HitPayload& h){ return h.hit;});
    }
    GlobalPattern pattern{std::move(hitPerStation)};
    pattern.setTheta(cache.theta);
    pattern.setPhi(cache.phi);
    // Set the pattern sector(s) and theta.
    pattern.setSector(cache.sector1);
    pattern.setSecondarySector(cache.sector2);
    // Set pattern quality information.
    pattern.setNPrecisionHits(cache.nPrecisionHits);
    pattern.setNEtaNonPrecisionHits(cache.nBendingTriggerHits);
    pattern.setNPhiHits(cache.nPhiHits);
    pattern.setMeanNormResidual2(cache.meanNormResidual2);
    return pattern;
}

GlobalPatternFinder::PatternVec
GlobalPatternFinder::convertToPattern(const PatternStateVec& cache) const {
    PatternVec patterns{};
    patterns.reserve(cache.size());
    std::transform(cache.begin(), cache.end(), std::back_inserter(patterns), [this](const PatternState& cacheEntry) {
        GlobalPattern pattern {convertToPattern(cacheEntry)};
        ATH_MSG_VERBOSE("convertToPattern() Converted new pattern "<<pattern);
        return pattern;
    });
    return patterns;
}

GlobalPatternFinder::SearchTree_t 
GlobalPatternFinder::constructTree(const ActsTrk::GeometryContext& gctx,
                                   const SpacePointContainerVec& spacepoints) const {
    SearchTree_t::vector_t rawData{};
    using SectorProjector = MsTrackSeeder::SectorProjector;
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
        ATH_MSG_VERBOSE("Adding to the search tree "<<spc->size()<<" space point buckets");
        for (const SpacePointBucket* bucket : *spc) {
            ATH_MSG_VERBOSE("Processing " << bucket->size() << " spacepoint...");
            const Amg::Transform3D& localToGlobal {bucket->msSector()->localToGlobalTransform(gctx)};
            const StIndex bucketStation {m_cfg.idHelperSvc->stationIndex(bucket->front()->identify())};
            const int sector = bucket->msSector()->sector();

            for (const auto& hit : *bucket) {
                // Ignore only-phi hits and MDT hits if desired
                if (!hit->measuresEta() || (!m_cfg.useMdtHits && hit->isStraw())) {
                    continue;
                }
                const Amg::Vector3D globalPos {localToGlobal * hit->localPosition()};
                const double globalTheta {globalPos.theta()};
                const HitPayload newHit {hit.get(), bucket, spc, bucketStation,
                    static_cast<uint8_t>(m_spSorter.sectorLayerNum(*hit)),globalPos.perp(), globalPos.z(), globalPos.phi()};
                
                /** Try to duplicate the hit in the neighboring sectors if it is close to the sector border. This ensures 
                 *  that we can find patterns crossing the sector borders. */ 
                for (const auto proj : {SectorProjector::leftOverlap, SectorProjector::center, SectorProjector::rightOverlap}) {
                    /// Check whether the hit belongs to the left or right sector as well
                    const int projSector = MsTrackSeeder::ringSector(sector + Acts::toUnderlying(proj));
                    if (hit->measuresPhi() && proj != SectorProjector::center && 
                        !sectorMap.insideSector(projSector, newHit.phi)) {
                        ATH_MSG_VERBOSE(__func__<<"() Hit @"<< *hit<<"\nwith globPhi "<<inDegrees(newHit.phi)
                            <<" is not in sector "<<projSector<<" ["<<inDegrees(sectorMap.sectorPhi(projSector)-sectorMap.sectorWidth(projSector))
                            <<", "<<inDegrees(sectorMap.sectorPhi(projSector)+sectorMap.sectorWidth(projSector))
                            <<"] which is "<<MsTrackSeeder::to_string(proj) <<" to "<< sector);
                        continue;
                    }
                    std::array<double, 2> coords{};
                    coords[Acts::toUnderlying(SeedCoords::eTheta)] = globalTheta;

                    /** Blow-up the number of sectors by a factor of 2. The even numbers represent the 
                    *  segments expressed @ the sector centre. The odd numbers represent the overlap region
                    *  between two adjacent sectors. For sector 16, the right overlap region is mapped to 1 */
                    coords[Acts::toUnderlying(SeedCoords::eSector)] = MsTrackSeeder::ringOverlap(2*sector + Acts::toUnderlying(proj));
                    ATH_MSG_VERBOSE(__func__<<"() Add hit @"<< *hit 
                                            <<"\nwith global position "<<Amg::toString(globalPos) <<" and coordinates "<<coords<<" to the search tree");
                    rawData.emplace_back(std::move(coords), newHit);
                }  
            }
        }
    }
    ATH_MSG_VERBOSE("Create a new tree with "<<rawData.size()<<" entries. ");
    return SearchTree_t{std::move(rawData)};
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
        if (hit1.layerNum == hit2.layerNum) {
            return eSameLayer;
        } else {
            return getLayerOrdering(hit1.layerNum < hit2.layerNum);
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
        } else if (layer1 == LayerIndex::Inner) {
            /** If both hits are in the inner layer, we use the global R, since in large sector BI comes first, while in small sector EI comes first. */
            return getLayerOrdering(hit1.R < hit2.R);
        } else {
            throw std::runtime_error("Unexpected to have two pattern-compatible hits one in BO and the other in EO.");
        }
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

GlobalPatternFinder::PatternState::PatternState(const HitPayload& seed,
                                                const uint8_t sectorCoord,
                                                const double seedTheta)
        : lastInsertedHit{seed.hit},
          prevLayerHit{seed.hit},
          theta{seedTheta},
          sectorCoord{sectorCoord},
          sector1{static_cast<uint8_t>(MsTrackSeeder::ringSector((sectorCoord - sectorCoord % 2) / 2))},
          sector2{static_cast<uint8_t>(MsTrackSeeder::ringSector((sectorCoord + sectorCoord % 2) / 2))} {
          
    StIndex stationHit {seed.station}; 
    hitsPerStation[stationHit].push_back(seed);
    stations.push_back(stationHit);

    if (seed.isPrecision) nPrecisionHits++;
    else if (seed->measuresEta()) nBendingTriggerHits++;

    if (seed->measuresPhi()) {
        phi = seed.phi;
        nPhiHits++;
    }
}
void GlobalPatternFinder::PatternState::addHit(const HitPayload& hit,
                                               const double residual,
                                               const double acceptWindow) {
    /** Update the pointers to last hit and previous layer hit */
    lastInsertedHit = hit.hit;
    if (const HitPayload& previousHit {getNthLastHit(1u)}; 
        checkLayerOrdering(hit, previousHit) != eSameLayer) {
        prevLayerHit = previousHit.hit;
    }
    /** Add now the new hit */
    StIndex stationHit {hit.station}; 
    hitsPerStation[stationHit].push_back(hit);
    if (stations.empty() || stations.back() != stationHit) {
        stations.push_back(stationHit);
    }
    /** Reset the number of missed layer hits */
    nMissedLayerHits = 0;
    /** Update the hit counts in bending direction */
    if (hit.isPrecision) nPrecisionHits++;
    else if (hit->measuresEta()) nBendingTriggerHits++;
    /** Update the phi of the pattern if there are no phi hits yet */
    if (hit->measuresPhi()) {
        if (nPhiHits == 0) phi = hit.phi;
        nPhiHits++;
    }
    /** Update the residual. Since we can also add phi-only hits without updating the residual, we need to check the accept window */
    if (acceptWindow > Acts::s_epsilon) {
        meanNormResidual2 += Acts::square(residual / acceptWindow);
        lastAcceptWindow = acceptWindow;
        lastResidual = residual;
    }
}
void GlobalPatternFinder::PatternState::overWriteHit(const HitPayload& oldHit,
                                                     const HitPayload& newHit,
                                                     const double newResidual,
                                                     const double newAcceptWindow) {
    if (oldHit.station != newHit.station) {
        throw std::runtime_error(std::format("Trying to overwrite a hit in station {} with another one from station {}", stName(oldHit.station), stName(newHit.station)));
    }
    // We expect to overwrite hits of the same type (precision/trigger), since we only branch when we have
    // compatible hits in the same layer, except for sTGC hits, where we have pad and strips in the same layer
    if ((oldHit.isPrecision != newHit.isPrecision || 
        oldHit->measuresEta() != newHit->measuresEta()) && newHit->type() != xAOD::UncalibMeasType::sTgcStripType) {
        std::stringstream ss {"Trying to overwrite a hit with incompatible type\n"};
        ss << "Old hit: " << *oldHit << ", isPrecision: " << oldHit.isPrecision << ", measuresEta: " << oldHit->measuresEta() << "\n";
        ss << "New hit: " << *newHit << ", isPrecision: " << newHit.isPrecision << ", measuresEta: " << newHit->measuresEta();
        throw std::runtime_error(ss.str());
    }
    assert(newAcceptWindow > Acts::s_epsilon && lastAcceptWindow > Acts::s_epsilon);
    StIndex lastStation {oldHit.station};
    auto it {hitsPerStation.find(lastStation)};
    if (it == hitsPerStation.end() || lastStation != stations.back()) {
        throw std::runtime_error("Trying to remove a hit from an invalid station");
    }
    std::vector<HitPayload>& hitsInStation {it->second};
    // Remove ALL hits in the same layer
    while (!hitsInStation.empty()) {
        HitPayload& lastHit {hitsInStation.back()};
        if (checkLayerOrdering(lastHit, oldHit) != eSameLayer) {
            break;
        }
        if (lastHit.isPrecision) nPrecisionHits--;
        else nBendingTriggerHits--; //This method is called only in pattern building in eta, so we don't have phi-only hits yet
        if (lastHit->measuresPhi()) nPhiHits--;
        hitsInStation.pop_back();
        /** MDT hits in the same layer are added with the same residual */
        meanNormResidual2 -= Acts::square(lastResidual / lastAcceptWindow);
    }
    // then insert the new hit
    hitsInStation.push_back(newHit);
    if (newHit.isPrecision) nPrecisionHits++;
    else nBendingTriggerHits++; //This method is called only in pattern building in eta, so we don't have phi-only hits yet
    /** Update the phi count and the pattern phi if needed*/
    if (newHit->measuresPhi()) {
        if (nPhiHits == 0) phi = newHit.phi;
        nPhiHits++;
    }
    /** Update the residual. When we replace a hit, we have always the new residual & accept window, so no checks are needed. */
    meanNormResidual2 += Acts::square(newResidual / newAcceptWindow);
    lastResidual = newResidual;
    lastAcceptWindow = newAcceptWindow;
    /** Update the pointers to last hit. Since we overwrite hits in the same layer, prevLayerHit remains unchanged */
    lastInsertedHit = newHit.hit;
}
void GlobalPatternFinder::PatternState::finalizePatternEta() {
    /** This method is called at the end of pattern building in eta, so we don't have phi-only hits yet */
    theta = 0.; 
    for (const auto& [_, hits] : hitsPerStation) {
        for (const auto& hit : hits) {
            /** Update the bucket map */
            auto& bucketVec {bucketsPerContainer[hit.container]};
            if (std::ranges::find(bucketVec, hit.bucket) == bucketVec.end()){
                bucketVec.push_back(hit.bucket);
            }
            /** Update eta */
            theta += atan2(hit.R, hit.Z);
        }
    }
    assert(nPrecisionHits + nBendingTriggerHits > Acts::s_epsilon);
    theta /= (nPrecisionHits + nBendingTriggerHits);
}
void GlobalPatternFinder::PatternState::finalizePatternPhi() {
    if (!nPhiHits) {
        /** If there are no phi hits, we just use the central phi of the sector/overlap region */
        phi = sectorMap.sectorOverlapPhi(sector1, sector2);
        return;
    }
    double deltaPhiAcc {0.};
    std::optional<double> centralPhi {};
    for (const auto& [_, hits] : hitsPerStation) {
        for (const auto& hit : hits) {
            if (!hit->measuresPhi()) {
                continue;
            }
            if (!centralPhi) {
                centralPhi = hit.phi;
            }
            deltaPhiAcc += CxxUtils::deltaPhi(hit.phi, *centralPhi);
        }
    }
    phi = CxxUtils::wrapToPi(centralPhi.value_or(0.) + deltaPhiAcc / nPhiHits);
}
void GlobalPatternFinder::addVisualInfo(const PatternState& cache,
                                        PatternHitVisualInfo::PatternStatus status,
                                        PatternHitVisualInfoVec* visualInfo) const {
    if (!visualInfo) {
        return;
    }
    GlobalPattern pattern {convertToPattern(cache)};
    // Check whether the visual info about this pattern is already in the container
    if (auto it =std::ranges::find_if(*visualInfo, [&pattern](const auto& v){ 
        return v.patternCopy && *v.patternCopy == pattern; }); it != visualInfo->end()) {
        it->status = status; // Update the status if the pattern is already in the container
        return;
    }
    visualInfo->push_back(*cache.visualInfo);
    auto& bucketVec {visualInfo->back().parentBuckets};
    for (const auto& [_, buckets] : cache.bucketsPerContainer) {
        bucketVec.insert(bucketVec.end(), buckets.begin(), buckets.end());
    }
    visualInfo->back().patternCopy = std::make_unique<GlobalPattern>(std::move(pattern));
    visualInfo->back().status = status;
}
const GlobalPatternFinder::HitPayload&
GlobalPatternFinder::PatternState::getNthLastHit(const uint8_t n) const {
    if (n >= nPrecisionHits + nBendingTriggerHits) {
        // If we have not yet inserted n hits, we return the seed hit
        return hitsPerStation.at(stations.front()).front();
    }
    std::size_t remaining {n};
    for (auto stIt = stations.rbegin(); stIt != stations.rend(); ++stIt) {
        const std::vector<HitPayload>& hits {hitsPerStation.at(*stIt)};
        const size_t nHits  {hits.size()};
        if (remaining <= nHits) {
            return hits.at(nHits - remaining);
        }
        remaining -= nHits;
    }
    // Fall-back return, should not happen if the input n is consistent with hit counts
    return hitsPerStation.at(stations.front()).front();
}
bool GlobalPatternFinder::PatternState::isInPattern(const HitPayload& hit) const {
    const auto seedStationIt {hitsPerStation.find(hit.station)};
    return seedStationIt != hitsPerStation.end() && 
           std::ranges::find(seedStationIt->second, hit) != seedStationIt->second.end();                                               
}
GlobalPatternFinder::HitPayload::HitPayload(const SpacePoint* hit, 
                                            const SpacePointBucket* bucket,
                                            const SpacePointContainer* container,
                                            StIndex station,
                                            uint8_t layerNum,
                                            double R, 
                                            double Z,
                                            double phi) 
    : hit{hit}, bucket{bucket}, container{container}, 
      R{R}, Z{Z}, phi{phi}, station{station}, layerNum{layerNum} {}
GlobalPatternFinder::HitPayload::HitPayload(const SpacePoint* hit, 
                                            StIndex station,
                                            uint8_t layerNum,
                                            double phi) 
    : hit{hit}, phi{phi}, station{station}, layerNum{layerNum} {}
bool GlobalPatternFinder::HitPayload::operator==(const HitPayload& other) const {
    return hit == other.hit;
}
void GlobalPatternFinder::PatternState::print(std::ostream& ostr) const {
    ostr<<"Pattern state, Expanded Sector: "<<sectorCoord<<", Theta: "<<theta << ", Phi: "<<phi;
    ostr<<", nPrecisionHits: "<<nPrecisionHits<<", nEtaNonPrecisionHits: "<<nBendingTriggerHits<<", nPhiHits: "<<nPhiHits;
    ostr<<", mean normalized residual squared: "<<meanNormResidual2;
    ostr<<", Hit per station: \n";
    for (const auto& [station,hits] : hitsPerStation) {
        ostr<<"  Station "<<Muon::MuonStationIndex::stName(station)<<" has "<<hits.size()<<" hits\n";
        for (const auto& hit : hits) {
            ostr<<"    "<<*hit<<", R: "<<hit.R<<", Phi:"<<hit.phi<<"\n";
        }
    }
}
}