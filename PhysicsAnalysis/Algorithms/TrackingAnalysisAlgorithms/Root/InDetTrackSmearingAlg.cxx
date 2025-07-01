/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Daniel Werner

//
// includes
//

#include <TrackingAnalysisAlgorithms/InDetTrackSmearingAlg.h>

//
// method implementations
//

namespace CP
{

  StatusCode InDetTrackSmearingAlg ::
  initialize ()
  {
    ANA_CHECK (m_smearingTool.retrieve());
    ANA_CHECK (m_tracksHandle.initialize (m_systematicsList));
    ANA_CHECK (m_preselection.initialize (m_systematicsList, m_tracksHandle, SG::AllowEmpty));
    ANA_CHECK (m_systematicsList.addSystematics (*m_smearingTool));
    ANA_CHECK (m_systematicsList.initialize());
    ANA_CHECK (m_outOfValidity.initialize());
    return StatusCode::SUCCESS;
  }



  StatusCode InDetTrackSmearingAlg ::
  execute ()
  {
    for (const auto& sys : m_systematicsList.systematicsVector())
    {
      // always need to call `getCopy` first to ensure that the shallow copies
      // are all there if requested
      xAOD::TrackParticleContainer *inDetTracks = nullptr;
      ANA_CHECK (m_tracksHandle.getCopy (inDetTracks, sys));

      ANA_CHECK (m_smearingTool->applySystematicVariation (sys));
      for (xAOD::TrackParticle *track : *inDetTracks)
      {
        if (m_preselection.getBool (*track, sys))
        {
          ANA_CHECK_CORRECTION (m_outOfValidity, *track, m_smearingTool->applyCorrection (*track));
        }
      }
    }
    return StatusCode::SUCCESS;
  }
}
