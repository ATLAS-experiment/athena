/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Daniel Werner



#ifndef TRACKING_ANALYSIS_ALGORITHMS__INDET_TRACK_SELECTION_ALG_H
#define TRACKING_ANALYSIS_ALGORITHMS__INDET_TRACK_SELECTION_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgTools/PropertyWrapper.h>
#include <InDetTrackSelectionTool/IInDetTrackSelectionTool.h>
#include <InDetTrackSystematicsTools/IInDetTrackTruthFilterTool.h>
#include <SelectionHelpers/ISelectionNameSvc.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SelectionHelpers/SysWriteSelectionHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <xAODTracking/TrackParticleContainer.h>

namespace CP
{
  /// \brief an algorithm for calling first the \ref IInDetTrackSelectionTool and then the IInDetTrackTruthFilterTool

  class InDetTrackSelectionAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    StatusCode initialize () override;
    StatusCode execute () override;



    /// \brief the smearing tool
  private:
    ToolHandle<InDet::IInDetTrackSelectionTool> m_selectionTool {this, "selectionTool", "", "the selection tool we apply"};

    /// \brief the filter tool
  private:
    ToolHandle<InDet::IInDetTrackTruthFilterTool> m_filterTool {this, "filterTool", "", "the truth filter tool we apply"};

    /// \brief the filter tool cast to an ISystematicsTool
  private:
    ISystematicsTool *m_sysFilterTool {nullptr};

    /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

    /// \brief the track collection we run on
  private:
    SysReadHandle<xAOD::TrackParticleContainer> m_tracksHandle {
      this, "inDetTracks", "", "the track collection to run on"};

    /// \brief the preselection we apply to our input
  private:
    SysReadSelectionHandle m_preselection {
      this, "preselection", "", "the preselection to apply"};

    /// \brief the decoration for the asg selection
  private:
    SysWriteSelectionHandle m_selectionHandle {
      this, "selectionDecoration", "", "the decoration for the asg selection"};

    /// \brief the ISelectionNameSvc
  private:
    ServiceHandle<ISelectionNameSvc> m_nameSvc {"SelectionNameSvc", "InDetTrackSelectionAlg"};

    /// \brief the bits to set for an object failing the preselection
  private:
    SelectionType m_setOnFail;

  private:
    asg::AcceptInfo m_acceptInfo;

  private:
    Gaudi::Property<std::string> m_filterWP{this, "filterWP", ""};
  };
}

#endif
