/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TauRunnerAlg.h"

#include "xAODJet/Jet.h"
#include "xAODJet/JetContainer.h"

#include "xAODTau/TauJetContainer.h"
#include "xAODTau/TauJetAuxContainer.h"
#include "xAODTau/TauDefs.h"
#include "xAODTau/TauTrackContainer.h"
#include "xAODTau/TauTrackAuxContainer.h"

#include "xAODCore/ShallowCopy.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include "CaloUtils/CaloClusterStoreHelper.h"

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
TauRunnerAlg::TauRunnerAlg(const std::string &name,
			   ISvcLocator * pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator) {
}

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
TauRunnerAlg::~TauRunnerAlg() = default;

//-----------------------------------------------------------------------------
// Initializer
//-----------------------------------------------------------------------------
StatusCode TauRunnerAlg::initialize() {

  //-------------------------------------------------------------------------
  // No tools allocated!
  //-------------------------------------------------------------------------
  if (m_tools.empty()) {
    ATH_MSG_ERROR("no tools given!");
    return StatusCode::FAILURE;
  }

  ATH_CHECK( m_tauInputContainer.initialize() );
  ATH_CHECK( m_pi0ClusterInputContainer.initialize() );

  ATH_CHECK( m_tauOutputContainer.initialize() );
  ATH_CHECK( m_neutralPFOOutputContainer.initialize() );
  ATH_CHECK( m_hadronicPFOOutputContainer.initialize() );
  ATH_CHECK( m_vertexOutputContainer.initialize() );
  ATH_CHECK( m_chargedPFOOutputContainer.initialize() );
  ATH_CHECK( m_pi0Container.initialize() );

  ATH_CHECK( m_tauShotClusOutputContainer.initialize() );
  ATH_CHECK( m_tauShotClusLinkContainer.initialize() );
  ATH_CHECK( m_tauShotPFOOutputContainer.initialize() );

  //-------------------------------------------------------------------------
  // Allocate tools
  //-------------------------------------------------------------------------
  ATH_CHECK( m_tools.retrieve() );

  ATH_MSG_INFO("List of tools in execution sequence:");
  ATH_MSG_INFO("------------------------------------");
  unsigned int tool_count = 0;
  for (ToolHandle<ITauToolBase>& tool : m_tools) {
    ++tool_count;
    ATH_MSG_INFO(tool->type() << " - " << tool->name());
  }
  ATH_MSG_INFO(" ");
  ATH_MSG_INFO("------------------------------------");

  if (tool_count == 0) {
    ATH_MSG_ERROR("could not allocate any tool!");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}


//-----------------------------------------------------------------------------
// Execution
//-----------------------------------------------------------------------------
StatusCode TauRunnerAlg::execute(const EventContext& ctx) const {
  
  // write neutral PFO container
  SG::WriteHandle<xAOD::PFOContainer> neutralPFOHandle(m_neutralPFOOutputContainer, ctx);
  ATH_CHECK(neutralPFOHandle.record(std::make_unique<xAOD::PFOContainer>(), std::make_unique<xAOD::PFOAuxContainer>()));
  xAOD::PFOContainer* neutralPFOContainer = neutralPFOHandle.ptr();

  // write hadronic cluster PFO container
  SG::WriteHandle<xAOD::PFOContainer> hadronicPFOHandle(m_hadronicPFOOutputContainer, ctx);
  ATH_CHECK(hadronicPFOHandle.record(std::make_unique<xAOD::PFOContainer>(), std::make_unique<xAOD::PFOAuxContainer>()));
  xAOD::PFOContainer* hadronicClusterPFOContainer = hadronicPFOHandle.ptr();

  // write secondary vertices
  SG::WriteHandle<xAOD::VertexContainer> vertOutHandle(m_vertexOutputContainer, ctx);
  ATH_CHECK(vertOutHandle.record(std::make_unique<xAOD::VertexContainer>(), std::make_unique<xAOD::VertexAuxContainer>()));
  xAOD::VertexContainer* pSecVtxContainer = vertOutHandle.ptr();

  // write charged PFO container
  SG::WriteHandle<xAOD::PFOContainer> chargedPFOHandle(m_chargedPFOOutputContainer, ctx);
  ATH_CHECK(chargedPFOHandle.record(std::make_unique<xAOD::PFOContainer>(), std::make_unique<xAOD::PFOAuxContainer>()));
  xAOD::PFOContainer* chargedPFOContainer = chargedPFOHandle.ptr();

  // write pi0 container
  SG::WriteHandle<xAOD::ParticleContainer> pi0Handle(m_pi0Container, ctx);
  ATH_CHECK(pi0Handle.record(std::make_unique<xAOD::ParticleContainer>(), std::make_unique<xAOD::ParticleAuxContainer>()));
  xAOD::ParticleContainer* pi0Container = pi0Handle.ptr();

  SG::WriteHandle<xAOD::CaloClusterContainer> tauShotClusHandle( m_tauShotClusOutputContainer, ctx );
  ATH_CHECK(tauShotClusHandle.record(std::make_unique<xAOD::CaloClusterContainer>(), std::make_unique<xAOD::CaloClusterAuxContainer>()));
  xAOD::CaloClusterContainer* tauShotClusContainer = tauShotClusHandle.ptr();

  SG::WriteHandle<xAOD::PFOContainer> tauShotPFOHandle( m_tauShotPFOOutputContainer, ctx );
  ATH_CHECK(tauShotPFOHandle.record(std::make_unique<xAOD::PFOContainer>(), std::make_unique<xAOD::PFOAuxContainer>()));
  xAOD::PFOContainer* tauShotPFOContainer = tauShotPFOHandle.ptr();

  // Read the CaloClusterContainer created by the CaloClusterMaker
  SG::ReadHandle<xAOD::CaloClusterContainer> pi0ClusterInHandle(m_pi0ClusterInputContainer, ctx);
  if (!pi0ClusterInHandle.isValid()) {
    ATH_MSG_ERROR ("Could not retrieve HiveDataObj with key " << pi0ClusterInHandle.key());
    return StatusCode::FAILURE;
  }
  const xAOD::CaloClusterContainer * pi0ClusterContainer = pi0ClusterInHandle.cptr();
  
  // Read in temporary tau jets
  SG::ReadHandle<xAOD::TauJetContainer> tauInputHandle(m_tauInputContainer, ctx);
  if (!tauInputHandle.isValid()) {
    ATH_MSG_ERROR ("Could not retrieve HiveDataObj with key " << tauInputHandle.key());
    return StatusCode::FAILURE;
  }
  const xAOD::TauJetContainer* pTauContainer = tauInputHandle.cptr();

  // Write the output tau jets, which is a deep copy of the input ones 
  SG::WriteHandle<xAOD::TauJetContainer> outputTauHandle(m_tauOutputContainer, ctx);
  ATH_CHECK(outputTauHandle.record(std::make_unique<xAOD::TauJetContainer>(), std::make_unique<xAOD::TauJetAuxContainer>()));    
  xAOD::TauJetContainer* newTauCon = outputTauHandle.ptr();

  for (const xAOD::TauJet* tau : *pTauContainer) {
    xAOD::TauJet* newTau = new xAOD::TauJet();
    newTauCon->push_back(newTau);
    *newTau = *tau;
  }
  
  // iterate over the copy
  for (xAOD::TauJet* pTau : *newTauCon) {
    //-----------------------------------------------------------------
    // Loop stops when Failure indicated by one of the tools
    //-----------------------------------------------------------------
    StatusCode sc;
    
    for (const ToolHandle<ITauToolBase>& tool : m_tools) {
      ATH_MSG_DEBUG("RunnerAlg Invoking tool " << tool->name());
      if ( tool->type() == "TauPi0ClusterCreator"){
	sc = tool->executePi0ClusterCreator(*pTau, *neutralPFOContainer, *hadronicClusterPFOContainer, *pi0ClusterContainer);
      }
      else if ( tool->type() == "TauVertexVariables"){
	sc = tool->executeVertexVariables(*pTau, *pSecVtxContainer);
      }
      else if (tool->type() == "TauShotFinder") {
        sc = tool->executeShotFinder(*pTau, *tauShotClusContainer,
                                     *tauShotPFOContainer);
      }
      else if ( tool->type() == "TauPi0ClusterScaler"){
	sc = tool->executePi0ClusterScaler(*pTau, *neutralPFOContainer, *chargedPFOContainer);
      }
      else if ( tool->type() == "TauPi0ScoreCalculator"){
	sc = tool->executePi0nPFO(*pTau, *neutralPFOContainer);
      }
      else if ( tool->type() == "TauPi0Selector"){
	sc = tool->executePi0nPFO(*pTau, *neutralPFOContainer);
      }
      else if ( tool->type() == "PanTau::PanTauProcessor"){
	sc = tool->executePanTau(*pTau, *pi0Container, *neutralPFOContainer);
      }
      else {
	sc = tool->execute(*pTau);
      }
      if (sc.isFailure())  break;
    }
    if (sc.isSuccess()) {
      ATH_MSG_VERBOSE("The tau candidate has been modified successfully by all the invoked tools.");
    }
  } // end iterator over shallow copy

  // build cell link container for shot clusters
  SG::WriteHandle<CaloClusterCellLinkContainer> tauShotClusLinkHandle( m_tauShotClusLinkContainer, ctx );
  ATH_CHECK(CaloClusterStoreHelper::finalizeClusters (tauShotClusLinkHandle, tauShotClusContainer));

  // sort taus by decreasing pt
  auto sortByPt = [](const xAOD::TauJet* tau1, const xAOD::TauJet* tau2 ) { return tau1->pt() > tau2->pt(); };
  newTauCon->sort( sortByPt );

  ATH_MSG_VERBOSE("The tau container has been processed");

  return StatusCode::SUCCESS;
}
