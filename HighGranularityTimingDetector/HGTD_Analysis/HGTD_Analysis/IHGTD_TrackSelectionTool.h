/**
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_Analysis/IHGTD_TrackSelectionTool.h
 *
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 *
 * @date February, 2023
 * @brief
 */

#ifndef IHGTD_TRACKSELECTIONTOOL_H
#define IHGTD_TRACKSELECTIONTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "xAODTracking/TrackParticle.h"

class MsgStream;

class IHGTD_TrackSelectionTool : virtual public IAlgTool {

public:
  DeclareInterfaceID(IHGTD_TrackSelectionTool, 1, 0);

  /**
   * @brief Returns true if the track passes a given selection
   */
  virtual bool
  trackPassesSelection(const xAOD::TrackParticle* track_particle) const = 0;
};

#endif // IHGTD_TRACKSELECTIONTOOL_H
