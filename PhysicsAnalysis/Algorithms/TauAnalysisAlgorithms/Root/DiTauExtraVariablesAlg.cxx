/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Antonio De Maria

#include <TauAnalysisAlgorithms/DiTauExtraVariablesAlg.h>

#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>

namespace CP {

  StatusCode DiTauExtraVariablesAlg::initialize() {

    if (m_omniScoreKey.contHandleKey().key() == m_omniScoreKey.key()) {
      m_omniScoreKey = m_ditausKey.key() + "." + m_omniScoreKey.key();
    }

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

    if (m_leadSubjetNTracksKey.contHandleKey().key() == m_leadSubjetNTracksKey.key()) {
      m_leadSubjetNTracksKey = m_ditausKey.key() + "." + m_leadSubjetNTracksKey.key();
    }

    if (m_leadSubjetChargeKey.contHandleKey().key() == m_leadSubjetChargeKey.key()) {
      m_leadSubjetChargeKey = m_ditausKey.key() + "." + m_leadSubjetChargeKey.key();
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

    if (m_subleadSubjetNTracksKey.contHandleKey().key() == m_subleadSubjetNTracksKey.key()) {
      m_subleadSubjetNTracksKey = m_ditausKey.key() + "." + m_subleadSubjetNTracksKey.key();
    }

    if (m_subleadSubjetChargeKey.contHandleKey().key() == m_subleadSubjetChargeKey.key()) {
      m_subleadSubjetChargeKey = m_ditausKey.key() + "." + m_subleadSubjetChargeKey.key();
    }

    ANA_CHECK(m_ditausKey.initialize());
    ANA_CHECK(m_omniScoreKey.initialize());
    ANA_CHECK(m_leadSubjetPtKey.initialize());
    ANA_CHECK(m_leadSubjetEtaKey.initialize());
    ANA_CHECK(m_leadSubjetPhiKey.initialize());
    ANA_CHECK(m_leadSubjetEKey.initialize());
    ANA_CHECK(m_leadSubjetNTracksKey.initialize());
    ANA_CHECK(m_leadSubjetChargeKey.initialize());
    ANA_CHECK(m_subleadSubjetPtKey.initialize());
    ANA_CHECK(m_subleadSubjetEtaKey.initialize());
    ANA_CHECK(m_subleadSubjetPhiKey.initialize());
    ANA_CHECK(m_subleadSubjetEKey.initialize());
    ANA_CHECK(m_subleadSubjetNTracksKey.initialize());
    ANA_CHECK(m_subleadSubjetChargeKey.initialize()); 

    return StatusCode::SUCCESS;
  }

  StatusCode DiTauExtraVariablesAlg::execute(const EventContext &ctx) const {

    SG::ReadHandle<xAOD::DiTauJetContainer> ditaus(m_ditausKey, ctx);

    SG::WriteDecorHandle<xAOD::DiTauJetContainer, float> omniScoreHandle(m_omniScoreKey, ctx);

    SG::WriteDecorHandle<xAOD::DiTauJetContainer, float> leadSubjetPtHandle(m_leadSubjetPtKey, ctx);
    SG::WriteDecorHandle<xAOD::DiTauJetContainer, float> leadSubjetEtaHandle(m_leadSubjetEtaKey, ctx);    
    SG::WriteDecorHandle<xAOD::DiTauJetContainer, float> leadSubjetPhiHandle(m_leadSubjetPhiKey, ctx);
    SG::WriteDecorHandle<xAOD::DiTauJetContainer, float> leadSubjetEHandle(m_leadSubjetEKey, ctx);
    SG::WriteDecorHandle<xAOD::DiTauJetContainer, int>  leadSubjetNTracksHandle(m_leadSubjetNTracksKey, ctx);
    SG::WriteDecorHandle<xAOD::DiTauJetContainer, int>  leadSubjetChargeHandle(m_leadSubjetChargeKey, ctx);

    SG::WriteDecorHandle<xAOD::DiTauJetContainer, float> subleadSubjetPtHandle(m_subleadSubjetPtKey, ctx);
    SG::WriteDecorHandle<xAOD::DiTauJetContainer, float> subleadSubjetEtaHandle(m_subleadSubjetEtaKey, ctx);
    SG::WriteDecorHandle<xAOD::DiTauJetContainer, float> subleadSubjetPhiHandle(m_subleadSubjetPhiKey, ctx);
    SG::WriteDecorHandle<xAOD::DiTauJetContainer, float> subleadSubjetEHandle(m_subleadSubjetEKey, ctx);
    SG::WriteDecorHandle<xAOD::DiTauJetContainer, int> subleadSubjetNTracksHandle(m_subleadSubjetNTracksKey, ctx);
    SG::WriteDecorHandle<xAOD::DiTauJetContainer, int> subleadSubjetChargeHandle(m_subleadSubjetChargeKey, ctx);

    // need to check if this decoration exist first because it's only available from ATLASDPD-2318 onward
    static const SG::ConstAccessor<float> acc_OmniScore("omni_score");

    for (const xAOD::DiTauJet *ditau : *ditaus) {

      // always require at least 2 subjets to have a good ditau	    
      if(ditau->nSubjets() < 2) continue;

      if ( acc_OmniScore.isAvailable(*ditau) ) {
        omniScoreHandle(*ditau) = acc_OmniScore(*ditau);
      }

      leadSubjetPtHandle(*ditau) = ditau->subjetPt(0);
      leadSubjetEtaHandle(*ditau) = ditau->subjetEta(0);
      leadSubjetPhiHandle(*ditau) = ditau->subjetPhi(0);
      leadSubjetEHandle(*ditau) = ditau->subjetE(0);

      subleadSubjetPtHandle(*ditau) = ditau->subjetPt(1);
      subleadSubjetEtaHandle(*ditau) = ditau->subjetEta(1);
      subleadSubjetPhiHandle(*ditau) = ditau->subjetPhi(1);
      subleadSubjetEHandle(*ditau) = ditau->subjetE(1);

      // leading and subleading subjet ntracks
      int lead_ntracks = 0;
      int lead_charge = 0;
      int subl_ntracks = 0;
      int subl_charge = 0;

      for (const auto& xTrack : ditau->trackLinks()) {
         if (!xTrack.isValid())
            continue;

         for (int i = 0; i < 2;  ++i) {  // loop over two leading subjets
             TLorentzVector tlvSubjet = TLorentzVector();
             tlvSubjet.SetPtEtaPhiE(ditau->subjetPt(i), ditau->subjetEta(i),ditau->subjetPhi(i), ditau->subjetE(i));
             double dR = tlvSubjet.DeltaR((*xTrack)->p4());

             if (dR < 0.1) {
                if (i == 0) {
                   lead_ntracks++;
		   lead_charge += (*xTrack)->charge();
                } else if (i == 1) {
                   subl_ntracks++;
		   subl_charge += (*xTrack)->charge();
                }
                break;  // prevents double counting of tracks
            }
         }  // loop over subjets
      }
      leadSubjetNTracksHandle(*ditau) = lead_ntracks;
      leadSubjetChargeHandle(*ditau) = lead_charge;
      subleadSubjetNTracksHandle(*ditau) = subl_ntracks;
      subleadSubjetChargeHandle(*ditau) = subl_charge;
    }
    return StatusCode::SUCCESS;
  }

} // namespace
