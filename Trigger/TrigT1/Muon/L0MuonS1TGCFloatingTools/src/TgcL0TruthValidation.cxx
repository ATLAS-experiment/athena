/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TgcL0TruthValidation.h"

#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenParticle.h"
#include "EventPrimitives/EventPrimitives.h"
#include "FourMomUtils/xAODP4Helpers.h"
#include "TrkEventPrimitives/ParticleHypothesis.h"
#include "TrkEventPrimitives/PropDirection.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkSurfaces/DiscSurface.h"
#include "TrkSurfaces/PerigeeSurface.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
#include <utility>
#include <vector>

namespace {

constexpr float invalidResidual = 999.F;

struct StationPosition {
  bool valid{false};
  float eta{0.F};
  float phi{0.F};
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
  float sumSquaredDeltaR = 0.F;
  std::size_t nStations = 0U;
  for (std::size_t station = 0U; station < truthPositions.size(); ++station) {
    if (!truthPositions[station].valid) continue;
    float candidateEta = 0.F;
    float candidatePhi = 0.F;
    if (!candidatePosition(candidate, station, candidateEta, candidatePhi)) {
      continue;
    }
    const float deltaEta = candidateEta - truthPositions[station].eta;
    const float deltaPhi = static_cast<float>(xAOD::P4Helpers::deltaPhi(
        candidatePhi, truthPositions[station].phi));
    sumSquaredDeltaR += deltaEta * deltaEta + deltaPhi * deltaPhi;
    ++nStations;
  }
  if (nStations == 0U) return invalidResidual;
  return std::sqrt(sumSquaredDeltaR / static_cast<float>(nStations));
}

}  // namespace

namespace L0Muon {
namespace TgcL0Floating {

TruthValidation::TruthValidation(TruthValidationConfig config)
    : m_config{std::move(config)} {}

StatusCode TruthValidation::validate(
    const McEventCollection& truthEvents, const Trk::IExtrapolator& extrapolator,
    const TgcL0CandidateContainer& candidates, const EventContext& ctx,
    TruthValidationSummary& summary) const {
  summary = TruthValidationSummary{};
  std::vector<bool> candidateMatched(candidates.size(), false);

  const std::array<double, 3> stationAbsZ{m_config.m1AbsZ, m_config.m2AbsZ,
                                         m_config.m3AbsZ};

  const auto processParticle = [&](const auto& particle) {
    if (!particle) return;
    const int pdgId = particle->pdg_id();
    if (std::abs(pdgId) != 13) return;
    const int status = particle->status();
    const int barcode = HepMC::barcode(particle);
    if (m_config.requiredStatus >= 0 && status != m_config.requiredStatus) return;
    if (m_config.maxAbsBarcode >= 0 &&
        std::abs(barcode) > m_config.maxAbsBarcode) {
      return;
    }

    const auto& momentum = particle->momentum();
    const double pt = momentum.perp();
    const double eta = momentum.eta();
    const double phi = momentum.phi();
    if (pt < m_config.minPt || std::abs(eta) < m_config.minAbsEta ||
        std::abs(eta) > m_config.maxAbsEta || !std::isfinite(phi)) {
      return;
    }

    ++summary.nSelectedTruthMuons;
    const double theta = 2.0 * std::atan(std::exp(-eta));
    const double momentumMagnitude = pt * std::cosh(eta);
    if (momentumMagnitude <= 0.0 || !std::isfinite(theta)) return;

    const double charge = pdgId == 13 ? -1.0 : 1.0;
    const Trk::PerigeeSurface perigeeSurface{Amg::Vector3D{0.0, 0.0, 0.0}};
    const Trk::Perigee perigee{0.0, 0.0, phi, theta,
                               charge / momentumMagnitude, perigeeSurface};

    StationPositions positions{};
    std::uint8_t extrapolatedStationMask{0U};
    for (std::size_t station = 0U; station < positions.size(); ++station) {
      const double z = eta >= 0.0 ? stationAbsZ[station] : -stationAbsZ[station];
      Amg::Transform3D transform = Amg::Transform3D::Identity();
      transform.translation().z() = z;
      const Trk::DiscSurface disc{transform, m_config.minR, m_config.maxR};
      const Trk::BoundaryCheck boundaryCheck{true};
      const auto extrapolated =
          extrapolator.extrapolate(ctx, perigee, disc, Trk::alongMomentum,
                                   boundaryCheck, Trk::muon);
      if (!extrapolated) continue;
      const Amg::Vector3D& position = extrapolated->position();
      if (!std::isfinite(position.eta()) || !std::isfinite(position.phi()) ||
          std::abs(position.z() - z) > 20.0) {
        continue;
      }
      positions[station] = StationPosition{
          true, static_cast<float>(position.eta()),
          static_cast<float>(xAOD::P4Helpers::deltaPhi(position.phi(), 0.))};
      extrapolatedStationMask |= stationBit(station);
    }

    if (extrapolatedStationMask == 0x7U) {
      ++summary.nFullyExtrapolatedTruthMuons;
    }

    int nearestCandidate{-1};
    float nearestResidual{invalidResidual};
    bool matched{false};
    for (std::size_t index = 0U; index < candidates.size(); ++index) {
      const float residual = meanDeltaR(candidates[index], positions);
      if (residual < nearestResidual) {
        nearestResidual = residual;
        nearestCandidate = static_cast<int>(index);
      }
      if (residual <= m_config.maxMeanDeltaR) {
        matched = true;
        candidateMatched[index] = true;
      }
    }

    if (!matched) return;
    ++summary.nTruthMuonsWithMatchedCandidate;
    if (nearestCandidate >= 0) {
      const std::uint8_t mask =
          candidates[static_cast<std::size_t>(nearestCandidate)]
              .positionStationMask;
      if (mask < summary.nNearestMatchedByPositionMask.size()) {
        ++summary.nNearestMatchedByPositionMask[mask];
      }
    }
  };

  for (const HepMC::GenEvent* event : truthEvents) {
    if (event == nullptr) continue;
#if __has_include("HepMC3/GenEvent.h")
    for (const auto& particle : event->particles()) processParticle(particle);
#else
    for (auto particle = event->particles_begin();
         particle != event->particles_end(); ++particle) {
      processParticle(*particle);
    }
#endif
  }

  summary.nUnmatchedCandidates = static_cast<std::size_t>(
      std::count(candidateMatched.begin(), candidateMatched.end(), false));
  return StatusCode::SUCCESS;
}

}  // namespace TgcL0Floating
}  // namespace L0Muon
