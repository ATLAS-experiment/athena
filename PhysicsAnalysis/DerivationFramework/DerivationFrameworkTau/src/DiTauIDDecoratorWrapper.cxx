/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "DerivationFrameworkTau/DiTauIDDecoratorWrapper.h"
#include "StoreGate/ReadHandle.h"
#include "xAODCore/ShallowCopy.h"
#include "GaudiKernel/Chrono.h"

namespace DerivationFramework {

  StatusCode DiTauIDDecoratorWrapper::initialize()
  {
    ATH_CHECK( m_tDiTauOnnxDiscriminantTool.retrieve() );
    ATH_CHECK( m_tDiTauWPDecoratorTool.retrieve() );

    if ( m_WPCuts.size() != m_WPDecorKeys.size() ) {
      ATH_MSG_ERROR ("Size mismatch between m_WPDecorKeys and m_WPCuts! Please check the configuration.");
      return StatusCode::FAILURE;
    }

    // initialize read/write handle keys
    ATH_CHECK( m_ditauContainerKey.initialize() );
    ATH_CHECK( m_scoreDecorKey.initialize() );
    ATH_CHECK( m_WPDecorKeys.initialize() );

    // get the chrono auditor
    ATH_CHECK ( m_chronoSvc.retrieve() );

    return StatusCode::SUCCESS;
  }

  StatusCode DiTauIDDecoratorWrapper::execute(const EventContext& ctx) const
  {
    // On your marks.... get set....
    Chrono chrono( &(*m_chronoSvc), name() );
    // GO!!

    // retrieve ditau container
    SG::ReadHandle<xAOD::DiTauJetContainer> ditauJetsReadHandle(m_ditauContainerKey, ctx);
    if (!ditauJetsReadHandle.isValid()) {
      ATH_MSG_ERROR ("Could not retrieve DiTauJetContainer with key " << ditauJetsReadHandle.key());
      return StatusCode::FAILURE;
    }
    const xAOD::DiTauJetContainer* ditauContainer = ditauJetsReadHandle.cptr();

    SG::WriteDecorHandle<xAOD::DiTauJetContainer, float> scoreDecor(m_scoreDecorKey, ctx);

    std::vector<SG::WriteDecorHandle<xAOD::DiTauJetContainer, char> > WPDecors;
    WPDecors.reserve (m_WPDecorKeys.size());
    for (const SG::WriteDecorHandleKey<xAOD::DiTauJetContainer>& k : m_WPDecorKeys) {
      WPDecors.emplace_back (k, ctx);
    }

    // create shallow copy
    auto shallowCopy = xAOD::shallowCopy (*ditauContainer, ctx);

    for (auto ditau : *shallowCopy.first) {

      float score = m_tDiTauOnnxDiscriminantTool->GetDiTauObjOnnxScore(*ditau);

      // copy over the relevant decorations (scores and working points)
      const xAOD::DiTauJet* xDiTau = ditauContainer->at(ditau->index());
      scoreDecor(*xDiTau) = score;

      int i=0;
      for (SG::WriteDecorHandle<xAOD::DiTauJetContainer, char>& dec : WPDecors) {
        bool decision = m_tDiTauWPDecoratorTool->passOmniWP(score, m_WPCuts.value().at(i));
        dec(*xDiTau) = decision;
        i++;
      }
    }

    return StatusCode::SUCCESS;
  }
}
