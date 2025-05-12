/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TauAODRunnerAlg.h"

TauAODRunnerAlg::TauAODRunnerAlg(const std::string &name, ISvcLocator *pSvcLocator) : 
  AthReentrantAlgorithm(name, pSvcLocator) {}


StatusCode TauAODRunnerAlg::initialize() {
  ATH_CHECK(m_tauContainer.initialize());
  ATH_CHECK(m_pi0ClusterInputContainer.initialize(SG::AllowEmpty));
  ATH_CHECK(m_tauOutContainer.initialize());
  ATH_CHECK(m_pi0Container.initialize(SG::AllowEmpty));
  ATH_CHECK(m_neutralPFOOutputContainer.initialize(SG::AllowEmpty));
  ATH_CHECK(m_chargedPFOOutputContainer.initialize(SG::AllowEmpty));
  ATH_CHECK(m_hadronicPFOOutputContainer.initialize(SG::AllowEmpty));
  ATH_CHECK(m_tauTrackOutputContainer.initialize(SG::AllowEmpty));
  ATH_CHECK(m_vertexOutputContainer.initialize(SG::AllowEmpty));

  ATH_CHECK(m_modificationTools.retrieve());
  ATH_CHECK(m_officialTools.retrieve());

  if(!m_modificationTools.empty()) {
    ATH_MSG_INFO("List of modification tools in execution sequence:");
    ATH_MSG_INFO("------------------------------------");
    for (const auto &tool : m_modificationTools) {
      ATH_MSG_INFO(tool->type() << " - " << tool->name());
    }
    ATH_MSG_INFO("------------------------------------");
  } else {
    ATH_MSG_INFO("Running without modification tools");
  }

  if(!m_officialTools.empty()) {
    ATH_MSG_INFO("List of official tools in execution sequence:");
    ATH_MSG_INFO("------------------------------------");
    for (const auto &tool : m_officialTools) {
      ATH_MSG_INFO(tool->type() << " - " << tool->name());

      if((tool->type() == "TauPi0ClusterCreator" && (m_neutralPFOOutputContainer.empty() || m_hadronicPFOOutputContainer.empty() || m_pi0ClusterInputContainer.empty()))
         || (tool->type() == "TauVertexVariables" && m_vertexOutputContainer.empty())
         || (tool->type() == "TauPi0ClusterScaler" && (m_neutralPFOOutputContainer.empty() || m_chargedPFOOutputContainer.empty()))
         || (tool->type() == "TauPi0ScoreCalculator" && m_neutralPFOOutputContainer.empty())
         || (tool->type() == "TauPi0Selector" && m_neutralPFOOutputContainer.empty())
         || (tool->type() == "PanTau::PanTauProcessor" && (m_neutralPFOOutputContainer.empty() || m_pi0Container.empty()))
         || (tool->type() == "tauRecTools::TauTrackRNNClassifier" && m_tauTrackOutputContainer.empty())) {
        ATH_MSG_ERROR("Missing input/output containers required for tool " << tool->name() << " (" << tool->type() << ")");
        return StatusCode::FAILURE;
      } 
    }
    ATH_MSG_INFO("------------------------------------");
  } else {
    ATH_MSG_INFO("Running without official tools");
  }

  if(m_modificationTools.empty() && m_officialTools.empty()) {
    ATH_MSG_ERROR("Could not allocate any tool!");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}


StatusCode TauAODRunnerAlg::execute (const EventContext& ctx) const {
  // Input TauJets
  SG::ReadHandle<xAOD::TauJetContainer> tauInputHandle(m_tauContainer, ctx);
  if (!tauInputHandle.isValid()) {
    ATH_MSG_ERROR("Could not retrieve TauJetContainer with key " << tauInputHandle.key());
    return StatusCode::FAILURE;
  }
  const xAOD::TauJetContainer *pTauContainer = tauInputHandle.cptr();
  

  // Output TauTracks
  xAOD::TauTrackContainer* newTauTrkCon = nullptr;
  SG::WriteHandle<xAOD::TauTrackContainer> outputTauTrackHandle;
  if(!m_tauTrackOutputContainer.empty()) {
    outputTauTrackHandle = SG::makeHandle(m_tauTrackOutputContainer, ctx);
    ATH_CHECK(outputTauTrackHandle.record(std::make_unique<xAOD::TauTrackContainer>(), std::make_unique<xAOD::TauTrackAuxContainer>()));
    newTauTrkCon = outputTauTrackHandle.ptr();
  }

  // Output TauJets
  SG::WriteHandle<xAOD::TauJetContainer> outputTauHandle(m_tauOutContainer, ctx);
  ATH_CHECK(outputTauHandle.record(std::make_unique<xAOD::TauJetContainer>(), std::make_unique<xAOD::TauJetAuxContainer>()));
  xAOD::TauJetContainer *newTauCon = outputTauHandle.ptr();

  static const SG::Accessor<ElementLink<xAOD::TauJetContainer>> acc_ori_tau_link("originalTauJet");
  static const SG::Accessor<char> acc_modified("ModifiedInAOD");

  for (const xAOD::TauJet *tau : *pTauContainer) {
    // Deep copy the tau container
    xAOD::TauJet* newTau = newTauCon->push_back(std::make_unique<xAOD::TauJet>());
    *newTau = *tau;

    // Link the original tau to the deepcopy
    ElementLink<xAOD::TauJetContainer> link_to_ori_tau;
    link_to_ori_tau.toContainedElement(*pTauContainer, tau);
    acc_ori_tau_link(*newTau) = link_to_ori_tau;

    // If the output TauTrackContainer is declared, create a new copy of all existing TauTracks
    // otherwise, reuse the same TauTracks (e.g. if only new TauID is being ran on a deep copy 
    // of the input TauJet container).
    if(newTauTrkCon) {
      // Clear the tautrack links to allow relinking.
      newTau->clearTauTrackLinks();
      for(const xAOD::TauTrack *tauTrk : tau->allTracks()) {
        // Deep copy the tau track
        xAOD::TauTrack* newTauTrk = newTauTrkCon->push_back(std::make_unique<xAOD::TauTrack>());
        *newTauTrk = *tauTrk;

        // Relink the tautrack
        ElementLink<xAOD::TauTrackContainer> linkToTauTrack;
        linkToTauTrack.toContainedElement(*newTauTrkCon, newTauTrk);
        newTau->addTauTrackLink(linkToTauTrack);
      }
    }

    // 'ModifiedInAOD' will be overriden by modification tools for relevant candidates
    acc_modified(*newTau) = static_cast<char>(false);

    // Execute all the modification tools (if provided)
    for(const ToolHandle<ITauToolBase> &tool : m_modificationTools) {
      ATH_MSG_DEBUG("RunnerAlg Invoking tool " << tool->name());
      if(tool->execute(*newTau).isFailure()) break;
    }

    // If tau candidate was not modified and we ran modification tools, remove it from container
    // The track cleanup is performed by the thinning algorithm downstream
    if(!m_modificationTools.empty() && !acc_modified(*newTau)) {
      newTauCon->pop_back();
    }
  }


  // Read the CaloClusterContainer
  const xAOD::CaloClusterContainer* pi0ClusterContainer = nullptr;
  SG::ReadHandle<xAOD::CaloClusterContainer> pi0ClusterInHandle;
  if(!m_pi0ClusterInputContainer.empty()) {
    pi0ClusterInHandle = SG::makeHandle(m_pi0ClusterInputContainer, ctx);
    if(!pi0ClusterInHandle.isValid()) {
      ATH_MSG_ERROR ("Could not retrieve HiveDataObj with key " << pi0ClusterInHandle.key());
      return StatusCode::FAILURE;
    }
    pi0ClusterContainer = pi0ClusterInHandle.cptr();
  }

  // Write charged PFO container
  xAOD::PFOContainer* chargedPFOContainer = nullptr;
  SG::WriteHandle<xAOD::PFOContainer> chargedPFOHandle;
  if(!m_chargedPFOOutputContainer.empty()) {
    chargedPFOHandle = SG::makeHandle(m_chargedPFOOutputContainer, ctx);
    ATH_CHECK(chargedPFOHandle.record(std::make_unique<xAOD::PFOContainer>(), std::make_unique<xAOD::PFOAuxContainer>()));
    chargedPFOContainer = chargedPFOHandle.ptr();
  }

  // Write neutral PFO container
  xAOD::PFOContainer* neutralPFOContainer = nullptr;
  SG::WriteHandle<xAOD::PFOContainer> neutralPFOHandle;
  if(!m_neutralPFOOutputContainer.empty()) {
    neutralPFOHandle = SG::makeHandle(m_neutralPFOOutputContainer, ctx);
    ATH_CHECK(neutralPFOHandle.record(std::make_unique<xAOD::PFOContainer>(), std::make_unique<xAOD::PFOAuxContainer>()));
    neutralPFOContainer = neutralPFOHandle.ptr();
  }

  // Write pi0 container
  xAOD::ParticleContainer* pi0Container = nullptr;
  SG::WriteHandle<xAOD::ParticleContainer> pi0Handle;
  if(!m_pi0Container.empty()) {
    pi0Handle = SG::makeHandle(m_pi0Container, ctx);
    ATH_CHECK(pi0Handle.record(std::make_unique<xAOD::ParticleContainer>(), std::make_unique<xAOD::ParticleAuxContainer>()));
    pi0Container = pi0Handle.ptr();
  }

  // Write hadronic cluster PFO container
  xAOD::PFOContainer* hadronicClusterPFOContainer = nullptr;
  SG::WriteHandle<xAOD::PFOContainer> hadronicPFOHandle;
  if(!m_hadronicPFOOutputContainer.empty()) {
    hadronicPFOHandle = SG::makeHandle(m_hadronicPFOOutputContainer, ctx);
    ATH_CHECK(hadronicPFOHandle.record(std::make_unique<xAOD::PFOContainer>(), std::make_unique<xAOD::PFOAuxContainer>()));
    hadronicClusterPFOContainer = hadronicPFOHandle.ptr();
  }

  // Write secondary vertices
  xAOD::VertexContainer* pSecVtxContainer = nullptr;
  SG::WriteHandle<xAOD::VertexContainer> vertOutHandle;
  if(!m_vertexOutputContainer.empty()) {
    vertOutHandle = SG::makeHandle(m_vertexOutputContainer, ctx);
    ATH_CHECK(vertOutHandle.record(std::make_unique<xAOD::VertexContainer>(), std::make_unique<xAOD::VertexAuxContainer>()));
    pSecVtxContainer = vertOutHandle.ptr();
  }

  
  // Execute all post-modification (_official_) tools
  for (xAOD::TauJet *pTau : *newTauCon) {
    StatusCode sc = StatusCode::SUCCESS;
    for (const ToolHandle<ITauToolBase> &tool : m_officialTools) {
      ATH_MSG_DEBUG("RunnerAlg Invoking tool " << tool->name());
      if (tool->type() == "TauPi0ClusterCreator")
        sc = tool->executePi0ClusterCreator(*pTau, *neutralPFOContainer, *hadronicClusterPFOContainer, *pi0ClusterContainer);
      else if (tool->type() == "TauVertexVariables")
        sc = tool->executeVertexVariables(*pTau, *pSecVtxContainer);
      else if (tool->type() == "TauPi0ClusterScaler")
        sc = tool->executePi0ClusterScaler(*pTau, *neutralPFOContainer, *chargedPFOContainer);
      else if (tool->type() == "TauPi0ScoreCalculator")
        sc = tool->executePi0nPFO(*pTau, *neutralPFOContainer);
      else if (tool->type() == "TauPi0Selector")
        sc = tool->executePi0nPFO(*pTau, *neutralPFOContainer);
      else if (tool->type() == "PanTau::PanTauProcessor")
        sc = tool->executePanTau(*pTau, *pi0Container, *neutralPFOContainer);
      else if (tool->type() == "tauRecTools::TauTrackRNNClassifier")
        sc = tool->executeTrackClassifier(*pTau, *newTauTrkCon);
      else
        sc = tool->execute(*pTau);

      if (sc.isFailure()) break;
    }
    if (sc.isSuccess()) ATH_MSG_VERBOSE("The tau candidate has been modified successfully by the invoked official tools.");
  }

  ATH_MSG_VERBOSE("The tau candidate container has been modified by the rest of the tools");
  ATH_MSG_DEBUG(newTauCon->size() << " / " << pTauContainer->size() <<" taus were modified");

  return StatusCode::SUCCESS;
}


// Helper 
bool TauAODRunnerAlg::isTauModified(const xAOD::TauJet* newtau) {
  static const SG::ConstAccessor<char> acc_modified("ModifiedInAOD");
  return acc_modified(*newtau);
}
