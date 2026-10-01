/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TauTruthMatchingWrapper.cxx
// Author: Evelina Bouhova-Thacker (e.bouhova@cern.ch)
///////////////////////////////////////////////////////////////////

#include "DerivationFrameworkTau/TauTruthMatchingWrapper.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "TauAnalysisTools/HelperFunctions.h"

namespace DerivationFramework {

  StatusCode TauTruthMatchingWrapper::initialize()
  {
    ATH_CHECK(m_tauKey.initialize());
    ATH_CHECK(m_isTruthMatchedKey.initialize());
    ATH_CHECK(m_truthJetLinkKey.initialize());
    ATH_CHECK(m_truthParticleLinkKey.initialize());
    ATH_CHECK(m_tauOriginKey.initialize());
    CHECK( m_tTauTruthMatchingTool.retrieve() );
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
    SG::WriteDecorHandle<xAOD::TauJetContainer, int> dec_tauOrigin(m_tauOriginKey, ctx);
    for(auto xTau : *xTauContainer) {
      m_tTauTruthMatchingTool->getTruth(*xTau, *truthTausEvent);
      dec_tauOrigin(*xTau) = TauAnalysisTools::tauOrigin(*xTau);
    }
    ATH_CHECK( m_tTauTruthMatchingTool->lockDecorations(*xTauContainer) );

    return StatusCode::SUCCESS;
  }
}
