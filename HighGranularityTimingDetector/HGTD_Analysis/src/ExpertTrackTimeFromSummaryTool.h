/**
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Analysis/src/ExpertTrackTimeFromSummaryTool.h
 *
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 *
 * @date Feb, 2023
 *
 * @brief Expert-only track-time accessor that takes the time from the
 *  xAOD::TrackParticle EDM and re-applies the HGTD selection by decoding the
 *  persisted HGTD_summaryinfo bitfield. Unlike
 *  ExpertTrackTimeFromClustersTool it therefore works on an AOD, where the
 *  track parameters at the last measurement are no longer available.
 *
 *  For expert studies and internal validation only -- see the warning in
 *  HGTD_Analysis/IHGTD_ExpertTrackTimeAccessor.h.
 */

#ifndef HGTD_EXPERTTRACKTIMEFROMSUMMARYTOOL_H
#define HGTD_EXPERTTRACKTIMEFROMSUMMARYTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "HGTD_Analysis/IHGTD_ExpertTrackTimeAccessor.h"

#include "AthContainers/ConstAccessor.h"

#include <cstdint>
#include <string>
#include <vector>

namespace HGTD {

class ExpertTrackTimeFromSummaryTool
    : public extends<AthAlgTool, IHGTD_ExpertTrackTimeAccessor> {

public:
  ExpertTrackTimeFromSummaryTool(const std::string&, const std::string&,
                                 const IInterface*);

  virtual ~ExpertTrackTimeFromSummaryTool() = default;

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
   * @brief Number of HGTD hits that survived the time consistency check,
   * decoded from the HGTD_summaryinfo bitfield. Kept private on purpose: a
   * caller that needs a hit count should declare the dependency on the
   * decoration it wants to read itself instead of going through the tool.
   */
  int nHits(const xAOD::TrackParticle& track_particle) const;

  /**
   * @brief Number of HGTD hits associated to a primary particle, decoded from
   * the HGTD_summaryinfo bitfield. Private for the same reason as nHits.
   */
  int nPrimaryHits(const xAOD::TrackParticle& track_particle) const;

  Gaudi::Property<bool> m_do_last_hit{this, "UseLastHitCut", true,
                                      "Default value"};
  Gaudi::Property<bool> m_do_min_nhits{this, "UseMinNHits", true,
                                       "Default value"};
  Gaudi::Property<float> m_min_eta{this, "MinEta", 3.5, "Default value"};
  Gaudi::Property<float> m_max_eta{this, "MaxEta", 3.9, "Default value"};

  SG::ConstAccessor<uint8_t> m_acc_has_valid_time{"hasValidTime"};
  SG::ConstAccessor<uint32_t> m_acc_summary_info{"HGTD_summaryinfo"};
  SG::ConstAccessor<float> m_acc_time_resolution{"timeResolution"};
  SG::ConstAccessor<std::vector<bool>> m_acc_perLayer_expectCluster{
      "HGTD_primary_expected"};

  // different shift distances for bitfield definition
  const short m_comp_ptrn_sft = 8;
  const short m_primes_ptrn_sft = 16;
  const short m_holes_ptrn_sft = 12;
};

} // namespace HGTD

#endif // HGTD_EXPERTTRACKTIMEFROMSUMMARYTOOL_H
