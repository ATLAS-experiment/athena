/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TgcL0TruthValidationAlg.h"

#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenParticle.h"
#include "EventPrimitives/EventPrimitives.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include "L0MuonS1TGCValidation/TgcL0ValidationEventCheck.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "TrkEventPrimitives/ParticleHypothesis.h"
#include "TrkEventPrimitives/PropDirection.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkSurfaces/DiscSurface.h"
#include "TrkSurfaces/PerigeeSurface.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <memory>
#include <numbers>
#include <vector>

namespace {

struct StationPosition {
  bool valid{false};
  float eta{L0Muon::TgcL0ValidationInvalidValue};
  float phi{L0Muon::TgcL0ValidationInvalidValue};
};

using StationPositions = std::array<StationPosition, 3>;

std::uint8_t stationBit(const std::size_t station) {
  return static_cast<std::uint8_t>(1U << station);
}

bool candidatePosition(const L0Muon::TgcL0Candidate& candidate,
                       const std::size_t station, float& eta, float& phi) {
  if ((candidate.positionStationMask & stationBit(station)) == 0U) return false;
  if (station == 0U) {
    eta = candidate.m1Eta;
    phi = candidate.m1Phi;
  } else if (station == 1U) {
    eta = candidate.m2Eta;
    phi = candidate.m2Phi;
  } else {
    eta = candidate.m3Eta;
    phi = candidate.m3Phi;
  }
  return std::isfinite(eta) && std::isfinite(phi);
}

float meanDeltaR(const L0Muon::TgcL0Candidate& candidate,
                 const StationPositions& truthPositions) {
  float sumSquaredDeltaR{0.F};
  std::size_t nStations{0U};
  for (std::size_t station = 0U; station < truthPositions.size(); ++station) {
    if (!truthPositions[station].valid) continue;
    float candidateEta{0.F};
    float candidatePhi{0.F};
    if (!candidatePosition(candidate, station, candidateEta, candidatePhi)) {
      continue;
    }
    const float deltaEta = candidateEta - truthPositions[station].eta;
    const float deltaPhi = static_cast<float>(xAOD::P4Helpers::deltaPhi(
        candidatePhi, truthPositions[station].phi));
    sumSquaredDeltaR += deltaEta * deltaEta + deltaPhi * deltaPhi;
    ++nStations;
  }
  if (nStations == 0U) return L0Muon::TgcL0ValidationInvalidValue;
  return std::sqrt(sumSquaredDeltaR / static_cast<float>(nStations));
}

StationPositions truthPositions(const L0Muon::TgcL0ValidationEvent& event,
                                const std::size_t truth) {
  const std::uint8_t mask = event.truth.extrapolatedStationMask[truth];
  StationPositions positions{};
  positions[0] = {(mask & 0x1U) != 0U, event.truth.m1Eta[truth],
                  event.truth.m1Phi[truth]};
  positions[1] = {(mask & 0x2U) != 0U, event.truth.m2Eta[truth],
                  event.truth.m2Phi[truth]};
  positions[2] = {(mask & 0x4U) != 0U, event.truth.m3Eta[truth],
                  event.truth.m3Phi[truth]};
  return positions;
}

int pivotStation(const std::uint8_t stationMask) {
  if ((stationMask & 0x4U) != 0U) return 2;
  if ((stationMask & 0x2U) != 0U) return 1;
  if ((stationMask & 0x1U) != 0U) return 0;
  return -1;
}

struct CandidateMatch {
  float residual{L0Muon::TgcL0ValidationInvalidValue};
  std::size_t truth{0U};
  std::size_t candidate{0U};
};

struct FinalCandidateMatch {
  float residual{L0Muon::TgcL0ValidationInvalidValue};
  std::size_t truth{0U};
  std::size_t candidate{0U};
};

struct SegmentMatch {
  float residual{L0Muon::TgcL0ValidationInvalidValue};
  std::size_t truth{0U};
  std::size_t segment{0U};
};

bool matchesTgcFields(const xAOD::TGCCandData& input,
                      const xAOD::SectorLogicCandData& output) {
  return output.pT() == input.pt() &&
         output.charge() == input.candCharge() &&
         output.rawPhi() == input.phi() &&
         output.rawEta() == input.eta() &&
         output.ptThresh() == input.threshold() &&
         output.TCID() == input.tcId() &&
         output.coinType() == input.coinType();
}

bool hasOnlyTgcFields(const xAOD::SectorLogicCandData& candidate) {
  return candidate.isMDT() == 0U && candidate.mdtFlag() == 0U &&
         candidate.numMDTSeg() == 0U && candidate.mdtSegQual() == 0U &&
         candidate.tileCoin() == 0U && candidate.exotTrig() == 0U;
}

float decodeEta(const xAOD::TGCCandData& candidate) {
  const float fraction =
      static_cast<float>(candidate.eta()) /
      static_cast<float>(xAOD::TGCCandData::etaBitRange());
  return fraction * (2.F * xAOD::TGCCandData::etaRange()) -
         xAOD::TGCCandData::etaRange();
}

float decodePhi(const xAOD::TGCCandData& candidate) {
  const float fraction =
      static_cast<float>(candidate.phi()) /
      static_cast<float>(xAOD::TGCCandData::phiBitRange());
  return fraction * xAOD::TGCCandData::phiRange() -
         std::numbers::pi_v<float>;
}

int sourceCandidateIndex(const xAOD::TGCCandData& candidate,
                         const L0Muon::TgcL0CandidateContainer& sources,
                         const float eta, const float phi) {
  const float etaTolerance =
      0.51F * 2.F * xAOD::TGCCandData::etaRange() /
      static_cast<float>(xAOD::TGCCandData::etaBitRange());
  const float phiTolerance =
      1.01F * xAOD::TGCCandData::phiRange() /
      static_cast<float>(xAOD::TGCCandData::phiBitRange());
  int result{-1};
  for (std::size_t index = 0U; index < sources.size(); ++index) {
    const L0Muon::TgcL0Candidate& source = sources[index];
    if (source.subdetectorId != candidate.subdetectorId() ||
        source.sectorId != candidate.sectorId() ||
        source.bcTag != candidate.bcTag() ||
        (source.charge > 0 ? 1U : 0U) != candidate.candCharge() ||
        source.goodMagneticField != candidate.goodMagneticField() ||
        std::abs(source.eta - eta) > etaTolerance ||
        std::abs(static_cast<float>(
            xAOD::P4Helpers::deltaPhi(source.phi, phi))) > phiTolerance) {
      continue;
    }
    if (result >= 0) return -2;
    result = static_cast<int>(index);
  }
  return result;
}

}  // namespace

namespace L0Muon {

StatusCode TgcL0TruthValidationAlg::initialize() {
  ATH_CHECK(m_candidateKey.initialize());
  ATH_CHECK(m_segmentKey.initialize());
  ATH_CHECK(m_truthEventKey.initialize());
  ATH_CHECK(m_finalCandidateKey.initialize(
      m_validateFinalCandidates.value() || m_validateSectorLogic.value()));
  ATH_CHECK(m_sectorLogicKey.initialize(m_validateSectorLogic));
  ATH_CHECK(m_outputKey.initialize());
  ATH_CHECK(m_extrapolator.retrieve());

  if (m_stationAbsZ.value().size() != 3U) {
    ATH_MSG_ERROR("StationAbsZ must contain exactly M1, M2, and M3 values");
    return StatusCode::FAILURE;
  }
  if (m_validationMinR.value() < 0.0 || m_validationMaxR.value() <= m_validationMinR.value()) {
    ATH_MSG_ERROR("Validation radii are invalid: min=" << m_validationMinR.value()
                  << ", max=" << m_validationMaxR.value());
    return StatusCode::FAILURE;
  }
  if (m_validationPlaneToleranceZ.value() < 0.0) {
    ATH_MSG_ERROR("ValidationPlaneToleranceZ must be non-negative");
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

StatusCode TgcL0TruthValidationAlg::execute(const EventContext& ctx) const {
  SG::ReadHandle<TgcL0CandidateContainer> candidates{m_candidateKey, ctx};
  if (!candidates.isValid()) {
    ATH_MSG_ERROR("Failed to retrieve " << m_candidateKey.fullKey());
    return StatusCode::FAILURE;
  }
  SG::ReadHandle<TgcL0SegmentContainer> segments{m_segmentKey, ctx};
  if (!segments.isValid()) {
    ATH_MSG_ERROR("Failed to retrieve " << m_segmentKey.fullKey());
    return StatusCode::FAILURE;
  }
  SG::ReadHandle<McEventCollection> truthEvents{m_truthEventKey, ctx};
  if (!truthEvents.isValid()) {
    ATH_MSG_ERROR("Failed to retrieve " << m_truthEventKey.fullKey());
    return StatusCode::FAILURE;
  }
  auto output = std::make_unique<TgcL0ValidationEvent>();
  output->event.runNumber = ctx.eventID().run_number();
  output->event.eventNumber = ctx.eventID().event_number();
  output->event.lumiBlock = ctx.eventID().lumi_block();
  output->event.bcid = ctx.eventID().bunch_crossing_id();

  const bool readFinalCandidates = m_validateFinalCandidates.value() ||
                                   m_validateSectorLogic.value();
  const xAOD::TGCCandDataContainer* finalCandidateContainer{nullptr};
  if (readFinalCandidates) {
    SG::ReadHandle<xAOD::TGCCandDataContainer> finalCandidates{
        m_finalCandidateKey, ctx};
    if (!finalCandidates.isValid()) {
      ATH_MSG_ERROR("Failed to retrieve " << m_finalCandidateKey.fullKey());
      return StatusCode::FAILURE;
    }
    finalCandidateContainer = finalCandidates.cptr();
    for (std::size_t inputIndex = 0U;
         inputIndex < finalCandidateContainer->size(); ++inputIndex) {
      const xAOD::TGCCandData* inputCandidate =
          (*finalCandidateContainer)[inputIndex];
      if (inputCandidate == nullptr) {
        ATH_MSG_ERROR("Null final TGC candidate at index " << inputIndex);
        return StatusCode::FAILURE;
      }

      const float eta = decodeEta(*inputCandidate);
      const float phi = decodePhi(*inputCandidate);
      int sourceIndex{-1};
      int station{-1};
      if (inputCandidate->tcId() != 0U) {
        sourceIndex =
            sourceCandidateIndex(*inputCandidate, *candidates, eta, phi);
        if (sourceIndex < 0) {
          ATH_MSG_ERROR("Final TGC candidate " << inputIndex
                        << (sourceIndex == -2 ? " has ambiguous"
                                              : " has no")
                        << " pre-Inner source candidate");
          return StatusCode::FAILURE;
        }
        station = pivotStation(
            (*candidates)[static_cast<std::size_t>(sourceIndex)]
                .positionStationMask);
        if (station < 0) {
          ATH_MSG_ERROR("Final TGC candidate " << inputIndex
                        << " has no valid reference station");
          return StatusCode::FAILURE;
        }
      }

      output->finalCandidates.sourceCandidateIndex.emplace_back(sourceIndex);
      output->finalCandidates.referenceStation.emplace_back(
          station < 0
              ? static_cast<std::uint8_t>(TgcL0ValidationStation::Invalid)
              : static_cast<std::uint8_t>(station));
      output->finalCandidates.subdetectorId.emplace_back(
          inputCandidate->subdetectorId());
      output->finalCandidates.triggerSector.emplace_back(
          inputCandidate->sectorId());
      output->finalCandidates.bcTag.emplace_back(inputCandidate->bcTag());
      output->finalCandidates.tcId.emplace_back(inputCandidate->tcId());
      output->finalCandidates.rawEta.emplace_back(inputCandidate->eta());
      output->finalCandidates.rawPhi.emplace_back(inputCandidate->phi());
      output->finalCandidates.eta.emplace_back(eta);
      output->finalCandidates.phi.emplace_back(phi);
      output->finalCandidates.ptCode.emplace_back(inputCandidate->pt());
      output->finalCandidates.pt.emplace_back(inputCandidate->ptValueGeV());
      output->finalCandidates.threshold.emplace_back(
          inputCandidate->threshold());
      output->finalCandidates.charge.emplace_back(
          inputCandidate->candCharge() != 0U ? 1 : -1);
      output->finalCandidates.innerCoincidence.emplace_back(
          inputCandidate->hasInnerCoincidence() ? 1U : 0U);
      output->finalCandidates.goodMagneticField.emplace_back(
          inputCandidate->goodMagneticField() ? 1U : 0U);
      output->finalCandidates.truthIndex.emplace_back(-1);
      output->finalCandidates.truthMatchDeltaR.emplace_back(
          TgcL0ValidationInvalidValue);
    }
  }

  if (m_validateSectorLogic) {
    SG::ReadHandle<xAOD::SectorLogicCandDataContainer> sectorLogicCandidates{
        m_sectorLogicKey, ctx};
    if (!sectorLogicCandidates.isValid()) {
      ATH_MSG_ERROR("Failed to retrieve " << m_sectorLogicKey.fullKey());
      return StatusCode::FAILURE;
    }

    std::size_t sectorLogicIndex{0U};
    for (std::size_t inputIndex = 0U;
         inputIndex < finalCandidateContainer->size();
         ++inputIndex) {
      const xAOD::TGCCandData* inputCandidate =
          (*finalCandidateContainer)[inputIndex];
      if (inputCandidate->tcId() == 0U) continue;
      if (sectorLogicIndex >= sectorLogicCandidates->size()) {
        ATH_MSG_ERROR(
            "Fewer Sector Logic candidates than non-empty TGC candidates");
        return StatusCode::FAILURE;
      }
      const xAOD::SectorLogicCandData* sectorLogicCandidate =
          (*sectorLogicCandidates)[sectorLogicIndex];
      if (sectorLogicCandidate == nullptr) {
        ATH_MSG_ERROR("Null Sector Logic candidate at index "
                      << sectorLogicIndex);
        return StatusCode::FAILURE;
      }
      if (!matchesTgcFields(*inputCandidate, *sectorLogicCandidate)) {
        ATH_MSG_ERROR("Sector Logic candidate " << sectorLogicIndex
                      << " does not preserve TGC candidate " << inputIndex);
        return StatusCode::FAILURE;
      }
      if (!hasOnlyTgcFields(*sectorLogicCandidate)) {
        ATH_MSG_ERROR("Sector Logic candidate " << sectorLogicIndex
                      << " contains non-TGC payload");
        return StatusCode::FAILURE;
      }
      if (sectorLogicCandidate->boardID() != 0U ||
          sectorLogicCandidate->fiberID() != 0U ||
          sectorLogicCandidate->BCIDOffset() != 0 ||
          sectorLogicCandidate->veto() != 0U) {
        ATH_MSG_ERROR("Sector Logic candidate " << sectorLogicIndex
                      << " has unexpected placeholder metadata");
        return StatusCode::FAILURE;
      }
      output->sectorLogic.inputCandidateIndex.emplace_back(
          static_cast<std::uint32_t>(inputIndex));
      output->sectorLogic.candWord.emplace_back(
          sectorLogicCandidate->candWord());
      output->sectorLogic.candExtraWord.emplace_back(
          sectorLogicCandidate->candExtraWord());
      output->sectorLogic.boardId.emplace_back(sectorLogicCandidate->boardID());
      output->sectorLogic.fiberId.emplace_back(sectorLogicCandidate->fiberID());
      output->sectorLogic.bcidOffset.emplace_back(
          sectorLogicCandidate->BCIDOffset());
      output->sectorLogic.veto.emplace_back(sectorLogicCandidate->veto());
      ++sectorLogicIndex;
    }
    if (sectorLogicIndex != sectorLogicCandidates->size()) {
      ATH_MSG_ERROR(
          "More Sector Logic candidates than non-empty TGC candidates");
      return StatusCode::FAILURE;
    }
  }

  const auto processParticle = [&](const auto& particle) -> StatusCode {
    if (!particle || std::abs(particle->pdg_id()) != 13 ||
        particle->status() != m_requiredTruthStatus.value() ||
        std::abs(HepMC::barcode(particle)) > m_maxAbsBarcode.value()) {
      return StatusCode::SUCCESS;
    }

    const auto& momentum = particle->momentum();
    const double pt = momentum.perp();
    const double eta = momentum.eta();
    const double phi = momentum.phi();
    if (pt < m_minPt.value() || std::abs(eta) < m_minAbsEta.value() ||
        std::abs(eta) > m_maxAbsEta.value() || !std::isfinite(phi)) {
      return StatusCode::SUCCESS;
    }

    const int pdgId = particle->pdg_id();
    const double theta = 2.0 * std::atan(std::exp(-eta));
    const double momentumMagnitude = pt * std::cosh(eta);
    if (momentumMagnitude <= 0.0 || !std::isfinite(theta)) {
      return StatusCode::SUCCESS;
    }

    const double charge = pdgId == 13 ? -1.0 : 1.0;
    const Trk::PerigeeSurface perigeeSurface{Amg::Vector3D{0.0, 0.0, 0.0}};
    const Trk::Perigee perigee{0.0, 0.0, phi, theta,
                               charge / momentumMagnitude, perigeeSurface};

    StationPositions positions{};
    std::uint8_t stationMask{0U};
    const std::vector<double>& stationAbsZ = m_stationAbsZ.value();
    for (std::size_t station = 0U; station < positions.size(); ++station) {
      const double z = eta >= 0.0 ? stationAbsZ[station] : -stationAbsZ[station];
      Amg::Transform3D transform = Amg::Transform3D::Identity();
      transform.translation().z() = z;
      const Trk::DiscSurface disc{transform, m_validationMinR.value(),
                                  m_validationMaxR.value()};
      const Trk::BoundaryCheck boundaryCheck{true};
      const auto extrapolated = m_extrapolator->extrapolate(
          ctx, perigee, disc, Trk::alongMomentum, boundaryCheck, Trk::muon);
      if (!extrapolated) continue;
      const Amg::Vector3D& position = extrapolated->position();
      if (!std::isfinite(position.eta()) || !std::isfinite(position.phi()) ||
          std::abs(position.z() - z) > m_validationPlaneToleranceZ.value()) {
        continue;
      }
      positions[station] = StationPosition{
          true, static_cast<float>(position.eta()),
          static_cast<float>(xAOD::P4Helpers::deltaPhi(position.phi(), 0.))};
      stationMask |= stationBit(station);
    }

    output->truth.pdgId.emplace_back(pdgId);
    output->truth.barcode.emplace_back(HepMC::barcode(particle));
    output->truth.pt.emplace_back(static_cast<float>(pt));
    output->truth.eta.emplace_back(static_cast<float>(eta));
    output->truth.phi.emplace_back(static_cast<float>(phi));
    output->truth.charge.emplace_back(static_cast<float>(charge));
    output->truth.extrapolatedStationMask.emplace_back(stationMask);
    output->truth.m1Eta.emplace_back(positions[0].eta);
    output->truth.m1Phi.emplace_back(positions[0].phi);
    output->truth.m2Eta.emplace_back(positions[1].eta);
    output->truth.m2Phi.emplace_back(positions[1].phi);
    output->truth.m3Eta.emplace_back(positions[2].eta);
    output->truth.m3Phi.emplace_back(positions[2].phi);
    output->truth.matched.emplace_back(0U);
    output->truth.matchedCandidateIndex.emplace_back(-1);
    output->truth.matchMeanDeltaR.emplace_back(TgcL0ValidationInvalidValue);
    output->truth.unmatchedReason.emplace_back(
        stationMask == 0x7U
            ? static_cast<std::uint8_t>(
                  TgcL0ValidationUnmatchedReason::NoCandidateInWindow)
            : static_cast<std::uint8_t>(
                  TgcL0ValidationUnmatchedReason::NotFullyExtrapolated));
    output->truth.finalCandidateMatched.emplace_back(0U);
    output->truth.matchedFinalCandidateIndex.emplace_back(-1);
    output->truth.finalCandidateMatchDeltaR.emplace_back(
        TgcL0ValidationInvalidValue);
    output->truth.finalCandidateUnmatchedReason.emplace_back(
        stationMask == 0x7U
            ? static_cast<std::uint8_t>(
                  TgcL0ValidationUnmatchedReason::NoCandidateInWindow)
            : static_cast<std::uint8_t>(
                  TgcL0ValidationUnmatchedReason::NotFullyExtrapolated));
    output->truth.wireSegmentMatched.emplace_back(0U);
    output->truth.stripSegmentMatched.emplace_back(0U);
    output->truth.matchedWireSegmentIndex.emplace_back(-1);
    output->truth.matchedStripSegmentIndex.emplace_back(-1);
    output->truth.wireSegmentMatchResidual.emplace_back(
        TgcL0ValidationInvalidValue);
    output->truth.stripSegmentMatchResidual.emplace_back(
        TgcL0ValidationInvalidValue);
    return StatusCode::SUCCESS;
  };

  for (const HepMC::GenEvent* event : *truthEvents) {
    if (event == nullptr) continue;
#if __has_include("HepMC3/GenEvent.h")
    for (const auto& particle : event->particles()) {
      ATH_CHECK(processParticle(particle));
    }
#else
    for (auto particle = event->particles_begin();
         particle != event->particles_end(); ++particle) {
      ATH_CHECK(processParticle(*particle));
    }
#endif
  }

  for (const TgcL0Candidate& candidate : *candidates) {
    output->candidates.subdetectorId.emplace_back(candidate.subdetectorId);
    output->candidates.triggerSector.emplace_back(candidate.sectorId);
    output->candidates.readoutSector.emplace_back(candidate.readoutSector);
    output->candidates.bcTag.emplace_back(candidate.bcTag);
    output->candidates.stationMask.emplace_back(candidate.positionStationMask);
    output->candidates.wireStationMask.emplace_back(candidate.wireStationMask);
    output->candidates.stripStationMask.emplace_back(candidate.stripStationMask);
    output->candidates.eta.emplace_back(candidate.eta);
    output->candidates.phi.emplace_back(candidate.phi);
    output->candidates.deltaTheta.emplace_back(candidate.deltaTheta);
    output->candidates.deltaPhi.emplace_back(candidate.deltaPhi);
    output->candidates.pt.emplace_back(candidate.preInnerCoincidencePt);
    output->candidates.threshold.emplace_back(
        candidate.preInnerCoincidenceThreshold);
    output->candidates.charge.emplace_back(candidate.charge);
    output->candidates.goodMagneticField.emplace_back(
        candidate.goodMagneticField ? 1U : 0U);
    output->candidates.truthIndex.emplace_back(-1);
  }

  std::vector<CandidateMatch> candidateMatches;
  std::vector<bool> hasCandidateInWindow(output->truth.pt.size(), false);
  for (std::size_t truth = 0U; truth < output->truth.pt.size(); ++truth) {
    if (output->truth.extrapolatedStationMask[truth] != 0x7U) continue;
    const StationPositions positions = truthPositions(*output, truth);
    for (std::size_t candidate = 0U; candidate < candidates->size();
         ++candidate) {
      const TgcL0Candidate& inputCandidate = (*candidates)[candidate];
      if (m_requiredBcTagMask.value() != 0U &&
          (inputCandidate.bcTag & m_requiredBcTagMask.value()) == 0U) {
        continue;
      }
      if (output->truth.eta[truth] * inputCandidate.eta < 0.F) continue;
      const float residual = meanDeltaR(inputCandidate, positions);
      if (!std::isfinite(residual) || residual > m_maxMeanDeltaR.value()) continue;
      hasCandidateInWindow[truth] = true;
      candidateMatches.push_back({residual, truth, candidate});
    }
  }
  std::stable_sort(candidateMatches.begin(), candidateMatches.end(),
                   [](const CandidateMatch& lhs, const CandidateMatch& rhs) {
                     if (lhs.residual != rhs.residual) {
                       return lhs.residual < rhs.residual;
                     }
                     if (lhs.truth != rhs.truth) return lhs.truth < rhs.truth;
                     return lhs.candidate < rhs.candidate;
                   });
  for (const CandidateMatch& match : candidateMatches) {
    if (output->truth.matchedCandidateIndex[match.truth] >= 0 ||
        output->candidates.truthIndex[match.candidate] >= 0) {
      continue;
    }
    output->truth.matched[match.truth] = 1U;
    output->truth.matchedCandidateIndex[match.truth] =
        static_cast<int>(match.candidate);
    output->truth.matchMeanDeltaR[match.truth] = match.residual;
    output->truth.unmatchedReason[match.truth] =
        TgcL0ValidationNoUnmatchedReason;
    output->candidates.truthIndex[match.candidate] =
        static_cast<int>(match.truth);
  }
  for (std::size_t truth = 0U; truth < output->truth.pt.size(); ++truth) {
    if (output->truth.matched[truth] != 0U ||
        output->truth.extrapolatedStationMask[truth] != 0x7U) {
      continue;
    }
    output->truth.unmatchedReason[truth] = static_cast<std::uint8_t>(
        hasCandidateInWindow[truth]
            ? TgcL0ValidationUnmatchedReason::CandidateCompetition
            : TgcL0ValidationUnmatchedReason::NoCandidateInWindow);
  }

  if (m_validateFinalCandidates) {
    std::vector<FinalCandidateMatch> finalCandidateMatches;
    std::vector<bool> hasFinalCandidateInWindow(output->truth.pt.size(),
                                                false);
    for (std::size_t truth = 0U; truth < output->truth.pt.size(); ++truth) {
      if (output->truth.extrapolatedStationMask[truth] != 0x7U) continue;
      const StationPositions positions = truthPositions(*output, truth);
      for (std::size_t candidate = 0U;
           candidate < output->finalCandidates.tcId.size(); ++candidate) {
        if (output->finalCandidates.tcId[candidate] == 0U) continue;
        if (m_requiredBcTagMask.value() != 0U &&
            (output->finalCandidates.bcTag[candidate] &
             m_requiredBcTagMask.value()) == 0U) {
          continue;
        }
        if (output->truth.eta[truth] *
                output->finalCandidates.eta[candidate] <
            0.F) {
          continue;
        }
        const std::size_t station =
            output->finalCandidates.referenceStation[candidate];
        if (station >= positions.size() || !positions[station].valid) continue;
        const float deltaEta = output->finalCandidates.eta[candidate] -
                               positions[station].eta;
        const float deltaPhi = static_cast<float>(
            xAOD::P4Helpers::deltaPhi(output->finalCandidates.phi[candidate],
                                      positions[station].phi));
        const float residual = std::hypot(deltaEta, deltaPhi);
        if (residual > m_maxFinalCandidateDeltaR.value()) {
          continue;
        }
        hasFinalCandidateInWindow[truth] = true;
        finalCandidateMatches.push_back({residual, truth, candidate});
      }
    }
    std::stable_sort(
        finalCandidateMatches.begin(), finalCandidateMatches.end(),
        [](const FinalCandidateMatch& lhs, const FinalCandidateMatch& rhs) {
          if (lhs.residual != rhs.residual) {
            return lhs.residual < rhs.residual;
          }
          if (lhs.truth != rhs.truth) return lhs.truth < rhs.truth;
          return lhs.candidate < rhs.candidate;
        });
    for (const FinalCandidateMatch& match : finalCandidateMatches) {
      if (output->truth.matchedFinalCandidateIndex[match.truth] >= 0 ||
          output->finalCandidates.truthIndex[match.candidate] >= 0) {
        continue;
      }
      output->truth.finalCandidateMatched[match.truth] = 1U;
      output->truth.matchedFinalCandidateIndex[match.truth] =
          static_cast<int>(match.candidate);
      output->truth.finalCandidateMatchDeltaR[match.truth] = match.residual;
      output->truth.finalCandidateUnmatchedReason[match.truth] =
          TgcL0ValidationNoUnmatchedReason;
      output->finalCandidates.truthIndex[match.candidate] =
          static_cast<int>(match.truth);
      output->finalCandidates.truthMatchDeltaR[match.candidate] =
          match.residual;
    }
    for (std::size_t truth = 0U; truth < output->truth.pt.size(); ++truth) {
      if (output->truth.finalCandidateMatched[truth] != 0U ||
          output->truth.extrapolatedStationMask[truth] != 0x7U) {
        continue;
      }
      output->truth.finalCandidateUnmatchedReason[truth] =
          static_cast<std::uint8_t>(
              hasFinalCandidateInWindow[truth]
                  ? TgcL0ValidationUnmatchedReason::CandidateCompetition
                  : TgcL0ValidationUnmatchedReason::NoCandidateInWindow);
    }
  }

  for (const TgcL0Segment& segment : *segments) {
    output->segments.subdetectorId.emplace_back(segment.subdetectorId);
    output->segments.triggerSector.emplace_back(segment.triggerSector);
    output->segments.bcTag.emplace_back(segment.bcTag);
    output->segments.projection.emplace_back(
        static_cast<std::uint8_t>(segment.projection));
    output->segments.stationMask.emplace_back(segment.stationMask);
    output->segments.summedQuality.emplace_back(segment.summedQuality);
    output->segments.nStations.emplace_back(segment.nStations);
    output->segments.eta.emplace_back(segment.eta);
    output->segments.phi.emplace_back(segment.phi);
    output->segments.residual.emplace_back(segment.residual);
    output->segments.outputResidual.emplace_back(segment.outputResidual);
    output->segments.consistency.emplace_back(segment.consistency);
    output->segments.pivotChannel.emplace_back(segment.pivotChannel);
    output->segments.truthIndex.emplace_back(-1);
    output->segments.truthMatchResidual.emplace_back(
        TgcL0ValidationInvalidValue);
  }

  const auto matchProjection = [&](const TgcL0ValidationProjection projection,
                                   const float maximumResidual) {
    const auto projectionValue = static_cast<std::uint8_t>(projection);
    std::vector<SegmentMatch> possibleMatches;
    for (std::size_t truth = 0U; truth < output->truth.pt.size(); ++truth) {
      for (std::size_t segment = 0U;
           segment < output->segments.projection.size(); ++segment) {
        if (output->segments.projection[segment] != projectionValue) continue;
        if (m_requiredBcTagMask.value() != 0U &&
            (output->segments.bcTag[segment] &
             m_requiredBcTagMask.value()) == 0U) {
          continue;
        }
        if (output->truth.eta[truth] * output->segments.eta[segment] < 0.F) {
          continue;
        }
        const int station = pivotStation(output->segments.stationMask[segment]);
        if (station < 0 ||
            (output->truth.extrapolatedStationMask[truth] &
             stationBit(static_cast<std::size_t>(station))) == 0U) {
          continue;
        }
        const std::array<float, 3> truthEta{
            output->truth.m1Eta[truth], output->truth.m2Eta[truth],
            output->truth.m3Eta[truth]};
        const std::array<float, 3> truthPhi{
            output->truth.m1Phi[truth], output->truth.m2Phi[truth],
            output->truth.m3Phi[truth]};
        const float residual =
            projection == TgcL0ValidationProjection::Wire
                ? std::abs(output->segments.eta[segment] - truthEta[station])
                : std::abs(static_cast<float>(xAOD::P4Helpers::deltaPhi(
                      output->segments.phi[segment], truthPhi[station])));
        if (std::isfinite(residual) && residual <= maximumResidual) {
          possibleMatches.push_back({residual, truth, segment});
        }
      }
    }
    std::stable_sort(possibleMatches.begin(), possibleMatches.end(),
                     [](const SegmentMatch& lhs, const SegmentMatch& rhs) {
                       if (lhs.residual != rhs.residual) {
                         return lhs.residual < rhs.residual;
                       }
                       if (lhs.truth != rhs.truth) return lhs.truth < rhs.truth;
                       return lhs.segment < rhs.segment;
                     });
    for (const SegmentMatch& match : possibleMatches) {
      int& truthSegment =
          projection == TgcL0ValidationProjection::Wire
              ? output->truth.matchedWireSegmentIndex[match.truth]
              : output->truth.matchedStripSegmentIndex[match.truth];
      if (truthSegment >= 0 ||
          output->segments.truthIndex[match.segment] >= 0) {
        continue;
      }
      truthSegment = static_cast<int>(match.segment);
      output->segments.truthIndex[match.segment] =
          static_cast<int>(match.truth);
      output->segments.truthMatchResidual[match.segment] = match.residual;
      if (projection == TgcL0ValidationProjection::Wire) {
        output->truth.wireSegmentMatched[match.truth] = 1U;
        output->truth.wireSegmentMatchResidual[match.truth] = match.residual;
      } else {
        output->truth.stripSegmentMatched[match.truth] = 1U;
        output->truth.stripSegmentMatchResidual[match.truth] = match.residual;
      }
    }
  };

  matchProjection(TgcL0ValidationProjection::Wire,
                  m_maxWireSegmentDeltaEta.value());
  matchProjection(TgcL0ValidationProjection::Strip,
                  m_maxStripSegmentDeltaPhi.value());

  const TgcL0ValidationCheckResult check = checkTgcL0ValidationEvent(*output);
  if (!check.valid) {
    ATH_MSG_ERROR("Refusing to record inconsistent validation data: "
                  << check.message);
    return StatusCode::FAILURE;
  }

  SG::WriteHandle<TgcL0ValidationEvent> outputHandle{m_outputKey, ctx};
  ATH_CHECK(outputHandle.record(std::move(output)));
  return StatusCode::SUCCESS;
}

}  // namespace L0Muon
