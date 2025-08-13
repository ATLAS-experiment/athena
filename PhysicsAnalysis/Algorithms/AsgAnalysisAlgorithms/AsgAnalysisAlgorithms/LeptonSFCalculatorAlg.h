/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#ifndef ASG__LEPTON_SF_CALCULATORALG_H
#define ASG__LEPTON_SF_CALCULATORALG_H

#include <AnaAlgorithm/AnaAlgorithm.h>
#include <SystematicsHandles/SysListHandle.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysReadDecorHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>

#include <xAODEgamma/ElectronContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODEgamma/PhotonContainer.h>
#include <xAODTau/TauJetContainer.h>
#include <xAODEventInfo/EventInfo.h>

namespace CP {
  class LeptonSFCalculatorAlg : public EL::AnaAlgorithm {
  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    virtual StatusCode initialize() final;
    virtual StatusCode execute() final;

  private:
    // systematics
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::ElectronContainer> m_electronsHandle {
      this, "electrons", "", "the electron container to use"
    };
    CP::SysReadSelectionHandle m_electronSelection {
      this, "electronSelection", "", "the selection on the input electrons"
    };

    CP::SysReadHandle<xAOD::MuonContainer> m_muonsHandle {
      this, "muons", "", "the muon container to use"
    };
    CP::SysReadSelectionHandle m_muonSelection {
      this, "muonSelection", "", "the selection on the input muons"
    };

    CP::SysReadHandle<xAOD::PhotonContainer> m_photonsHandle {
      this, "photons", "", "the photon container to use"
    };
    CP::SysReadSelectionHandle m_photonSelection {
      this, "photonSelection", "", "the selection on the input photons"
    };

    CP::SysReadHandle<xAOD::TauJetContainer> m_tausHandle {
      this, "taus", "", "the tau container to use"
    };
    CP::SysReadSelectionHandle m_tauSelection {
      this, "tauSelection", "", "the selection on the input taus"
    };

    CP::SysReadHandle<xAOD::EventInfo> m_eventInfoHandle {
      this, "eventInfo", "EventInfo", "the EventInfo container to decorate selection decisions to"
    };

    CP::SysReadDecorHandleArray<float> m_electronSFs {
      this, "electronSFs", {}, "Array of decorated per-electron SFs"
    };

    CP::SysReadDecorHandleArray<float> m_muonSFs {
      this, "muonSFs", {}, "Array of decorated per-muon SFs"
    };

    CP::SysReadDecorHandleArray<float> m_photonSFs {
      this, "photonSFs", {}, "Array of decorated per-photon SFs"
    };

    CP::SysReadDecorHandleArray<float> m_tauSFs {
      this, "tauSFs", {}, "Array of decorated per-tau SFs"
    };

    CP::SysWriteDecorHandle<float> m_event_leptonSF {
      this, "event_leptonSF", "leptonSF_%SYS%", "Decoration for per-event leptonSF"
    };
  };

} // namespace

#endif
