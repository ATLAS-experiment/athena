/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "DerivationFrameworkTau/TauIDDecoratorWrapper.h"
#include "StoreGate/ReadHandle.h"
#include "xAODCore/ShallowCopy.h"
#include "AthContainers/Decorator.h"

namespace DerivationFramework {

  TauIDDecoratorWrapper::TauIDDecoratorWrapper(const std::string& t, const std::string& n, const IInterface* p) : 
    base_class(t,n,p)
  {
  }

  StatusCode TauIDDecoratorWrapper::initialize()
  {
    // initialize tauRecTools tools
    ATH_CHECK( m_tauIDTools.retrieve() );

    // parse the properties of TauWPDecorator tools
    for (const auto& tool : m_tauIDTools) {
      if (tool->type() != "TauWPDecorator") continue;

      // check whether we must compute eVeto WPs, as this requires the recalculation of a variable
      BooleanProperty useAbsEta("UseAbsEta", false);
      ATH_CHECK( tool->getProperty(&useAbsEta) ); 
      if (useAbsEta.value()) {
	m_doEvetoWP = true;
      }

      // retrieve the names of ID scores and WPs
      StringProperty scoreName("ScoreName", "");
      ATH_CHECK( tool->getProperty(&scoreName) );
      // the original RNNEleScore should not be overriden
      if (scoreName.value() != "RNNEleScore") {
	m_scores.push_back(scoreName.value());
      }

      StringProperty newScoreName("NewScoreName", "");
      ATH_CHECK( tool->getProperty(&newScoreName) );
      m_scores.push_back(newScoreName.value());
      
      StringArrayProperty decorWPNames("DecorWPNames", {});
      ATH_CHECK( tool->getProperty(&decorWPNames) );
      for (const auto& WP : decorWPNames.value()) m_WPs.push_back(WP);
    }

    // declare decorations to the scheduler
    for (const std::string& score : m_scores) {
      m_scoreDecorKeys.emplace_back(m_tauContainerKey.key() + "." + score);
    }
    for (const std::string& WP : m_WPs) {
      m_WPDecorKeys.emplace_back(m_tauContainerKey.key() + "." + WP);
    }

    // initialize read/write handle keys
    ATH_CHECK( m_tauContainerKey.initialize() );
    ATH_CHECK( m_muonContainerKey.initialize() );
    ATH_CHECK( m_vtxContainerKey.initialize() );
    ATH_CHECK( m_scoreDecorKeys.initialize() );
    ATH_CHECK( m_WPDecorKeys.initialize() );
    ATH_CHECK( m_trackWidthKey.initialize() );
    ATH_CHECK( m_passTATTauMuonOLRKey.initialize() );

    return StatusCode::SUCCESS;
  }

  StatusCode TauIDDecoratorWrapper::finalize()
  {
    return StatusCode::SUCCESS;
  }

  StatusCode TauIDDecoratorWrapper::addBranches() const
  {
    const EventContext& ctx = Gaudi::Hive::currentContext();

    // retrieve tau container
    SG::ReadHandle<xAOD::TauJetContainer> tauJetsReadHandle(m_tauContainerKey, ctx);
    if (!tauJetsReadHandle.isValid()) {
      ATH_MSG_ERROR ("Could not retrieve TauJetContainer with key " << tauJetsReadHandle.key());
      return StatusCode::FAILURE;
    }
    const xAOD::TauJetContainer* tauContainer = tauJetsReadHandle.cptr();

    // retrieve PrimaryVertices container
    SG::ReadHandle<xAOD::VertexContainer> vtxReadHandle(m_vtxContainerKey, ctx);
    if (!vtxReadHandle.isValid()) {
      ATH_MSG_ERROR ("Could not retrieve VertexContainer with key " << vtxReadHandle.key());
      return StatusCode::FAILURE;
    }
    const xAOD::VertexContainer* vtxContainer = vtxReadHandle.cptr();
    const xAOD::Vertex* pVtx = nullptr;
    float sumpt_PV0 = 0., sumpt2_PV0 = 0.;

    // Check that PV container exists and is non-empty, find the PV if possible
    if (vtxContainer != nullptr && !vtxContainer->empty()) {
      auto itrVtx = std::find_if(vtxContainer->begin(), vtxContainer->end(),
				 [](const xAOD::Vertex* vtx) {
				   return vtx->vertexType() == xAOD::VxType::PriVtx;
				 });
      pVtx = (itrVtx == vtxContainer->end() ? nullptr : *itrVtx);
      if (pVtx == nullptr){
        ATH_MSG_DEBUG("No PV found, using the first element instead!");
        pVtx = vtxContainer->at(0);
      }

      for (const ElementLink<xAOD::TrackParticleContainer>& trk : pVtx->trackParticleLinks()) {
	sumpt_PV0 += (*trk)->pt();
	sumpt2_PV0 += std::pow((*trk)->pt(), 2.);
      }
    }
    
    //Create accessors  
    static const SG::Accessor<float> acc_absEtaLead("ABS_ETA_LEAD_TRACK");
    static const SG::Accessor<float> acc_dz0_TV_PV0("dz0_TV_PV0");
    static const SG::Accessor<float> acc_log_sumpt_TV("log_sumpt_TV");
    static const SG::Accessor<float> acc_log_sumpt2_TV("log_sumpt2_TV");
    static const SG::Accessor<float> acc_log_sumpt_PV0("log_sumpt_PV0");
    static const SG::Accessor<float> acc_log_sumpt2_PV0("log_sumpt2_PV0");

    std::vector<SG::WriteDecorHandle<xAOD::TauJetContainer, float> > scoreDecors;
    scoreDecors.reserve (m_scores.size());
    for (const SG::WriteDecorHandleKey<xAOD::TauJetContainer>& k : m_scoreDecorKeys) {
      scoreDecors.emplace_back (k, ctx);
    }
    std::vector<SG::WriteDecorHandle<xAOD::TauJetContainer, char> > WPDecors;
    WPDecors.reserve (m_WPs.size());
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
      
      //Add in the TV/PV0 vertex variables needed for some calculators in TauGNNUtils.cxx (for GNTau)
      float dz0_TV_PV0 = -999., sumpt_TV = 0., sumpt2_TV = 0.;
      if (pVtx!=nullptr) {
        dz0_TV_PV0 = tau->vertex()->z() - pVtx->z();
        for (const ElementLink<xAOD::TrackParticleContainer>& trk : tau->vertex()->trackParticleLinks()) {
          sumpt_TV += (*trk)->pt();
          sumpt2_TV += std::pow((*trk)->pt(), 2.);
        }
      }
      acc_dz0_TV_PV0(*tau) = dz0_TV_PV0;
      acc_log_sumpt_TV(*tau) = (sumpt_TV>0.) ? std::log(sumpt_TV) : 0.;
      acc_log_sumpt2_TV(*tau) = (sumpt2_TV>0.) ? std::log(sumpt2_TV) : 0.;
      acc_log_sumpt_PV0(*tau) = (sumpt_PV0>0.) ? std::log(sumpt_PV0) : 0.;
      acc_log_sumpt2_PV0(*tau) = (sumpt2_PV0>0.) ? std::log(sumpt2_PV0) : 0.;

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
    SG::ReadHandle<xAOD::MuonContainer> muonReadHandle(m_muonContainerKey);
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
