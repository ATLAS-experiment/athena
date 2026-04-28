/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TauCombinedTESWrapper.cxx
///////////////////////////////////////////////////////////////////

#include "DerivationFrameworkTau/TauCombinedTESWrapper.h"
#include "StoreGate/ReadHandle.h"
#include "AthContainers/Decorator.h"

namespace DerivationFramework {

  StatusCode TauCombinedTESWrapper::initialize()
  {
    ATH_CHECK(m_tauKey.initialize());
    ATH_CHECK(m_tesCompatibilityKey.initialize());
    CHECK( m_tTauCombinedTESTool.retrieve() );
    return StatusCode::SUCCESS;
  }


  StatusCode TauCombinedTESWrapper::addBranches(const EventContext& ctx) const
  {
    // Event context

    // Read handle
    SG::ReadHandle<xAOD::TauJetContainer> xTauContainer(m_tauKey,ctx);
    if (!xTauContainer.isValid()) {
        ATH_MSG_ERROR("Couldn't retrieve TauJetContainer with name " << m_tauKey);
        return StatusCode::FAILURE;
    }

    SG::WriteDecorHandle<xAOD::TauJetContainer, char> dec_tesCompatibility (m_tesCompatibilityKey, ctx);

    // Loop over taus
    for(auto xTau : *xTauContainer){
       dec_tesCompatibility(*xTau) = m_tTauCombinedTESTool->getTESCompatibility(*xTau);  	    
    }

    return StatusCode::SUCCESS;
  }
}



