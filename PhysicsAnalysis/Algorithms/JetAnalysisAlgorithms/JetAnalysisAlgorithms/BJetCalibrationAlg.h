/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Thomas Strebler

#ifndef JET_ANALYSIS_ALGORITHMS__BJET_CALIBRATION_ALG_H
#define JET_ANALYSIS_ALGORITHMS__BJET_CALIBRATION_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <SystematicsHandles/SysCopyHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>

#include <JetAnalysisInterfaces/IMuonInJetCorrectionTool.h>
#include <JetAnalysisInterfaces/IBJetCorrectionTool.h>
#include "MuonAnalysisInterfaces/IMuonSelectionTool.h"

#include "xAODJet/JetContainer.h"
#include "xAODMuon/MuonContainer.h"

namespace CP
{
  /// \brief an algorithm to apply b-jet specific energy correction

  class BJetCalibrationAlg final : public EL::AnaAlgorithm
  {
  public:
    /// \brief the standard constructor
    using EL::AnaAlgorithm::AnaAlgorithm;
    StatusCode initialize () override;
    StatusCode execute () override;

  private:

    /// \brief the muon-in-jet selection tool, unused if muonPreselection set
    ToolHandle<CP::IMuonSelectionTool> m_muonSelectionTool{this, "muonSelectionTool", "", "tool for muon quality selection"};

    /// \brief the muon-in-jet correction tool
    ToolHandle<IMuonInJetCorrectionTool> m_muonInJetTool {this, "muonInJetTool", "MuonInJetCorrectionTool", "the muon-in-jet correction tool tool we apply"};
    
    /// \brief the b-jet pt correction tool
    ToolHandle<IBJetCorrectionTool> m_bJetTool {this, "bJetTool", "", "the b-jet pt correction tool tool we apply"};
    
    SysListHandle m_systematicsList {this};

    SysCopyHandle<xAOD::JetContainer> m_jetHandle {
      this, "jets", "", "the jet collection to run on"};

    SysReadSelectionHandle m_jetPreselection {
      this, "jetPreselection", "", "the preselection to apply to jets"};

    SysReadHandle<xAOD::MuonContainer> m_muonHandle{
      this, "muons", "", "Muon collection to retrieve"};

    SysReadSelectionHandle m_muonPreselection {
      this, "muonPreselection", "", "the preselection to apply to muons"};

    SysWriteDecorHandle<int> m_nmuons {
      this, "NMuonsDecorName", "n_muons_%SYS%", "Name of output decorator for n_muons"};

  };

}

#endif
