/**
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_RecAlgs/src/TimeCompatibilityCheckAlg.cxx
 * @author Valentina Raskina <valentina.raskina@cern.ch>
 * @author
 * @date July, 2022
 * @brief
 */

#include "TimeCompatibilityCheckAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "xAODTruth/TruthParticleContainer.h"
#include <cmath>

namespace HGTD {

TimeCompatibilityCheckAlg::TimeCompatibilityCheckAlg(const std::string& name,   
                                            ISvcLocator* pSvcLocator)
        : AthReentrantAlgorithm(name, pSvcLocator){}

StatusCode TimeCompatibilityCheckAlg::initialize() {

  ATH_CHECK(m_extensionTool.retrieve());
  ATH_CHECK(m_trackParticleContainerKey.initialize());
  ATH_CHECK(m_layerHasExtensionKey.initialize());
  ATH_CHECK(m_layerExtensionChi2Key.initialize());
  ATH_CHECK(m_layerClusterRawTimeKey.initialize());
  ATH_CHECK(m_layerClusterTimeKey.initialize());
  ATH_CHECK(m_layerClusterTruthClassKey.initialize());
  ATH_CHECK(m_layerClusterShadowedKey.initialize());
  ATH_CHECK(m_layerClusterMergedKey.initialize());
  ATH_CHECK(m_layerPrimaryExpectedKey.initialize());
  ATH_CHECK(m_extrapXKey.initialize());
  ATH_CHECK(m_extrapYKey.initialize());
  ATH_CHECK(m_compatibleHitsTimesKey.initialize ());
  ATH_CHECK(m_lastHitInITkCutKey.initialize());
  ATH_CHECK(m_holesHGTDKey.initialize());
  ATH_CHECK(m_holesITkKey.initialize());

  return StatusCode::SUCCESS;
}

 ////////////////////////////////////////////////////////////////////////////////
 ////////////////////////////////////////////////////////////////////////////////

StatusCode TimeCompatibilityCheckAlg::execute(const EventContext& ctx) const {

    SG::ReadHandle<xAOD::TrackParticleContainer> trk_ptkl_container_handle(
        m_trackParticleContainerKey, ctx);
    const xAOD::TrackParticleContainer* track_particles = trk_ptkl_container_handle.cptr();
if (not track_particles) {
    ATH_MSG_ERROR("[TimeCompatibilityCheckAlg] TrackParticleContainer not found, "
                   "aborting execute!");
     return StatusCode::FAILURE;
}

    SG::WriteDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> compatibleHitsTimesHandle(
        m_compatibleHitsTimesKey, ctx);  
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, char> lastHitInITkCutHandle(
        m_lastHitInITkCutKey, ctx);
    SG::WriteDecorHandle<xAOD::TrackParticleContainer, int> holesITkHandle(
        m_holesITkKey, ctx);
        
    SG::ReadDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> layerExtensionChi2Handle(
        m_layerExtensionChi2Key, ctx);
    SG::ReadDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> layerClusterRawTimeHandle(
        m_layerClusterRawTimeKey, ctx);
    SG::ReadDecorHandle<xAOD::TrackParticleContainer, std::vector<int>> layerClusterTruthClassHandle(
        m_layerClusterTruthClassKey, ctx);
    SG::ReadDecorHandle<xAOD::TrackParticleContainer, std::vector<char>> layerClusterShadowedHandle(
        m_layerClusterShadowedKey, ctx);
    SG::ReadDecorHandle<xAOD::TrackParticleContainer, std::vector<char>> layerClusterMergedHandle(
        m_layerClusterMergedKey, ctx);
    SG::ReadDecorHandle<xAOD::TrackParticleContainer, std::vector<char>> layerPrimaryExpectedHandle(
        m_layerPrimaryExpectedKey, ctx);
    SG::ReadDecorHandle<xAOD::TrackParticleContainer, float> extrapXHandle(
        m_extrapXKey, ctx);
    SG::ReadDecorHandle<xAOD::TrackParticleContainer, float> extrapYHandle(
        m_extrapYKey, ctx);

  for (const auto* track_ptkl : *track_particles) {

// Filling here the decoration with times of the compatible hits per track

    for(const auto hit : getTimeCompatibleHits(track_ptkl)){
        compatibleHitsTimesHandle(*track_ptkl).push_back(hit.m_time);
    }
    lastHitInITkCutHandle(*track_ptkl) = lastHitIsOnLastSurface(*track_ptkl);

    holesITkHandle(*track_ptkl) = (m_extensionTool->getHolesITk(ctx, *track_ptkl)).size();


    /**
     * @brief This is a first attempt to design a bitfield with an HGTD track summary 
     * 
     * uint32_t HGTD_track_summary = 0;
     * 
     * Field will contain the following values:
     *            HGTD_acceptance : 1;
     *            HGTD_has_extension : 3; (max 4 extensions)
     *            HGTD_extension_chi2 : 12; (each layer (4) has 1 option (3 bits): <1 / <2 / <3 / <4)
     *            HGTD_number_of_holes : 3; (max 4 holes)
     *            HGTD_hits_compatible_in_time : 3 (max 4)
     *            
     */

  }
  return StatusCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

float TimeCompatibilityCheckAlg::calculateChi2(const std::vector<Hit>& hits) const{

    //Calculation of the mean
    float sum = 0.;
    for (const auto& hit : hits){
        sum += hit.m_time;
    }
    float mean = sum / (float)hits.size();

    //Calculation of the Chi2
    float chi2 = 0.;
    for (size_t i = 0; i < hits.size(); i++) {
        chi2 += (hits.at(i).m_time - mean) * (hits.at(i).m_time - mean) /
                (hits.at(i).m_resolution * hits.at(i).m_resolution);
    }
    return chi2; 
 }

std::vector<TimeCompatibilityCheckAlg::Hit>
TimeCompatibilityCheckAlg::getValidHits(const xAOD::TrackParticle* track_particle) const {

  SG::ReadDecorHandle<xAOD::TrackParticleContainer, std::vector<float>> layerClusterTimeHandle(m_layerClusterTimeKey);
  std::vector<float> times = layerClusterTimeHandle(*track_particle);

  SG::ReadDecorHandle<xAOD::TrackParticleContainer, std::vector<char>> layerHasExtensionHandle(m_layerHasExtensionKey);
  std::vector<char> has_clusters = layerHasExtensionHandle(*track_particle);
    
  std::vector<Hit> valid_hits;
  valid_hits.reserve(4);

for (size_t i = 0; i < has_clusters.size(); i++){
    if (not has_clusters.at(i)) {
       ATH_MSG_DEBUG("[TimeCompatibilityCheckAlg::getValidHits] NO clusters found");
       continue;  
    }
    Hit newhit;

    newhit.m_time = times.at(i);

    valid_hits.push_back(newhit);
}
return valid_hits;

}

bool TimeCompatibilityCheckAlg::passesDeltaT(const std::vector<Hit>& hits) const {
  if (not(hits.size()==2)){
    return false;
  }
// pass if the distance in units of the resolution passes the cut
if (std::fabs(hits.at(0).m_time - hits.at(1).m_time) <
    m_delta_cut * std::hypot(hits.at(0).m_resolution, hits.at(1).m_resolution)) {
        return true;
    }
    return false;
}

std::vector<TimeCompatibilityCheckAlg::Hit> TimeCompatibilityCheckAlg::getTimeCompatibleHits(
    const xAOD::TrackParticle* track_particle) const {
// get all available hits (see the struct Hit) in a first step
auto valid_hits = getValidHits(track_particle);

size_t vts = valid_hits.size();

// if there is only one hit, not time consistency check can be done
if (vts <= 1) {
    return valid_hits;
}

// in case of two hits, check for time consistencyf
if (vts == 2) {
    if (passesDeltaT(valid_hits)) {
        return valid_hits;
    } else {
        // if times are too far away form each other, do not accept time
        return {};
    }
}

// if there are 3 or 4 hits, perfome chi2 outlier removal
// calculate the chi2 value of the available hits in a first step
float chi2 = calculateChi2(valid_hits);

// if the chi2 value does not surpass the set threshold, the hits are accepted as compatible in time
if (chi2 < m_chi2_threshold) {
    return valid_hits;
}

std::vector<Hit> time_candidates_copy = valid_hits; // TODO do I need this copy?
  bool searching = true;
  while (searching) {
    // calculate chi2 contribution of each value
    std::vector<float> chi2_contributions(time_candidates_copy.size(), 0.0);
    for (size_t i = 0; i < time_candidates_copy.size(); i++) {
      std::vector<Hit> buff = time_candidates_copy;
      buff.erase(buff.begin() + i);

      // calculate the chi2 value we would get when removing the i-th hit
      double local_chi2 = calculateChi2(buff);

      chi2_contributions.at(i) = local_chi2;
    }
    // if removing one of the hits gives a much smaller chi2, it should be
    // removed so find the position where the "local chi2" is the smallest, and
    // this is the hit that should be removed (since it gave a big
    // contribution)]

    // find minimum local chi2
    int position = std::distance(
        chi2_contributions.begin(),
        std::min_element(chi2_contributions.begin(), chi2_contributions.end()));

    // and remove it from the hits
    time_candidates_copy.erase(time_candidates_copy.begin() + position);

    // recompute chi2 value
    chi2 = calculateChi2(time_candidates_copy);

    // check for accepted chi2
    if (chi2 < m_chi2_threshold) {
      // if the threshold is now fulfilled, break out of the while loop
      searching = false;
    } 
    // if everything except 2 values has been removed, stop algo
    if (time_candidates_copy.size() == 2) {
      if (passesDeltaT(time_candidates_copy)) {
        return time_candidates_copy;
      } else {
       ATH_MSG_DEBUG("[TimeCompatibilityCheckAlg::getValidHits] times of hits are too far away");
        // if times are too far away, don't accept any TODO maybe accept one,
        // can the spatial chi2 be used?
       return {};
      }
    }
  }
  return time_candidates_copy;
}


const Trk::TrackParameters*
TimeCompatibilityCheckAlg::getLastHitOnTrack(const Trk::Track& track) const {

  const DataVector<const Trk::TrackStateOnSurface>* tsos =
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

bool TimeCompatibilityCheckAlg::lastHitIsOnLastSurface(const xAOD::TrackParticle& track_particle) const {
  const Trk::Track* track = track_particle.track();
  const Trk::TrackParameters* last_hit_param = getLastHitOnTrack(*track);
  double radius = std::hypot(last_hit_param->position().x(), last_hit_param->position().y());
  double abs_z = std::fabs(last_hit_param->position().z());

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