/**
 * Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Analysis/src/TrackTimeAccTool.h
 *
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 *
 * @date April, 2022
 *
 * @brief
 *
 * TODOs:
 * - values in lastHitIsOnLastSurface need to be confirmed for 21.9
 * - the 2-hit cut in the problematic PP1 region needs to be re-evaluated
 * - test if in the 1-hit case, a check of the spatial chi2 might help
 */

#ifndef HGTD_TRACKTIMEACCTOOL_H
#define HGTD_TRACKTIMEACCTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "HGTD_Analysis/IHGTD_TrackTimeAccessor.h"

#include <string>
#include <vector>

#include "TVector3.h"

namespace HGTD {

class TrackTimeAccTool : public extends<AthAlgTool, IHGTD_TrackTimeAccessor> {

  struct Hit {
    float time;
    float resolution;
    bool isprime;
    TVector3 position;
  };

  using HitVec_t = std::vector<Hit>;
  using FloatVec_t = std::vector<float>;

public:
  TrackTimeAccTool(const std::string&, const std::string&, const IInterface*);

  virtual ~TrackTimeAccTool() = default;

  //////////////////////////////////////////////////////////////////////////////
  // for AthAlgTool interface
  virtual StatusCode initialize() override final;

  // for IHGTD_TrackTimeAccessor interface

  virtual bool
  hasTime(const xAOD::TrackParticle& track_particle) override final;

  virtual float time(const xAOD::TrackParticle& track_particle) override final;

  virtual float
  timeRes(const xAOD::TrackParticle& track_particle) override final;

  virtual int nHits(const xAOD::TrackParticle& track_particle) override final;

  virtual std::string toolName() override final { return "TrackTimeAccTool"; };

  //////////////////////////////////////////////////////////////////////////////

  int nPrimaryHits(const xAOD::TrackParticle& track_particle);

  float
  fracPrimaryHits(const xAOD::TrackParticle& track_particle) override final;

  virtual int numberPotentialPrimaryHits(
      const xAOD::TrackParticle& track_particle) override final;

private:
  /**
   * @brief Retrieve the hit information from the decorators and build a vector
   * of hits.
   */
  HitVec_t getValidHits(const xAOD::TrackParticle& track_particle);

  /**
   * @brief Retrieve the hit information from the decorators and build a vector
   * of hits for those hits that survive the time compatibility checks.
   */
  HitVec_t getTimeCompatibleHits(const xAOD::TrackParticle& track_particle);

  /**
   * @brief In the samples used for HGTD studies the last hit on track in ITk is
   * written to file and can be used for the cleaning procedure.
   * It is returned as a TVector3 object.
   */
  TVector3 getLastMeasurement(const xAOD::TrackParticle& track);

  /**
   * @brief Check wheather the last hit on track is on a surface close to HGTD.
   * The hardcoded values correspond to ITk Step3.1 layout and define where in
   * z and r the last hit position is accepted as close enough to HGTD.
   */
  bool lastHitIsOnLastSurface(const xAOD::TrackParticle& track);

  float calculateMean(const std::vector<float>& vals);

  /**
   * @brief Returns the arithmetic average mean of the hit times.
   */
  float calculateMean(const HitVec_t& hits);

  /**
   * @brief Tests if a given set of exactly 2 Hit objects passes the set time
   * difference cut, which is defined in units of the combined resolution
   */
  bool passesDeltaT(const HitVec_t& hits);

  // std::vector<float>
  // getAcceptedTimes(const xAOD::TrackParticle &track_particle);

  float calculateChi2(const std::vector<float>& vals);
  float calculateChi2(const HitVec_t& vals);

  float calculateTrackResolution(const HitVec_t& vals);

  int numberOfPrimaryHits(const HitVec_t& vals);

  Gaudi::Property<bool> m_do_last_hit{this, "UseLastHitCut", true,
                                      "Default value"};
  Gaudi::Property<bool> m_do_time_cons{this, "UseTimeConsistency", true,
                                       "Default value"};
  Gaudi::Property<float> m_deltat_cut{this, "DeltaTCut", 2.0, "Default value"};
  Gaudi::Property<float> m_chi2_threshold{this, "TimeChi2Cut", 1.5,
                                          "Default value"};
  Gaudi::Property<bool> m_do_min_nhits{this, "UseMinNHits", true,
                                       "Default value"};
  Gaudi::Property<float> m_min_eta{this, "MinEta", 3.5, "Default value"};
  Gaudi::Property<float> m_max_eta{this, "MaxEta", 3.9, "Default value"};
  Gaudi::Property<bool> m_do_smearing{this, "DoSmearing", false,
                                      "Default value"};

  // looks like HGTD_PerformanceStudies.TrackTimeAccTool_isset, but
  // can't have '.'
  std::string m_name;

  // FIXME
  // TrackTimeResolution m_track_time_resolution;

  std::unique_ptr<SG::AuxElement::Decorator<bool>> m_dec_isset;
  std::unique_ptr<SG::AuxElement::Decorator<bool>> m_dec_hastime;
  std::unique_ptr<SG::AuxElement::Decorator<float>> m_dec_time;
  std::unique_ptr<SG::AuxElement::Decorator<int>> m_dec_nhits;
  std::unique_ptr<SG::AuxElement::Decorator<int>> m_dec_nprimehits;
  std::unique_ptr<SG::AuxElement::Decorator<float>> m_dec_resolution;

  std::unique_ptr<SG::AuxElement::Accessor<bool>> m_acc_isset;
  std::unique_ptr<SG::AuxElement::Accessor<bool>> m_acc_hastime;
  std::unique_ptr<SG::AuxElement::Accessor<float>> m_acc_time;
  std::unique_ptr<SG::AuxElement::Accessor<int>> m_acc_nhits;
  std::unique_ptr<SG::AuxElement::Accessor<int>> m_acc_nprimehits;
  std::unique_ptr<SG::AuxElement::Accessor<float>> m_acc_resolution;

  std::unique_ptr<SG::AuxElement::Accessor<std::vector<bool>>>
      m_dec_perLayer_hasCluster;
  std::unique_ptr<SG::AuxElement::Accessor<std::vector<float>>>
      m_dec_perLayer_clusterChi2;
  std::unique_ptr<SG::AuxElement::Accessor<std::vector<float>>>
      m_dec_perLayer_clusterDeltaT;
  std::unique_ptr<SG::AuxElement::Accessor<std::vector<int>>>
      m_dec_perLayer_clusterTruthClassification;
  std::unique_ptr<SG::AuxElement::Accessor<std::vector<bool>>>
      m_dec_perLayer_expectCluster;
};

} // namespace HGTD

#endif // HGTD_TRACKTIMEACCTOOL_H
