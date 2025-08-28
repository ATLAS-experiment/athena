/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// PhysValDdiTau.cxx 
// Implementation file for class PhysValDiTau
// Author: S.Binet<binet@cern.ch>
// Author: A.DeMaria<antonio.de.maria@cern.ch>

// PhysVal includes
#include "PhysValDiTau.h"

// STL includes
#include <vector>

// FrameWork includes
#include "GaudiKernel/IToolSvc.h"
#include "xAODJet/JetContainer.h"
#include "AthenaBaseComps/AthCheckMacros.h"
#include "TruthUtils/HepMCHelpers.h"
#include "AthContainers/ConstAccessor.h"


PhysValDiTau::PhysValDiTau(const std::string& type, 
		         const std::string& name, 
                         const IInterface* parent) : 
  ManagedMonitorToolBase(type, name, parent)
{
}


StatusCode PhysValDiTau::initialize()
{
  ATH_MSG_INFO ("Initializing " << name() << "...");    
  ATH_CHECK(ManagedMonitorToolBase::initialize());

  // selections are configured in PhysicsValidation job options
  ATH_CHECK(m_nomiDiTauSel.retrieve());

  if ( m_isMC ) {
    ATH_CHECK(m_truthTool.retrieve());
  }

  return StatusCode::SUCCESS;
}

StatusCode PhysValDiTau::bookHistograms()
{
  ATH_MSG_INFO ("Booking hists " << name() << "...");
   
  // Physics validation plots are level 10
  m_oDiTauValidationPlots.reset(new DiTauValidationPlots(0,"Tau/" + m_DiTauJetContainerName + "_", m_DiTauJetContainerName));
  m_oDiTauValidationPlots->setDetailLevel(100);
  m_oDiTauValidationPlots->initialize();
  std::vector<HistData> hists = m_oDiTauValidationPlots->retrieveBookedHistograms();
  ATH_MSG_INFO ("Filling n of hists " << hists.size() << " ");
  for (const auto& hist : hists) {
    ATH_CHECK(regHist(hist.first,hist.second,all));
  }
   
  return StatusCode::SUCCESS;      
}

StatusCode PhysValDiTau::fillHistograms()
{
  ATH_MSG_DEBUG ("Filling hists " << name() << "...");

  // Retrieve tau container
  const xAOD::DiTauJetContainer* ditaus = nullptr;
  if(evtStore()->contains<xAOD::DiTauJetContainer>(m_DiTauJetContainerName)){
    ATH_CHECK( evtStore()->retrieve(ditaus, m_DiTauJetContainerName) ); 
  } else {
    ATH_MSG_INFO("Input collection " << m_DiTauJetContainerName << " not found. Skip the monitoring ..");
    return StatusCode::SUCCESS;   
  } 


  ATH_MSG_DEBUG("Number of ditaus: " << ditaus->size());
  
  // Retrieve event info and beamSpotWeight
  const xAOD::EventInfo* eventInfo = nullptr;
  ATH_CHECK( evtStore()->retrieve(eventInfo, "EventInfo") );
  
  float weight = eventInfo->beamSpotWeight();

  // Loop through recoonstructed tau container
  for (auto ditau : *ditaus) {
    if ( m_detailLevel < 10 ) continue;

    bool nominal = static_cast<bool>(m_nomiDiTauSel->accept(*ditau));
      
    // fill histograms for reconstructed taus
    m_oDiTauValidationPlots->m_oNewCorePlots.fill(*ditau, weight);
    if(nominal) {
       m_oDiTauValidationPlots->m_oNewCorePlotsNom.fill(*ditau, weight);
    }

    // Don't fill truth and fake histograms if we are running on data.
    if ( !m_isMC ) continue;

    ATH_MSG_DEBUG("Trying to truth-match ditau");
    m_truthTool->getTruth(*ditau);

    static const SG::ConstAccessor<char> IsTruthMatchedAcc("IsTruthHadronic");
    if ( (bool)IsTruthMatchedAcc(*ditau) ) {
       m_oDiTauValidationPlots->m_oNewCorePlotsTrue.fill(*ditau, weight);
       if(nominal){
          m_oDiTauValidationPlots->m_oNewCorePlotsNomTrue.fill(*ditau, weight);
          m_oDiTauValidationPlots->m_oNewResolutionPlotsTrue.fill(*ditau, weight);	  
       }  
    } else {
       m_oDiTauValidationPlots->m_oNewCorePlotsFake.fill(*ditau, weight);
       if(nominal){
          m_oDiTauValidationPlots->m_oNewCorePlotsNomFake.fill(*ditau, weight); 	       
       }
    }
  }

  return StatusCode::SUCCESS;
}

StatusCode PhysValDiTau::procHistograms()
{
  ATH_MSG_INFO ("Finalising hists " << name() << "...");
  return StatusCode::SUCCESS;
}
