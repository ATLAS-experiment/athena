#ifndef HGTD_PERFORMANCESTUDIES_H
#define HGTD_PERFORMANCESTUDIES_H

#include "HGTD_AnalysisAlgBase.h"

#include "GaudiKernel/ToolHandle.h"
#include "HGTD_Analysis/IHGTD_TrackTimeAccessor.h"
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

class HGTD_TrkTimePerformanceStudies : public HGTD_AnalysisAlgBase {

public:
  HGTD_TrkTimePerformanceStudies(const std::string& name,
                                 ISvcLocator* svc_locator);

  virtual ~HGTD_TrkTimePerformanceStudies();

  virtual StatusCode initialize();
  virtual StatusCode execute(const EventContext& ctx);
  virtual StatusCode finalize();

private:
  static const int m_number_primes_fractions = 9;

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

  ToolHandleArray<IHGTD_TrackTimeAccessor> m_track_time_tools{
      this,
      "TrackTimeTools",
      {},
      "Tools to retrieve a specified track-time working point"};

  ToolHandleArray<IHGTD_TrackSelectionTool> m_track_sel_tools{
      this,
      "TrackSelectionTools",
      {},
      "Tools to retrieve a HGTD specified track selection"};

  //   Gaudi::Property<std::string> m_trkpart_cont_name{
  //       this, "TrackParticleContainerName", "InDetTrackParticles",
  //       "Name of the track particle container that should be retrieved in the
  //       " "execute"};

  //   const xAOD::TrackParticleContainer* m_track_particles = nullptr;

  SG::ReadHandleKey<xAOD::TrackParticleContainer> m_track_particles_key{
      this, "TrackParticleContainerName", "InDetTrackParticles",
      "Name of the track particle container that should be retrieved in the "
      "execute"};

  //   const xAOD::TruthEventContainer* m_truth_event_container = nullptr;
  SG::ReadHandleKey<xAOD::TruthEventContainer> m_truth_event_container_key{
      this, "TruthEventsContainerName", "TruthEvents",
      "Name of the truth event container that should be retrieved"};

  //   const xAOD::TruthPileupEventContainer* m_pileup_truth_container =
  //   nullptr;
  SG::ReadHandleKey<xAOD::TruthPileupEventContainer>
      m_pileup_truth_container_key{
          this, "TruthPileupEventsContainerName", "TruthPileupEvents",
          "Name of the pileup truth event container that should be retrieved"};

  std::vector<std::string> m_primes_fractions = {"AllPrimes",
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

#endif
