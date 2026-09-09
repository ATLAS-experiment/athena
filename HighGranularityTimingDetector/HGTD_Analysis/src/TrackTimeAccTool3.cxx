/**
 * Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Analysis/src/TrackTimeAccTool3.cxx
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 * @date Feb, 2023
 * @brief
 */

#include "TrackTimeAccTool3.h"

#include <numeric>

using namespace HGTD;

TrackTimeAccTool3::TrackTimeAccTool3(const std::string& t, const std::string& n,
                                    const IInterface* p)
    : AthAlgTool(t, n, p) {}

StatusCode TrackTimeAccTool3::initialize() {
  StatusCode sc = AthAlgTool::initialize();

  m_dec_perLayer_expectCluster =
  std::make_unique<SG::AuxElement::Accessor<std::vector<bool>>>(
      "HGTD_primary_expected");

  m_hasValidTime = 
  std::make_unique<SG::AuxElement::Accessor<uint8_t>>("hasValidTime");

  m_summaryInfo =
  std::make_unique<SG::AuxElement::Accessor<uint32_t>>("HGTD_summaryinfo");

  m_timeResolution =
  std::make_unique<SG::AuxElement::Accessor<float>>("timeResolution");

  return sc;
}

bool TrackTimeAccTool3::hasTime(const xAOD::TrackParticle& track_particle) {
  bool hasTime = false;
  // The validTime decoration checks for time consistency and
  // extensions with chi2 bellow threashold  
  if(m_hasValidTime->operator()(track_particle) == 1){
    hasTime = true;
    if (m_do_min_nhits and (this->nHits(track_particle) <= 1)) {
      // if a 2 hit minimum is required, reject the case of a single associated
      // hit if the track falls into the defined eta region
      float fabs_eta = std::abs(track_particle.eta());
      if (fabs_eta > m_min_eta and fabs_eta < m_max_eta) {
        hasTime = false;        
      }
    }
    // Check if the last measurement of ITk track is close to HGTD
    bool lastHitNotOnLastSurface = m_summaryInfo->operator()(track_particle) & (1 << m_holes_ptrn_sft); 
    if (m_do_last_hit and lastHitNotOnLastSurface) {
      hasTime = false;
    }
  }
  
  return hasTime;
}

float TrackTimeAccTool3::time(const xAOD::TrackParticle& track_particle) {
  return track_particle.time();
}

float TrackTimeAccTool3::timeRes(const xAOD::TrackParticle& track_particle) {
  return m_timeResolution->operator()(track_particle);
}

int TrackTimeAccTool3::nHits(const xAOD::TrackParticle& track_particle) {
  //Get 4-bit word that indicate valid hits after time consistency check 
  uint8_t n_comp = (m_summaryInfo->operator()(track_particle) >> m_comp_ptrn_sft) & 0x0F;
  return std::popcount(n_comp);
}

int TrackTimeAccTool3::nPrimaryHits(const xAOD::TrackParticle& track_particle) {
  //Get 4-bit word that indicate hits associated with primary particles
  uint8_t n_primes = (m_summaryInfo->operator()(track_particle) >> m_primes_ptrn_sft) & 0x0F;
  return std::popcount(n_primes);
}

float TrackTimeAccTool3::fracPrimaryHits(const xAOD::TrackParticle& track_particle) {
  if (not hasTime(track_particle)) {
    ATH_MSG_WARNING("[TrackTimeAccTool3::fracPrimaryHits]"
                    "No available hits, returning -999.");
    return -999.;
  }
  short n_hits = this->nHits(track_particle);
  if(n_hits > 0) {
    return (float) this->nPrimaryHits(track_particle)/(float) n_hits;
  }
  else{
    return 0;
  }

}

int TrackTimeAccTool3::numberPotentialPrimaryHits(const xAOD::TrackParticle& track_particle) {
  if (not m_dec_perLayer_expectCluster->isAvailable(track_particle)) {
    ATH_MSG_WARNING("[TrackTimeAccTool3::numberPotentialPrimaryHits]"
                    "Expected clusters not available, returning 0\n");
    return 0;
  }
  auto expected_hits = m_dec_perLayer_expectCluster->operator()(track_particle);
  return std::count(expected_hits.begin(), expected_hits.end(), true);
}

