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

#ifndef HGTD_TRACKTIMEACCTOOL2_H
#define HGTD_TRACKTIMEACCTOOL2_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "HGTD_Analysis/IHGTD_TrackTimeAccessor.h"

#include <string>
#include <vector>

#include "TVector3.h"

namespace HGTD {

class TrackTimeAccTool2 : virtual public IHGTD_TrackTimeAccessor,
                          public AthAlgTool {

public:
  TrackTimeAccTool2(const std::string&, const std::string&, const IInterface*);

  virtual ~TrackTimeAccTool2() = default;

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

  virtual std::string toolName() override final { return "TrackTimeAccTool2"; };

  //////////////////////////////////////////////////////////////////////////////

  int nPrimaryHits(const xAOD::TrackParticle& track_particle);

  float fracPrimaryHits(const xAOD::TrackParticle& track_particle);

  virtual int numberPotentialPrimaryHits(
      const xAOD::TrackParticle& track_particle) override final;

  // FIXME add back once this is possible
  // int nPotentialPrimaryHits(const xAOD::TrackParticle& track_particle);

private:
};

} // namespace HGTD

#endif // HGTD_TRACKTIMEACCTOOL_H
