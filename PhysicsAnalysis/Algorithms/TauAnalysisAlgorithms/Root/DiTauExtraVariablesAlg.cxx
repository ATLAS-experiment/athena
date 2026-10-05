/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Antonio De Maria

#include <TauAnalysisAlgorithms/DiTauExtraVariablesAlg.h>

#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/WriteDecorHandle.h>

#include <Math/Vector4D.h>
#include <Math/VectorUtil.h>

#include <array>

namespace CP {

  StatusCode DiTauExtraVariablesAlg::initialize() {

    ANA_CHECK(m_ditausKey.initialize());

    for (SG::WriteDecorHandleKey<xAOD::DiTauJetContainer> *key : {
           &m_nSubjetsKey, &m_omniScoreKey,
           &m_leadSubjetPtKey, &m_leadSubjetEtaKey, &m_leadSubjetPhiKey,
           &m_leadSubjetEKey, &m_leadSubjetNTracksKey, &m_leadSubjetChargeKey,
           &m_subleadSubjetPtKey, &m_subleadSubjetEtaKey, &m_subleadSubjetPhiKey,
           &m_subleadSubjetEKey, &m_subleadSubjetNTracksKey, &m_subleadSubjetChargeKey}) {
      if (key->contHandleKey().key() == key->key()) {
        *key = m_ditausKey.key() + "." + key->key();
      }
      ANA_CHECK(key->initialize());
    }

    return StatusCode::SUCCESS;
  }

  StatusCode DiTauExtraVariablesAlg::execute(const EventContext &ctx) const {

    SG::ReadHandle<xAOD::DiTauJetContainer> ditaus(m_ditausKey, ctx);

    SG::WriteDecorHandle<xAOD::DiTauJetContainer, float> nSubjets(m_nSubjetsKey, ctx);

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

      nSubjets(*ditau) = ditau->nSubjets(); 

      if ( acc_OmniScore.isAvailable(*ditau) ) {
        omniScoreHandle(*ditau) = acc_OmniScore(*ditau);
      }

      // always require at least 2 subjets to have a good ditau	    
      if(ditau->nSubjets() < 2) {
        leadSubjetPtHandle(*ditau) = -999.;
        leadSubjetEtaHandle(*ditau) = -999.;
        leadSubjetPhiHandle(*ditau) = -999.;
        leadSubjetEHandle(*ditau) = -999.;
        leadSubjetNTracksHandle(*ditau) = -999;
        leadSubjetChargeHandle(*ditau) = -999;
        subleadSubjetPtHandle(*ditau) = -999.;
        subleadSubjetEtaHandle(*ditau) = -999.;
        subleadSubjetPhiHandle(*ditau) = -999.;
        subleadSubjetEHandle(*ditau) = -999.;
        subleadSubjetNTracksHandle(*ditau) = -999;
        subleadSubjetChargeHandle(*ditau) = -999;
        continue;
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

      const std::array<ROOT::Math::PtEtaPhiEVector, 2> subjets {
        ROOT::Math::PtEtaPhiEVector(ditau->subjetPt(0), ditau->subjetEta(0), ditau->subjetPhi(0), ditau->subjetE(0)),
        ROOT::Math::PtEtaPhiEVector(ditau->subjetPt(1), ditau->subjetEta(1), ditau->subjetPhi(1), ditau->subjetE(1))};

      for (const auto& xTrack : ditau->trackLinks()) {
         if (!xTrack.isValid())
            continue;

         for (int i = 0; i < 2;  ++i) {  // loop over two leading subjets
             double dR = ROOT::Math::VectorUtil::DeltaR(subjets[i], (*xTrack)->p4());

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
