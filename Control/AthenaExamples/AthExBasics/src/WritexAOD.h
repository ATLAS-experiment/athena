/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**@brief  Algorithm demonstrating writing of xAOD containers.
           Loops over a container of inner detector tracks, extracts the
           pT from each track and copies tracks above the threshold into a 
           new container */

#ifndef ATHEXBASICS_WRITEXAOD_H
#define ATHEXBASICS_WRITEXAOD_H

#include <atomic>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackParticleAuxContainer.h"

class WritexAOD : public AthReentrantAlgorithm {
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

 private:
    /// Read handle for the offline object container - set to tracks by default. **/
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackKey{this, "TrackParticlesKey", "InDetTrackParticles"};
    /// Read handle for the new track container **/
    SG::WriteHandleKey<xAOD::TrackParticleContainer> m_newKey{this, "NewTrackParticlesKey", "InDetTrackParticles"};
    /// Tool handle for the track selection tool */
    ToolHandle<InDet::IInDetTrackSelectionTool> m_trackSelectionTool{this, "TrackSelectionTool", "InDetTrackSelectionTool", "Tool for selecting tracks"};
};

#endif
