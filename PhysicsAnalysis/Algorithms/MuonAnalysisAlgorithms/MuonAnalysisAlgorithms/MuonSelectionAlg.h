/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tadej Novak

#ifndef MUON_ANALYSIS_ALGORITHMS__MUON_SELECTION_ALG_H
#define MUON_ANALYSIS_ALGORITHMS__MUON_SELECTION_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <MuonAnalysisInterfaces/IMuonSelectionTool.h>
#include <SelectionHelpers/ISelectionNameSvc.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SelectionHelpers/SysWriteSelectionHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <xAODMuon/MuonContainer.h>

namespace CP
{
  /// \brief an algorithm for calling \ref IMuonSelectionTool

  class MuonSelectionAlgV2 final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    StatusCode initialize () override;
    StatusCode execute () override;
    


    /// \brief the selection tool
  private:
    ToolHandle<IMuonSelectionTool> m_selectionTool {this, "selectionTool", "", "the selection tool we apply"};

    /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

    /// \brief the preselection we apply to our input
  private:
    SysReadSelectionHandle m_preselection {
      this, "preselection", "", "the preselection to apply"};

    /// \brief the particle continer we run on
  private:
    SysReadHandle<xAOD::MuonContainer> m_muonsHandle {
      this, "muons", "Muons", "the muons collection to run on"};

    /// \brief the decoration for the quality selection
  private:
    SysWriteSelectionHandle m_selectionHandle {
      this, "selectionDecoration", "", "the decoration for the quality selection"};

    /// \brief the decoration for the bad muon veto
  private:
    SysWriteSelectionHandle m_badMuonVetoHandle {
      this, "badMuonVetoDecoration", "", "the decoration for the bad muon veto"};

    /// \brief the ISelectionNameSvc
  private:
    ServiceHandle<ISelectionNameSvc> m_nameSvc {"SelectionNameSvc", "MuonSelectionAlgV2"};

    /// \brief the bits to set for an object failing the preselection
  private:
    SelectionType m_setOnFail;
  };
}

#endif
