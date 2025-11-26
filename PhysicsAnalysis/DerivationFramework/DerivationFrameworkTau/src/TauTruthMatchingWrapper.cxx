/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TauTruthMatchingWrapper.cxx
// Author: Evelina Bouhova-Thacker (e.bouhova@cern.ch)
///////////////////////////////////////////////////////////////////

#include "DerivationFrameworkTau/TauTruthMatchingWrapper.h"
#include "StoreGate/ReadHandle.h"

namespace DerivationFramework {

  TauTruthMatchingWrapper::TauTruthMatchingWrapper(const std::string& t, const std::string& n, const IInterface* p) : 
    base_class(t,n,p)
  {
  }

  StatusCode TauTruthMatchingWrapper::initialize()
  {
    ATH_CHECK(m_tauKey.initialize());
    CHECK( m_tTauTruthMatchingTool.retrieve() );
    return StatusCode::SUCCESS;
  }

  StatusCode TauTruthMatchingWrapper::finalize()
  {
    return StatusCode::SUCCESS;
  }

  StatusCode TauTruthMatchingWrapper::addBranches(const EventContext& ctx) const
  {
    // Event context

    // Read handle
    SG::ReadHandle<xAOD::TauJetContainer> xTauContainer(m_tauKey,ctx);
    if (!xTauContainer.isValid()) {
        ATH_MSG_ERROR("Couldn't retrieve TauJetContainer with name " << m_tauKey);
        return StatusCode::FAILURE;
    }

    // Loop over taus
    std::unique_ptr<TauAnalysisTools::ITauTruthMatchingTool::ITruthTausEvent>
      truthTausEvent = m_tTauTruthMatchingTool->getEvent();
    for(auto xTau : *xTauContainer)
      m_tTauTruthMatchingTool->getTruth(*xTau, *truthTausEvent);
    ATH_CHECK( m_tTauTruthMatchingTool->lockDecorations(*xTauContainer) );
    
    return StatusCode::SUCCESS;
  }  
}
