/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/*
 * @file DerivationFrameworkSUSY/SUSYGenFilterTool.h
 * @author TJ Khoo
 * @date July 2015
 * @brief tool to decorate EventInfo with quantities needed to disentangle generator filtered samples
*/


#ifndef DerivationFramework_SUSYGenFilterTool_H
#define DerivationFramework_SUSYGenFilterTool_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"

#include "xAODEventInfo/EventInfo.h"
#include "xAODJet/JetContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "MCTruthClassifier/MCTruthClassifierDefs.h"
#include "MCTruthClassifier/IMCTruthClassifier.h"

namespace DerivationFramework {

  class SUSYGenFilterTool : public extends<AthAlgTool, IAugmentationTool> {

  public:
    SUSYGenFilterTool(const std::string& t, const std::string& n, const IInterface* p);
    ~SUSYGenFilterTool();
    virtual StatusCode initialize() override;
    virtual StatusCode addBranches(const EventContext& ctx) const override;

  private:
    StatusCode getGenFiltVars(const xAOD::TruthParticleContainer& tpc, float& genFiltHT, float& genFiltMET, const EventContext& ctx) const;
    bool isPrompt( const xAOD::TruthParticle* tp ) const;
    MCTruthPartClassifier::ParticleOrigin getPartOrigin(const xAOD::TruthParticle* tp) const;

    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this, "EventInfoName", "EventInfo"};
    SG::ReadHandleKey<xAOD::TruthParticleContainer> m_mcName{this, "MCCollectionName", "TruthParticles"};
    SG::ReadHandleKey<xAOD::JetContainer> m_truthJetsName{this, "TruthJetCollectionName", "AntiKt4TruthWZJets"};

    Gaudi::Property<float> m_MinJetPt{this, "MinJetPt", 35e3};  //!< Min pT for the truth jets
    Gaudi::Property<float> m_MaxJetEta{this, "MaxJetEta", 2.5}; //!< Max eta for the truth jets
    Gaudi::Property<float> m_MinLepPt{this, "MinLeptonPt", 25e3};  //!< Min pT for the truth leptons
    Gaudi::Property<float> m_MaxLepEta{this, "MaxLeptonEta", 2.5}; //!< Max eta for the truth leptons

    PublicToolHandle<IMCTruthClassifier> m_classif{this, "MCTruthClassifier", "MCTruthClassifier/SUSYGenFilt_MCTruthClassifier"};

  };

}

#endif
