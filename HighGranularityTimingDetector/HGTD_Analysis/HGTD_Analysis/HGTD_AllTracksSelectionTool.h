/**
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Analysis/HGTD_AllTracksSelectionTool.h
 *
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 *
 * @date February, 2023
 *
 * @brief
 */

#ifndef HGTD_ALLTRACKSSELECTIONTOOL_H
#define HGTD_ALLTRACKSSELECTIONTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "HGTD_Analysis/IHGTD_TrackSelectionTool.h"

class HGTD_AllTracksSelectionTool
    : public extends<AthAlgTool, IHGTD_TrackSelectionTool> {

public:
  HGTD_AllTracksSelectionTool(const std::string&, const std::string&,
                              const IInterface*);

  virtual ~HGTD_AllTracksSelectionTool() = default;

  //////////////////////////////////////////////////////////////////////////////
  // for AthAlgTool interface
  virtual StatusCode initialize() override final;

  // for IHGTD_TrackSelectionTool interface

  virtual bool trackPassesSelection(
      const xAOD::TrackParticle* track_particle) const override final;
  //////////////////////////////////////////////////////////////////////////////
};

#endif // HGTD_ALLTRACKSSELECTIONTOOL_H
