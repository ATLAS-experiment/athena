/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "DerivationFrameworkTau/DiTauIDDecoratorWrapper.h"
#include "StoreGate/ReadHandle.h"
#include "xAODCore/ShallowCopy.h"

namespace DerivationFramework {

  StatusCode DiTauIDDecoratorWrapper::initialize()
  {
    ATH_CHECK( m_tDiTauOnnxDiscriminantTool.retrieve() ); 
    ATH_CHECK( m_tDiTauWPDecoratorTool.retrieve() );

    // declare decorations to the scheduler
    m_scoreDecorKey = m_ditauContainerKey.key() + ".omni_score";

    StringArrayProperty decorWPNames("DecorWPNames", {});
    ATH_CHECK( m_tDiTauWPDecoratorTool->getProperty(&decorWPNames) );

    FloatArrayProperty decorWPCuts("DecorWPCuts", {});
    ATH_CHECK( m_tDiTauWPDecoratorTool->getProperty(&decorWPCuts) ); 

    for (const auto& WP : decorWPNames.value()) m_WPs.push_back(WP);

    for (const std::string& WP : m_WPs) {
      m_WPDecorKeys.emplace_back(m_ditauContainerKey.key() + "." + WP);
    }

    for (const auto cut : decorWPCuts) m_WPCuts.push_back(cut);

    // initialize read/write handle keys
    ATH_CHECK( m_ditauContainerKey.initialize() );  
    ATH_CHECK( m_scoreDecorKey.initialize() );
    ATH_CHECK( m_WPDecorKeys.initialize() );

    return StatusCode::SUCCESS;
  }

  StatusCode DiTauIDDecoratorWrapper::addBranches(const EventContext& ctx) const
  {

    // retrieve ditau container
    SG::ReadHandle<xAOD::DiTauJetContainer> ditauJetsReadHandle(m_ditauContainerKey, ctx);
    if (!ditauJetsReadHandle.isValid()) {
      ATH_MSG_ERROR ("Could not retrieve DiTauJetContainer with key " << ditauJetsReadHandle.key());
      return StatusCode::FAILURE;
    }
    const xAOD::DiTauJetContainer* ditauContainer = ditauJetsReadHandle.cptr();

    SG::WriteDecorHandle<xAOD::DiTauJetContainer, float> scoreDecor(m_scoreDecorKey, ctx);
    
    std::vector<SG::WriteDecorHandle<xAOD::DiTauJetContainer, char> > WPDecors;
    WPDecors.reserve (m_WPs.size());
    for (const SG::WriteDecorHandleKey<xAOD::DiTauJetContainer>& k : m_WPDecorKeys) {
       WPDecors.emplace_back (k, ctx);
    }

    // create shallow copy
    auto shallowCopy = xAOD::shallowCopyContainer (*ditauContainer);

    for (auto ditau : *shallowCopy.first) {

      float score = m_tDiTauOnnxDiscriminantTool->GetDiTauObjOnnxScore(*ditau); 

      // copy over the relevant decorations (scores and working points)
      const xAOD::DiTauJet* xDiTau = ditauContainer->at(ditau->index());
      scoreDecor(*xDiTau) = score;

      int i=0;
      for (SG::WriteDecorHandle<xAOD::DiTauJetContainer, char>& dec : WPDecors) {
        bool decision = m_tDiTauWPDecoratorTool->passOmniWP(score, m_WPCuts.at(i));
	dec(*xDiTau) = decision;
	i++;
      }
    }

    delete shallowCopy.first;
    delete shallowCopy.second;

    return StatusCode::SUCCESS;
  }
}
