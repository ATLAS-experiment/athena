/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TgcL0FloatingCandidateBuilderTool.h"

#include "PathResolver/PathResolver.h"
#include "StoreGate/ReadCondHandle.h"
#include "TgcL0FloatingData.h"
#include "TgcL0FloatingPtLut.h"
#include "TgcL0RdoDecoder.h"
#include "TgcL0OverlapClassification.h"
#include "TgcL0SegmentReconstruction.h"
#include "TgcL0StationCoincidence.h"

#include <sstream>
#include <utility>
#include <vector>

namespace {

struct CoincidenceQualityCounts {
  std::size_t nThreeOfThree{0};
  std::size_t nTwoOfThree{0};
  std::size_t nOneOfThree{0};
  std::size_t nTwoOfTwo{0};
  std::size_t nOneOfTwo{0};
};

void countCoincidenceQuality(
    const L1Muon::TgcL0Floating::StationCoincidence& coincidence,
    CoincidenceQualityCounts& counts) {
  if (coincidence.nominalLayers == 3U) {
    if (coincidence.observedLayers == 3U) {
      ++counts.nThreeOfThree;
    } else if (coincidence.observedLayers == 2U) {
      ++counts.nTwoOfThree;
    } else if (coincidence.observedLayers == 1U) {
      ++counts.nOneOfThree;
    }
  } else if (coincidence.nominalLayers == 2U) {
    if (coincidence.observedLayers == 2U) {
      ++counts.nTwoOfTwo;
    } else if (coincidence.observedLayers == 1U) {
      ++counts.nOneOfTwo;
    }
  }
}

}  // namespace

namespace L1Muon {

StatusCode TgcL0FloatingCandidateBuilderTool::initialize() {
  ATH_CHECK(m_idHelperSvc.retrieve());
  ATH_CHECK(m_cablingKey.initialize());
  ATH_CHECK(m_detectorManagerKey.initialize());

  const std::string calibrationDirectory =
      PathResolver::find_calib_directory("dev");
  if (calibrationDirectory.empty()) {
    ATH_MSG_ERROR("Could not resolve the GroupData development directory");
    return StatusCode::FAILURE;
  }
  const std::string ptCalibrationPath =
      calibrationDirectory + "/" + m_ptCalibrationFile.value();
  std::string error;
  m_ptLut = TgcL0FloatingPtLut::loadAscii(ptCalibrationPath, error);
  if (!m_ptLut) {
    ATH_MSG_ERROR("Failed to load Floating-pT calibration: " << error);
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("Loaded Floating-pT calibration "
               << m_ptLut->version() << " from " << ptCalibrationPath
               << " (eta bins=" << m_ptLut->etaBins()
               << ", folded-phi bins=" << m_ptLut->phiBinsPerFold()
               << ", knots=" << m_ptLut->knotCount()
               << ", mode="
               << (m_ptLut->isDevelopmentPayload() ? "development"
                                                    : "production")
               << ")");

  const std::string goodMagMapPath =
      calibrationDirectory + "/" + m_goodMagMapFile.value();
  error.clear();
  m_goodMagMap = TgcL0GoodMagMap::loadAscii(goodMagMapPath, error);
  if (!m_goodMagMap) {
    ATH_MSG_ERROR("Failed to load GoodMag map: " << error);
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("Loaded GoodMag map "
               << m_goodMagMap->version() << " from " << goodMagMapPath
               << " (eta bins=" << m_goodMagMap->etaBins()
               << ", folded-phi bins=" << m_goodMagMap->phiBinsPerFold()
               << ", poor bins=" << m_goodMagMap->poorBinCount() << ")");
  return StatusCode::SUCCESS;
}

StatusCode TgcL0FloatingCandidateBuilderTool::build(
    const TgcRdoContainer& rdos, TgcL0CandidateContainer& candidates,
    const EventContext& ctx) const {
  return build(rdos, candidates, nullptr, ctx);
}

StatusCode TgcL0FloatingCandidateBuilderTool::build(
    const TgcRdoContainer& rdos, TgcL0CandidateContainer& candidates,
    TgcL0SegmentContainer* segments, const EventContext& ctx) const {

  const Muon::TgcCablingMap* cabling{};
  ATH_CHECK(SG::get(cabling, m_cablingKey, ctx));

  SG::ReadCondHandle<MuonGM::MuonDetectorManager> detectorManagerHandle{
      m_detectorManagerKey, ctx};
  if (!detectorManagerHandle.isValid()) {
    ATH_MSG_ERROR("Failed to retrieve " << m_detectorManagerKey.fullKey());
    return StatusCode::FAILURE;
  }
  const MuonGM::MuonDetectorManager* detectorManager =
      detectorManagerHandle.cptr();

  TgcL0Floating::HitGroups hitGroups;
  TgcL0Floating::DecodeStatistics statistics;
  const TgcL0Floating::RdoDecoder decoder;
  ATH_CHECK(decoder.decode(rdos, *cabling, *m_idHelperSvc, *detectorManager,
                           hitGroups, statistics));

  TgcL0Floating::StationCoincidenceContainer coincidences;
  const TgcL0Floating::StationCoincidenceBuilder coincidenceBuilder;
  ATH_CHECK(coincidenceBuilder.build(hitGroups, coincidences));


  TgcL0Floating::SegmentReconstructionConfig segmentConfig;
  segmentConfig.maxPivotWireStripDeltaEta =
      m_maxPivotWireStripDeltaEta.value();
  segmentConfig.maxPivotWireStripDeltaPhi =
      m_maxPivotWireStripDeltaPhi.value();
  segmentConfig.maxSegmentCombinationsPerGroup =
      m_maxSegmentCombinationsPerGroup.value();
  segmentConfig.maxCandidatesPerLocalBin =
      m_maxCandidatesPerLocalBin.value();
  TgcL0Floating::SegmentStatistics segmentStatistics;
  const TgcL0Floating::SegmentReconstruction segmentReconstruction{
      std::move(segmentConfig)};
  ATH_CHECK(segmentReconstruction.build(coincidences, candidates,
                                        segmentStatistics, segments));

  TgcL0Floating::OverlapClassificationStatistics overlapStatistics;
  const TgcL0Floating::OverlapClassification overlapClassification;
  overlapClassification.classify(candidates, overlapStatistics);

  std::size_t nValidPtEstimates = 0U;
  for (TgcL0Candidate& candidate : candidates) {
    const TgcL0FloatingPtEvaluation evaluation =
        m_ptLut->evaluate(candidate.eta, candidate.phi, candidate.deltaTheta);
    candidate.goodMagneticField = m_goodMagMap->isGood(
        candidate.eta, candidate.phi, m_ptLut->absEtaMin(),
        m_ptLut->absEtaMax());
    if (!evaluation.ptEstimateValid) continue;
    candidate.preInnerCoincidencePt = evaluation.ptEstimateGeV;
    candidate.preInnerCoincidenceThreshold = evaluation.thresholdCode;
    candidate.charge = evaluation.estimatedCharge;
    ++nValidPtEstimates;
  }
  const std::size_t nInvalidPtEstimates =
      candidates.size() - nValidPtEstimates;
  ATH_MSG_DEBUG("Floating-pT evaluation: valid=" << nValidPtEstimates
                                                  << ", invalid="
                                                  << nInvalidPtEstimates);

  CoincidenceQualityCounts totalCounts;
  CoincidenceQualityCounts m1WireCounts;
  CoincidenceQualityCounts m1StripCounts;
  CoincidenceQualityCounts m2WireCounts;
  CoincidenceQualityCounts m2StripCounts;
  CoincidenceQualityCounts m3WireCounts;
  CoincidenceQualityCounts m3StripCounts;

  for (const TgcL0Floating::StationCoincidence& coincidence : coincidences) {
    countCoincidenceQuality(coincidence, totalCounts);

    CoincidenceQualityCounts* stationCounts{nullptr};
    if (coincidence.station == TgcL0Floating::Station::M1) {
      stationCounts = coincidence.isStrip ? &m1StripCounts : &m1WireCounts;
    } else if (coincidence.station == TgcL0Floating::Station::M2) {
      stationCounts = coincidence.isStrip ? &m2StripCounts : &m2WireCounts;
    } else if (coincidence.station == TgcL0Floating::Station::M3) {
      stationCounts = coincidence.isStrip ? &m3StripCounts : &m3WireCounts;
    }
    if (stationCounts != nullptr) {
      countCoincidenceQuality(coincidence, *stationCounts);
    }
  }

  ATH_MSG_DEBUG("Decoded " << statistics.nHits << " TGC hits from "
                            << statistics.nRawData << " raw-data words in "
                            << hitGroups.size() << " groups: wire="
                            << statistics.nWireHits << ", strip="
                            << statistics.nStripHits << ", M1="
                            << statistics.nM1Hits << ", M2="
                            << statistics.nM2Hits << ", M3="
                            << statistics.nM3Hits << ", inner="
                            << statistics.nInnerHits << ", unknown="
                            << statistics.nUnknownStation
                            << ", mapping failures="
                            << statistics.nMappingFailures
                            << "; station coincidences="
                            << coincidences.size() << " (3/3="
                            << totalCounts.nThreeOfThree << ", 2/3="
                            << totalCounts.nTwoOfThree << ", 1/3="
                            << totalCounts.nOneOfThree << ", 2/2="
                            << totalCounts.nTwoOfTwo << ", 1/2="
                            << totalCounts.nOneOfTwo << ")");

  ATH_MSG_DEBUG("Station coincidence breakdown: M1 wire=(3/3="
                << m1WireCounts.nThreeOfThree << ", 2/3="
                << m1WireCounts.nTwoOfThree << ", 1/3="
                << m1WireCounts.nOneOfThree << "), M1 strip=(2/2="
                << m1StripCounts.nTwoOfTwo << ", 1/2="
                << m1StripCounts.nOneOfTwo << "), M2 wire=(2/2="
                << m2WireCounts.nTwoOfTwo << ", 1/2="
                << m2WireCounts.nOneOfTwo << "), M2 strip=(2/2="
                << m2StripCounts.nTwoOfTwo << ", 1/2="
                << m2StripCounts.nOneOfTwo << "), M3 wire=(2/2="
                << m3WireCounts.nTwoOfTwo << ", 1/2="
                << m3WireCounts.nOneOfTwo << "), M3 strip=(2/2="
                << m3StripCounts.nTwoOfTwo << ", 1/2="
                << m3StripCounts.nOneOfTwo << ")");

  ATH_MSG_DEBUG("Segment reconstruction: wire="
                << segmentStatistics.nWireSegments << ", strip="
                << segmentStatistics.nStripSegments << ", candidates="
                << candidates.size() << " (M1M2M3="
                << segmentStatistics.nM1M2M3Candidates << ", M1M2="
                << segmentStatistics.nM1M2Candidates << ", M1M3="
                << segmentStatistics.nM1M3Candidates << ", M2M3="
                << segmentStatistics.nM2M3Candidates
                << ", M1only="
                << segmentStatistics.nM1OnlyPositionCandidates
                << ", M2only="
                << segmentStatistics.nM2OnlyPositionCandidates
                << ", M3only="
                << segmentStatistics.nM3OnlyPositionCandidates
                << "), projectionRejected="
                << segmentStatistics.nRejectedProjectionCombinations
                << ", projectionDuplicates="
                << segmentStatistics.nDuplicateProjectionSegments
                << ", projectionLimited="
                << segmentStatistics.nLimitedProjectionSegments
                << ", pairRejected=" << segmentStatistics.nRejectedPairs
                << ", candidateDuplicates="
                << segmentStatistics.nDuplicateCandidates
                << ", localDuplicates="
                << segmentStatistics.nLocalDuplicateCandidates
                << ", candidateLimited="
                << segmentStatistics.nLimitedCandidates);

  ATH_MSG_DEBUG("Overlap classification: groups="
                << overlapStatistics.nGroups << ", candidates="
                << overlapStatistics.nCandidates << ", maxMultiplicity="
                << overlapStatistics.maxMultiplicity);

  for (std::size_t groupId = 1U;
       groupId <= overlapStatistics.nGroups; ++groupId) {
    std::vector<std::size_t> memberIndices;
    for (std::size_t index = 0U; index < candidates.size(); ++index) {
      if (candidates[index].overlapGroupId == groupId) {
        memberIndices.emplace_back(index);
      }
    }
    if (memberIndices.empty()) continue;
    std::ostringstream members;
    for (std::size_t position = 0U; position < memberIndices.size(); ++position) {
      if (position != 0U) members << ",";
      members << memberIndices[position];
    }
    ATH_MSG_DEBUG("Overlap group detail: groupId="
                  << groupId << ", multiplicity=" << memberIndices.size()
                  << ", members=[" << members.str() << "]");
  }

  for (std::size_t candidateIndex = 0; candidateIndex < candidates.size();
       ++candidateIndex) {
    const TgcL0Candidate& candidate = candidates[candidateIndex];
    ATH_MSG_DEBUG("Reconstructed candidate: index="
                  << candidateIndex << ", bcTag=" << candidate.bcTag
                  << ", subdetectorId=" << candidate.subdetectorId
                  << ", triggerSector=" << candidate.sectorId
                  << ", readoutSector=" << candidate.readoutSector
                  << ", stationMask="
                  << static_cast<unsigned int>(candidate.stationMask)
                  << ", wireStationMask="
                  << static_cast<unsigned int>(candidate.wireStationMask)
                  << ", stripStationMask="
                  << static_cast<unsigned int>(candidate.stripStationMask)
                  << ", positionStationMask="
                  << static_cast<unsigned int>(
                         candidate.positionStationMask)
                  << ", eta=" << candidate.eta << ", phi="
                  << candidate.phi << ", deltaTheta="
                  << candidate.deltaTheta << ", deltaPhi="
                  << candidate.deltaPhi << ", preInnerCoincidencePt="
                  << candidate.preInnerCoincidencePt
                  << ", preInnerCoincidenceThreshold="
                  << static_cast<unsigned int>(
                         candidate.preInnerCoincidenceThreshold)
                  << ", charge=" << static_cast<int>(candidate.charge)
                  << ", goodMagneticField="
                  << candidate.goodMagneticField
                  << ", wireQuality="
                  << static_cast<unsigned int>(candidate.wireQuality)
                  << ", wireQualities=("
                  << static_cast<unsigned int>(candidate.m1WireQuality)
                  << ","
                  << static_cast<unsigned int>(candidate.m2WireQuality)
                  << ","
                  << static_cast<unsigned int>(candidate.m3WireQuality)
                  << "), stripQualities=("
                  << static_cast<unsigned int>(candidate.m1StripQuality)
                  << ","
                  << static_cast<unsigned int>(candidate.m2StripQuality)
                  << ","
                  << static_cast<unsigned int>(candidate.m3StripQuality)
                  << "), chambers=(M1:" << candidate.m1StationEta << "/"
                  << candidate.m1StationPhi << ", M2:"
                  << candidate.m2StationEta << "/" << candidate.m2StationPhi
                  << ", M3:" << candidate.m3StationEta << "/"
                  << candidate.m3StationPhi << "), overlapGroupId="
                  << candidate.overlapGroupId << ", overlapMultiplicity="
                  << static_cast<unsigned int>(candidate.overlapMultiplicity)
                  << ", inChamberOverlap=" << candidate.inChamberOverlap);
  }



  return StatusCode::SUCCESS;
}

}  // namespace L1Muon
