/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef TAU_ANALYSIS_ALGORITHMS__DI_TAU_TRUTH_MATCHING_ALG_H
#define TAU_ANALYSIS_ALGORITHMS__DI_TAU_TRUTH_MATCHING_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <TauAnalysisTools/IDiTauTruthMatchingTool.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <xAODTau/DiTauJetContainer.h>

namespace CP
{
  /// \brief an algorithm for calling \ref IDiTauTruthMatchingTool

  class DiTauTruthMatchingAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    StatusCode initialize () override;
    StatusCode execute (const EventContext& ctx) override;



    /// \brief the matching tool
  private:
    ToolHandle<TauAnalysisTools::IDiTauTruthMatchingTool> m_matchingTool {this, "matchingTool", "TauAnalysisTools::DiTauTruthMatchingTool", "the matching tool we apply"};

    /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

    /// \brief the tau collection we run on
  private:
    SysReadHandle<xAOD::DiTauJetContainer> m_tauHandle {
      this, "taus", "DiTauJets", "the tau collection to run on"};

    /// \brief the preselection we apply to our input
  private:
    SysReadSelectionHandle m_preselection {
      this, "preselection", "", "the preselection to apply"};

    /// \brief output decorations, written by the matching tool (or by
    /// us for ditaus failing the preselection)
  private:
    SysWriteDecorHandle<float> m_truthVisLeadPtDecor {"TruthVisLeadPt", this};
    SysWriteDecorHandle<float> m_truthVisLeadEtaDecor {"TruthVisLeadEta", this};
    SysWriteDecorHandle<float> m_truthVisLeadPhiDecor {"TruthVisLeadPhi", this};
    SysWriteDecorHandle<float> m_truthVisLeadMDecor {"TruthVisLeadM", this};
    SysWriteDecorHandle<int> m_truthLeadPdgIDDecor {"TruthLeadPdgID", this};
    SysWriteDecorHandle<float> m_truthVisSubleadPtDecor {"TruthVisSubleadPt", this};
    SysWriteDecorHandle<float> m_truthVisSubleadEtaDecor {"TruthVisSubleadEta", this};
    SysWriteDecorHandle<float> m_truthVisSubleadPhiDecor {"TruthVisSubleadPhi", this};
    SysWriteDecorHandle<float> m_truthVisSubleadMDecor {"TruthVisSubleadM", this};
    SysWriteDecorHandle<int> m_truthSubleadPdgIDDecor {"TruthSubleadPdgID", this};
    SysWriteDecorHandle<float> m_truthVisDeltaRDecor {"TruthVisDeltaR", this};
    SysWriteDecorHandle<float> m_truthVisMassDecor {"TruthVisMass", this};
    SysWriteDecorHandle<char> m_isTruthMatchedDecor {"IsTruthMatched", this};
    SysWriteDecorHandle<char> m_isTruthHadronicDecor {"IsTruthHadronic", this};
  };
}

#endif
