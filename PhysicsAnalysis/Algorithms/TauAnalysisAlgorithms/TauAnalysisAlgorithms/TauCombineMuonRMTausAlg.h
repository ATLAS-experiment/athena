/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Qichen Dong

#ifndef TAU_ANALYSIS_ALGORITHMS__TAU_COMBINE_MUON_RM_TAUS_ALG_H
#define TAU_ANALYSIS_ALGORITHMS__TAU_COMBINE_MUON_RM_TAUS_ALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>
#include <SystematicsHandles/SysWriteHandle.h>
#include <xAODTau/TauJetContainer.h>
#include <xAODTau/TauJetAuxContainer.h>
#include <TauAnalysisTools/HelperFunctions.h>

namespace CP
{
  class TauCombineMuonRMTausAlg final : public EL::AnaAlgorithm
  {
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    StatusCode initialize () override;
    StatusCode execute (const EventContext& ctx) override;

  private:
    SysListHandle m_systematicsList {this};
    SysReadHandle<xAOD::TauJetContainer> m_tauHandle {
      this, "taus", "TauJets", "the tau collection to run on"
    };

    SysReadHandle<xAOD::TauJetContainer> m_MuonRMtauHandle {
      this, "muonrm_taus", "TauJets_MuonRM", "the muon-removal tau collection to run on"
    };

    SysWriteHandle<xAOD::TauJetContainer, xAOD::TauJetAuxContainer> m_outputTauHandle {
      this, "combined_taus", "TauJets_Combined", "the output tau collection with combined taus"
    };

    SysWriteDecorHandle<char> m_tauSelectionDecor {
      "SelectedByMuonRemovalCombination", this
    };

    SysWriteDecorHandle<char> m_MuonRMtauSelectionDecor {
      "SelectedByMuonRemovalCombination", this
    };
  };
}

#endif
