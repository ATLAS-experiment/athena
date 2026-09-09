/**
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Analysis/src/HGTD_TrkTimePerformanceStudies.h
 *
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 *
 * @brief Algorithm comparing the HGTD track-time working points provided by
 *  the IHGTD_ExpertTrackTimeAccessor implementations against truth, for a set
 *  of track selections.
 */

#ifndef HGTD_ANALYSIS_TRKTIMEPERFORMANCESTUDIES_H
#define HGTD_ANALYSIS_TRKTIMEPERFORMANCESTUDIES_H

#include "HGTD_AnalysisAlgBase.h"

#include "GaudiKernel/ToolHandle.h"
#include "HGTD_Analysis/IHGTD_ExpertTrackTimeAccessor.h"
#include "HGTD_Analysis/IHGTD_TrackSelectionTool.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTruth/TruthEventContainer.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthPileupEventContainer.h"
#include "xAODTruth/TruthVertex.h"
#include "xAODTruth/xAODTruthHelpers.h"

#include "TEfficiency.h"
#include "TH1F.h"

#include <string>
#include <vector>

class HGTD_TrkTimePerformanceStudies : public HGTD_AnalysisAlgBase {

public:
  HGTD_TrkTimePerformanceStudies(const std::string& name,
                                 ISvcLocator* svc_locator);

  virtual ~HGTD_TrkTimePerformanceStudies();

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) override;

private:
  enum PrimesFractions {
    AllPrimes,
    HalfPrimesHasPrimes,
    LessThanHalfPrimes,
    MoreThanHalfPrimes,
    NoPrimesNoPossiblePrimes,
    NoPrimes1PossiblePrimes,
    NoPrimes2PossiblePrimes,
    NoPrimes3PossiblePrimes,
    NoPrimes4PossiblePrimes
  };

  ToolHandleArray<IHGTD_ExpertTrackTimeAccessor> m_track_time_tools{
      this,
      "TrackTimeTools",
      {},
      "Tools to retrieve a specified track-time working point"};

  ToolHandleArray<IHGTD_TrackSelectionTool> m_track_sel_tools{
      this,
      "TrackSelectionTools",
      {},
      "Tools to retrieve a HGTD specified track selection"};

  SG::ReadHandleKey<xAOD::TrackParticleContainer> m_track_particles_key{
      this, "TrackParticleContainerName", "InDetTrackParticles",
      "Name of the track particle container that should be retrieved in the "
      "execute"};

  SG::ReadHandleKey<xAOD::TruthEventContainer> m_truth_event_container_key{
      this, "TruthEventsContainerName", "TruthEvents",
      "Name of the truth event container that should be retrieved"};

  SG::ReadHandleKey<xAOD::TruthPileupEventContainer>
      m_pileup_truth_container_key{
          this, "TruthPileupEventsContainerName", "TruthPileupEvents",
          "Name of the pileup truth event container that should be retrieved"};

  /// Histogram-name suffixes, indexed by PrimesFractions.
  const std::vector<std::string> m_primes_fractions = {
      "AllPrimes",
      "HalfPrimesHasPrimes",
      "LessThanHalfPrimes",
      "MoreThanHalfPrimes",
      "NoPrimesNoPossiblePrimes",
      "NoPrimes1PossiblePrimes",
      "NoPrimes2PossiblePrimes",
      "NoPrimes3PossiblePrimes",
      "NoPrimes4PossiblePrimes"};

  const xAOD::TruthVertex*
  getTruthVertex(const xAOD::TruthParticle* truth_particle);
};

#endif // HGTD_ANALYSIS_TRKTIMEPERFORMANCESTUDIES_H
