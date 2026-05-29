// extra include(s):
#include <EventBookkeeperTools/FilterReporter.h>
#include "AsgDataHandles/WriteDecorHandle.h"

// EDM include(s):
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODTracking/VertexContainer.h"

// RootCore include(s):
#include "PATInterfaces/CorrectionCode.h"
#include "TrigConfxAOD/xAODConfigTool.h"
#include "TrigDecisionTool/TrigDecisionTool.h"

// local include(s):
#include "IPPerformance/EventSelectorAlg.h"

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
      : AthAlgorithm(name, pSvcLocator),
      m_cutflowHist(nullptr) 
{
  ATH_MSG_INFO("EventSelectorAlg(): Calling constructor");

}

EventSelectorAlg ::~EventSelectorAlg() {}

StatusCode EventSelectorAlg::initialize() {
 
  //@TODO: use ATH logging
  ATH_MSG_INFO("initialize(): Setting up cutflow...");
  ATH_CHECK( m_inVertexKey.initialize() );
  // write the cutflows to this file so algos downstream can pick up the pointer
  ServiceHandle<ITHistSvc> histSvc("THistSvc","EventSelectorAlg");
  ATH_CHECK( histSvc.retrieve() );
  m_cutflowHist = new TH1D("cutflow", "cutflow", 1, 1, 2);
  ATH_CHECK( histSvc->regHist("/MYSTREAM/cutflow",m_cutflowHist) );
 
  m_cutflowHist->SetCanExtend(TH1::kAllAxes);

  m_cutflow_all = m_cutflowHist->GetXaxis()->FindBin("all");

  if (!m_isMC) {
    m_cutflow_lar = m_cutflowHist->GetXaxis()->FindBin("LAr");
    m_cutflow_tile = m_cutflowHist->GetXaxis()->FindBin("tile");
    m_cutflow_core = m_cutflowHist->GetXaxis()->FindBin("core");
  }  // m_isMC

  m_cutflow_npv = m_cutflowHist->GetXaxis()->FindBin("NPV");

  ATH_MSG_INFO("initialize(): Setting Up Tools");

  m_eventCounter = 0;

  ATH_MSG_INFO("initialize(): EventSelectorAlg Interface succesfully initialized!");

  return StatusCode::SUCCESS;
}

StatusCode EventSelectorAlg::finalize() {

  ATH_MSG_INFO("finalize(): Number of processed events      = " << m_eventCounter);

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

  m_cutflowHist->Fill(m_cutflow_all, 1);

  if (!m_isMC) {

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

  //*****Primary Vertex****************
  if (m_applyPrimaryVertexCut) {
    if (!passPrimaryVertexSelection(vertices.cptr(), m_PVNTrack)) {
      return StatusCode::SUCCESS;
    }
  }
  m_cutflowHist->Fill(m_cutflow_npv, 1);
  //***********************************

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
