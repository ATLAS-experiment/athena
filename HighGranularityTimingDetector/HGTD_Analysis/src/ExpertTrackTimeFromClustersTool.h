/**
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Analysis/src/ExpertTrackTimeFromClustersTool.h
 *
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 *
 * @date April, 2022
 *
 * @brief Expert-only track-time accessor that re-derives the HGTD track time
 *  from the per-layer cluster decorations (HGTD_has_extension,
 *  HGTD_cluster_time, HGTD_cluster_truth_class, HGTD_primary_expected),
 *  applying its own last-hit, time-consistency and minimum-hit selection.
 *
 *  For expert studies and internal validation only -- see the warning in
 *  HGTD_Analysis/IHGTD_ExpertTrackTimeAccessor.h.
 *
 * TODOs:
 * - values in lastHitIsOnLastSurface need to be confirmed for 21.9
 * - the 2-hit cut in the problematic PP1 region needs to be re-evaluated
 * - test if in the 1-hit case, a check of the spatial chi2 might help
 */

#ifndef HGTD_EXPERTTRACKTIMEFROMCLUSTERSTOOL_H
#define HGTD_EXPERTTRACKTIMEFROMCLUSTERSTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "HGTD_Analysis/IHGTD_ExpertTrackTimeAccessor.h"

#include "AthContainers/ConstAccessor.h"

#include <memory>
#include <string>
#include <vector>

#include "TVector3.h"

namespace HGTD {

class ExpertTrackTimeFromClustersTool
    : public extends<AthAlgTool, IHGTD_ExpertTrackTimeAccessor> {

  struct Hit {
    float time{};
    float resolution{};
    bool isprime{};
    TVector3 position{};
  };

  using HitVec_t = std::vector<Hit>;
  using FloatVec_t = std::vector<float>;

public:
  ExpertTrackTimeFromClustersTool(const std::string&, const std::string&,
                                  const IInterface*);

  virtual ~ExpertTrackTimeFromClustersTool() = default;

  //////////////////////////////////////////////////////////////////////////////
  // for AthAlgTool interface
  virtual StatusCode initialize() override final;

  // for IHGTD_ExpertTrackTimeAccessor interface

  virtual bool
  expertHasTime(const xAOD::TrackParticle& track_particle) const override final;

  virtual float
  expertTime(const xAOD::TrackParticle& track_particle) const override final;

  virtual float
  expertTimeRes(const xAOD::TrackParticle& track_particle) const override final;

  virtual float
  fracPrimaryHits(const xAOD::TrackParticle& track_particle) const override final;

  virtual int numberPotentialPrimaryHits(
      const xAOD::TrackParticle& track_particle) const override final;

  //////////////////////////////////////////////////////////////////////////////

private:
  /**
   * @brief Number of hits that were used to build the track time. Kept private
   * on purpose: a caller that needs it should declare the dependency on the
   * decoration itself, e.g.
   *   SG::ConstAccessor<int> nhit_acc(<toolname> + "_nhits");
   */
  int nHits(const xAOD::TrackParticle& track_particle) const;

  /**
   * @brief Number of hits used for the track time that were left by a primary
   * particle. Private for the same reason as nHits.
   */
  int nPrimaryHits(const xAOD::TrackParticle& track_particle) const;

  /**
   * @brief Retrieve the hit information from the decorators and build a vector
   * of hits.
   */
  HitVec_t getValidHits(const xAOD::TrackParticle& track_particle) const;

  /**
   * @brief Retrieve the hit information from the decorators and build a vector
   * of hits for those hits that survive the time compatibility checks.
   */
  HitVec_t getTimeCompatibleHits(const xAOD::TrackParticle& track_particle) const;

  /**
   * @brief In the samples used for HGTD studies the last hit on track in ITk is
   * written to file and can be used for the cleaning procedure.
   * It is returned as a TVector3 object.
   */
  TVector3 getLastMeasurement(const xAOD::TrackParticle& track) const;

  /**
   * @brief Check wheather the last hit on track is on a surface close to HGTD.
   * The hardcoded values correspond to ITk Step3.1 layout and define where in
   * z and r the last hit position is accepted as close enough to HGTD.
   */
  bool lastHitIsOnLastSurface(const xAOD::TrackParticle& track) const;

  /**
   * @brief Returns the arithmetic average mean of the hit times.
   */
  float calculateMean(const HitVec_t& hits) const;

  /**
   * @brief Tests if a given set of exactly 2 Hit objects passes the set time
   * difference cut, which is defined in units of the combined resolution
   */
  bool passesDeltaT(const HitVec_t& hits) const;

  float calculateChi2(const HitVec_t& vals) const;

  float calculateTrackResolution(const HitVec_t& vals) const;

  int numberOfPrimaryHits(const HitVec_t& vals) const;

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

  /// Prefix of the decorations this tool writes. It is the tool instance name
  /// with '.' replaced by '_', since aux variable names may not contain a '.'
  /// (the instance name of a private tool does, e.g.
  /// HGTD_TrkTimePerformanceStudies.ExpertTrackTimeFromClusters).
  std::string m_dec_prefix;

  std::unique_ptr<SG::AuxElement::Decorator<bool>> m_dec_isset;
  std::unique_ptr<SG::AuxElement::Decorator<bool>> m_dec_hastime;
  std::unique_ptr<SG::AuxElement::Decorator<float>> m_dec_time;
  std::unique_ptr<SG::AuxElement::Decorator<int>> m_dec_nhits;
  std::unique_ptr<SG::AuxElement::Decorator<int>> m_dec_nprimehits;
  std::unique_ptr<SG::AuxElement::Decorator<float>> m_dec_resolution;

  std::unique_ptr<SG::ConstAccessor<bool>> m_acc_isset;
  std::unique_ptr<SG::ConstAccessor<bool>> m_acc_hastime;
  std::unique_ptr<SG::ConstAccessor<float>> m_acc_time;
  std::unique_ptr<SG::ConstAccessor<int>> m_acc_nhits;
  std::unique_ptr<SG::ConstAccessor<int>> m_acc_nprimehits;
  std::unique_ptr<SG::ConstAccessor<float>> m_acc_resolution;

  /// Input decorations produced by the HGTD track extension, read with fixed
  /// names.
  SG::ConstAccessor<std::vector<bool>> m_acc_perLayer_hasCluster{
      "HGTD_has_extension"};
  SG::ConstAccessor<std::vector<float>> m_acc_perLayer_clusterTime{
      "HGTD_cluster_time"};
  SG::ConstAccessor<std::vector<int>> m_acc_perLayer_clusterTruthClass{
      "HGTD_cluster_truth_class"};
  SG::ConstAccessor<std::vector<bool>> m_acc_perLayer_expectCluster{
      "HGTD_primary_expected"};
};

} // namespace HGTD

#endif // HGTD_EXPERTTRACKTIMEFROMCLUSTERSTOOL_H
