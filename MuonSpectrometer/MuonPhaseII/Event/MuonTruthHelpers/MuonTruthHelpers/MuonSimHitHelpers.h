/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRUTHHELPERS_MUONSIMHITHELPERS_H
#define MUONTRUTHHELPERS_MUONSIMHITHELPERS_H

#include <xAODMeasurementBase/UncalibratedMeasurement.h>
#include <xAODMuonSimHit/MuonSimHit.h>
#include <xAODMuon/MuonSegment.h>
#include <xAODTruth/TruthParticle.h>

#include <unordered_set>
/** @brief Set of helper functions to fetch the links to the xAOD::MuonSimHits from the uncalibrated measurements. The links are decorated by 
 *         the PrepDataToSimHitAssocAlg which is residing inside the MuonTruthAlgsR4 package. Before using these functions ensure that the
 *         algorithms are ran accordingly
 */
namespace MuonR4 {
    class SpacePoint;
    class CalibratedSpacePoint;
    class Segment;
    class SegmentSeed;
    class SpacePointBucket;
    /** @brief Returns the MuonSimHit, if there's any, matched to the uncalibrated muon measurement. */
    const xAOD::MuonSimHit* getTruthMatchedHit(const xAOD::UncalibratedMeasurement& prdHit);
    /** @brief: Returns all sim hits matched to a xAOD::MuonSegment */
    std::unordered_set<const xAOD::MuonSimHit*> getMatchingSimHits(const xAOD::MuonSegment& segment);
    /** @brief Returns all sim hits that are matched to a collection of space points. For each spacepoint, the hit truth matching to
     *         the primary and secondary prd are retrieved. The secondary hit is only added if it's different from the primary one. */
    std::unordered_set<const xAOD::MuonSimHit*> getMatchingSimHits(const std::vector<const SpacePoint*>& spacePoints);
    /** @brief Returns all sim hits that are matched to a collection of calibrated space points. */
    std::unordered_set<const xAOD::MuonSimHit*> getMatchingSimHits(const std::vector<const CalibratedSpacePoint*>& measurements);
    /** @brief Returns all sim hits that are matched to the reconstructed segment */
    std::unordered_set<const xAOD::MuonSimHit*> getMatchingSimHits(const Segment& seg);
    /** @brief Returns all sim hits that are matched to the segmentSeed */
    std::unordered_set<const xAOD::MuonSimHit*> getMatchingSimHits(const SegmentSeed& seed);
    /** @brief Returns all sim hits that are matched to the spacePoint bucket */
    std::unordered_set<const xAOD::MuonSimHit*> getMatchingSimHits(const SpacePointBucket& bucket);
    /** @brief Returns the particle truth-matched to the segment */
    const xAOD::TruthParticle* getTruthMatchedParticle(const xAOD::MuonSegment& segment);
}

#endif
