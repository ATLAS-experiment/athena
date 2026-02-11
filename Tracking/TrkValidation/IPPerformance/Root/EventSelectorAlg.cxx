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
    : ETAlgorithm(name, pSvcLocator),
      //: AthAlgorithm(name, pSvcLocator),
      m_PU_default_channel(0),
      m_cutflowHist(nullptr) 
{
  Info("EventSelectorAlg()", "Calling constructor");

  m_debug                  = false;                 // debug flag
  m_applyGRLCut            = true;                  // apply GRL cut
  m_applyPrimaryVertexCut  = true;                  // apply primary vertex cut
  m_applyTriggerCut        = true;                  // apply trigger cuts flat
  m_applyEventCleaningCut  = true;                  // apply Event Cleaning flag
  m_applyPUreweighting     = false;                 // apply the pileup reweighting

  m_triggerSelection       = "HLT_j[0-9]*";         // list of triggers
  m_inVertexContName       = "PrimaryVertices";     // primary vertex container name
  m_PVNTrack               = 3;                     // number of tracks required for a primary vertex
  m_GRLExcludeList         = "";                    // exclude these runs (even if on GRL)
  m_GRLxml                 =
      "$ROOTCOREBIN/data/IPPerformance/data15_13TeV.periodAllYear_DetStatus-v73-pro19-08_DQDefects-00-01-02_PHYS_StandardGRL_All_Good_25ns.xml";  // data15_13TeV.periodAllYear_DetStatus-v71-pro19-06_DQDefects-00-01-02_PHYS_StandardGRL_All_Good_25ns_tolerable_IBLSTANDBY-DISABLE.xml";//data15_13TeV.periodAllYear_DetStatus-v63-pro18-01_DQDefects-00-01-02_PHYS_StandardGRL_All_Good.xml";
                                             // //https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/GoodRunListsForAnalysis

  m_lumiCalcFileNames = "";  // do i need these?
  m_PRWFileNames = "";       //

  // This is not necessary
}

EventSelectorAlg ::~EventSelectorAlg() {}

StatusCode EventSelectorAlg::initialize() {
 
  // configure()!!!
  Info("initialize()", "Initializing event selection ...");

  setConfig(m_configFileName);

  //TODO: use TEnv to config or just configure the variables in python script?
  if (!getConfig().empty()) {
    Info("configure()", "Configuing EventSelectorAlg Interface. User configuration read from : %s ", getConfig().c_str());

    TEnv* config = new TEnv(getConfig(true).c_str());

    // config variables
    m_applyGRLCut                  = config->GetValue("ApplyGRLCut", m_applyGRLCut);
    m_GRLxml                       = config->GetValue("GRL", m_GRLxml.value().c_str());
    m_GRLExcludeList               = config->GetValue("GRLExclude", m_GRLExcludeList.value().c_str());
    m_inVertexContName             = config->GetValue("VertexContainer", m_inVertexContName.c_str());
    m_applyPrimaryVertexCut        = config->GetValue("ApplyPrimaryVertexCut", m_applyPrimaryVertexCut);
    m_PVNTrack                     = config->GetValue("NTrackForPrimaryVertex", m_PVNTrack);
    m_applyEventCleaningCut        = config->GetValue("ApplyEventCleaningCut", m_applyEventCleaningCut);
    m_triggerSelection             = config->GetValue("Trigger", m_triggerSelection.c_str());
    m_applyTriggerCut              = config->GetValue("ApplyTriggerCut", m_applyTriggerCut);
    m_testTrigger                  = config->GetValue("testTrigger", m_testTrigger);
    m_applyPUreweighting           = config->GetValue("ApplyPUreweighting", m_applyPUreweighting);
    m_lumiCalcFileNames            = config->GetValue("LumiCalcFiles", m_lumiCalcFileNames.c_str());
    m_PRWFileNames                 = config->GetValue("PRWFiles", m_PRWFileNames.c_str());
    m_PU_default_channel           = config->GetValue("PUDefaultChannel", m_PU_default_channel);
    m_debug                        = config->GetValue("Debug", m_debug);

    if (!m_triggerSelection.empty())
      Info("configure()", "Using Trigger %s",
           m_triggerSelection.c_str());
    if (!m_applyTriggerCut)
      Info("configure()", "WILL NOT CUT ON TRIGGER AS YOU REQUESTED!");

    if (m_applyPUreweighting) {
      if (m_lumiCalcFileNames.size() == 0) {
        Error("BasicEventSelection()",
              "Pileup Reweighting is requested but no LumiCalc file is "
              "specified. Exiting");
        return StatusCode::FAILURE;
      }
      if (m_PRWFileNames.size() == 0) {
        Error("BasicEventSelection()",
              "Pileup Reweighting is requested but no PRW file is specified. "
              "Exiting");
        return StatusCode::FAILURE;
      }
    }

    config->Print();
    Info("configure()", "EventSelectorAlg Interface succesfully configured! ");

    delete config;
    config = nullptr;
  }  // configure() end!!
 

  //TODO: the eventInfo can't be retrived in initialize()?
  // evtStore()->retrieve (eventInfo, "EventInfo")
  //  const xAOD::EventInfo* eventInfo = 0;
  //  ATH_CHECK (evtStore()->retrieve (eventInfo, "EventInfo"));
  //
  //  Info("initialize()", "Checking if this is data or MC...");
  //  m_isMC = eventInfo->eventType( xAOD::EventInfo::IS_SIMULATION );

  if (m_debug) {
    Info("initialize()", "Is MC? %i", static_cast<int>(m_isMC));
  }

  //@TODO: use ATH logging
  Info("initialize()", "Setting up cutflow...");

  // write the cutflows to this file so algos downstream can pick up the pointer

  m_cutflowHist = new TH1D("cutflow", "cutflow", 1, 1, 2);
  ATH_CHECK( histSvc()->regHist("/MYSTREAM/cutflow",m_cutflowHist) );
 
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
  const xAOD::EventInfo* eventInfo = 0;
  ATH_CHECK(evtStore()->retrieve(eventInfo, "EventInfo"));
  
  ++m_eventCounter;
  const xAOD::VertexContainer* vertices = 0;
  if (!evtStore()->retrieve(vertices, m_inVertexContName).isSuccess()) {
    Error("execute()",
          "Failed to retrieve Input Vertex container from event. Exiting.");
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
    if (!passPrimaryVertexSelection(vertices, m_PVNTrack)) {
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
