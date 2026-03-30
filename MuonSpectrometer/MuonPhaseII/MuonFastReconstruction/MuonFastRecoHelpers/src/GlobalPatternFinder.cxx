/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonFastRecoHelpers/GlobalPatternFinder.h"

#include "MuonSpacePoint/SpacePointHelpers.h"
#include "xAODMuonPrepData/MdtDriftCircle.h"
#include "MuonDetDescrUtils/MuonSectorMapping.h"
#include "MuonTrackFindingTools/MsTrackSeeder.h"

#include "Acts/Utilities/VectorHelpers.hpp"
#include "Acts/Utilities/Helpers.hpp"

#include "CxxUtils/phihelper.h"

namespace {
    static const Muon::MuonSectorMapping sectorMap{};

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
    /** @brief Helper function to incrementally update a mean 
     *  @param currentMean: The current mean value
     *  @param nPrev: The number of previous values
     *  @param newValue: The new value to be added
     *  @param add: Whether to add or remove the new value
     */
    void updateMean(double& currentMean, const unsigned nPrev, const double newValue, const bool add = true) {
        if (add) {
            currentMean += (newValue - currentMean) / (nPrev + 1);
        } else {
            if (nPrev <= 1) {
                currentMean = 0.;
                return;
            }
            currentMean -= (newValue - currentMean) / (nPrev - 1);
        }
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
                return v.status == PatternHitVisualInfo::PatternStatus::eSuccessful;});
        
        visualInfo->erase(first,last);
    }
}

namespace MuonR4::FastReco {
GlobalPatternFinder::GlobalPatternFinder(const std::string& name, Config&& config) :
    AthMessaging{name},
    m_cfg{config} {
};


GlobalPatternFinder::PatternVec 
GlobalPatternFinder::findPatterns(const ActsTrk::GeometryContext& gctx,
                                  const SpacePointContainerVec& spacepoints,
                                  BucketPerContainer& outBuckets) const {
    /** Create the search tree by ordering hits in theta and **expanded** spectrometer sector */                                
    SearchTree_t orderedSpacepoints {constructTree(gctx, spacepoints)};
    /** Find patterns in eta first */
    auto visualInfo {m_cfg.visionTool ? std::make_unique<PatternHitVisualInfoVec>() : nullptr};
    PatternStateVec patterns{findPatternsInEta(orderedSpacepoints, visualInfo.get())};

    // TO DO search for phi hits snd attach them to (compatible) patterns
    
    /** Fill the output buckets */
    for (const PatternState& pat : patterns) {
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
        m_cfg.visionTool->plotPatternBuckets(Gaudi::Hive::currentContext(), "GlobPatternFinder", std::move(*visualInfo));
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
        ATH_MSG_VERBOSE("Start pattern search in eta with seeding layer "<< layerName(seedingLayer));
        /** We try to build a pattern in eta starting from every hit in the three */
        for (const auto& [seedCoords, seed] : orderedSpacepoints) {
            const SpacePoint* seedHit {seed.hit};
            /** Check the seed is in the current seeding layer, measures eta and if seeding from MDT hits is enabled  */
            const LayerIndex seedLayer {toLayerIndex(seed.station)};
            if (seedLayer != seedingLayer || !seedHit->measuresEta() || (seedHit->isStraw() && !m_cfg.seedFromMdt)) {
                continue;
            }
            /** check how many existing (and overlapping) patterns contain this hit */
            auto nExistingPatterns {std::ranges::count_if(patterns, [&seed, &seedCoords, this](const PatternState& pattern){
                if (std::abs(pattern.theta - seedCoords[thetaIdx]) > m_cfg.thetaSearchWindow ||
                    pattern.sectorCoord != static_cast<int>(seedCoords[sectorIdx])) {
                    return false;
                }
                return pattern.isInPattern(seed);
            })};
            if (nExistingPatterns >= m_cfg.maxSeedAttempts) {
                ATH_MSG_DEBUG("The seed hit "<<*seedHit<<"\n coordinates "<<seedCoords<<" has already been used to build "<<nExistingPatterns<<" patterns, which is above the maximum number of attempts allowed to build a pattern from hits already used in existing patterns. Do not use it as seed for a new pattern.");
                continue;
            }
            /** Define the search range. */    
            SearchTree_t::range_t selectRange{};
            /** Allowing just one value  for the **expanded** sector range, we also include patterns extending in the overlap region of neighbouring **physical** sectors */
            selectRange[sectorIdx].shrink(seedCoords[sectorIdx] - 0.1, seedCoords[sectorIdx] + 0.1);
            /** Pattern search is performed within a theta window of size m_thetaSearchWindow. This window corresponds to the maximum allowed theta spread for the target pT. */ 
            const double thetaWindow {(seedLayer == LayerIndex::Inner || seedLayer == LayerIndex::Outer) 
                                    ? m_cfg.thetaSearchWindow : 0.5*m_cfg.thetaSearchWindow};
            selectRange[thetaIdx].shrink(seedCoords[thetaIdx] - thetaWindow, seedCoords[thetaIdx] + thetaWindow);
            /** Search for compatible spacepoints with the seed and check if there are enough to build a pattern */
            auto candidateHits = orderedSpacepoints.rangeSearchWithKey(selectRange);
            if (candidateHits.size() < m_cfg.minBendingTriggerHits + m_cfg.minBendingPrecisionHits) {
                ATH_MSG_VERBOSE("Found "<<candidateHits.size()<<" candidate hits for seed hit "<<*seedHit<<", coordinates "<<seedCoords
                                <<". This is below the minimum number of hits required to build a pattern. Do not create a pattern.");
                continue;
            }
            /** Check that the candidate hits extend at least in two layers */
            if (std::ranges::find_if(candidateHits, [this, seedLayer](const auto& c){
                    return m_cfg.idHelperSvc->layerIndex(c.second.hit->identify()) != seedLayer;
                    }) == candidateHits.end()) {
                ATH_MSG_VERBOSE("All candidate hits for seed hit "<<*seedHit<<", coordinates "<<seedCoords
                                <<" are in the same layer. We need hits in at least two layers to build a pattern. Do not create a pattern.");
                continue;
            }
            /** Sort the compatible spacepoints by global logical layer */
            std::ranges::sort(candidateHits, [this](const auto& c1, const auto& c2){
                const HitPayload& hit1 {c1.second};
                const HitPayload& hit2 {c2.second};
                LayerOrdering ordering {checkLayerOrdering(hit1, hit2)};
                if (ordering == LayerOrdering::eSameLayer) {
                    /** If the two hits are in the same layer, sort them by local y coordinate. */
                    return hit1.hit->localPosition().y() < hit2.hit->localPosition().y();
                }
                return ordering == LayerOrdering::eLowerLayer;
            });
            const auto seedItr {std::ranges::find_if(candidateHits,
                [&seed](const auto& c){ return c.second == seed; })};
            assert(seedItr != candidateHits.end());
            /** Initialize a pattern from the seed hit*/
            std::vector<PatternState> activePatterns{};
            activePatterns.emplace_back(seed, static_cast<int>(seedCoords[sectorIdx]), seedCoords[thetaIdx]);
            if (visualInfo) {
                activePatterns.back().visualInfo = std::make_unique<PatternHitVisualInfo>(seedHit, seedCoords[thetaIdx] - thetaWindow, seedCoords[thetaIdx] + thetaWindow);
            }

            const HitPayload* prevCandidate {&seed};
            /** Helper function to process a new candidate hit. During pattern building, we can have pattern branching when the initial pattern is compatible 
                with multiple hits in the same layer. These patters will be stored in activePatterns. For each new hit, extendPatterns will try to extend every 
                active pattern and remove the ones not meeting continuation criteria. */
            auto processNewHit = [&](const TreeNode& testPair){
                const HitPayload& test {testPair.second};
                /** Only-phi measurement will be treated later */
                if (!test.hit->measuresEta()) {
                    return;
                }
                ATH_MSG_VERBOSE("** Check compatibility of hit "<<*test.hit<<", coordinates "<<testPair.first << ". We start from " << activePatterns.size() << " active patterns.");
                extendPatterns(activePatterns, test, seed, *prevCandidate, visualInfo);
                prevCandidate = &test;
            };
            /** Helper funtion insuring we have at the end one pattern for seed hit. If multiple survived at the end, we keep the best. */
            auto ensureOnePattern = [&activePatterns, &visualInfo, this]() {
                if (activePatterns.size() > 1) {
                    ATH_MSG_VERBOSE("Found "<<activePatterns.size()<<" patterns during the search. Remove overlaps.");
                    activePatterns = resolveOverlaps(std::move(activePatterns), visualInfo);
                    resetVisualToOverlap(visualInfo);
                    assert(activePatterns.size() == 1);
                }
            };
            activePatterns.back().nInsertedHits = 0;
            /** First search for compatible hits from the seed layer onwards */
            ATH_MSG_VERBOSE("Search between " << candidateHits.size() <<" candidates for compatible hits to "<<*seedHit<<", coordinates: "<<seedCoords);
            for (auto it = std::next(seedItr); it != candidateHits.end(); ++it) {
                processNewHit(*it);
            }
            ensureOnePattern();
            activePatterns.back().nInsertedHits = 0;
            /** Then try to proceed toward the innermost layer */
            for (auto it = std::reverse_iterator(seedItr); it != candidateHits.rend(); ++it) {
                processNewHit(*it);
            }
            ensureOnePattern();
            PatternState newPattern {std::move(activePatterns.back())};
            /** Check that the pattern meets the minimum requirements for trigger and precision hits */
            if (newPattern.nBendingTriggerHits < m_cfg.minBendingTriggerHits || newPattern.nPrecisionHits < m_cfg.minBendingPrecisionHits) {
                ATH_MSG_VERBOSE("Pattern " << newPattern << " does not meet minimum hit requirements, n bending precision/trigger hits = " << newPattern.nPrecisionHits << "/" << newPattern.nBendingTriggerHits << ". Do not create a pattern.");
                addVisualInfo(newPattern, PatternHitVisualInfo::PatternStatus::eFailed, visualInfo);
                continue;
            }
            newPattern.finalizePattern();
            ATH_MSG_VERBOSE("Add new pattern "<<newPattern);
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
    PatternStateVec nextPatterns{};
    nextPatterns.reserve(activePatterns.size()* 2);

    const unsigned refMissedLayerHits {std::ranges::min_element(activePatterns,
            [](const auto& a, const auto& b) { return a.nMissedLayerHits < b.nMissedLayerHits; }
        )->nMissedLayerHits};
    
    for (PatternState& pattern : activePatterns) {

        if (pattern.nMissedLayerHits >= m_cfg.maxMissedLayerHits && (!nextPatterns.empty() || pattern.nMissedLayerHits > refMissedLayerHits)) {
            ATH_MSG_VERBOSE("Pattern " << pattern << " has already missed " << pattern.nMissedLayerHits << " hits in different layers, which is above the maximum allowed. Do not try to extend it with new hits.");
            addVisualInfo(pattern, PatternHitVisualInfo::PatternStatus::eFailed, visualInfo);
            continue;
        }

        /** Check angular compatibility of the test hit and the pattern */
        LineCompatibilityResult compResult {checkLineCompatibility(seed, test, pattern)};
        switch (compResult.result) {
            case CompatibilityResult::eAddHit: {
                /** Add the test hit to the pattern */
                pattern.addHit(test, compResult.residual, compResult.acceptanceWindow);
                ATH_MSG_VERBOSE("Hit is compatible, add it to the pattern. Updated pattern: " << pattern);
                break;
            }
            case CompatibilityResult::eBranchPattern: {
                /** We branch only once, otherwise we would create duplicate patterns */
                auto isNew = std::ranges::find_if(nextPatterns, [&test](const PatternState& p) {
                        const HitPayload& lastHit {p.getNthLastHit(1u)};
                        return lastHit == test;
                    }) == nextPatterns.end();
                if (!isNew) {
                    ATH_MSG_VERBOSE("Hit is compatible & in the same layer of the last added hit. The branched pattern is already in the list of next patterns. Do not add it again.");
                    continue;
                }
                /** Branch the pattern: we clone it and overwrite the existing hit with the test hit. Watch out, from now on lastPatHit will be the test hit */
                PatternState newPattern {pattern};
                const HitPayload& lastPatHit {pattern.getNthLastHit(1u)};
                newPattern.overWriteHit(lastPatHit, test, compResult.residual, compResult.acceptanceWindow);
                ATH_MSG_VERBOSE("Hit is compatible & in the same layer of the last added hit. Branch the pattern, new pattern: " << newPattern);
                /** Update visual information of the original pattern */
                if (pattern.visualInfo) {
                    pattern.visualInfo->replacedHits.push_back(test.hit);
                    newPattern.visualInfo->replacedHits.push_back(lastPatHit.hit);
                }
                /** Add the new pattern to the list of next patterns */
                nextPatterns.push_back(std::move(newPattern));
                break;
            }
            case CompatibilityResult::eRejectHit: {
                ATH_MSG_VERBOSE("Hit is not compatible with the pattern.");
                if (pattern.visualInfo) {
                    pattern.visualInfo->discardedHits.push_back(test.hit);
                }
                if (checkLayerOrdering(test, prevCandidate) != LayerOrdering::eSameLayer){
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
                                            const PatternState& pattern) const {
    if (!isPhiCompatible(test, seed, pattern)) {
        return LineCompatibilityResult{CompatibilityResult::eRejectHit, 0., 0.};
    }
    /** Here we can retrieve the beamspot if desired */
    const Amg::Vector3D beamSpot{Amg::Vector3D::Zero()};

    /** Helper function to get whether the test hit is within the acceptance window of the last pattern hit */
    auto checkResidualInWindow = [&](const HitPayload& lastPatHit,
                                               const bool isUnique) {
        // Check whether we have to use the beamspot instead of the last pattern hit to draw the line with the seed.
        const bool useSeed2Beamspot {lastPatHit.hit == seed.hit || 
                                     std::abs( lastPatHit.Z - seed.Z) <= m_cfg.minZDiff4Line ||
                                     std::abs( lastPatHit.R - seed.R) <= m_cfg.minRDiff4Line};
        const double lineSlope {computeLineSlope(lastPatHit, seed, useSeed2Beamspot, beamSpot)};
        const double testResidual {computeResidual(test, seed, lineSlope)};
        const double acceptanceWindow {computeAcceptanceWindow(test, seed, lastPatHit, lineSlope, useSeed2Beamspot, beamSpot)};
        ATH_MSG_VERBOSE("Check residual in window... Residual: " << testResidual << ", Acceptance Window: " << acceptanceWindow);
        if (pattern.visualInfo) {
            pattern.visualInfo->hitLineInfo[test.hit] = std::make_pair(lineSlope, acceptanceWindow);
        }
        if (testResidual < acceptanceWindow) {
            return LineCompatibilityResult{isUnique ? CompatibilityResult::eAddHit : CompatibilityResult::eBranchPattern, testResidual, acceptanceWindow};
        }
        return LineCompatibilityResult{CompatibilityResult::eRejectHit, 0., 0.};
    };

    /** Fetch the last filled station and the last inserted hit */
    const HitPayload& lastPatHit {pattern.getNthLastHit(1u)};
    ATH_MSG_VERBOSE("Last pat hit: " << *lastPatHit.hit << ", station: " << Muon::MuonStationIndex::stName(lastPatHit.station));
    /** Check the layer ordering between the last inserted hit and the test hit */
    const LayerOrdering orderingTest2Last {checkLayerOrdering(test, lastPatHit)};

    /** Check whether the test hit is in the same layer as the last hit in the pattern */
    if(orderingTest2Last == LayerOrdering::eSameLayer) {
        /** Check that the hits are not the same */
        if (test == lastPatHit) {
            ATH_MSG_VERBOSE("The test hit is the same as the last inserted hit. Do not add it to the pattern.");
            return LineCompatibilityResult{CompatibilityResult::eRejectHit, 0., 0.};
        }
        /** We add the test hit to the pattern if they are consecutive MDT hits */
        if (areConsecutiveMdt(*test.hit, *lastPatHit.hit)) {
            ATH_MSG_VERBOSE("The test hit is in the same layer of the last inserted one. They are consecutive MDT hits, so they are compatible.");
            return LineCompatibilityResult{CompatibilityResult::eAddHit, 0., 0.};
        }
        /** If they are not consecutive MDT hits && we have no inserted hits yet (last hit = seed), we give priority to the seed and reject the test hit */
        if (pattern.nInsertedHits == 0) {
            ATH_MSG_VERBOSE("The test hit is in the same layer of the seed. They are not consecutive MDT hits, so they are not compatible.");
            return LineCompatibilityResult{CompatibilityResult::eRejectHit, 0., 0.};
        }
        /** If they are not consecutive MDT hits && we have inserted hits, we We draw a line using the seed and last hit being on a different layer 
         *  from the second-to-last inserted hit, and we check that the test hit residual is within the acceptance window. If we have no at least 2 
         *  inserted hits yet, we compute the line using the seed and the beamspot. */
        std::size_t n {2};
        while (n < pattern.nInsertedHits) {
            const HitPayload& nthHit {pattern.getNthLastHit(n)};
            if (checkLayerOrdering(test, nthHit) != LayerOrdering::eSameLayer) {
                return checkResidualInWindow(nthHit, false);
            }
            ++n;
        }
        /** Fallback when we have all the previously inserted hits on the same layer */
        return checkResidualInWindow(pattern.getNthLastHit(n), false);
    }
    /** The new hit is in a new layer. We draw a line using the last inserted hit and the seed, and we check that the test hit residual is within the acceptance 
     *  window. If we have no inserted hits yet, we compute the line using the seed and the beamspot. */
    return checkResidualInWindow(lastPatHit, true);
}
bool GlobalPatternFinder::isPhiCompatible(const HitPayload& test,
                                          const HitPayload& seed, 
                                          const PatternState& pattern) const {
    const SpacePoint* testHit {test.hit};
    const SpacePoint* seedHit {seed.hit};
    /**  If the test hits does not measure phi but the pattern has already a phi, we check that the pattern is within the test hit sector. */
    if (!testHit->measuresPhi()) {
        if (pattern.nPhiHits && !sectorMap.insideSector(testHit->msSector()->sector(), pattern.phi)) {
            ATH_MSG_VERBOSE("The pattern with phi = "<<inDegrees(pattern.phi)<<" is not inside the sector of the test hit: "<<testHit->msSector()->sector());
            return false;
        }
    /** If the test hit measures phi we check that it's compatible with the pattern phi, if avalilable, which is the average of the phi values of 
     *  accepted hits of the pattern. If the pattern doesn't have a defined phi yet, we check that the test hit is in the same sector as the seed hit */
    } else {
        if (pattern.nPhiHits) {
            if (std::abs(CxxUtils::deltaPhi(pattern.phi, test.phi)) > m_cfg.phiTolerance) {
                ATH_MSG_VERBOSE("The pattern with phi = "<<inDegrees(pattern.phi)<<" is not compatible with the test hit with phi "<<inDegrees(test.phi));
                return false;
            }
        } else if (!sectorMap.insideSector(seedHit->msSector()->sector(), test.phi)) {
            ATH_MSG_VERBOSE("The test hit with phi = "<<inDegrees(test.phi)<<" is not inside the sector of the seed hit: "<<seedHit->msSector()->sector());
            return false;
        }
    }
    return true;
}

double GlobalPatternFinder::computeLineSlope(const HitPayload& lastPatHit,
                                                const HitPayload& seed,
                                                const bool useSeed2Beamspot,
                                                const Amg::Vector3D& beamSpot) const {
    /** Safe-guard check: if we are inserting the first hit or if the last pattern hit and the seed are too close in Z, we use the beamspot */
    const double deltaR {(useSeed2Beamspot ? beamSpot.perp() - seed.R : lastPatHit.R - seed.R) };
    const double deltaZ {(useSeed2Beamspot ? beamSpot.z() - seed.Z : lastPatHit.Z - seed.Z)};
    ATH_MSG_VERBOSE("Computing line seed-to-lastPatHit... deltaR = " << lastPatHit.R - seed.R << ", deltaZ = " << lastPatHit.Z - seed.Z 
                    << ", we use" << (useSeed2Beamspot ? " beamSpot" : " lastPatHit") << " giving " << std::format("deltaR_slope: {}, deltaZ_slope: {}", deltaR, deltaZ)
                    << ". The line slope is: " << deltaR / deltaZ);
    return deltaR / deltaZ;
}

double GlobalPatternFinder::computeResidual(const HitPayload& testHit,
                                               const HitPayload& seed,
                                               const double lineSlope) const {
    const double signedResidual { testHit.R - seed.R - lineSlope * (testHit.Z - seed.Z)};
    ATH_MSG_VERBOSE("Computing residual... slope: " << lineSlope << ", deltaR_residual: " << (testHit.R - seed.R) << ", deltaZ_residual: " << (testHit.Z - seed.Z) << ", signed residual: " << signedResidual);
    return std::abs(signedResidual);
}

double GlobalPatternFinder::computeAcceptanceWindow(const HitPayload& testHit,
                                                       const HitPayload& seed,
                                                       const HitPayload& lastPatHit,
                                                       const double lineSlope,
                                                       const bool useSeed2Beamspot,
                                                       const Amg::Vector3D& beamSpot) const {
    /** The acceptance window is defined according to the error propagation law for the residual.
     *  We have a contribution given by the geometrical amplification of the measurement error and
     *  a contribution from the uncertainty in the line slope */
    const double geometricalFactor {1.+ Acts::square(lineSlope)};
    /** Reference z: last hit in the pattern or beamspot */
    const double& refZ {useSeed2Beamspot ? beamSpot.z() : lastPatHit.Z};
    /** Alpha parameter determining the propagation/ line extrapolation magnitude */
    const double alpha {(testHit.Z-seed.Z) / (refZ - seed.Z)};
    const double propagationFactor {1. - alpha + Acts::square(alpha)};
    ATH_MSG_VERBOSE("Computing the acceptance window... Geometrical Factor: " << std::sqrt(geometricalFactor) << " alpha: " << alpha
                    << ", Propagation Factor: " << std::sqrt(propagationFactor) << ", Computed Window: " << m_cfg.baseRWindow * std::sqrt(geometricalFactor * propagationFactor));
    return m_cfg.baseRWindow * std::sqrt(2*geometricalFactor * propagationFactor);
}

GlobalPatternFinder::PatternStateVec
GlobalPatternFinder::resolveOverlaps(PatternStateVec&& toResolve,
                                        PatternHitVisualInfoVec* visualInfo) const {
    PatternStateVec outputPatterns{};
    outputPatterns.reserve(toResolve.size());
    /** Check if two patterns overlap in space */
    auto areOverlapping = [this](const PatternState& a, const PatternState& b) {
        const bool isSectorOverlap {a.sectorCoord == b.sectorCoord || 
            (std::abs(a.sectorCoord - b.sectorCoord) == 1 && std::abs(a.phi - b.phi) <= m_cfg.phiTolerance)};
        return isSectorOverlap && std::abs(a.theta - b.theta) <= m_cfg.thetaSearchWindow;
    };
    /** Determine if one pattern is better than another */
    auto isBetter = [](const PatternState& a, const PatternState& b) {
        if (a.nBendingTriggerHits != b.nBendingTriggerHits) {
            return a.nBendingTriggerHits > b.nBendingTriggerHits;
        }
        if (a.nPrecisionHits != b.nPrecisionHits) {
            return a.nPrecisionHits > b.nPrecisionHits;
        }
        return a.totalResidual < b.totalResidual;
    };
    for (auto it = toResolve.begin(); it != toResolve.end(); ++it) {
        if (it->isOverlap) {
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
        addVisualInfo(*it, it->isOverlap ? PatternHitVisualInfo::PatternStatus::eOverlap 
                                                        : PatternHitVisualInfo::PatternStatus::eSuccessful, visualInfo);
        if (!it->isOverlap) {
            outputPatterns.push_back( std::move(*it));
        }
    }
    ATH_MSG_VERBOSE("Patterns surviving overlap removal: "<< outputPatterns.size());
    return outputPatterns;
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
    const int& secCoord = cache.sectorCoord;
    pattern.setSector((secCoord - secCoord % 2) / 2);
    pattern.setSecondarySector((secCoord + secCoord % 2) / 2);
    // Set pattern quality information.
    pattern.setNPrecisionHits(cache.nPrecisionHits);
    pattern.setNEtaNonPrecisionHits(cache.nBendingTriggerHits);
    pattern.setNPhiHits(cache.nPhiHits);
    pattern.setTotalResidual(cache.totalResidual);
    pattern.setTotalNormalizedResidual(cache.totalRes2AcceptWindow);
    return pattern;
}

GlobalPatternFinder::PatternVec
GlobalPatternFinder::convertToPattern(const PatternStateVec& cache) const {
    PatternVec patterns{};
    patterns.reserve(cache.size());
    std::transform(cache.begin(), cache.end(), std::back_inserter(patterns), [this](const PatternState& cacheEntry) {
        GlobalPattern pattern {convertToPattern(cacheEntry)};
        ATH_MSG_VERBOSE("Converted new pattern "<<pattern);
        return pattern;
    });
    return patterns;
}

GlobalPatternFinder::SearchTree_t 
GlobalPatternFinder::constructTree(const ActsTrk::GeometryContext& gctx,
                                      const SpacePointContainerVec& spacepoints) const {
    SearchTree_t::vector_t rawData{};
    using SectorProjector = MsTrackSeeder::SectorProjector;
    for (const SpacePointContainer* spc : spacepoints) {
        ATH_MSG_VERBOSE("Adding to the search tree "<<spc->size()<<" space point buckets");
        for (const SpacePointBucket* bucket : *spc) {
            ATH_MSG_VERBOSE("Processing " << bucket->size() << " spacepoint...");
            // We can have up to 3 entries per hit (when the hit does not measure phi).
            rawData.reserve(rawData.size() + 3*bucket->size());
            const int sector = bucket->msSector()->sector();

            for (const auto& hit : *bucket) {
                if (hit->isStraw() && !m_cfg.useMdtHits) {
                    continue;
                }
                const Amg::Vector3D globalPos {bucket->msSector()->localToGlobalTransform(gctx) * hit->localPosition()};
                // Try to duplicate the hit in the neighboring sectors if it is close to the sector border. This ensures that we can find patterns crossing the sector borders.
                for (const auto proj : {SectorProjector::leftOverlap, SectorProjector::center, SectorProjector::rightOverlap}) {
                    /// Check whether the hit belongs to the left or right sector as well
                    const int projSector = MsTrackSeeder::ringSector(sector + Acts::toUnderlying(proj));
                    const double phi {globalPos.phi()};
                    if (hit->measuresPhi() && proj != SectorProjector::center && 
                        !sectorMap.insideSector(projSector, phi)) {
                        ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" Hit @"<< *hit
                            <<" is not in sector "<<projSector<<" which is "<<MsTrackSeeder::to_string(proj) <<" to "<< sector);
                        continue;
                    }
                    std::array<double, 2> coords{};
                    coords[Acts::toUnderlying(SeedCoords::eTheta)] = globalPos.theta();

                    /** Blow-up the number of sectors by a factor of 2. The even numbers represent the 
                    *  segments expressed @ the sector centre. The odd numbers represent the overlap region
                    *  between two adjacent sectors. For sector 16, the right overlap region is mapped to 1 */
                    coords[Acts::toUnderlying(SeedCoords::eSector)] = MsTrackSeeder::ringOverlap(2*sector + Acts::toUnderlying(proj));
                    ATH_MSG_VERBOSE(__func__<<"() "<<__LINE__<<" Add hit @"<< *hit 
                                            <<"\nwith global position "<<Amg::toString(globalPos) <<" and coordinates "<<coords<<" to the search tree");
                    rawData.emplace_back(std::move(coords), 
                                         HitPayload{hit.get(), bucket, spc, m_cfg.idHelperSvc->stationIndex(hit->identify()),
                                                        m_spSorter.sectorLayerNum(*hit),globalPos.perp(), globalPos.z(), phi});
                }  
            }
        }
    }
    ATH_MSG_VERBOSE("Create a new tree with "<<rawData.size()<<" entries. ");
    return SearchTree_t{std::move(rawData)};
}


GlobalPatternFinder::LayerOrdering
GlobalPatternFinder::checkLayerOrdering(const HitPayload& hit1,
                                        const HitPayload& hit2) const {
    auto getLayerOrdering = [](const bool isLayer1Lower) {
        return isLayer1Lower ? LayerOrdering::eLowerLayer : LayerOrdering::eHigherLayer;
    };
    /** Hits in the same spectrometer sector */
    if (hit1.hit->msSector() == hit2.hit->msSector()) {
        if (hit1.layerNum == hit2.layerNum) {
            return LayerOrdering::eSameLayer;
        } else {
            return getLayerOrdering(hit1.layerNum < hit2.layerNum);
        }
    }
    StIndex st1 {hit1.station};
    StIndex st2 {hit2.station};
    /** Hits in the same station and different sectors. We can have this case for hits in the overlap region of two adjacent sectors. */
    if (st1 == st2) {
        return getLayerOrdering(hit1.hit->msSector()->barrel() ? hit1.R < hit2.R : hit1.Z < hit2.Z);
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
                                                const int sectorCoord,
                                                const double seedTheta)
        : sectorCoord{sectorCoord}, theta{seedTheta} {
    addHit(seed, 0.0, 0.0);
}
void GlobalPatternFinder::PatternState::addHit(const HitPayload& hit,
                                               const double residual,
                                               const double acceptWindow) {
    StIndex stationHit {hit.station}; 
    hitsPerStation[stationHit].push_back(hit);
    if (stations.empty() || stations.back() != stationHit) {
        stations.push_back(stationHit);
    }
    /** Increment the number of inserted hits */
    nInsertedHits++;
    /** Reset the number of missed layer hits */
    nMissedLayerHits = 0;
    /** Update the hit counts in bending direction */
    if (isPrecisionHit(*hit.hit)) nPrecisionHits++;
    else if (hit.hit->measuresEta()) nBendingTriggerHits++;
    /** Update the phi hit count and the average phi value */
    if (hit.hit->measuresPhi()) {
        updateMean(phi, nPhiHits, hit.phi);
        nPhiHits++;
    }
    /** Update the residual */
    totalResidual += residual;
    lastResidual = residual;
    if (acceptWindow > 1e-3) {
        totalRes2AcceptWindow += residual / acceptWindow;
        lastAccepWindow = acceptWindow;
    }
}

void GlobalPatternFinder::PatternState::overWriteHit(const HitPayload& oldHit,
                                                     const HitPayload& newHit,
                                                     const double newResidual,
                                                     const double newAcceptWindow) {
    if (oldHit.station != newHit.station) {
        throw std::runtime_error(std::format("Trying to overwrite a hit in station {} with another one from station {}", stName(oldHit.station), stName(newHit.station)));
        return;
    }
    // We expect to overwrite hits of the same type (precision/trigger), since we only branch when we have
    // compatible hits in the same layer, except for sTGC hits, where we have pad and strips in the same layer
    if ((isPrecisionHit(*oldHit.hit) != isPrecisionHit(*newHit.hit) || 
        oldHit.hit->measuresEta() != newHit.hit->measuresEta()) && newHit.hit->type() != xAOD::UncalibMeasType::sTgcStripType) {
        std::stringstream ss {"Trying to overwrite a hit with incompatible type\n"};
        ss << "Old hit: " << *oldHit.hit << ", isPrecision: " << isPrecisionHit(*oldHit.hit) << ", measuresEta: " << oldHit.hit->measuresEta() << "\n";
        ss << "New hit: " << *newHit.hit << ", isPrecision: " << isPrecisionHit(*newHit.hit) << ", measuresEta: " << newHit.hit->measuresEta();
        throw std::runtime_error(ss.str());
        return;
    }
    auto it = hitsPerStation.find(oldHit.station);
    if (it == hitsPerStation.end()) {
        throw std::runtime_error("Trying to remove a hit from a station that is not in the pattern cache");
        return;
    }
    auto& hitsInThisStation = it->second;
    auto toRemove = std::ranges::find(hitsInThisStation, oldHit);
    if (toRemove == hitsInThisStation.end()) {
        throw std::runtime_error("Trying to remove a hit that is not in the pattern cache station hits");
        return;
    }
    *toRemove = newHit;

    /** Update the phi hit count and average phi of the pattern */
    if (oldHit.hit->measuresPhi()) {
        updateMean(phi, nPhiHits, oldHit.phi, false);
        nPhiHits--;
    }
    if (newHit.hit->measuresPhi()) {
        updateMean(phi, nPhiHits, newHit.phi, true);
        nPhiHits++;
    }
    // Update the residual
    totalResidual += newResidual - lastResidual;
    lastResidual = newResidual;
    totalRes2AcceptWindow += newResidual / newAcceptWindow - lastResidual / lastAccepWindow;
    lastAccepWindow = newAcceptWindow;
}
void GlobalPatternFinder::PatternState::finalizePattern() {
    theta = 0.;
    unsigned nHits {0};
    for (const auto& [_, hits] : hitsPerStation) {
        for (const auto& hit : hits) {
            /** Update the bucket map */
            auto& bucketVec {bucketsPerContainer[hit.container]};
            if (std::ranges::find(bucketVec, hit.bucket) == bucketVec.end()){
                bucketVec.push_back(hit.bucket);
            }
            /** Update eta */
            theta += atan2(hit.R, hit.Z);
            nHits++;
        }
    }
    theta /= nHits;
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
GlobalPatternFinder::PatternState::getNthLastHit(const std::size_t n) const {
    if (n > nInsertedHits) {
        // If we have not yet inserted n hits, we return the seed hit
        return hitsPerStation.at(stations.front()).front();
    }
    std::size_t remaining {n};
    for (auto stIt = stations.rbegin(); stIt != stations.rend(); ++stIt) {
        const auto& hits {hitsPerStation.at(*stIt)};
        const size_t nHits  {hits.size()};
        if (remaining <= nHits) {
            return hits.at(nHits - remaining);
        }
        remaining -= nHits;
    }
    // Fall-back return, should not happen if the input n is consistent with nInsertedHits
    return hitsPerStation.at(stations.front()).front();
}
bool GlobalPatternFinder::PatternState::isInPattern(const HitPayload& hit) const {
    const auto seedStationIt {hitsPerStation.find(hit.station)};
    return seedStationIt != hitsPerStation.end() && 
           std::ranges::find(seedStationIt->second, hit) != seedStationIt->second.end();                                               
}
bool GlobalPatternFinder::PatternState::operator==(const PatternState& other) const {
    return hitsPerStation == other.hitsPerStation;
}
bool GlobalPatternFinder::HitPayload::operator==(const HitPayload& other) const {
    return hit == other.hit;
}
void GlobalPatternFinder::PatternState::print(std::ostream& ostr) const {
    ostr<<"Pattern state, Expanded Sector: "<<sectorCoord<<", Theta: "<<theta << ", Phi: "<<phi;
    ostr<<", nPrecisionHits: "<<nPrecisionHits<<", nEtaNonPrecisionHits: "<<nBendingTriggerHits<<", nPhiHits: "<<nPhiHits;
    ostr<<", totRes: "<<totalResidual << ", totNormResidual: "<<totalRes2AcceptWindow;
    ostr<<", Hit per station: \n";
    for (const auto& [station,hits] : hitsPerStation) {
        ostr<<"  Station "<<Muon::MuonStationIndex::stName(station)<<" has "<<hits.size()<<" hits\n";
        for (const auto& hit : hits) {
            ostr<<"    "<<*hit.hit<<", R: "<<hit.R<<", Phi:"<<hit.phi<<"\n";
        }
    }
}
}