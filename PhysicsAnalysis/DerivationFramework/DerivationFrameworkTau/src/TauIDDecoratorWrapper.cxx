/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "DerivationFrameworkTau/TauIDDecoratorWrapper.h"
#include "StoreGate/ReadHandle.h"
#include "xAODCore/ShallowCopy.h"
#include "AthContainers/Decorator.h"

namespace DerivationFramework {

  StatusCode TauIDDecoratorWrapper::initialize()
  {
    // initialize tauRecTools tools
    ATH_CHECK( m_tauIDTools.retrieve() );

    // initialize read/write handle keys
    ATH_CHECK( m_tauContainerKey.initialize() );
    ATH_CHECK( m_muonContainerKey.initialize() );
    ATH_CHECK( m_scoreDecorKeys.initialize() );
    ATH_CHECK( m_WPDecorKeys.initialize() );
    ATH_CHECK( m_trackWidthKey.initialize() );
    ATH_CHECK( m_passTATTauMuonOLRKey.initialize() );

    return StatusCode::SUCCESS;
  }


  StatusCode TauIDDecoratorWrapper::addBranches(const EventContext& ctx) const
  {

    // retrieve tau container
    SG::ReadHandle<xAOD::TauJetContainer> tauJetsReadHandle(m_tauContainerKey, ctx);
    if (!tauJetsReadHandle.isValid()) {
      ATH_MSG_ERROR ("Could not retrieve TauJetContainer with key " << tauJetsReadHandle.key());
      return StatusCode::FAILURE;
    }
    const xAOD::TauJetContainer* tauContainer = tauJetsReadHandle.cptr();

    //Create accessors
    static const SG::Accessor<float> acc_absEtaLead("ABS_ETA_LEAD_TRACK");

    std::vector<SG::WriteDecorHandle<xAOD::TauJetContainer, float> > scoreDecors;
    scoreDecors.reserve (m_scoreDecorKeys.size());
    for (const SG::WriteDecorHandleKey<xAOD::TauJetContainer>& k : m_scoreDecorKeys) {
      scoreDecors.emplace_back (k, ctx);
    }
    std::vector<SG::WriteDecorHandle<xAOD::TauJetContainer, char> > WPDecors;
    WPDecors.reserve (m_WPDecorKeys.size());
    for (const SG::WriteDecorHandleKey<xAOD::TauJetContainer>& k : m_WPDecorKeys) {
      WPDecors.emplace_back (k, ctx);
    }

    SG::WriteDecorHandle<xAOD::TauJetContainer, float> dec_trackWidth (m_trackWidthKey, ctx);
    SG::WriteDecorHandle<xAOD::TauJetContainer, bool> dec_passTATTauMuonOLR (m_passTATTauMuonOLRKey, ctx);
    for (const auto tau : *tauContainer) {
      float tauTrackBasedWidth = 0.;
      // equivalent to tracks(xAOD::TauJetParameters::TauTrackFlag::classifiedCharged)
      std::vector<const xAOD::TauTrack *> tauTracks = tau->tracks();
      for (const xAOD::TauTrack *trk : tau->tracks(xAOD::TauJetParameters::TauTrackFlag::classifiedIsolation)) {
        tauTracks.push_back(trk);
      }
      double sumWeightedDR = 0.;
      double ptSum = 0.;
      for (const xAOD::TauTrack *track : tauTracks) {
        double deltaR = tau->p4().DeltaR(track->p4());
        sumWeightedDR += deltaR * track->pt();
        ptSum += track->pt();
      }
      if (ptSum > 0.) {
        tauTrackBasedWidth = sumWeightedDR / ptSum;
      }

      dec_trackWidth(*tau) = tauTrackBasedWidth;
    }

    // create shallow copy
    auto shallowCopy = xAOD::shallowCopyContainer (*tauContainer);

    for (auto tau : *shallowCopy.first) {

      // ABS_ETA_LEAD_TRACK is removed from the AOD content and must be redecorated when computing eVeto WPs
      // note: this redecoration is not robust against charged track thinning, but charged tracks should never be thinned
      if (m_doEvetoWP) {
        float absEtaLead = -1111.;
        if(tau->nTracks() > 0) {
          const xAOD::TrackParticle* track = tau->track(0)->track();
          absEtaLead = std::abs( track->eta() );
        }
        acc_absEtaLead(*tau) = absEtaLead;
      }

      // pass the shallow copy to the tools
      for (const auto& tool : m_tauIDTools) {
        ATH_CHECK( tool->execute(*tau) );
      }

      // copy over the relevant decorations (scores and working points)
      const xAOD::TauJet* xTau = tauContainer->at(tau->index());
      for (SG::WriteDecorHandle<xAOD::TauJetContainer, float>& dec : scoreDecors) {
        SG::ConstAccessor<float> scoreAcc (dec.auxid());
        dec(*xTau) = scoreAcc(*tau);
      }
      for (SG::WriteDecorHandle<xAOD::TauJetContainer, char>& dec : WPDecors) {
        SG::ConstAccessor<char> WPAcc (dec.auxid());
        dec(*xTau) = WPAcc(*tau);
      }
    }

    delete shallowCopy.first;
    delete shallowCopy.second;

    // add TauAnalysisTool MuonOLR
    SG::ReadHandle<xAOD::MuonContainer> muonReadHandle(m_muonContainerKey, ctx);
    if (!muonReadHandle.isValid()) {
      ATH_MSG_DEBUG ("Could not retrieve MuonContainer with key " << muonReadHandle.key() << " so won't add TAT MuonOLR flag");
      return StatusCode::SUCCESS;
    }
    const xAOD::MuonContainer* muonContainer = muonReadHandle.cptr();

    for (const auto tau : *tauContainer) {
      bool bTauMuonOLR = true;
      for (auto muon : *muonContainer){
        if(muon->pt() < 2000.) continue; // pt > 2 GeV
        if(muon->muonType() == xAOD::Muon::CaloTagged) continue; // not calo-tagged
        if(muon->p4().DeltaR( tau->p4() ) > 0.2 ) continue; // delta R < 0.2
        bTauMuonOLR = false; // muon-tau overlapped
        break;
      }
      dec_passTATTauMuonOLR(*tau) = bTauMuonOLR;
    }

    return StatusCode::SUCCESS;
  }
}
