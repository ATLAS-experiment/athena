/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef  TAUANALYSISTOOLS_BUILDTRUTHTAUS_H
#define  TAUANALYSISTOOLS_BUILDTRUTHTAUS_H

/*
  author: Dirk Duschinger
  documentation: https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/TauID/TauAnalysisTools/doc/README-TauTruthMatchingTool.rst
*/

// Framework include(s):
#include "AsgTools/AsgMetadataTool.h"
#include "AsgTools/AnaToolHandle.h"
#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/WriteHandleKey.h"

// Core include(s):
#include "MCTruthClassifier/IMCTruthClassifier.h"

// EDM include(s):
#include "xAODJet/JetContainer.h"

// Local include(s):
#include "TauAnalysisTools/IBuildTruthTaus.h"

namespace TauAnalysisTools
{

class BuildTruthTaus
  : public virtual TauAnalysisTools::IBuildTruthTaus
  , public asg::AsgMetadataTool
{
  /// Create a proper constructor for Athena
  ASG_TOOL_CLASS( BuildTruthTaus,
                  TauAnalysisTools::IBuildTruthTaus )

public:

  struct TruthTausEvent
    : public ITruthTausEvent
  {
    const xAOD::TruthParticleContainer* m_xTruthTauContainerConst = nullptr;
    const xAOD::TruthParticleContainer* m_xTruthMuonContainerConst = nullptr;
    const xAOD::TruthParticleContainer* m_xTruthElectronContainerConst = nullptr;
    const xAOD::JetContainer* m_xTruthJetContainerConst = nullptr;
    xAOD::TruthParticleContainer* m_xTruthTauContainer = nullptr;
    xAOD::TruthParticleAuxContainer* m_xTruthTauAuxContainer = nullptr;
    const xAOD::TruthParticleContainer* m_xTruthParticleContainer = nullptr;
  };

  BuildTruthTaus( const std::string& name );

  virtual ~BuildTruthTaus() = default;

  // initialize the tool
  virtual StatusCode initialize() override;

  virtual void setTruthMatchingMode() override {
    m_truthMatchingMode = true;
  };

  virtual StatusCode retrieveTruthTaus() override;
  virtual StatusCode retrieveTruthTaus(ITruthTausEvent& truthTausEvent) const override;


protected:
  StatusCode retrieveTruthTaus(TruthTausEvent& truthTausEvent) const;


private:

  struct TauTruthInfo
  {
    size_t m_iNChargedPions = 0;
    size_t m_iNNeutralPions = 0;
    size_t m_iNChargedOthers = 0;
    size_t m_iNNeutralOthers = 0;
    size_t m_iNChargedDaughters = 0;
    std::vector<int> m_vDecayMode;
    // default false, if there is a hadron in decay products, it is switched to true
    bool m_bIsHadronicTau = false;

    // truth visible kinematic variables
    TLorentzVector m_vTruthVisTLV;
    TLorentzVector m_vTruthVisTLVCharged;
    TLorentzVector m_vTruthVisTLVNeutral;

    // truth vertices
    TVector3 m_vDecayVertex;
    TVector3 m_vProdVertex; 

  };


  StatusCode buildTruthTausFromTruthParticles(TruthTausEvent& truthTausEvent) const;
  StatusCode examineTruthTau(const xAOD::TruthParticle& xTruthParticle) const;
  StatusCode examineTruthTauDecay(const xAOD::TruthParticle& xTruthParticle,
                                  TauTruthInfo& truthInfo) const;
  void printDecay(const xAOD::TruthParticle& xTruthParticle, int depth = 0) const;

protected:

  TruthTausEvent m_truthTausEvent;

  // temporary, drop at the first occasion
  bool m_bTruthTauAvailable{};

private:

  // input containers
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthTauInputContainer { this, "TruthTauContainerName", "TruthTaus", "Truth tau input container name (truth matching mode)" };
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthParticleContainer { this, "TruthParticleContainerName", "TruthParticles", "Truth particles input container name" };
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthElectronContainer { this, "TruthElectronContainerName", "TruthElectrons", "Truth electrons input container name" };
  SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthMuonContainer { this, "TruthMuonContainerName", "TruthMuons", "Truth muons input container name" };
  SG::ReadHandleKey<xAOD::JetContainer> m_truthJetContainer { this, "TruthJetContainerName", "AntiKt4TruthDressedWZJets", "Truth jets input container name" };
  // output container
  SG::WriteHandleKey<xAOD::TruthParticleContainer> m_truthTauOutputContainer { this, "NewTruthTauContainerName", "TruthTaus", "Truth tau output container name" };

  bool m_truthMatchingMode = false;
  bool m_bWriteInvisibleFourMomentum{};
  bool m_bWriteVisibleChargedFourMomentum{};
  bool m_bWriteVisibleNeutralFourMomentum{};
  bool m_bWriteDecayModeVector{};
  bool m_bWriteVertices{};

  asg::AnaToolHandle<IMCTruthClassifier> m_tMCTruthClassifier;

}; // class BuildTruthTaus

}
#endif // TAUANALYSISTOOLS_BUILDTRUTHTAUS_H
