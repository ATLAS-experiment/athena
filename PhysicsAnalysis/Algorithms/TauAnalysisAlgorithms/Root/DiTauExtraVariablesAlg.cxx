/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Antonio De Maria

#include <TauAnalysisAlgorithms/DiTauExtraVariablesAlg.h>

#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>

namespace CP {

  StatusCode DiTauExtraVariablesAlg::initialize() {

    // leading subjet info	  
    if (m_leadSubjetPtKey.contHandleKey().key() == m_leadSubjetPtKey.key()) {
      m_leadSubjetPtKey = m_ditausKey.key() + "." + m_leadSubjetPtKey.key();
    }

    if (m_leadSubjetEtaKey.contHandleKey().key() == m_leadSubjetEtaKey.key()) {
      m_leadSubjetEtaKey = m_ditausKey.key() + "." + m_leadSubjetEtaKey.key();
    }

    if (m_leadSubjetPhiKey.contHandleKey().key() == m_leadSubjetPhiKey.key()) {
      m_leadSubjetPhiKey = m_ditausKey.key() + "." + m_leadSubjetPhiKey.key();
    }

    if (m_leadSubjetEKey.contHandleKey().key() == m_leadSubjetEKey.key()) {
      m_leadSubjetEKey = m_ditausKey.key() + "." + m_leadSubjetEKey.key();
    }

    // subleading subjet info
    if (m_subleadSubjetPtKey.contHandleKey().key() == m_subleadSubjetPtKey.key()) {
      m_subleadSubjetPtKey = m_ditausKey.key() + "." + m_subleadSubjetPtKey.key();
    }

    if (m_subleadSubjetEtaKey.contHandleKey().key() == m_subleadSubjetEtaKey.key()) {
      m_subleadSubjetEtaKey = m_ditausKey.key() + "." + m_subleadSubjetEtaKey.key();
    }

    if (m_subleadSubjetPhiKey.contHandleKey().key() == m_subleadSubjetPhiKey.key()) {
      m_subleadSubjetPhiKey = m_ditausKey.key() + "." + m_subleadSubjetPhiKey.key();
    }

    if (m_subleadSubjetEKey.contHandleKey().key() == m_subleadSubjetEKey.key()) {
      m_subleadSubjetEKey = m_ditausKey.key() + "." + m_subleadSubjetEKey.key();
    }


    ANA_CHECK(m_ditausKey.initialize());
    ANA_CHECK(m_leadSubjetPtKey.initialize());
    ANA_CHECK(m_leadSubjetEtaKey.initialize());
    ANA_CHECK(m_leadSubjetPhiKey.initialize());
    ANA_CHECK(m_leadSubjetEKey.initialize());
    ANA_CHECK(m_subleadSubjetPtKey.initialize());
    ANA_CHECK(m_subleadSubjetEtaKey.initialize());
    ANA_CHECK(m_subleadSubjetPhiKey.initialize());
    ANA_CHECK(m_subleadSubjetEKey.initialize());
   
    return StatusCode::SUCCESS;
  }

  StatusCode DiTauExtraVariablesAlg::execute(const EventContext &ctx) const {

    SG::ReadHandle<xAOD::DiTauJetContainer> ditaus(m_ditausKey, ctx);

    SG::WriteDecorHandle<xAOD::DiTauJetContainer, float> leadSubjetPtHandle(m_leadSubjetPtKey, ctx);
    SG::WriteDecorHandle<xAOD::DiTauJetContainer, float> leadSubjetEtaHandle(m_leadSubjetEtaKey, ctx);    
    SG::WriteDecorHandle<xAOD::DiTauJetContainer, float> leadSubjetPhiHandle(m_leadSubjetPhiKey, ctx);
    SG::WriteDecorHandle<xAOD::DiTauJetContainer, float> leadSubjetEHandle(m_leadSubjetEKey, ctx);

    SG::WriteDecorHandle<xAOD::DiTauJetContainer, float> subleadSubjetPtHandle(m_subleadSubjetPtKey, ctx);
    SG::WriteDecorHandle<xAOD::DiTauJetContainer, float> subleadSubjetEtaHandle(m_subleadSubjetEtaKey, ctx);
    SG::WriteDecorHandle<xAOD::DiTauJetContainer, float> subleadSubjetPhiHandle(m_subleadSubjetPhiKey, ctx);
    SG::WriteDecorHandle<xAOD::DiTauJetContainer, float> subleadSubjetEHandle(m_subleadSubjetEKey, ctx);

    for (const xAOD::DiTauJet *ditau : *ditaus) {

      // always require at least 2 subjets to have a good ditau	    
      if(ditau->nSubjets() < 2) continue;

      leadSubjetPtHandle(*ditau) = ditau->subjetPt(0);
      leadSubjetEtaHandle(*ditau) = ditau->subjetEta(0);
      leadSubjetPhiHandle(*ditau) = ditau->subjetPhi(0);
      leadSubjetEHandle(*ditau) = ditau->subjetE(0);

      subleadSubjetPtHandle(*ditau) = ditau->subjetPt(1);
      subleadSubjetEtaHandle(*ditau) = ditau->subjetEta(1);
      subleadSubjetPhiHandle(*ditau) = ditau->subjetPhi(1);
      subleadSubjetEHandle(*ditau) = ditau->subjetE(1);

    }
    

    return StatusCode::SUCCESS;
  }

} // namespace
