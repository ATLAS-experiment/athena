/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Daniel Werner



#ifndef TRACKING_ANALYSIS_ALGORITHMS__INDET_TRACK_SMEARING_ALG_H
#define TRACKING_ANALYSIS_ALGORITHMS__INDET_TRACK_SMEARING_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgTools/PropertyWrapper.h>
#include <InDetTrackSystematicsTools/IInDetTrackSmearingTool.h>
#include <SelectionHelpers/OutOfValidityHelper.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SystematicsHandles/SysCopyHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <xAODTracking/TrackParticleContainer.h>

namespace CP
{
  /// \brief an algorithm for calling \ref InDetTrackSmearingTool

  class InDetTrackSmearingAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    StatusCode initialize () override;
    StatusCode execute () override;

    /// \brief the smearing tool
  private:
    ToolHandle<InDet::IInDetTrackSmearingTool> m_smearingTool {this, "smearingTool", "InDetTrackSmearingTool", "the smearing tool we apply"};

    /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

    /// \brief the track collection we run on
  private:
    SysCopyHandle<xAOD::TrackParticleContainer> m_tracksHandle {
      this, "inDetTracks", "", "the track collection to run on"};

    /// \brief the preselection we apply to our input
  private:
    SysReadSelectionHandle m_preselection {
      this, "preselection", "", "the preselection to apply"};

    /// \brief the helper for OutOfValidity results
  private:
    OutOfValidityHelper m_outOfValidity {this};
  };
}

#endif
