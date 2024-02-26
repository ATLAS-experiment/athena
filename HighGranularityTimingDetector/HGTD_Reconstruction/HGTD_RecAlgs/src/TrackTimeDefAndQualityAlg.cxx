/**
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_RecAlgs/src/TrackTimeDefAndQualityAlg.cxx
 * @author Valentina Raskina <valentina.raskina@cern.ch>
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 * @date December, 2022
 * @brief
 */

#include "TrackTimeDefAndQualityAlg.h"

#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"

namespace HGTD {

TrackTimeDefAndQualityAlg::TrackTimeDefAndQualityAlg(const std::string& name,
                                                     ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode TrackTimeDefAndQualityAlg::initialize() {

  ATH_CHECK(m_trackParticleContainerKey.initialize());
  ATH_CHECK(m_layerHasExtensionKey.initialize());
  ATH_CHECK(m_layerClusterTimeKey.initialize());
  ATH_CHECK(m_layerClusterTruthClassKey.initialize());
  ATH_CHECK(m_time_dec_key.initialize());
  ATH_CHECK(m_time_res_dec_key.initialize());
  ATH_CHECK(m_hasValidTime_dec_key.initialize());
  ATH_CHECK(m_summarypattern_dec_key.initialize());

  return StatusCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

StatusCode TrackTimeDefAndQualityAlg::execute(const EventContext& ctx) const {

  SG::ReadHandle<xAOD::TrackParticleContainer> trk_ptkl_container_handle(
      m_trackParticleContainerKey, ctx);
  const xAOD::TrackParticleContainer* track_particles =
      trk_ptkl_container_handle.cptr();
  if (not track_particles) {
    ATH_MSG_ERROR(
        "[TrackTimeDefAndQualityAlg] TrackParticleContainer not found, "
        "aborting execute!");
    return StatusCode::FAILURE;
  }

  SG::WriteDecorHandle<xAOD::TrackParticleContainer, float> time_handle(
      m_time_dec_key, ctx);
  SG::WriteDecorHandle<xAOD::TrackParticleContainer, float> timeres_handle(
      m_time_res_dec_key, ctx);
  SG::WriteDecorHandle<xAOD::TrackParticleContainer, uint8_t> hasValidTime_handle(
      m_hasValidTime_dec_key, ctx);
  SG::WriteDecorHandle<xAOD::TrackParticleContainer, uint32_t> summary_handle(
      m_summarypattern_dec_key, ctx);

  for (const auto* track_ptkl : *track_particles) {

    // runs the time consistency checks
    // if no hits are found in HGTD, returns a default time
    CleaningResult res = runTimeConsistencyCuts(track_ptkl);

    // check if the last hit on track was within the predefined area
    if (lastHitIsOnLastSurface(*track_ptkl)) {
      res.m_field |= (0b0000 << m_holes_ptrn_sft);
    } else {
      res.m_field |= (0b0001 << m_holes_ptrn_sft);
    }

    // keep which of the hits associated in reco were primary hits (truth info!)
    short prime_pattern = 0x0;
    for (short i = 0; i < n_hgtd_layers; i++) {
      if (res.m_hits.at(i).m_isprime) {
        prime_pattern |= (1 << i);
      }
    }
    res.m_field |= (prime_pattern << m_primes_ptrn_sft);

    // decorate the track again with this info
    time_handle(*track_ptkl) = res.m_time;
    timeres_handle(*track_ptkl) = res.m_resolution;
    hasValidTime_handle(*track_ptkl) = res.m_hasValidTime;
    summary_handle(*track_ptkl) = res.m_field;
  }
  return StatusCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

TrackTimeDefAndQualityAlg::CleaningResult
TrackTimeDefAndQualityAlg::runTimeConsistencyCuts(
    const xAOD::TrackParticle* track_particle) const {

  // get all available hits (see the struct Hit) in a first step
  std::array<Hit, n_hgtd_layers> valid_hits = getValidHits(track_particle);

  CleaningResult result;
  result.m_hits = valid_hits;
  result.m_field = 0x0;
  result.m_time = m_default_time;
  result.m_resolution = m_default_time_res;
  result.m_hasValidTime = 0;

  short recoed_pattern = getValidPattern(valid_hits);
  // stored the pattern of hits as retrieved from the iterative extension
  result.m_field |= (recoed_pattern << m_recoed_ptrn_sft);

  short nhits = std::count_if(valid_hits.begin(), valid_hits.end(),
                              [](const Hit& hit) { return hit.m_isvalid; });
  if (nhits < 2) {
    // fill the patern with the 1 hit (or none) and return
    result.m_field |= (recoed_pattern << m_comp_ptrn_sft);
    result.m_time = meanTime(valid_hits);
    result.m_resolution = trackTimeResolution(valid_hits);
    result.m_hasValidTime = recoed_pattern ? 1 : 0;
    return result;
  } else if (nhits == 2) {
    // if the deltaT cut is  passed, the pattern stays the same, otherwise set
    // to 0 as no hit passes
    // TODO: find better way to treat this!
    if (passesDeltaT(valid_hits)) {
      result.m_field |= (recoed_pattern << m_comp_ptrn_sft); // stays the same
      result.m_time = meanTime(valid_hits);
      result.m_resolution = trackTimeResolution(valid_hits);
      result.m_hasValidTime = 1;
      return result;
    } else {
      result.m_field |= (0b0000 << m_comp_ptrn_sft); // no hit passes
      result.m_time = m_default_time; // TODO should I just use the mean?
      result.m_resolution = m_default_time_res;
      return result;
    }

  } else {
    // for 3 or 4 hits, remove hit(s) with worst chi2 if needed
    float chi2 = calculateChi2(valid_hits);
    // if the chi2 is below the threshold, keep all hits
    bool searching = chi2 > m_chi2_threshold;
    while (searching) {
      short remove_layer = findLayerWithBadChi2(valid_hits);
      setLayerAsInvalid(valid_hits, remove_layer);
      float new_chi2 = calculateChi2(valid_hits);
      nhits = std::count_if(valid_hits.begin(), valid_hits.end(),
                            [](const Hit& hit) { return hit.m_isvalid; });
      if (new_chi2 <= m_chi2_threshold or nhits < 3) {
        searching = false;
      }
    } // while loop ended

    short chi2_rej_pattern = getValidPattern(valid_hits);

    if (nhits == 2) {
      if (passesDeltaT(valid_hits)) {
        result.m_field |= (chi2_rej_pattern << m_comp_ptrn_sft);
        result.m_time = meanTime(valid_hits);
        result.m_resolution = trackTimeResolution(valid_hits);
        result.m_hasValidTime = 1;
        return result;
      } else {
        result.m_field |= (0b0000 << m_comp_ptrn_sft); // no hit passes
        result.m_time = m_default_time; // TODO should I just use the mean?
        result.m_resolution = m_default_time_res;
        return result;
      }
    } else {
      // 3 or 4 hits, chi2 passed
      result.m_field |= (chi2_rej_pattern << m_comp_ptrn_sft);
      result.m_time = meanTime(valid_hits);
      result.m_resolution = trackTimeResolution(valid_hits);
      result.m_hasValidTime = 1;
      return result;
    }
  }
}

std::array<TrackTimeDefAndQualityAlg::Hit, n_hgtd_layers>
TrackTimeDefAndQualityAlg::getValidHits(
    const xAOD::TrackParticle* track_particle) const {

  SG::ReadDecorHandle<xAOD::TrackParticleContainer, std::vector<float>>
      layerClusterTimeHandle(m_layerClusterTimeKey);
  std::vector<float> times = layerClusterTimeHandle(*track_particle);

  SG::ReadDecorHandle<xAOD::TrackParticleContainer, std::vector<bool>>
      layerHasExtensionHandle(m_layerHasExtensionKey);
  std::vector<bool> has_clusters = layerHasExtensionHandle(*track_particle);

  SG::ReadDecorHandle<xAOD::TrackParticleContainer, std::vector<int>>
      layerClusterTruthClassHandle(m_layerClusterTruthClassKey);
  std::vector<int> hit_classification =
      layerClusterTruthClassHandle(*track_particle);

  std::array<Hit, n_hgtd_layers> valid_hits;

  for (size_t i = 0; i < n_hgtd_layers; i++) {
    Hit newhit;
    if (has_clusters.at(i)) {
      newhit.m_time = times.at(i);
      newhit.m_isprime = hit_classification.at(i) == 1;
      newhit.m_isvalid = true;
    }
    newhit.m_layer = i;
    valid_hits.at(i) = newhit;
  }

  return valid_hits;
}

short TrackTimeDefAndQualityAlg::getValidPattern(
    const std::array<TrackTimeDefAndQualityAlg::Hit, n_hgtd_layers>& hits)
    const {
  short pattern = 0x0;
  for (short i = 0; i < n_hgtd_layers; i++) {
    if (hits.at(i).m_isvalid) {
      pattern |= (1 << i);
    }
  }
  return pattern;
}

float TrackTimeDefAndQualityAlg::calculateChi2(
    const std::array<Hit, n_hgtd_layers>& hits) const {

  float mean = meanTime(hits);

  float chi2 = 0.;
  for (const auto& hit : hits) {
    if (hit.m_isvalid) {
      chi2 += (hit.m_time - mean) * (hit.m_time - mean) /
              (hit.m_resolution * hit.m_resolution);
    }
  }
  return chi2;
}

bool TrackTimeDefAndQualityAlg::passesDeltaT(
    const std::array<TrackTimeDefAndQualityAlg::Hit, n_hgtd_layers>& hits)
    const {
  // don't trust the user here.
  short n_valid = std::count_if(hits.begin(), hits.end(),
                                [](const Hit& hit) { return hit.m_isvalid; });
  if (n_valid != 2) {
    return false;
  }
  // FIXME this should be doable in a simpler manner...
  std::vector<float> times;
  std::vector<float> res;
  for (const auto& hit : hits) {
    if (hit.m_isvalid) {
      times.push_back(hit.m_time);
      res.push_back(hit.m_resolution);
    }
  }
  // pass if the distance in units of the resolution passes the cut
  return std::abs(times.at(0) - times.at(1)) <
         m_deltat_cut * hypot(res.at(0), res.at(1));
}

float TrackTimeDefAndQualityAlg::meanTime(
    const std::array<TrackTimeDefAndQualityAlg::Hit, n_hgtd_layers>& hits)
    const {
  float sum = 0.;
  short n = 0;
  for (const auto& hit : hits) {
    if (hit.m_isvalid) {
      sum += hit.m_time;
      n++;
    }
  }
  return n == 0 ? m_default_time.value() : sum / (float)n;
}

float TrackTimeDefAndQualityAlg::trackTimeResolution(
    const std::array<TrackTimeDefAndQualityAlg::Hit, n_hgtd_layers>& hits)
    const {

  float sum = 0.;
  for (const auto& hit : hits) {
    if (hit.m_isvalid) {
      sum += 1. / (hit.m_resolution * hit.m_resolution);
    }
  }
  return sum == 0. ? m_default_time_res.value()
                   : static_cast<float>(std::sqrt(1. / sum));
}

short TrackTimeDefAndQualityAlg::findLayerWithBadChi2(
    std::array<TrackTimeDefAndQualityAlg::Hit, n_hgtd_layers> hits) const {
  short remove_layer = -1;
  float local_min_chi2 = 999999;
  for (auto& hit : hits) {
    // "turn off" hits one after the other to test their impact on the chi2
    bool validbuff = hit.m_isvalid;
    hit.m_isvalid = false;
    float local_chi2 = calculateChi2(hits);
    hit.m_isvalid = validbuff;
    if (local_chi2 < local_min_chi2) {
      local_min_chi2 = local_chi2;
      remove_layer = hit.m_layer;
    }
  }
  return remove_layer;
}

void TrackTimeDefAndQualityAlg::setLayerAsInvalid(
    std::array<TrackTimeDefAndQualityAlg::Hit, n_hgtd_layers>& hits,
    short layer) const {
  for (auto& hit : hits) {
    if (hit.m_layer == layer) {
      hit.m_isvalid = false;
    }
  }
}

const Trk::TrackParameters*
TrackTimeDefAndQualityAlg::getLastHitOnTrack(const Trk::Track& track) const {

  const Trk::TrackStates* tsos =
      track.trackStateOnSurfaces();
  if (not tsos) {
    ATH_MSG_ERROR("Failed to retrieve track state on surfaces");
    return nullptr;
  }
  // loop over the associated hits in ITk in reverse order, since we want to
  // select the one closest to HGTD to start the extrapolation
  for (auto i = tsos->rbegin(); i != tsos->rend(); ++i) {
    const auto* curr_last_tsos = *i;
    if (not curr_last_tsos) {
      continue;
    }
    if (curr_last_tsos->type(Trk::TrackStateOnSurface::Measurement) and
        curr_last_tsos->trackParameters() and
        curr_last_tsos->measurementOnTrack()) {
      return curr_last_tsos->trackParameters();
    }
  }
  return nullptr;
}

bool TrackTimeDefAndQualityAlg::lastHitIsOnLastSurface(
    const xAOD::TrackParticle& track_particle) const {
  const Trk::Track* track = track_particle.track();
  const Trk::TrackParameters* last_hit_param = getLastHitOnTrack(*track);
  float radius = std::hypot(last_hit_param->position().x(),
                             last_hit_param->position().y());
  float abs_z = std::abs(last_hit_param->position().z());

  if (abs_z > 2700) {
    return true;
  }
  if (radius < 350 and abs_z > 2400) {
    return true;
  }
  // region 2
  if (radius > 205 and radius < 350 and abs_z > 2100) {
    return true;
  }
  // region 3
  if (radius < 220 and abs_z > 2200) {
    return true;
  }

  if (radius < 140 and abs_z > 1890) {
    return true;
  }

  return false;
}

} // namespace HGTD
