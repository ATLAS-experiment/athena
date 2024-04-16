/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**@brief  Algorithm demonstrating reading of xAOD containers.
           Loops over a container of inner detector tracks, extracts the
           pT from each object counts how many tracks fall above and below a cut
           provided by the user. Note the user of atomic to ensure the counters 
           are thread safe */

#ifndef ATHEXBASICS_READXAOD_H
#define ATHEXBASICS_READXAOD_H

#include <atomic>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"
#include "xAODTracking/TrackParticleContainer.h"

class ReadxAOD : public AthReentrantAlgorithm {
 public:
  ReadxAOD(const std::string &name, ISvcLocator *pSvcLocator);

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;
  virtual StatusCode finalize() override;

 private:
    /** Counter for tracks that have pT below the cut */
    mutable std::atomic<unsigned int> m_nTracksBelow{0};
    /** Counter for tracks that have pT above the cut */
    mutable std::atomic<unsigned int> m_nTracksAbove{0};
    /** pT cut in MeV */
    Gaudi::Property<float> m_cut
      {this, "PtCut", 500.0, "pT Cut to apply to the tracks in MeV"}; 
    /** Read handle for the offline object container - set to muons by default. **/
    SG::ReadHandleKey<xAOD::TrackParticleContainer> m_trackKey{this, "TrackParticlesKey", "InDetTrackParticles"};
    /** Tool handle for the track selection tool */
    ToolHandle<InDet::IInDetTrackSelectionTool> m_trackSelectionTool{this, "TrackSelectionTool", "InDetTrackSelectionTool", "Tool for selecting tracks"};
};

#endif
