/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_BKGELECTRONCLASSIFICATION_H
#define DERIVATIONFRAMEWORK_BKGELECTRONCLASSIFICATION_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
//
#include "GaudiKernel/ToolHandle.h"
#include "MCTruthClassifier/IMCTruthClassifier.h"
///
#include "StoreGate/ReadHandle.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandle.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODTruth/TruthParticleContainer.h"

namespace DerivationFramework {

  class BkgElectronClassification : public extends<AthAlgTool, IAugmentationTool>
  {
  public:

    using base_class::base_class;

    virtual StatusCode initialize() override final;
    virtual StatusCode addBranches(const EventContext& ctx) const override final;

  private:
    /** @brief MCTruthClassifier **/
    ToolHandle<IMCTruthClassifier> m_mcTruthClassifier{
      this,
        "MCTruthClassifierTool",
        "",
        "Handle to the MCTruthClassifier"
        };

    /** @brief input electron container **/
    SG::ReadHandleKey<xAOD::ElectronContainer> m_electronContainer{
      this,
      "ElectronContainerName",
      "Electrons",
      "Input Electrons"
    };
  SG::ReadDecorHandleKey<xAOD::ElectronContainer>
    m_electronTruthParticleLink{ this, "electronTruthParticleLink",
    m_electronContainer, "truthParticleLink", "" }; // Decoration applied in egammaTruthAssociationAlg

    /** @brief Input truth particle container **/
    SG::ReadHandleKey<xAOD::TruthParticleContainer> m_truthContainer{
      this,
      "TruthParticleContainerName",
      "TruthParticles",
      "Input Truth Particles"
    };

    // Write decoration handle keys
    SG::WriteDecorHandleKey<xAOD::ElectronContainer>
    m_truthPdgId{ this, "DoNotSet_truthPdgId", m_electronContainer, "truthPdgId", "" };
    SG::WriteDecorHandleKey<xAOD::ElectronContainer>
    m_firstEgMotherTruthType{ this, "firstEgMotherTruthType", m_electronContainer, "firstEgMotherTruthType", "" };
    SG::WriteDecorHandleKey<xAOD::ElectronContainer>
    m_firstEgMotherTruthOrigin{ this, "firstEgMotherTruthOrigin", m_electronContainer, "firstEgMotherTruthOrigin", "" };
    SG::WriteDecorHandleKey<xAOD::ElectronContainer>
    m_firstEgMotherTruthClassification{ this, "firstEgMotherTruthClassification",
      m_electronContainer, "firstEgMotherTruthClassification", "" };
    SG::WriteDecorHandleKey<xAOD::ElectronContainer>
    m_firstEgMotherTruthParticleLink{ this,
      "firstEgMotherTruthParticleLink",
      m_electronContainer, "firstEgMotherTruthParticleLink",
      "" };
    SG::WriteDecorHandleKey<xAOD::ElectronContainer>
    m_firstEgMotherPdgId{ this, "firstEgMotherPdgId", m_electronContainer, "firstEgMotherPdgId", "" };
    SG::WriteDecorHandleKey<xAOD::ElectronContainer>
    m_lastEgMotherTruthType{ this, "lastEgMotherTruthType", m_electronContainer, "lastEgMotherTruthType", "" };
    SG::WriteDecorHandleKey<xAOD::ElectronContainer>
    m_lastEgMotherTruthOrigin{ this, "lastEgMotherTruthOrigin", m_electronContainer, "lastEgMotherTruthOrigin", "" };
    SG::WriteDecorHandleKey<xAOD::ElectronContainer>
    m_lastEgMotherTruthClassification{ this, "lastEgMotherTruthClassification",
      m_electronContainer, "lastEgMotherTruthClassification", "" };
    SG::WriteDecorHandleKey<xAOD::ElectronContainer>
    m_lastEgMotherTruthParticleLink{ this,
      "lastEgMotherTruthParticleLink",
      m_electronContainer, "lastEgMotherTruthParticleLink",
      "" };
    SG::WriteDecorHandleKey<xAOD::ElectronContainer>
    m_lastEgMotherPdgId{ this, "lastEgMotherPdgId", m_electronContainer, "lastEgMotherPdgId", "" };
  };
}

#endif // DERIVATIONFRAMEWORK_BKGELECTRONCLASSIFICATION_H
