/**
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Analysis/src/ExpertTrackTimeFromEDMTool.h
 *
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 *
 * @date Feb, 2023
 *
 * @brief Expert-only track-time accessor that hands back the track time as it
 *  is stored in the xAOD::TrackParticle EDM, without applying any additional
 *  selection. It exists as the unmodified reference point the other two
 *  implementations are compared against in HGTD validation studies.
 *
 *  For expert studies and internal validation only -- see the warning in
 *  HGTD_Analysis/IHGTD_ExpertTrackTimeAccessor.h. Anything that just needs
 *  the track time has to call xAOD::TrackParticle::time() directly instead of
 *  going through this tool.
 */

#ifndef HGTD_EXPERTTRACKTIMEFROMEDMTOOL_H
#define HGTD_EXPERTTRACKTIMEFROMEDMTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "HGTD_Analysis/IHGTD_ExpertTrackTimeAccessor.h"

#include <string>

namespace HGTD {

class ExpertTrackTimeFromEDMTool
    : public extends<AthAlgTool, IHGTD_ExpertTrackTimeAccessor> {

public:
  ExpertTrackTimeFromEDMTool(const std::string&, const std::string&,
                             const IInterface*);

  virtual ~ExpertTrackTimeFromEDMTool() = default;

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
};

} // namespace HGTD

#endif // HGTD_EXPERTTRACKTIMEFROMEDMTOOL_H
