/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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

  ATH_CHECK( m_tauContainerKey.initialize() );
  ATH_CHECK( m_truthTauContainerKey.initialize() );

  m_IsTruthMatchedKey = m_tauContainerKey.key() + "." + m_IsTruthMatchedKey.key();
  ATH_CHECK( m_IsTruthMatchedKey.initialize() );

  m_IsHadronicTauKey = m_truthTauContainerKey.key() + "." + m_IsHadronicTauKey.key();
  ATH_CHECK( m_IsHadronicTauKey.initialize() );  

  return StatusCode::SUCCESS;
}

StatusCode PhysValTau::bookHistograms()
{
  ATH_MSG_INFO ("Booking hists " << name() << "...");

  // Physics validation plots are level 10
  m_oTauValidationPlotsNominal.reset(new TauValidationPlotsNominal(0,"Tau/" + m_tauContainerKey.key() + "_", m_tauContainerKey.key()));
  m_oTauValidationPlotsNominal->setDetailLevel(100);
  m_oTauValidationPlotsNominal->initialize();
  std::vector<HistData> hists_nominal = m_oTauValidationPlotsNominal->retrieveBookedHistograms();
  ATH_MSG_INFO ("Filling n of nominal hists " << hists_nominal.size() << " ");
  for (const auto& hist : hists_nominal) {
    ATH_CHECK(regHist(hist.first,hist.second,all));
  }


  if(m_tauContainerKey.key()=="TauJets"){

    m_oTauValidationPlotsNoCuts.reset(new TauValidationPlotsNoCuts(0,"Tau/" + m_tauContainerKey.key() + "_", m_tauContainerKey.key()));
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

StatusCode PhysValTau::fillHistograms(const EventContext& ctx)
{
  ATH_MSG_DEBUG ("Filling hists " << name() << "...");

  SG::ReadHandle<xAOD::TauJetContainer> tauJetsReadHandle(m_tauContainerKey, ctx);
  if (!tauJetsReadHandle.isValid()) {
    ATH_MSG_ERROR ("Could not retrieve TauJetContainer with key " << tauJetsReadHandle.key());
    return StatusCode::FAILURE;
  }
  const xAOD::TauJetContainer* taus = tauJetsReadHandle.cptr();


  ATH_MSG_DEBUG("Number of taus: " << taus->size());

  bool found_truth_taus = false;
  const xAOD::TruthParticleContainer* truth_taus =  nullptr;
  // Retrieve truth tau container for efficiency calculation
  if ( m_isMC ) {
      SG::ReadHandle<xAOD::TruthParticleContainer> truthTauJetsReadHandle(m_truthTauContainerKey, ctx);
      if(!truthTauJetsReadHandle.isValid()) {
          ATH_MSG_INFO("Input collection " << m_truthTauContainerKey.key() << " not found. Won't do reco efficiency plots ..");
          found_truth_taus = false;
      } else {
	truth_taus = truthTauJetsReadHandle.cptr();      
        found_truth_taus = true;
      }
  }   
  // Vectors to calculate the reco efficiency
  std::vector<const xAOD::TruthParticle*> vec_truth_taus;
  std::vector<const xAOD::TauJet*> vec_reco_taus;

  // Retrieve event info and beamSpotWeight
  SG::ReadHandle<xAOD::EventInfo> eventInfoReadHandle("EventInfo", ctx);
  const xAOD::EventInfo* eventInfo = eventInfoReadHandle.cptr();

  float weight = eventInfo->beamSpotWeight();
  float avg_mu = eventInfo->averageInteractionsPerCrossing();

  m_oTauValidationPlotsNominal->m_oEventPlotsNom.fill(avg_mu,weight);

  // Loop through recoonstructed tau container
  for (auto tau : *taus) {
    if ( m_detailLevel < 10 ) continue;
    if ( !static_cast<bool>(m_primTauSel->accept(*tau)) ) continue;
    bool nominal = static_cast<bool>(m_nomiTauSel->accept(*tau));
      
    // fill histograms for reconstructed taus
    if(m_tauContainerKey.key()=="TauJets"){
      m_oTauValidationPlotsNoCuts->m_oRecoTauAllProngsPlots.fill(*tau, weight);
      m_oTauValidationPlotsNoCuts->m_oNewCorePlots.fill(*tau, weight);
      m_oTauValidationPlotsNoCuts->m_oRecTauEffPlots.fill(*tau, weight, avg_mu);
      m_oTauValidationPlotsNoCuts->m_oRecoGeneralTauAllProngsPlots.fill(*tau, weight);
    }
    if ( nominal ) {
      m_oTauValidationPlotsNominal->m_oRecoGeneralNom.fill(*tau, weight);
      m_oTauValidationPlotsNominal->m_oRecTauEffPlotsNom.fill(*tau, weight, avg_mu);
      m_oTauValidationPlotsNominal->m_oRecTauRecoTauPlotsNom.fill(*tau, weight);
      m_oTauValidationPlotsNominal->m_oNewCoreRecTauPlotsNom.fill(*tau, weight);
    }
    int recProng = tau->nTracks();
    if ( recProng == 1 ) {
      if(m_tauContainerKey.key()=="TauJets"){
	m_oTauValidationPlotsNoCuts->m_oRecoHad1ProngPlots.fill(*tau, weight);
	m_oTauValidationPlotsNoCuts->m_oRecTauEff1PPlots.fill(*tau, weight, avg_mu);
      }
      if ( nominal ) {
	m_oTauValidationPlotsNominal->m_oRecoHad1ProngNom.fill(*tau, weight);
	m_oTauValidationPlotsNominal->m_oRecTauEff1PPlotsNom.fill(*tau, weight, avg_mu);
      }
    }
    else if ( recProng == 3 ) {
      if(m_tauContainerKey.key()=="TauJets"){
	m_oTauValidationPlotsNoCuts->m_oRecoHad3ProngPlots.fill(*tau, weight);
	m_oTauValidationPlotsNoCuts->m_oRecTauEff3PPlots.fill(*tau, weight, avg_mu);
      }
      if ( nominal ) {
	m_oTauValidationPlotsNominal->m_oRecoHad3ProngNom.fill(*tau, weight);
	m_oTauValidationPlotsNominal->m_oRecTauEff3PPlotsNom.fill(*tau, weight, avg_mu);
      }
    }
      
    // Don't fill truth and fake histograms if we are running on data.
    if ( !m_isMC ) continue;
      
    ATH_MSG_DEBUG("Trying to truth-match tau");
    auto trueTau = m_truthTool->getTruth(*tau);

    // Fill truth and fake histograms
    SG::ReadDecorHandle<xAOD::TauJetContainer, char> isTruthMatched{m_IsTruthMatchedKey, ctx};   
    if( (bool) isTruthMatched(*tau) && (!(MC::isSMQuark(trueTau) || MC::isGluon(trueTau))) ) {
      ATH_MSG_DEBUG("Tau is truth-matched and not with a quark or a jet");
      if ( trueTau->isTau() ) {
        SG::ReadDecorHandle<xAOD::TruthParticleContainer, char> isHadronicTau{m_IsHadronicTauKey, ctx};
	if( (bool) isHadronicTau(*trueTau) ) { 
	  ATH_MSG_DEBUG("Tau is hadronic tau");
	  if(m_tauContainerKey.key()=="TauJets"){
	    m_oTauValidationPlotsNoCuts->m_oGeneralTauAllProngsPlots.fill(*tau, weight);
	    m_oTauValidationPlotsNoCuts->m_oNewCoreMatchedPlots.fill(*tau, weight);
	    m_oTauValidationPlotsNoCuts->m_oMatchedResolutionPlots.fill(*tau, *trueTau, weight);
	    
	    // Substructure/PFO histograms 
	    m_oTauValidationPlotsNoCuts->m_oMatchedTauAllProngsPlots.fill(*tau, weight);
	    m_oTauValidationPlotsNoCuts->m_oMatchedTauEffPlots.fill(*tau, weight, avg_mu);
	  }
	  if ( nominal ) {
	    m_oTauValidationPlotsNominal->m_oMatchedGeneralNom.fill(*tau, weight);
	    m_oTauValidationPlotsNominal->m_oMatchedResolutionPlotsNom.fill(*tau, *trueTau, weight);
	    m_oTauValidationPlotsNominal->m_oMatchedTauEffPlotsNom.fill(*tau, weight, avg_mu);
	    m_oTauValidationPlotsNominal->m_oMatchedTauRecoTauPlotsNom.fill(*tau, weight);
	    m_oTauValidationPlotsNominal->m_oNewCoreMatchedPlotsNom.fill(*tau, weight);

            if(found_truth_taus){
              vec_reco_taus.push_back(tau); 
	    }   
	  }
	  if ( recProng == 1 ) {
	    if(m_tauContainerKey.key()=="TauJets"){
	      m_oTauValidationPlotsNoCuts->m_oHad1ProngPlots.fill(*tau, weight);
	      m_oTauValidationPlotsNoCuts->m_oMatchedTauEff1PPlots.fill(*tau, weight, avg_mu);
	      m_oTauValidationPlotsNoCuts->m_oMatchedResolution1PPlots.fill(*tau, *trueTau, weight);
	    }
	    if ( nominal ) {
	      m_oTauValidationPlotsNominal->m_oMatchedHad1ProngNom.fill(*tau, weight);
	      m_oTauValidationPlotsNominal->m_oMatchedTauEff1PPlotsNom.fill(*tau, weight, avg_mu);
	      m_oTauValidationPlotsNominal->m_oMatchedResolution1PPlotsNom.fill(*tau, *trueTau, weight);
	    }
	  }
	  else if ( recProng == 3 ) {
	    if(m_tauContainerKey.key()=="TauJets"){
	      m_oTauValidationPlotsNoCuts->m_oHad3ProngPlots.fill(*tau, weight);
	      m_oTauValidationPlotsNoCuts->m_oMatchedTauEff3PPlots.fill(*tau, weight, avg_mu);
	      m_oTauValidationPlotsNoCuts->m_oMatchedResolution3PPlots.fill(*tau, *trueTau, weight);
	    }
	    if ( nominal ) {
	      m_oTauValidationPlotsNominal->m_oMatchedHad3ProngNom.fill(*tau, weight);
	      m_oTauValidationPlotsNominal->m_oMatchedTauEff3PPlotsNom.fill(*tau, weight, avg_mu);
	      m_oTauValidationPlotsNominal->m_oMatchedResolution3PPlotsNom.fill(*tau, *trueTau, weight);
	    }
	  }

	  xAOD::TauJetParameters::DecayMode trueMode = m_truthTool->getDecayMode(*trueTau);
	  if(m_tauContainerKey.key()=="TauJets") m_oTauValidationPlotsNoCuts->m_oMigrationPlots.fill(*tau, trueMode, weight);
	  if ( nominal ) {
	    m_oTauValidationPlotsNominal->m_oMigrationPlotsNom.fill(*tau, trueMode, weight);
	  }
	}
      } else if(trueTau->isElectron()) {
	ATH_MSG_DEBUG("Tau is matched to an electron");
	if(m_tauContainerKey.key()=="TauJets"){
	  m_oTauValidationPlotsNoCuts->m_oElMatchedParamPlots.fill(*tau, weight);
	  m_oTauValidationPlotsNoCuts->m_oElMatchedEVetoPlots.fill(*tau, weight);
	}
        if ( nominal ) {
	  m_oTauValidationPlotsNominal->m_oElMatchedParamPlotsNom.fill(*tau, weight);
	  m_oTauValidationPlotsNominal->m_oElMatchedEVetoPlotsNom.fill(*tau, weight);
	  if(recProng == 1) m_oTauValidationPlotsNominal->m_oElMatchedEff1PPlotsNom.fill(*tau, weight, avg_mu);
        }
      }	       
    }
    else {
      ATH_MSG_DEBUG("Tau is matched to a jet or Tau is unmatched - consider it as fake");
      if(m_tauContainerKey.key()=="TauJets"){
	m_oTauValidationPlotsNoCuts->m_oFakeGeneralTauAllProngsPlots.fill(*tau, weight);
	// Substructure/PFO histograms
	m_oTauValidationPlotsNoCuts->m_oFakeTauAllProngsPlots.fill(*tau, weight);
	m_oTauValidationPlotsNoCuts->m_oNewCoreFakePlots.fill(*tau, weight);
	m_oTauValidationPlotsNoCuts->m_oFakeTauEffPlots.fill(*tau, weight, avg_mu);
      }
      if ( nominal ) {
	m_oTauValidationPlotsNominal->m_oFakeGeneralNom.fill(*tau, weight);
	m_oTauValidationPlotsNominal->m_oFakeTauEffPlotsNom.fill(*tau, weight, avg_mu);
	m_oTauValidationPlotsNominal->m_oFakeTauRecoTauPlotsNom.fill(*tau, weight);
	m_oTauValidationPlotsNominal->m_oNewCoreFakePlotsNom.fill(*tau, weight);
      }
      if ( recProng == 1 ) {
	if(m_tauContainerKey.key()=="TauJets"){
	  m_oTauValidationPlotsNoCuts->m_oFakeHad1ProngPlots.fill(*tau, weight);
	  m_oTauValidationPlotsNoCuts->m_oFakeTauEff1PPlots.fill(*tau, weight, avg_mu);
	}
	if ( nominal ) {
	  m_oTauValidationPlotsNominal->m_oFakeHad1ProngNom.fill(*tau, weight);
	  m_oTauValidationPlotsNominal->m_oFakeTauEff1PPlotsNom.fill(*tau, weight, avg_mu);
	}
      }
      if ( recProng == 3 ) {
	if(m_tauContainerKey.key()=="TauJets"){
	  m_oTauValidationPlotsNoCuts->m_oFakeTauEff3PPlots.fill(*tau, weight, avg_mu);
	  m_oTauValidationPlotsNoCuts->m_oFakeHad3ProngPlots.fill(*tau, weight);
	}
	if ( nominal ) {
	  m_oTauValidationPlotsNominal->m_oFakeHad3ProngNom.fill(*tau, weight);
	  m_oTauValidationPlotsNominal->m_oFakeTauEff3PPlotsNom.fill(*tau, weight, avg_mu);
	}
      }
    }
  }

  // plots for tau reco efficiency
  if(found_truth_taus){
    for (auto truth_tau : *truth_taus) {
      vec_truth_taus.push_back(truth_tau);
    }

    // fill histograms
    m_oTauValidationPlotsNominal->m_oMatchedTauRecoEffPlotsNom.fill(vec_truth_taus, vec_reco_taus, weight, avg_mu);
    m_oTauValidationPlotsNominal->m_oMatchedTauTrkClassEffPlotsNom.fill(vec_truth_taus, vec_reco_taus, weight, avg_mu);
  }

  ATH_CHECK( m_truthTool->lockDecorations (*taus) );
  return StatusCode::SUCCESS;
}

StatusCode PhysValTau::procHistograms()
{
  ATH_MSG_INFO ("Finalising hists " << name() << "...");
  return StatusCode::SUCCESS;
}
