// extra include(s):
#include <EventBookkeeperTools/FilterReporter.h>
#include "AsgDataHandles/WriteDecorHandle.h"

// EDM include(s):
#include "xAODEventInfo/EventInfo.h"
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODTracking/VertexContainer.h"

// RootCore include(s):
#include "PATInterfaces/CorrectionCode.h"
#include "TrigConfxAOD/xAODConfigTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"

// local include(s):
#include "IPPerformance/ETAlgorithm.h"
#include "IPPerformance/EventSelectorAlg.h"
#include "IPPerformance/ReturnCheck.h"

// ROOT include(s):
#include "TEnv.h"
#include "TFile.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TSystem.h"

// c++ include(s):
#include <iostream>
#include <sstream>
#include <typeinfo>

EventSelectorAlg ::EventSelectorAlg(const std::string& name,
                                    ISvcLocator* pSvcLocator)
    //: ETAlgorithm(name, pSvcLocator),
      : AthAlgorithm(name, pSvcLocator),
      m_cutflowHist(nullptr) 
{
  Info("EventSelectorAlg()", "Calling constructor");

  m_GRLxml                 =
      "$ROOTCOREBIN/data/IPPerformance/data15_13TeV.periodAllYear_DetStatus-v73-pro19-08_DQDefects-00-01-02_PHYS_StandardGRL_All_Good_25ns.xml";  // data15_13TeV.periodAllYear_DetStatus-v71-pro19-06_DQDefects-00-01-02_PHYS_StandardGRL_All_Good_25ns_tolerable_IBLSTANDBY-DISABLE.xml";//data15_13TeV.periodAllYear_DetStatus-v63-pro18-01_DQDefects-00-01-02_PHYS_StandardGRL_All_Good.xml";
                                             // //https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/GoodRunListsForAnalysis


  // This is not necessary
}

EventSelectorAlg ::~EventSelectorAlg() {}

StatusCode EventSelectorAlg::initialize() {
 
  //@TODO: use ATH logging
  Info("initialize()", "Setting up cutflow...");
  ATH_CHECK( m_inVertexKey.initialize() );
  // write the cutflows to this file so algos downstream can pick up the pointer
  ServiceHandle<ITHistSvc> histSvc("THistSvc","EventSelectorAlg");
  ATH_CHECK( histSvc.retrieve() );
  m_cutflowHist = new TH1D("cutflow", "cutflow", 1, 1, 2);
  ATH_CHECK( histSvc->regHist("/MYSTREAM/cutflow",m_cutflowHist) );
 
  m_cutflowHist->SetCanExtend(TH1::kAllAxes);

  m_cutflow_all = m_cutflowHist->GetXaxis()->FindBin("all");

  if (!m_isMC) {
    if (m_applyGRLCut) {
      m_cutflow_grl = m_cutflowHist->GetXaxis()->FindBin("GRL");
    }
    m_cutflow_lar = m_cutflowHist->GetXaxis()->FindBin("LAr");
    m_cutflow_tile = m_cutflowHist->GetXaxis()->FindBin("tile");
    m_cutflow_core = m_cutflowHist->GetXaxis()->FindBin("core");
  }  // m_isMC

  m_cutflow_npv = m_cutflowHist->GetXaxis()->FindBin("NPV");
  if (!m_triggerSelection.empty() > 0 && m_applyTriggerCut) {
    m_cutflow_trigger = m_cutflowHist->GetXaxis()->FindBin("Trigger");
  }

  Info("initialize()", "Setting Up Tools");

  // setup GRL tool
  ATH_CHECK(m_grlKey.initialize());
  if (m_applyGRLCut) {
    Info("initialize()", "GRL");
    std::vector<std::string> vecStringGRL;
    m_GRLxml = gSystem->ExpandPathName( m_GRLxml.value().c_str() );
    Info("GRLxml is", m_GRLxml.value().c_str()); 
    vecStringGRL.push_back(m_GRLxml);

    ATH_CHECK(m_grl.retrieve());
    ATH_CHECK(m_grl->setProperty("GoodRunsListVec", vecStringGRL));
    ATH_CHECK(m_grl->setProperty("PassThrough", false));
  }

  // setup Trigger tool
  /*if (m_applyTriggerCut) {
    Info("initialize()", "Trigger");
    ATH_CHECK(m_trigConfTool.retrieve());
    ATH_CHECK(m_trigDecisionTool.retrieve());

    ToolHandle<TrigConf::ITrigConfigTool> configHandle(m_trigConfTool.get());
    ATH_CHECK(m_trigDecisionTool->setProperty("ConfigTool", configHandle));
    ATH_CHECK(m_trigDecisionTool->setProperty("TrigDecisionKey", "xTrigDecision"));
    ATH_CHECK(m_trigDecisionTool->setProperty("OutputLevel", MSG::ERROR));
    ATH_CHECK(m_trigDecisionTool->setProperty("AcceptMultipleInstance", true));
  }*/

  m_eventCounter = 0;

  Info("initialize()", "EventSelectorAlg Interface succesfully initialized!");

  return StatusCode::SUCCESS;
}

StatusCode EventSelectorAlg::finalize() {

  // Info("finalize()", "Deleting tool instances...");

  Info("finalize()", "Number of processed events      = %i", m_eventCounter);

  return StatusCode::SUCCESS;
}

StatusCode EventSelectorAlg::execute() {
  
  const EventContext& ctx = Gaudi::Hive::currentContext();
  const xAOD::EventInfo* eventInfo = 0;
  ATH_CHECK(evtStore()->retrieve(eventInfo, "EventInfo"));
  
  ++m_eventCounter;
  SG::ReadHandle<xAOD::VertexContainer> vertices{m_inVertexKey, ctx};
  if (!vertices.isValid()) {
    ATH_MSG_ERROR ("Failed to retrieve Input Vertex container from event. Exiting: " << m_inVertexKey.key() );
    return StatusCode::FAILURE;
  }

  /*if (m_eventCounter == 1 && m_applyTriggerCut) {
    Info("execute()", "*** Triggers used are:\n");
    auto printingTriggerChainGroup =
        m_trigDecisionTool->getChainGroup("HLT_j[0-9]*");
    for (auto& trig : printingTriggerChainGroup->getListOfTriggers()) {
      auto cg = m_trigDecisionTool->getChainGroup(trig);
      if (cg->isPassed()) {
       printf("    %s\n", trig.c_str());
      }
    }
  }*/

  float mcEvtWeight(1.0);
  // float pileupWeight(1.0);
  if (m_isMC) {
    const std::vector<float> weights = eventInfo->mcEventWeights();  // The weights of all the MC events used in the simulation
    if (weights.size() > 0)
      mcEvtWeight = weights[0];
  }

  // decorate with mc event weight
  // TODO: Use WriteDecorHandle for this
  static SG::AuxElement::Decorator<float> mcEvtWeightDecor("mcEventWeight");
  mcEvtWeightDecor(*eventInfo) = mcEvtWeight;

  m_cutflowHist->Fill(m_cutflow_all, 1);

  if (!m_isMC) {

    // *****GRL***********************
    SG::WriteDecorHandle<xAOD::EventInfo, char> dec_isGRLDecorator(m_grlKey);
    if (m_applyGRLCut) {
      bool isSelected = m_grl->passRunLB(*eventInfo);
      dec_isGRLDecorator(*eventInfo) = isSelected;
      if (!isSelected) {
        return StatusCode::SUCCESS;
      }
      if (m_cutflowHist) {
        m_cutflowHist->Fill(m_cutflow_grl, 1);
      }
    }
    //********************************

    // *****Event Cleaning*************
    if (m_applyEventCleaningCut &&
        (eventInfo->errorState(xAOD::EventInfo::LAr) ==
         xAOD::EventInfo::Error)) {
      return StatusCode::SUCCESS;
    }
    m_cutflowHist->Fill(m_cutflow_lar, 1);

    if (m_applyEventCleaningCut &&
        (eventInfo->errorState(xAOD::EventInfo::Tile) ==
         xAOD::EventInfo::Error)) {
      return StatusCode::SUCCESS;
    }
    m_cutflowHist->Fill(m_cutflow_tile, 1);

    if (m_applyEventCleaningCut &&
        (eventInfo->isEventFlagBitSet(xAOD::EventInfo::Core, 18))) {
      return StatusCode::SUCCESS;
    }
    m_cutflowHist->Fill(m_cutflow_core, 1);

  }  // if !m_isMC

  /*if (m_applyTriggerCut) {
    auto triggerChainGroup = m_trigDecisionTool->getChainGroup(
        m_triggerSelection);  // const Trig::ChainGroup*
    if (!triggerChainGroup->isPassed()) {
      return StatusCode::SUCCESS;
    }
    m_cutflowHist->Fill(m_cutflow_trigger, 1);

  }*/  // m_applyTriggerCut
  //**************************************

  //*****Primary Vertex****************
  if (m_applyPrimaryVertexCut) {
    if (!passPrimaryVertexSelection(vertices.cptr(), m_PVNTrack)) {
      return StatusCode::SUCCESS;
    }
  }
  m_cutflowHist->Fill(m_cutflow_npv, 1);
  //***********************************

  // ++m_eventCounter;
  // std::cout << m_eventCounter << std::endl;
  return StatusCode::SUCCESS;
}

// Get Number of Vertices with at least Ntracks
bool EventSelectorAlg::passPrimaryVertexSelection(
    const xAOD::VertexContainer* vertexContainer, int Ntracks) {
  const xAOD::Vertex* primaryVertex = getPrimaryVertex(vertexContainer);
  if (!primaryVertex) {
    return false;
  }
  if ((int)(primaryVertex)->nTrackParticles() < Ntracks) {
    return false;
  }
  return true;
}

const xAOD::Vertex* EventSelectorAlg::getPrimaryVertex(
    const xAOD::VertexContainer* vertexContainer) {
  // vertex types are listed on L328 of
  // https://svnweb.cern.ch/trac/atlasoff/browser/Event/xAOD/xAODTracking/trunk/xAODTracking/TrackingPrimitives.h
  for (auto vtx_itr : *vertexContainer) {
    if (vtx_itr->vertexType() != xAOD::VxType::VertexType::PriVtx) {
      continue;
    }
    return vtx_itr;
  }

  return 0;
}
