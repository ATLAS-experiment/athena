///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// METMakerAlg.h

#ifndef METMakerAlg_H
#define METMakerAlg_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"

#include "xAODEgamma/Electron.h"
#include "xAODEgamma/Photon.h"
#include "xAODMuon/Muon.h"
#include "xAODTau/TauJet.h"

#include "xAODJet/JetContainer.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODTau/TauJetContainer.h"

#include "xAODMissingET/MissingETContainer.h"
#include "xAODMissingET/MissingETAssociationMap.h"
#include "TauAnalysisTools/ITauSelectionTool.h"
#include "EgammaAnalysisInterfaces/IAsgPhotonIsEMSelector.h"
#include "MuonAnalysisInterfaces/IMuonSelectionTool.h"
#include "EgammaAnalysisInterfaces/IAsgElectronLikelihoodTool.h"


class IMETMaker;
class IAsgElectronLikelihoodTool;
class IAsgPhotonIsEMSelector;
namespace CP {
  class IMuonSelectionTool;
}

namespace met {
  class METMakerAlg : public AthReentrantAlgorithm {

  public: 

    /// Constructor with parameters:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    /// Destructor:
    virtual ~METMakerAlg();

    /// Athena algorithm's Hooks
    virtual StatusCode  initialize() override;
    virtual StatusCode  execute(const EventContext& ctx) const override;
    virtual StatusCode  finalize() override;

  protected: // was private childORMETMaker

    virtual bool accept(const xAOD::Electron* el) const;
    virtual bool accept(const xAOD::Photon* ph) const;
    virtual bool accept(const xAOD::TauJet* tau) const;
    virtual bool accept(const xAOD::Muon* muon) const;

    Gaudi::Property<std::string> m_softclname{this, "METSoftClName", "SoftClus"};
    Gaudi::Property<std::string> m_softtrkname{this, "METSoftTrkName", "PVSoftTrk"};

    SG::ReadHandleKey<xAOD::ElectronContainer>      m_ElectronContainerKey{this, "InputElectrons", "Electrons"};
    SG::ReadHandleKey<xAOD::PhotonContainer>        m_PhotonContainerKey{this, "InputPhotons", "Photons"};
    SG::ReadHandleKey<xAOD::TauJetContainer>        m_TauJetContainerKey{this, "InputTaus", "TauJets"};
    SG::ReadHandleKey<xAOD::MuonContainer>          m_MuonContainerKey{this, "InputMuons", "Muons"};
    SG::ReadHandleKey<xAOD::JetContainer>           m_JetContainerKey{this, "InputJets", "AntiKt4LCTopoJets"};

    SG::ReadHandleKey<xAOD::MissingETContainer>           m_CoreMetKey{this, "METCoreName", "MET_Core"};

    SG::WriteHandleKey<xAOD::MissingETContainer> m_metKey{this, "METName", "MET_Reference", "MET container"};
    SG::ReadHandleKey<xAOD::MissingETAssociationMap> m_metMapKey{this, "METMapName", "METAssoc"};


    Gaudi::Property<bool> m_doTruthLep{this, "DoTruthLeptons", false};
    
    /// Athena configured tools
    ToolHandle<IMETMaker> m_metmaker{this, "Maker", "Maker"};

    ToolHandle<CP::IMuonSelectionTool> m_muonSelTool{this, "MuonSelectionTool", "MuonSelectionTool"};
    ToolHandle<IAsgElectronLikelihoodTool> m_elecSelLHTool{this, "ElectronLHSelectionTool", "ElectronLHSelectionTool"};
    ToolHandle<IAsgPhotonIsEMSelector>     m_photonSelIsEMTool{this, "PhotonIsEMSelectionTool", "PhotonIsEMSelectionTool"};
    ToolHandle<TauAnalysisTools::ITauSelectionTool> m_tauSelTool{this, "TauSelectionTool", "TauSelectionTool"};

  }; 

}

#endif
