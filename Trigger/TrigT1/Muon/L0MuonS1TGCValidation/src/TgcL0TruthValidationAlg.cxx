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
#include <limits>
#include <memory>
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

struct SegmentMatch {
  float residual{L0Muon::TgcL0ValidationInvalidValue};
  std::size_t truth{0U};
  std::size_t segment{0U};
};


}  // namespace

namespace L0Muon {

StatusCode TgcL0TruthValidationAlg::initialize() {
  ATH_CHECK(m_candidateKey.initialize());
  ATH_CHECK(m_segmentKey.initialize());
  ATH_CHECK(m_truthEventKey.initialize());
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
  if (m_requiredBcTag.value() < -1 ||
      m_requiredBcTag.value() >
          static_cast<int>(std::numeric_limits<std::uint16_t>::max())) {
    ATH_MSG_ERROR("RequiredBcTag must be -1 or a uint16 value");
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
      if (m_requiredBcTag.value() >= 0 &&
          inputCandidate.bcTag != static_cast<std::uint16_t>(m_requiredBcTag.value())) {
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
        if (m_requiredBcTag.value() >= 0 &&
            output->segments.bcTag[segment] !=
                static_cast<std::uint16_t>(m_requiredBcTag.value())) {
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
