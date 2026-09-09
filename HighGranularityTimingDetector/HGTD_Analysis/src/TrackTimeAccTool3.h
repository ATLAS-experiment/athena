/**
 * Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Analysis/src/TrackTimeAccTool2.h
 *
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 *
 * @date Feb, 2023
 *
 * @brief Uses trk time directly
 *
 */

#ifndef HGTD_TRACKTIMEACCTOOL3_H
#define HGTD_TRACKTIMEACCTOOL3_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "HGTD_Analysis/IHGTD_TrackTimeAccessor.h"

#include <string>
#include <vector>

#include "TVector3.h"

namespace HGTD {

class TrackTimeAccTool3 : virtual public IHGTD_TrackTimeAccessor,
                          public AthAlgTool {

public:
  TrackTimeAccTool3(const std::string&, const std::string&, const IInterface*);

  virtual ~TrackTimeAccTool3() = default;

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

  virtual std::string toolName() override final { return "TrackTimeAccTool3"; };

  //////////////////////////////////////////////////////////////////////////////

  int nPrimaryHits(const xAOD::TrackParticle& track_particle);

  float fracPrimaryHits(const xAOD::TrackParticle& track_particle);

  virtual int numberPotentialPrimaryHits(
      const xAOD::TrackParticle& track_particle) override final;

  // FIXME add back once this is possible
  // int nPotentialPrimaryHits(const xAOD::TrackParticle& track_particle);

private:

  Gaudi::Property<bool> m_do_last_hit{this, "UseLastHitCut", true,
                                      "Default value"};
  Gaudi::Property<bool> m_do_min_nhits{this, "UseMinNHits", true,
                                      "Default value"};
  Gaudi::Property<float> m_min_eta{this, "MinEta", 3.5, "Default value"};
  Gaudi::Property<float> m_max_eta{this, "MaxEta", 3.9, "Default value"};


  std::unique_ptr<SG::AuxElement::Accessor<uint8_t>>
      m_hasValidTime;

  std::unique_ptr<SG::AuxElement::Accessor<uint32_t>>
      m_summaryInfo;

  std::unique_ptr<SG::AuxElement::Accessor<float>>
      m_timeResolution;

  std::unique_ptr<SG::AuxElement::Accessor<std::vector<bool>>>
      m_dec_perLayer_expectCluster;

  // different shift distances for bitfield definition
  const short m_comp_ptrn_sft = 8;
  const short m_primes_ptrn_sft = 16;
  const short m_holes_ptrn_sft = 12;
    

};

} // namespace HGTD

#endif // HGTD_TRACKTIMEACCTOOL_H
