/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef TAU_ANALYSIS_ALGORITHMS__TAU_TRUTH_MATCHING_ALG_H
#define TAU_ANALYSIS_ALGORITHMS__TAU_TRUTH_MATCHING_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <TauAnalysisTools/ITauTruthMatchingTool.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <xAODJet/JetContainer.h>
#include <xAODTau/TauJetContainer.h>
#include <xAODTruth/TruthParticleContainer.h>

namespace CP
{
  /// \brief an algorithm for calling \ref ITauTruthMatchingTool

  class TauTruthMatchingAlg final : public EL::AnaAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    StatusCode initialize () override;
    StatusCode execute () override;



    /// \brief the matching tool
  private:
    ToolHandle<TauAnalysisTools::ITauTruthMatchingTool> m_matchingTool {this, "matchingTool", "TauAnalysisTools::TauTruthMatchingTool", "the matching tool we apply"};

    /// \brief the systematics list we run
  private:
    SysListHandle m_systematicsList {this};

    /// \brief the tau collection we run on
  private:
    SysReadHandle<xAOD::TauJetContainer> m_tauHandle {
      this, "taus", "TauJets", "the tau collection to run on"};

    /// \brief the preselection we apply to our input
  private:
    SysReadSelectionHandle m_preselection {
      this, "preselection", "", "the preselection to apply"};

    /// \brief output decoration for truth matching flag
  private:
    SysWriteDecorHandle<char> m_isTruthMatchedDecor {
      this, "isTruthMatchedDecoration", "IsTruthMatched",
      "decoration for truth matching flag"};

    /// \brief output decoration for truth jet link
  private:
    SysWriteDecorHandle<ElementLink<xAOD::JetContainer>> m_truthJetLinkDecor {
      this, "truthJetLinkDecoration", "truthJetLink",
      "decoration for truth jet link"};

    /// \brief output decoration for truth particle link
  private:
    SysWriteDecorHandle<ElementLink<xAOD::TruthParticleContainer>> m_truthParticleLinkDecor {
      this, "truthParticleLinkDecoration", "truthParticleLink",
      "decoration for truth particle link"};
  };
}

#endif
