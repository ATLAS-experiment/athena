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

    // declare decorations to the scheduler
    m_scoreDecorKey = m_ditauContainerKey.key() + ".omni_score";
  
    // initialize read/write handle keys
    ATH_CHECK( m_ditauContainerKey.initialize() );  
    ATH_CHECK( m_scoreDecorKey.initialize() );

    return StatusCode::SUCCESS;
  }

  StatusCode DiTauIDDecoratorWrapper::addBranches() const
  {
    const EventContext& ctx = Gaudi::Hive::currentContext();

    // retrieve ditau container
    SG::ReadHandle<xAOD::DiTauJetContainer> ditauJetsReadHandle(m_ditauContainerKey, ctx);
    if (!ditauJetsReadHandle.isValid()) {
      ATH_MSG_ERROR ("Could not retrieve DiTauJetContainer with key " << ditauJetsReadHandle.key());
      return StatusCode::FAILURE;
    }
    const xAOD::DiTauJetContainer* ditauContainer = ditauJetsReadHandle.cptr();

    SG::WriteDecorHandle<xAOD::DiTauJetContainer, float> scoreDecor(m_scoreDecorKey, ctx);

    // create shallow copy
    auto shallowCopy = xAOD::shallowCopyContainer (*ditauContainer);

    for (auto ditau : *shallowCopy.first) {

      float score = m_tDiTauOnnxDiscriminantTool->GetDiTauObjOnnxScore(*ditau); 

      // copy over the relevant decorations (scores and working points)
      const xAOD::DiTauJet* xDiTau = ditauContainer->at(ditau->index());
      scoreDecor(*xDiTau) = score;
    }

    delete shallowCopy.first;
    delete shallowCopy.second;

    return StatusCode::SUCCESS;
  }
}
