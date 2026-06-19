/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Daniel Werner



#ifndef TRACKING_ANALYSIS_ALGORITHMS__INDET_TRACK_SELECTION_ALG_H
#define TRACKING_ANALYSIS_ALGORITHMS__INDET_TRACK_SELECTION_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/ReadHandle.h>
#include <AsgTools/PropertyWrapper.h>
#include <InDetTrackSelectionTool/IInDetTrackSelectionTool.h>
#include <InDetTrackSystematicsTools/IInDetTrackTruthFilterTool.h>
#include <SelectionHelpers/ISelectionNameSvc.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SelectionHelpers/SysWriteSelectionHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <xAODTracking/TrackParticleContainer.h>
#include <xAODTracking/VertexContainer.h>

namespace CP
{
  /// \brief an algorithm for calling first the \ref IInDetTrackSelectionTool and then the IInDetTrackTruthFilterTool

  class InDetTrackSelectionAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    StatusCode initialize () override;
    StatusCode execute (const EventContext& ctx) override;



  private:
    /// \brief the smearing tool
    ToolHandle<InDet::IInDetTrackSelectionTool> m_selectionTool {this, "selectionTool", "", "the selection tool we apply"};

    /// \brief the filter tool
    ToolHandle<InDet::IInDetTrackTruthFilterTool> m_filterTool {this, "filterTool", "", "the truth filter tool we apply"};

    /// \brief the systematics list we run
    SysListHandle m_systematicsList {this};

    /// \brief the track collection we run on
    SysReadHandle<xAOD::TrackParticleContainer> m_tracksHandle {
      this, "inDetTracks", "", "the track collection to run on"};

    /// \brief the vertex collection to use for the selection (optional)
    SG::ReadHandleKey<xAOD::VertexContainer> m_vertexContainerKey{
      this, "vertices", "", "the vertex container to use"};

    /// \brief the preselection we apply to our input
    SysReadSelectionHandle m_preselection {
      this, "preselection", "", "the preselection to apply"};

    /// \brief the decoration for the asg selection
    SysWriteSelectionHandle m_selectionHandle {
      this, "selectionDecoration", "", "the decoration for the asg selection"};

    /// \brief the ISelectionNameSvc
    ServiceHandle<ISelectionNameSvc> m_nameSvc {"SelectionNameSvc", "InDetTrackSelectionAlg"};

    /// \brief the bits to set for an object failing the preselection
    SelectionType m_setOnFail;

    asg::AcceptInfo m_acceptInfo;

    Gaudi::Property<std::string> m_filterWP{this, "filterWP", ""};
  };
}

#endif
