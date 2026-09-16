/**
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Analysis/src/ExpertTrackTimeFromSummaryTool.cxx
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 * @date Feb, 2023
 * @brief
 */

#include "ExpertTrackTimeFromSummaryTool.h"

#include <algorithm>
#include <bit>

using namespace HGTD;

ExpertTrackTimeFromSummaryTool::ExpertTrackTimeFromSummaryTool(
    const std::string& t, const std::string& n, const IInterface* p)
    : base_class(t, n, p) {}

StatusCode ExpertTrackTimeFromSummaryTool::initialize() {
  ATH_CHECK(AthAlgTool::initialize());
  return StatusCode::SUCCESS;
}

bool ExpertTrackTimeFromSummaryTool::expertHasTime(
    const xAOD::TrackParticle& track_particle) const {
  // The validTime decoration checks for time consistency and
  // extensions with chi2 bellow threashold
  if (m_acc_has_valid_time(track_particle) != 1) {
    return false;
  }

  if (m_do_min_nhits and (this->nHits(track_particle) <= 1)) {
    // if a 2 hit minimum is required, reject the case of a single associated
    // hit if the track falls into the defined eta region
    float fabs_eta = std::abs(track_particle.eta());
    if (fabs_eta > m_min_eta and fabs_eta < m_max_eta) {
      return false;
    }
  }

  // Check if the last measurement of ITk track is close to HGTD
  bool last_hit_not_on_last_surface =
      m_acc_summary_info(track_particle) & (1 << m_holes_ptrn_sft);
  if (m_do_last_hit and last_hit_not_on_last_surface) {
    return false;
  }

  return true;
}

float ExpertTrackTimeFromSummaryTool::expertTime(
    const xAOD::TrackParticle& track_particle) const {
  return track_particle.time();
}

float ExpertTrackTimeFromSummaryTool::expertTimeRes(
    const xAOD::TrackParticle& track_particle) const {
  return m_acc_time_resolution(track_particle);
}

int ExpertTrackTimeFromSummaryTool::nHits(
    const xAOD::TrackParticle& track_particle) const {
  // Get 4-bit word that indicate valid hits after time consistency check
  uint8_t n_comp =
      (m_acc_summary_info(track_particle) >> m_comp_ptrn_sft) & 0x0F;
  return std::popcount(n_comp);
}

int ExpertTrackTimeFromSummaryTool::nPrimaryHits(
    const xAOD::TrackParticle& track_particle) const {
  // Get 4-bit word that indicate hits associated with primary particles
  uint8_t n_primes =
      (m_acc_summary_info(track_particle) >> m_primes_ptrn_sft) & 0x0F;
  return std::popcount(n_primes);
}

float ExpertTrackTimeFromSummaryTool::fracPrimaryHits(
    const xAOD::TrackParticle& track_particle) const {
  if (not expertHasTime(track_particle)) {
    ATH_MSG_WARNING("[ExpertTrackTimeFromSummaryTool::fracPrimaryHits]"
                    "No available hits, returning -999.");
    return -999.;
  }
  short n_hits = this->nHits(track_particle);
  if (n_hits > 0) {
    return (float)this->nPrimaryHits(track_particle) / (float)n_hits;
  } else {
    return 0;
  }
}

int ExpertTrackTimeFromSummaryTool::numberPotentialPrimaryHits(
    const xAOD::TrackParticle& track_particle) const {
  const std::vector<bool>& expected_hits =
      m_acc_perLayer_expectCluster(track_particle);
  return std::count(expected_hits.begin(), expected_hits.end(), true);
}
