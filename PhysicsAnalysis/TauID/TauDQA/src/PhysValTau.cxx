/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// PhysValTau.cxx 
// Implementation file for class PhysValTau
// Author: S.Binet<binet@cern.ch>

// PhysVal includes
#include "PhysValTau.h"

// FrameWork includes
#include "GaudiKernel/IToolSvc.h"
#include "TruthUtils/HepMCHelpers.h"


PhysValTau::PhysValTau(const std::string& type, 
		       const std::string& name, 
                       const IInterface* parent) : 
  ManagedMonitorToolBase(type, name, parent)
{
}


StatusCode PhysValTau::initialize()
{
  ATH_MSG_INFO ("Initializing " << name() << "...");    
  ATH_CHECK(ManagedMonitorToolBase::initialize());

  if ( m_isMC ) {
    ATH_CHECK(m_truthTool.retrieve());
  }
  // selections are configured in PhysicsValidation job options
  ATH_CHECK(m_primTauSel.retrieve());
  ATH_CHECK(m_nomiTauSel.retrieve());
   
  return StatusCode::SUCCESS;
}

StatusCode PhysValTau::bookHistograms()
{
  ATH_MSG_INFO ("Booking hists " << name() << "...");
   
  // Physics validation plots are level 10
  m_oTauValidationPlotsNominal.reset(new TauValidationPlotsNominal(0,"Tau/" + m_TauJetContainerName + "_", m_TauJetContainerName));
  m_oTauValidationPlotsNominal->setDetailLevel(100);
  m_oTauValidationPlotsNominal->initialize();
  std::vector<HistData> hists_nominal = m_oTauValidationPlotsNominal->retrieveBookedHistograms();
  ATH_MSG_INFO ("Filling n of nominal hists " << hists_nominal.size() << " ");
  for (const auto& hist : hists_nominal) {
    ATH_CHECK(regHist(hist.first,hist.second,all));
  }


  if(m_TauJetContainerName=="TauJets"){

    m_oTauValidationPlotsNoCuts.reset(new TauValidationPlotsNoCuts(0,"Tau/" + m_TauJetContainerName + "_", m_TauJetContainerName));
    m_oTauValidationPlotsNoCuts->setDetailLevel(100);
    m_oTauValidationPlotsNoCuts->initialize();
    std::vector<HistData> hists_nocuts = m_oTauValidationPlotsNoCuts->retrieveBookedHistograms();
    ATH_MSG_INFO ("Filling n of no cuts hists " << hists_nocuts.size() << " ");
    for (const auto& hist : hists_nocuts) {
      ATH_CHECK(regHist(hist.first,hist.second,all));
    }

  }
   
  return StatusCode::SUCCESS;      
}

StatusCode PhysValTau::fillHistograms()
{
  ATH_MSG_DEBUG ("Filling hists " << name() << "...");

  // Retrieve tau container
  const xAOD::TauJetContainer* taus = nullptr;
  if(evtStore()->contains<xAOD::TauJetContainer>(m_TauJetContainerName)){
      ATH_CHECK( evtStore()->retrieve(taus, m_TauJetContainerName) ); 
  } else {
      ATH_MSG_INFO("Input collection " << m_TauJetContainerName << " not found. Skip the monitoring ..");
      return StatusCode::SUCCESS;   
  } 

  ATH_MSG_DEBUG("Number of taus: " << taus->size());

  
  // Retrieve event info and beamSpotWeight
  const xAOD::EventInfo* eventInfo = nullptr;
  ATH_CHECK( evtStore()->retrieve(eventInfo, "EventInfo") );
  
  float weight = eventInfo->beamSpotWeight();

  // Loop through recoonstructed tau container
  for (auto tau : *taus) {
    if ( m_detailLevel < 10 ) continue;
    if ( !static_cast<bool>(m_primTauSel->accept(*tau)) ) continue;
    bool nominal = static_cast<bool>(m_nomiTauSel->accept(*tau));
      
    // fill histograms for reconstructed taus
    if(m_TauJetContainerName=="TauJets"){
      m_oTauValidationPlotsNoCuts->m_oRecoTauAllProngsPlots.fill(*tau, weight);
      m_oTauValidationPlotsNoCuts->m_oNewCorePlots.fill(*tau, weight);
      m_oTauValidationPlotsNoCuts->m_oRecTauEffPlots.fill(*tau, weight);
      m_oTauValidationPlotsNoCuts->m_oRecoGeneralTauAllProngsPlots.fill(*tau, weight);
    }
    if ( nominal ) {
      m_oTauValidationPlotsNominal->m_oRecoGeneralNom.fill(*tau, weight);
      m_oTauValidationPlotsNominal->m_oRecTauEffPlotsNom.fill(*tau, weight);
      m_oTauValidationPlotsNominal->m_oRecTauRecoTauPlotsNom.fill(*tau, weight);
      m_oTauValidationPlotsNominal->m_oNewCoreRecTauPlotsNom.fill(*tau, weight);
    }
    int recProng = tau->nTracks();
    if ( recProng == 1 ) {
      if(m_TauJetContainerName=="TauJets"){
	m_oTauValidationPlotsNoCuts->m_oRecoHad1ProngPlots.fill(*tau, weight);
	m_oTauValidationPlotsNoCuts->m_oRecTauEff1PPlots.fill(*tau, weight);
      }
      if ( nominal ) {
	m_oTauValidationPlotsNominal->m_oRecoHad1ProngNom.fill(*tau, weight);
	m_oTauValidationPlotsNominal->m_oRecTauEff1PPlotsNom.fill(*tau, weight);
      }
    }
    else if ( recProng == 3 ) {
      if(m_TauJetContainerName=="TauJets"){
	m_oTauValidationPlotsNoCuts->m_oRecoHad3ProngPlots.fill(*tau, weight);
	m_oTauValidationPlotsNoCuts->m_oRecTauEff3PPlots.fill(*tau, weight);
      }
      if ( nominal ) {
	m_oTauValidationPlotsNominal->m_oRecoHad3ProngNom.fill(*tau, weight);
	m_oTauValidationPlotsNominal->m_oRecTauEff3PPlotsNom.fill(*tau, weight);
      }
    }
      
    // Don't fill truth and fake histograms if we are running on data.
    if ( !m_isMC ) continue;
      
    ATH_MSG_DEBUG("Trying to truth-match tau");
    auto trueTau = m_truthTool->getTruth(*tau);

    // Fill truth and fake histograms
    static const SG::ConstAccessor<char> IsTruthMatchedAcc("IsTruthMatched");
    if ( (bool)IsTruthMatchedAcc(*tau) && (!(MC::isSMQuark(trueTau) || MC::isGluon(trueTau))) ) {
      ATH_MSG_DEBUG("Tau is truth-matched and not with a quark or a jet");
      if ( trueTau->isTau() ) {
        static const SG::ConstAccessor<char> IsHadronicTauAcc("IsHadronicTau");
	if ( (bool)IsHadronicTauAcc(*trueTau) ) {
	  ATH_MSG_DEBUG("Tau is hadronic tau");
	  if(m_TauJetContainerName=="TauJets"){
	    m_oTauValidationPlotsNoCuts->m_oGeneralTauAllProngsPlots.fill(*tau, weight);
	    m_oTauValidationPlotsNoCuts->m_oNewCoreMatchedPlots.fill(*tau, weight);
	    m_oTauValidationPlotsNoCuts->m_oMatchedResolutionPlots.fill(*tau, *trueTau, weight);
	    
	    // Substructure/PFO histograms 
	    m_oTauValidationPlotsNoCuts->m_oMatchedTauAllProngsPlots.fill(*tau, weight);
	    m_oTauValidationPlotsNoCuts->m_oMatchedTauEffPlots.fill(*tau, weight);
	  }
	  if ( nominal ) {
	    m_oTauValidationPlotsNominal->m_oMatchedGeneralNom.fill(*tau, weight);
	    m_oTauValidationPlotsNominal->m_oMatchedResolutionPlotsNom.fill(*tau, *trueTau, weight);
	    m_oTauValidationPlotsNominal->m_oMatchedTauEffPlotsNom.fill(*tau, weight);
	    m_oTauValidationPlotsNominal->m_oMatchedTauRecoTauPlotsNom.fill(*tau, weight);
	    m_oTauValidationPlotsNominal->m_oNewCoreMatchedPlotsNom.fill(*tau, weight);
	  }
	  if ( recProng == 1 ) {
	    if(m_TauJetContainerName=="TauJets"){
	      m_oTauValidationPlotsNoCuts->m_oHad1ProngPlots.fill(*tau, weight);
	      m_oTauValidationPlotsNoCuts->m_oMatchedTauEff1PPlots.fill(*tau, weight);
	      m_oTauValidationPlotsNoCuts->m_oMatchedResolution1PPlots.fill(*tau, *trueTau, weight);
	    }
	    if ( nominal ) {
	      m_oTauValidationPlotsNominal->m_oMatchedHad1ProngNom.fill(*tau, weight);
	      m_oTauValidationPlotsNominal->m_oMatchedTauEff1PPlotsNom.fill(*tau, weight);
	      m_oTauValidationPlotsNominal->m_oMatchedResolution1PPlotsNom.fill(*tau, *trueTau, weight);
	    }
	  }
	  else if ( recProng == 3 ) {
	    if(m_TauJetContainerName=="TauJets"){
	      m_oTauValidationPlotsNoCuts->m_oHad3ProngPlots.fill(*tau, weight);
	      m_oTauValidationPlotsNoCuts->m_oMatchedTauEff3PPlots.fill(*tau, weight);
	      m_oTauValidationPlotsNoCuts->m_oMatchedResolution3PPlots.fill(*tau, *trueTau, weight);
	    }
	    if ( nominal ) {
	      m_oTauValidationPlotsNominal->m_oMatchedHad3ProngNom.fill(*tau, weight);
	      m_oTauValidationPlotsNominal->m_oMatchedTauEff3PPlotsNom.fill(*tau, weight);
	      m_oTauValidationPlotsNominal->m_oMatchedResolution3PPlotsNom.fill(*tau, *trueTau, weight);
	    }
	  }

	  xAOD::TauJetParameters::DecayMode trueMode = m_truthTool->getDecayMode(*trueTau);
	  if(m_TauJetContainerName=="TauJets") m_oTauValidationPlotsNoCuts->m_oMigrationPlots.fill(*tau, trueMode, weight);
	  if ( nominal ) {
	    m_oTauValidationPlotsNominal->m_oMigrationPlotsNom.fill(*tau, trueMode, weight);
	  }
	}
      } else if(trueTau->isElectron()) {
	ATH_MSG_DEBUG("Tau is matched to an electron");
	if(m_TauJetContainerName=="TauJets"){
	  m_oTauValidationPlotsNoCuts->m_oElMatchedParamPlots.fill(*tau, weight);
	  m_oTauValidationPlotsNoCuts->m_oElMatchedEVetoPlots.fill(*tau, weight);
	}
        if ( nominal ) {
	  m_oTauValidationPlotsNominal->m_oElMatchedParamPlotsNom.fill(*tau, weight);
	  m_oTauValidationPlotsNominal->m_oElMatchedEVetoPlotsNom.fill(*tau, weight);
	  if(recProng == 1) m_oTauValidationPlotsNominal->m_oElMatchedEff1PPlotsNom.fill(*tau, weight);
        }
      }	       
    }
    else {
      ATH_MSG_DEBUG("Tau is matched to a jet or Tau is unmatched - consider it as fake");
      if(m_TauJetContainerName=="TauJets"){
	m_oTauValidationPlotsNoCuts->m_oFakeGeneralTauAllProngsPlots.fill(*tau, weight);
	// Substructure/PFO histograms
	m_oTauValidationPlotsNoCuts->m_oFakeTauAllProngsPlots.fill(*tau, weight);
	m_oTauValidationPlotsNoCuts->m_oNewCoreFakePlots.fill(*tau, weight);
	m_oTauValidationPlotsNoCuts->m_oFakeTauEffPlots.fill(*tau, weight);
      }
      if ( nominal ) {
	m_oTauValidationPlotsNominal->m_oFakeGeneralNom.fill(*tau, weight);
	m_oTauValidationPlotsNominal->m_oFakeTauEffPlotsNom.fill(*tau, weight);
	m_oTauValidationPlotsNominal->m_oFakeTauRecoTauPlotsNom.fill(*tau, weight);
	m_oTauValidationPlotsNominal->m_oNewCoreFakePlotsNom.fill(*tau, weight);
      }
      if ( recProng == 1 ) {
	if(m_TauJetContainerName=="TauJets"){
	  m_oTauValidationPlotsNoCuts->m_oFakeHad1ProngPlots.fill(*tau, weight);
	  m_oTauValidationPlotsNoCuts->m_oFakeTauEff1PPlots.fill(*tau, weight);
	}
	if ( nominal ) {
	  m_oTauValidationPlotsNominal->m_oFakeHad1ProngNom.fill(*tau, weight);
	  m_oTauValidationPlotsNominal->m_oFakeTauEff1PPlotsNom.fill(*tau, weight);
	}
      }
      if ( recProng == 3 ) {
	if(m_TauJetContainerName=="TauJets"){
	  m_oTauValidationPlotsNoCuts->m_oFakeTauEff3PPlots.fill(*tau, weight);
	  m_oTauValidationPlotsNoCuts->m_oFakeHad3ProngPlots.fill(*tau, weight);
	}
	if ( nominal ) {
	  m_oTauValidationPlotsNominal->m_oFakeHad3ProngNom.fill(*tau, weight);
	  m_oTauValidationPlotsNominal->m_oFakeTauEff3PPlotsNom.fill(*tau, weight);
	}
      }
    }
  }

  ATH_CHECK( m_truthTool->lockDecorations (*taus) );
  return StatusCode::SUCCESS;
}

StatusCode PhysValTau::procHistograms()
{
  ATH_MSG_INFO ("Finalising hists " << name() << "...");
  return StatusCode::SUCCESS;
}
