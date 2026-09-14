/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "DerivationFrameworkTau/TauMassDecorator.h"
#include "xAODTau/TauJetContainer.h"
#include "xAODTau/TauJetAuxContainer.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "TauAnalysisTools/HelperFunctions.h"
#include "StoreGate/WriteDecorHandle.h"

namespace DerivationFramework {

  TauMassDecorator::TauMassDecorator(const std::string& name,
                                     ISvcLocator* pSvcLocator)
   : AthReentrantAlgorithm(name, pSvcLocator)
  {}

  StatusCode TauMassDecorator::initialize()
  {
    // initialize read/write handle keys
    ATH_CHECK(m_tauOutputKey.initialize());
    ATH_CHECK(m_tauInputKey.initialize());
    //ATH_CHECK( m_massKey.initialize() ); 

    return StatusCode::SUCCESS;
  }

  StatusCode TauMassDecorator::execute(const EventContext& ctx) const 
  {
  
    // retrieve input tau container
    SG::ReadHandle<xAOD::TauJetContainer> tau_inputContainer(m_tauInputKey, ctx);
    if (!tau_inputContainer.isValid()) {
      ATH_MSG_ERROR ("Could not retrieve TauJetContainer with key " << tau_inputContainer.key());
      return StatusCode::FAILURE;
    }
  
    SG::WriteHandle<xAOD::TauJetContainer> tau_outputContainer(m_tauOutputKey,ctx);  
    ATH_CHECK(tau_outputContainer.record(std::make_unique<xAOD::TauJetContainer>(),
                                         std::make_unique<xAOD::TauJetAuxContainer>()));

    xAOD::TauJetContainer* taus = tau_outputContainer.ptr();
    taus->reserve(tau_inputContainer->size());
    for (const xAOD::TauJet* old_tau : *tau_inputContainer) { 
        xAOD::TauJet*  tau = taus->push_back(std::make_unique<xAOD::TauJet>());
        *tau=*old_tau;
    }	    

    /*
    SG::WriteDecorHandle<xAOD::TauJetContainer, float> dec_mass (m_massKey, ctx);

    // update the mass value
    for (const auto tau : *taus) {
	std::vector<TLorentzVector> combinedPi0sP4;
        TauAnalysisTools::createPi0Vectors(tau, combinedPi0sP4);
        TLorentzVector tau_comp(0,0,0,0);
     
	for (size_t i = 0; i < tau->nTracks(); i++) {
            tau_comp += tau->track(i)->p4();		
        }
 
	for(unsigned int iPi0=0; iPi0 < combinedPi0sP4.size(); iPi0++) {
            tau_comp += combinedPi0sP4[iPi0]; 
        }
	dec_mass(*tau) = tau_comp.M();
    }
    */
    return StatusCode::SUCCESS;
  }
}


