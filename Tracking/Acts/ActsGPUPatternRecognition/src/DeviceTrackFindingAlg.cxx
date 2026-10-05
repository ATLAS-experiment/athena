/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "DeviceTrackFindingAlg.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

// traccc EDM
#include "traccc/edm/track_parameters.hpp"
#include "traccc/edm/track_container.hpp"

// vecmem
#include "vecmem/memory/memory_resource.hpp"

template <typename scalar_t>
using unit = detray::unit<scalar_t>;

namespace ActsTrk {

// -----------------------------------------------------------------------
StatusCode DeviceTrackFindingAlg::initialize()
{
  ATH_MSG_DEBUG("Initializing " << name());

  ATH_CHECK(m_trkFindingAlgProviderTool.retrieve());
  
  ATH_CHECK(m_inputMeasKey.initialize());
  ATH_CHECK(m_inputTrkParamKey.initialize());
  ATH_CHECK(m_outputTracksKey.initialize());

  ATH_CHECK(detStore()->retrieve(m_deviceMagField, m_inputMagFieldKey.value()));
  ATH_CHECK(detStore()->retrieve(m_deviceDetector, m_deviceDetectorObjectName.value()));

  ATH_CHECK(configureTrackFinding());

  ATH_MSG_DEBUG("Successfully initialized");
  return StatusCode::SUCCESS;
}

StatusCode DeviceTrackFindingAlg::configureTrackFinding()
{

  ATH_MSG_INFO("Setting up configs");

  m_finding_cfg.max_num_branches_per_seed = m_maxNumBranchesPerSeed;
  m_finding_cfg.max_num_branches_per_surface = m_maxNumBranchesPerSurface;
  m_finding_cfg.min_track_candidates_per_track = m_minTrackCandidatesPerTrack;
  m_finding_cfg.max_track_candidates_per_track = m_maxTrackCandidatesPerTrack;
  m_finding_cfg.min_step_length_for_next_surface =
      m_minStepLengthForNextSurface * unit<float>::mm;
  m_finding_cfg.max_step_counts_for_next_surface = m_maxStepCountsForNextSurface;
  m_finding_cfg.chi2_max = m_chi2Max;
  m_finding_cfg.max_num_skipping_per_cand = m_maxNumSkippingPerCand;
  m_finding_cfg.max_num_consecutive_skipped = m_maxNumConsecutiveSkipped;
  m_finding_cfg.min_pT = m_minPt * unit<float>::MeV;
  m_finding_cfg.min_p = m_minP * unit<float>::MeV;

  m_finding_cfg.propagation.stepping.min_stepsize = 1e-4f * unit<float>::mm;
  m_finding_cfg.propagation.stepping.rk_error_tol = 1e-4f * unit<float>::mm;
  m_finding_cfg.propagation.stepping.step_constraint =
      std::numeric_limits<float>::max();
  m_finding_cfg.propagation.stepping.path_limit = 5.f * unit<float>::m;
  m_finding_cfg.propagation.stepping.max_rk_updates = 10000u;
  m_finding_cfg.propagation.stepping.use_mean_loss = true;
  m_finding_cfg.propagation.stepping.use_eloss_gradient = false;
  m_finding_cfg.propagation.stepping.use_field_gradient = false;
  m_finding_cfg.propagation.stepping.do_covariance_transport = true;
  m_finding_cfg.propagation.navigation.intersection.overstep_tolerance =
      -300.f * unit<float>::um;
  m_finding_cfg.propagation.navigation.search_window = {20u, 20u};    

  m_finding_cfg.max_num_tracks_per_measurement = m_maxNumTracksPerMeasurement;
  m_finding_cfg.initial_links_per_seed = m_initialLinksPerSeed;

  return StatusCode::SUCCESS;

}

StatusCode DeviceTrackFindingAlg::execute(const EventContext& ctx) const
{
  ATH_MSG_DEBUG("Executing device track finding.");

  // ---- 1. Read traccc input from StoreGate --------------------------------
  auto inputTracccMeasurements = SG::makeHandle(m_inputMeasKey, ctx);
  ATH_CHECK(inputTracccMeasurements.isValid());
  ATH_MSG_DEBUG("Read traccc measurements from '"
                         << inputTracccMeasurements.key() << "'");

  auto inputTracccTrkParam = SG::makeHandle(m_inputTrkParamKey, ctx);
  ATH_CHECK(inputTracccTrkParam.isValid());
  ATH_MSG_DEBUG("Read traccc trk param from '"
                         << inputTracccTrkParam.key() << "'");  

  // ---- 2. Get traccc track finding alg ---------------------------------------------
  auto trkfinding_alg = m_trkFindingAlgProviderTool->getAlgorithm(ctx, m_finding_cfg);
  
  // ---- 3. Run traccc track finding ---------------------------------------------
  auto tracks_container_buffer =  (*trkfinding_alg)(*m_deviceDetector, *m_deviceMagField, *inputTracccMeasurements, *inputTracccTrkParam);
  ATH_MSG_DEBUG("Reconstructed " << trkfinding_alg.copy().get_size(tracks_container_buffer.tracks) << " track parameters.");

  // ---- 4. Write output traccc tracks and track states to StoreGate -------------------------
  auto outputTracccTracks = SG::makeHandle(m_outputTracksKey, ctx);
  ATH_CHECK(outputTracccTracks.record(
    std::make_unique<traccc_track_container::buffer>(
        std::move(tracks_container_buffer))));
  ATH_MSG_DEBUG("Wrote tracks buffer to '" << m_outputTracksKey.key() << "'");

  return StatusCode::SUCCESS;
}



} // namespace ActsTrk