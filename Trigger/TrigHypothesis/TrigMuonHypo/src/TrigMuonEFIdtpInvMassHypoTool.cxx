/*
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigMuonEFIdtpInvMassHypoTool.h"
#include "TrigMuonEFIdtpCommon.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "xAODMuon/MuonContainer.h"
#include "TrkTrack/TrackCollection.h"
#include "FourMomUtils/xAODP4Helpers.h"

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

TrigMuonEFIdtpInvMassHypoTool::TrigMuonEFIdtpInvMassHypoTool(const std::string & type, const std::string & name, const IInterface* parent):
   ComboHypoToolBase(type, name, parent) {
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

StatusCode TrigMuonEFIdtpInvMassHypoTool::initialize()
{
   if( m_muonqualityCut ) {
      if( m_muonSelTool.retrieve().isFailure() ) {
	 ATH_MSG_ERROR("Unable to retrieve " << m_muonSelTool);
	 return StatusCode::FAILURE;
      }
   } else m_muonSelTool.disable();

   if( m_acceptAll ) {
      ATH_MSG_DEBUG("Accepting all the events!");
   } else {
      if( m_invMassLow<0 && m_invMassHigh<0 ) { 
	 ATH_MSG_ERROR("Both mass cuts are <0. This is probably a configuration mistake.");
	 return StatusCode::FAILURE;
      }
   }

   if ( not m_monTool.name().empty() ) {
      ATH_CHECK( m_monTool.retrieve() );
      ATH_MSG_DEBUG("MonTool name: " << m_monTool);
   }

   ATH_MSG_DEBUG("Initialization completed successfully");
   return StatusCode::SUCCESS;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

bool TrigMuonEFIdtpInvMassHypoTool::executeAlg(const std::vector<Combo::LegDecision>& combination, const EventContext& /*ctx*/) const
{
   ATH_MSG_VERBOSE("in executeAlg");
   bool result = false;

   if( m_acceptAll ) {
      ATH_MSG_DEBUG("Accept property is set: taking all the events");
      return true;
   }

   // monitored variables
   std::vector<float>  mnt_invMass;
   auto mon_invMass   = Monitored::Collection("Mass", mnt_invMass);
   auto monitorIt     = Monitored::Group(m_monTool, mon_invMass);

   // retrieve muon and tracks
   std::vector<const xAOD::TrackParticle*> tracks_pt;
   std::vector<const xAOD::TrackParticle*> tracks_ftf;
   std::vector<const xAOD::Muon*> muons_cb;
   std::vector<const xAOD::Muon*> muons_sa;
   int i_leg_idperf = -1;
   for(auto leg: combination) {
      auto decision= (*(leg.second));
      auto i_leg = TrigCompositeUtils::getIndexFromLeg(leg.first);
      ATH_MSG_VERBOSE("i_leg="<<i_leg);
      auto muonLinks = TrigCompositeUtils::findLinks<xAOD::MuonContainer>( decision, TrigCompositeUtils::featureString(), TrigDefs::lastFeatureOfType);
      if( muonLinks.size() != 1 )        continue;
      if( ! muonLinks.at(0).isValid() )  continue;
      const xAOD::Muon *mu = *(muonLinks.at(0).link);
      if( ! mu->primaryTrackParticle() ) continue;

      bool is_idperf_muon = false;
      ATH_MSG_VERBOSE("... selected: muonType="<<mu->muonType()<<", pT="<<mu->pt()/Gaudi::Units::GeV<<", eta="<<mu->eta()<<", phi="<<mu->phi()<<", author="<<mu->author());
      if(mu->author()==xAOD::Muon::Author::MuidCo && mu->muonType()==xAOD::Muon::MuonType::Combined) {
	 muons_cb.push_back(mu);
      }
      else if(mu->author()==xAOD::Muon::Author::MuidSA && mu->muonType()==xAOD::Muon::MuonType::MuonStandAlone) {
	 muons_sa.push_back(mu);
	 is_idperf_muon = true;
	 if( i_leg_idperf == -1 ) { i_leg_idperf = i_leg; }
	 else if( i_leg_idperf != (int)i_leg ) {
	    ATH_MSG_ERROR("i_leg for idperf looks inconsistent: i_leg_idperf / i_leg="<<i_leg_idperf<<" / "<<i_leg);
	    return result;
	 }
      }
      
      // if this leg is idperf leg (muon is SA) get ID tracks
      if( is_idperf_muon ) {
	 const std::vector< TrigCompositeUtils::LinkInfo<xAOD::TrackParticleContainer> > ptLinks = TrigCompositeUtils::findLinks< xAOD::TrackParticleContainer >( decision, "HLT_IDTrack_Muon_IDTrig", TrigDefs::lastFeatureOfType);
	 ATH_MSG_VERBOSE("PT TrackParticleContainer Links size = "<<ptLinks.size());
	 if( ptLinks.size() == 1 && ptLinks.at(0).isValid() ) {
	    const xAOD::TrackParticle* track = *(ptLinks.at(0).link);
	    tracks_pt.push_back(track);
	    float pt  = track->pt();
	    float eta = track->eta();
	    float phi = track->phi();
	    ATH_MSG_VERBOSE("... pt / eta / phi = "<<pt/Gaudi::Units::GeV << " / " << eta << " / " << phi<<", fitter="<<track->trackFitter());
	 }
	 const std::vector< TrigCompositeUtils::LinkInfo<xAOD::TrackParticleContainer> > ftfLinks = TrigCompositeUtils::findLinks< xAOD::TrackParticleContainer >( decision, "HLT_IDTrack_Muon_FTF", TrigDefs::lastFeatureOfType);
	 ATH_MSG_VERBOSE("FTF TrackParticleContainer Links size = "<< ftfLinks.size());
	 if( ftfLinks.size() == 1 && ftfLinks.at(0).isValid() ) {
	    const xAOD::TrackParticle* track = *(ftfLinks.at(0).link);
	    tracks_ftf.push_back(track);
	    float pt  = track->pt();
	    float eta = track->eta();
	    float phi = track->phi();
	    ATH_MSG_VERBOSE("... pt / eta / phi = "<<pt/Gaudi::Units::GeV << " / " << eta << " / " << phi<<", fitter="<<track->trackFitter());
	 }
      }
   } // end of combination loop

   // mass between CB and SA
   for(auto muon_cb : muons_cb) {
      if( m_muonqualityCut && ! passedCBQualityCuts(muon_cb) ) continue;

      for(auto muon_sa : muons_sa) {
	 if( m_muonqualityCut && ! passedSAQualityCuts(muon_sa) ) continue;

	 const xAOD::TrackParticle* tr_cb = muon_cb->trackParticle(xAOD::Muon::TrackParticleType::CombinedTrackParticle);
	 const xAOD::TrackParticle* tr_sa = muon_sa->trackParticle(xAOD::Muon::TrackParticleType::ExtrapolatedMuonSpectrometerTrackParticle);
	 if (!tr_cb || !tr_sa) {
	    ATH_MSG_ERROR("Either CB or SA TrackParticle not found.");
	    continue;
	 }
	 
	 if( m_selOS && (muon_cb->charge()*muon_sa->charge() > 0) ) continue;

	 float diMuMass = (tr_cb->p4()+tr_sa->p4()).M()/Gaudi::Units::GeV;
	 mnt_invMass.push_back(diMuMass);
	 ATH_MSG_VERBOSE("pt CB / pt SA = " << (*tr_cb).pt()/Gaudi::Units::GeV << " / " << (*tr_sa).pt()/Gaudi::Units::GeV << ", mass =" << diMuMass);

	 if( ((m_invMassLow >=0 && diMuMass>m_invMassLow ) || m_invMassLow <0) &&
	     ((m_invMassHigh>=0 && diMuMass<m_invMassHigh) || m_invMassHigh<0) ) {
	    result = true;
	    StatusCode sc = doTPIdperf(tr_sa, tracks_pt, tracks_ftf);
	    if( sc != StatusCode::SUCCESS ) {
	       ATH_MSG_ERROR("doTPIdperf failed with StatuCode="<<sc);
	    }
	 }
      }
   }
   
   //
   ATH_MSG_VERBOSE("idperf TP overall result is: "<<(result?"true":"false"));
   return result;	
}
  
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

StatusCode TrigMuonEFIdtpInvMassHypoTool::doTPIdperf(const xAOD::TrackParticle* metrack, const std::vector<const xAOD::TrackParticle*>& tracks_pt, const std::vector<const xAOD::TrackParticle*>& tracks_ftf) const
{
   ATH_MSG_VERBOSE("----- doTPIdperf -----");

   // monitored variables
   std::vector<float>  mnt_PT_dr,  mnt_PT_qovp;
   std::vector<float>  mnt_FTF_dr, mnt_FTF_qovp;
   auto mon_PT_dr     = Monitored::Collection("PT_dr",    mnt_PT_dr);
   auto mon_PT_qovp   = Monitored::Collection("PT_qovp",  mnt_PT_qovp);
   auto mon_FTF_dr    = Monitored::Collection("FTF_dr",   mnt_FTF_dr);
   auto mon_FTF_qovp  = Monitored::Collection("FTF_qovp", mnt_FTF_qovp);
   auto mon_phi_effi  = Monitored::Scalar<float>("probePhiEfficiency", -999.0);
   auto mon_eta_effi  = Monitored::Scalar<float>("probeEtaEfficiency", -999.0);
   auto mon_pt_found  = Monitored::Scalar<float>("PTfound",  -1);
   auto mon_ftf_found = Monitored::Scalar<float>("FTFfound", -1);
   auto mon_pt_phi    = Monitored::Scalar<float>("PTphi", -999.0);
   auto mon_pt_pix    = Monitored::Scalar<float>("PTpixelFound", -1);
   auto mon_pt_pixnext= Monitored::Scalar<float>("PTpixelNextToFound", -1);
   auto monitorIt     = Monitored::Group(m_monTool, mon_PT_dr, mon_PT_qovp, mon_FTF_dr, mon_FTF_qovp, mon_phi_effi, mon_eta_effi, mon_pt_found, mon_ftf_found, mon_pt_phi, mon_pt_pix, mon_pt_pixnext);

   // probe values
   auto mnt_probe_pt  = Monitored::Scalar<float>("probe_pt",  0);
   auto mnt_probe_eta = Monitored::Scalar<float>("probe_eta", 0);
   auto monProbe      = Monitored::Group(m_monTool, mnt_probe_pt, mnt_probe_eta);
   mnt_probe_pt  = metrack->pt()/Gaudi::Units::GeV;
   mnt_probe_eta = metrack->eta();

   // PT 
   const float PT_DR_CUT   = 0.1;
   const float PT_QOVP_CUT = 2.0;
   ATH_MSG_VERBOSE("PT: n size="<<tracks_pt.size());
   int n_pt_matched = 0;
   float min_dr_pt_matched = 999;
   uint8_t pt_expectInnermost=0;
   uint8_t pt_numberInnermost=0;
   uint8_t pt_expectNextToInnermost=0;
   uint8_t pt_numberNextToInnermost=0;
   for(auto idtrack : tracks_pt) {
      float dr_pt   = xAOD::P4Helpers::deltaR(metrack,idtrack);
      float qovp_pt = TrigMuonEFIdtpCommon::qOverPMatching(metrack,idtrack);
      mnt_PT_dr.push_back(dr_pt);
      mnt_PT_qovp.push_back(qovp_pt);
      ATH_MSG_VERBOSE("... dr (ME-PT): "<<dr_pt);
      ATH_MSG_VERBOSE("... Q/p match (ME-PT): "<<qovp_pt);
      if( dr_pt > PT_DR_CUT ) continue;
      if( qovp_pt > PT_QOVP_CUT ) continue;
      ++n_pt_matched;
      if( dr_pt < min_dr_pt_matched ) {
	 min_dr_pt_matched = dr_pt;
	 mon_pt_phi = idtrack->phi();
	 idtrack->summaryValue(pt_expectInnermost,xAOD::expectInnermostPixelLayerHit);
	 idtrack->summaryValue(pt_numberInnermost,xAOD::numberOfInnermostPixelLayerHits);
	 idtrack->summaryValue(pt_expectNextToInnermost,xAOD::expectNextToInnermostPixelLayerHit);
	 idtrack->summaryValue(pt_numberNextToInnermost,xAOD::numberOfNextToInnermostPixelLayerHits);	 
      }
   }
   if( n_pt_matched > 1 ) n_pt_matched=1;

   // FTF
   const float FTF_DR_CUT   = 0.2;
   const float FTF_QOVP_CUT = 3.0;
   ATH_MSG_VERBOSE("FTF: n size="<<tracks_ftf.size());
   int n_ftf_matched = 0;
   for(auto idtrack : tracks_ftf) {
      float dr_pt   = xAOD::P4Helpers::deltaR(metrack,idtrack);
      float qovp_pt = TrigMuonEFIdtpCommon::qOverPMatching(metrack,idtrack);
      mnt_FTF_dr.push_back(dr_pt);
      mnt_FTF_qovp.push_back(qovp_pt);
      ATH_MSG_VERBOSE("... dr (ME-FTF): "<<dr_pt);
      ATH_MSG_VERBOSE("... Q/p match (ME-FTF): "<<qovp_pt);
      if( dr_pt > FTF_DR_CUT ) continue;
      if( qovp_pt > FTF_QOVP_CUT ) continue;
      ++n_ftf_matched;
   }
   if( n_ftf_matched > 1 ) n_ftf_matched=1;
   
   // efficiency
   float me_eta    = metrack->eta();
   float me_phi    = metrack->phi();
   float me_pt_gev = metrack->pt()/Gaudi::Units::GeV;

   int eta_nr = 0;
   if( std::abs(me_eta) > 2.0 ) { eta_nr = 2; }
   else if( std::abs(me_eta) > 1.0 ) { eta_nr = 1; }
   int   pt_nr = 0;
   if( me_pt_gev > 20.0 ) { pt_nr = 1; }

   std::stringstream ss;
   ss << "PT_effi_pt" << pt_nr << "_eta" << eta_nr;
   auto mnt_PT_effi = Monitored::Scalar<int>(ss.str(), 0);
   ss.clear();
   ss << "PT_effi_pt" << pt_nr << "_eta" << eta_nr;
   auto mnt_FTF_effi = Monitored::Scalar<int>(ss.str(), 0);
   auto monEffi = Monitored::Group(m_monTool, mnt_PT_effi, mnt_FTF_effi);
   mnt_PT_effi  = n_pt_matched;
   mnt_FTF_effi = n_ftf_matched;

   // TnP monitoring
   mon_phi_effi  = me_phi;
   mon_eta_effi  = me_eta;
   mon_pt_found  = n_pt_matched;
   mon_ftf_found = n_ftf_matched;
   if( n_pt_matched != 0 ) {
      if( pt_expectInnermost == 1 ) {
	 if( pt_numberInnermost>0 ) {
	    mon_pt_pix=1;
	 } else {
	    mon_pt_pix=0;
	 }
      }
      if( pt_expectNextToInnermost == 1 ) {
	 if( pt_numberNextToInnermost>0 ) {
	    mon_pt_pixnext=1;
	 } else {
	    mon_pt_pixnext=0;
	 }
      }
   }

   //
   return StatusCode::SUCCESS;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

bool TrigMuonEFIdtpInvMassHypoTool::passedCBQualityCuts(const xAOD::Muon* muon) const
{
   bool passCut = false;

   const xAOD::TrackParticle* idtrack = muon->trackParticle( xAOD::Muon::InnerDetectorTrackParticle );
   const xAOD::TrackParticle* metrack = muon->trackParticle( xAOD::Muon::ExtrapolatedMuonSpectrometerTrackParticle );

   const float CHI2_CUT = 8.0;
   const float QOVP_CUT = 7.0;
    
   if( idtrack && metrack ) {
      float qOverPsignif = TrigMuonEFIdtpCommon::qOverPMatching(metrack,idtrack);
      float reducedChi2  = muon->primaryTrackParticle()->chiSquared()/muon->primaryTrackParticle()->numberDoF(); 
      // Selection criteria based on the requirements that are part of the muon quality working points (offline)
      if(std::abs(reducedChi2) < CHI2_CUT && !m_muonSelTool->isBadMuon(*muon) && qOverPsignif<QOVP_CUT && muon->author()==xAOD::Muon::MuidCo) passCut = true;
   }
   
   return passCut;
}


bool TrigMuonEFIdtpInvMassHypoTool::passedSAQualityCuts(const xAOD::Muon* muon) const
{
   bool passCut = false;

   const xAOD::TrackParticle* metrack = muon->trackParticle( xAOD::Muon::ExtrapolatedMuonSpectrometerTrackParticle );

   const float CHI2_CUT = 8.0;

   if( metrack ) {
      float reducedChi2 = muon->primaryTrackParticle()->chiSquared()/muon->primaryTrackParticle()->numberDoF(); 
      if( std::abs(reducedChi2) < CHI2_CUT && !m_muonSelTool->isBadMuon(*muon) ) passCut = true;
   }
   
   return passCut;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
