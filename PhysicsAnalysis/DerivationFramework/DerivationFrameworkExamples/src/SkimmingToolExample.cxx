/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// SkimmingToolExample.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Author: James Catmore (James.Catmore@cern.ch)
// Based on the Integrated Simulation Framework
// This is a trivial example of an implementation of a skimming tool 
// which only passes events with N combined muons passing a pt cut of M

#include "SkimmingToolExample.h"
#include "StoreGate/ReadHandle.h"
#include <vector>
#include <string>


StatusCode DerivationFramework::SkimmingToolExample::initialize()
{
  ATH_CHECK(m_muonSGKey.initialize());
  return StatusCode::SUCCESS;
}


StatusCode DerivationFramework::SkimmingToolExample::finalize()
{
  ATH_MSG_INFO("Processed "<< m_ntot <<" events, "<< m_npass<<" events passed filter ");
  return StatusCode::SUCCESS;
}


// The filter itself
bool DerivationFramework::SkimmingToolExample::eventPassesFilter(const EventContext& ctx) const
{
  ++m_ntot;

  // Retrieve muon container
  SG::ReadHandle<xAOD::MuonContainer> muons(m_muonSGKey, ctx);
  if (!muons.isValid()) {
    ATH_MSG_FATAL("No muon collection with name " << m_muonSGKey.key() << " found in StoreGate!");
    return false;
  }
     
  // Loop over muons, count up and set decision
  unsigned int nGoodMu{0};
  for (const xAOD::Muon* muon : *muons) {
    if ( muon->muonType() == xAOD::Muon::MuonType::Combined && muon->pt() > m_muonPtCut ) ++nGoodMu;
  }
  bool acceptEvent{false};
  if (nGoodMu >= m_nMuons) {
    acceptEvent = true;
    ++m_npass;
  }
  return acceptEvent;

}  
  
