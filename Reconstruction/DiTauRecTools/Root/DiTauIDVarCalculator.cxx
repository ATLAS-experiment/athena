/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "DiTauRecTools/DiTauIDVarCalculator.h"

// Core include(s):
#include "AthLinks/ElementLink.h"
#include "AthContainers/Decorator.h"
#include "AthContainers/ConstAccessor.h"

// EDM include(s):
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTracking/TrackParticle.h"

#include "xAODTau/DiTauJet.h"

using namespace DiTauRecTools;

using TrackParticleLinks_t = std::vector<ElementLink<xAOD::TrackParticleContainer>>;

//=================================PUBLIC-PART==================================
//______________________________________________________________________________
DiTauIDVarCalculator::DiTauIDVarCalculator( const std::string& name )
  : AsgTool(name)
{
  declareProperty( "DefaultValue", m_dDefault = -1234);
}

//______________________________________________________________________________
DiTauIDVarCalculator::~DiTauIDVarCalculator( )
= default;

//______________________________________________________________________________
StatusCode DiTauIDVarCalculator::initialize()
{
  ATH_MSG_INFO( "Initializing DiTauIDVarCalculator" );
  
  return StatusCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
//                              Wrapper functions                             //
////////////////////////////////////////////////////////////////////////////////

StatusCode DiTauIDVarCalculator::calculateIDVariables(const xAOD::DiTauJet& xDiTau){
  return execute(xDiTau);
}

StatusCode DiTauIDVarCalculator::execute(const xAOD::DiTauJet& xDiTau)
{
  
  ATH_MSG_DEBUG("Calculate DiTau ID variables");
  
  static const SG::Decorator< int > n_subjetsDec("n_subjets");
  n_subjetsDec(xDiTau) = n_subjets(xDiTau);
  ATH_CHECK( decorNtracks(xDiTau) );

  static const SG::Decorator< float > ditau_ptDec("ditau_pt");
  static const SG::Decorator< float > f_core_leadDec("f_core_lead");
  static const SG::Decorator< float > f_core_sublDec("f_core_subl");
  static const SG::Decorator< float > f_subjet_leadDec("f_subjet_lead");
  static const SG::Decorator< float > f_subjet_sublDec("f_subjet_subl");
  static const SG::Decorator< float > f_subjetsDec("f_subjets");
  static const SG::Decorator< float > f_track_leadDec("f_track_lead");
  static const SG::Decorator< float > f_track_sublDec("f_track_subl");
  static const SG::Decorator< float > R_max_leadDec("R_max_lead");
  static const SG::Decorator< float > R_max_sublDec("R_max_subl");
  static const SG::Decorator< int >   n_trackDec("n_track");
  static const SG::Decorator< int >   n_tracks_leadDec("n_tracks_lead");
  static const SG::Decorator< int >   n_tracks_sublDec("n_tracks_subl");
  static const SG::Decorator< int >   n_isotrackDec("n_isotrack");
  static const SG::Decorator< float > R_trackDec("R_track");
  static const SG::Decorator< float > R_track_coreDec("R_track_core");
  static const SG::Decorator< float > R_track_allDec("R_track_all");
  static const SG::Decorator< float > R_isotrackDec("R_isotrack");
  static const SG::Decorator< float > R_core_leadDec("R_core_lead");
  static const SG::Decorator< float > R_core_sublDec("R_core_subl");
  static const SG::Decorator< float > R_tracks_leadDec("R_tracks_lead");
  static const SG::Decorator< float > R_tracks_sublDec("R_tracks_subl");
  static const SG::Decorator< float > M_trackDec("m_track");
  static const SG::Decorator< float > M_track_coreDec("m_track_core");
  static const SG::Decorator< float > M_core_leadDec("m_core_lead");
  static const SG::Decorator< float > M_core_sublDec("m_core_subl");
  static const SG::Decorator< float > M_track_allDec("m_track_all");
  static const SG::Decorator< float > M_tracks_leadDec("m_tracks_lead");
  static const SG::Decorator< float > M_tracks_sublDec("m_tracks_subl");
  static const SG::Decorator< float > E_frac_sublDec("E_frac_subl");
  static const SG::Decorator< float > E_frac_subsublDec("E_frac_subsubl");
  static const SG::Decorator< float > R_subjets_sublDec("R_subjets_subl");
  static const SG::Decorator< float > R_subjets_subsublDec("R_subjets_subsubl");
  static const SG::Decorator< float > d0_leadtrack_leadDec("d0_leadtrack_lead");
  static const SG::Decorator< float > d0_leadtrack_sublDec("d0_leadtrack_subl");
  static const SG::Decorator< float > f_isotracksDec("f_isotracks");

  ditau_ptDec(xDiTau) = ditau_pt(xDiTau);
  f_core_leadDec(xDiTau) = f_core(xDiTau, 0);
  f_core_sublDec(xDiTau) = f_core(xDiTau, 1);
  f_subjet_leadDec(xDiTau) = f_subjet(xDiTau, 0);
  f_subjet_sublDec(xDiTau) = f_subjet(xDiTau, 1);
  f_subjetsDec(xDiTau) = f_subjets(xDiTau);
  f_track_leadDec(xDiTau) = f_track(xDiTau, 0);
  f_track_sublDec(xDiTau) = f_track(xDiTau, 1);
  R_max_leadDec(xDiTau) = R_max(xDiTau, 0);
  R_max_sublDec(xDiTau) = R_max(xDiTau, 1);
  n_trackDec(xDiTau) = n_track(xDiTau);
  n_tracks_leadDec(xDiTau) = n_tracks(xDiTau, 0);
  n_tracks_sublDec(xDiTau) = n_tracks(xDiTau, 1);
  n_isotrackDec(xDiTau) = n_isotrack(xDiTau);
  R_trackDec(xDiTau) = R_track(xDiTau);
  R_track_coreDec(xDiTau) = R_track_core(xDiTau);
  R_track_allDec(xDiTau) = R_track_all(xDiTau);
  R_isotrackDec(xDiTau) = R_isotrack(xDiTau);
  R_core_leadDec(xDiTau) = R_core(xDiTau, 0);
  R_core_sublDec(xDiTau) = R_core(xDiTau, 1);
  R_tracks_leadDec(xDiTau) = R_tracks(xDiTau, 0);
  R_tracks_sublDec(xDiTau) = R_tracks(xDiTau, 1);
  M_trackDec(xDiTau) = mass_track(xDiTau);
  M_track_coreDec(xDiTau) = mass_track_core(xDiTau);
  M_core_leadDec(xDiTau) = mass_core(xDiTau, 0);
  M_core_sublDec(xDiTau) = mass_core(xDiTau, 1);
  M_track_allDec(xDiTau) = mass_track_all(xDiTau);
  M_tracks_leadDec(xDiTau) = mass_tracks(xDiTau, 0);
  M_tracks_sublDec(xDiTau) = mass_tracks(xDiTau, 1);
  E_frac_sublDec(xDiTau) = E_frac(xDiTau,1);
  E_frac_subsublDec(xDiTau) = E_frac(xDiTau, 2);
  R_subjets_sublDec(xDiTau) = R_subjets(xDiTau, 1);
  R_subjets_subsublDec(xDiTau) = R_subjets(xDiTau, 2);
  d0_leadtrack_leadDec(xDiTau) = d0_leadtrack(xDiTau, 0);
  d0_leadtrack_sublDec(xDiTau) = d0_leadtrack(xDiTau, 1);
  f_isotracksDec(xDiTau) = f_isotracks(xDiTau);

  return StatusCode::SUCCESS;
}

//=================================PRIVATE-PART=================================
//______________________________________________________________________________

float DiTauIDVarCalculator::n_subjets(const xAOD::DiTauJet& xDiTau) 
{
  int nSubjet = 0;
  while (xDiTau.subjetPt(nSubjet) > 0. )
  {
    nSubjet++;
  }

  return nSubjet;
}


float DiTauIDVarCalculator::ditau_pt(const xAOD::DiTauJet& xDiTau) const
{
  static const SG::ConstAccessor<int> n_subjetsAcc("n_subjets");
  if (n_subjetsAcc(xDiTau) < 2 ) {
    return m_dDefault;
  }

  return xDiTau.subjetPt(0)+xDiTau.subjetPt(1);
}


//______________________________________________________________________________;
float DiTauIDVarCalculator::f_core(const xAOD::DiTauJet& xDiTau, int iSubjet) const 
{
  static const SG::ConstAccessor<int> n_subjetsAcc("n_subjets");
  if (iSubjet < 0 || iSubjet >= n_subjetsAcc(xDiTau)) {
    return m_dDefault;
  }

  return xDiTau.fCore(iSubjet);
}


//______________________________________________________________________________;
float DiTauIDVarCalculator::f_subjet(const xAOD::DiTauJet& xDiTau, int iSubjet) const
{
  static const SG::ConstAccessor<int> n_subjetsAcc("n_subjets");
  if (iSubjet < 0 || iSubjet >= n_subjetsAcc(xDiTau)) {
    return m_dDefault;
  }

  return xDiTau.subjetPt(iSubjet) / xDiTau.pt();
}


//______________________________________________________________________________;
float DiTauIDVarCalculator::f_subjets(const xAOD::DiTauJet& xDiTau) const
{
  static const SG::ConstAccessor<int> n_subjetsAcc("n_subjets");
  if (n_subjetsAcc(xDiTau) < 2 ) {
    return m_dDefault;
  }

  return (xDiTau.subjetPt(0) + xDiTau.subjetPt(1))/ xDiTau.pt();
}


//______________________________________________________________________________;
float DiTauIDVarCalculator::f_track(const xAOD::DiTauJet& xDiTau, int iSubjet) const
{
  static const SG::ConstAccessor<int> n_subjetsAcc("n_subjets");
  if (iSubjet < 0 || iSubjet >= n_subjetsAcc(xDiTau)) {
    return m_dDefault;
  }

  static const SG::ConstAccessor<TrackParticleLinks_t> trackLinksAcc("trackLinks");
  if (!trackLinksAcc.isAvailable(xDiTau) )
  {
    ATH_MSG_WARNING("Link not available");
  } 

  TrackParticleLinks_t xTracks = xDiTau.trackLinks();

  TLorentzVector tlvSubjet;
  tlvSubjet.SetPtEtaPhiE( xDiTau.subjetPt(iSubjet),
                          xDiTau.subjetEta(iSubjet),
                          xDiTau.subjetPhi(iSubjet),
                          xDiTau.subjetE(iSubjet) );
      
  TLorentzVector tlvTrack;
  TLorentzVector tlvLeadTrack;
  tlvLeadTrack.SetPtEtaPhiE( 0,0,0, 0);

  for (const auto &xTrack: xTracks) 
  { 
    if (!xTrack) 
    {
      ATH_MSG_ERROR("Could not read Track");
      continue;
    }
    tlvTrack.SetPtEtaPhiE( (*xTrack)->pt(),
                           (*xTrack)->eta(),
                           (*xTrack)->phi(),
                           (*xTrack)->e() );

    if ( tlvSubjet.DeltaR(tlvTrack) < 0.2 )
    {
      if (tlvLeadTrack.Pt() < tlvTrack.Pt()) 
      {
        tlvLeadTrack = tlvTrack;
      }
    }
  }

  return tlvLeadTrack.Pt() / tlvSubjet.Pt();
}


//______________________________________________________________________________;
float DiTauIDVarCalculator::R_max(const xAOD::DiTauJet& xDiTau, int iSubjet) const
{ 
  static const SG::ConstAccessor<int> n_subjetsAcc("n_subjets");
  if (iSubjet < 0 || iSubjet >= n_subjetsAcc(xDiTau)) {
    return m_dDefault;
  }

  TrackParticleLinks_t xTracks = xDiTau.trackLinks();

  TLorentzVector tlvSubjet;
  tlvSubjet.SetPtEtaPhiE( xDiTau.subjetPt(iSubjet),
                          xDiTau.subjetEta(iSubjet),
                          xDiTau.subjetPhi(iSubjet),
                          xDiTau.subjetE(iSubjet) );
      
  TLorentzVector tlvTrack;
  TLorentzVector tlvRmaxTrack;
  double Rmax = 0;
  static const SG::ConstAccessor<float> R_subjetAcc("R_subjet");
  for (const auto &xTrack: xTracks) 
  {
    tlvTrack.SetPtEtaPhiE( (*xTrack)->pt(),
                           (*xTrack)->eta(),
                           (*xTrack)->phi(),
                           (*xTrack)->e() );

    if ( tlvSubjet.DeltaR(tlvTrack) < R_subjetAcc(xDiTau) )
    {
      if (tlvTrack.DeltaR(tlvSubjet) > Rmax) 
      {
        Rmax = tlvTrack.DeltaR(tlvSubjet);
      }
    }
  }

  return Rmax;
}


//______________________________________________________________________________;
int DiTauIDVarCalculator::n_track(const xAOD::DiTauJet& xDiTau) 
{ 
  return xDiTau.nTracks();
}

//______________________________________________________________________________;
int DiTauIDVarCalculator::n_tracks(const xAOD::DiTauJet& xDiTau, int iSubjet) const
{
  static const SG::ConstAccessor<int> n_subjetsAcc("n_subjets");
  if (iSubjet < 0 || iSubjet >= n_subjetsAcc(xDiTau)) {
    return m_dDefault;
  }

  static const SG::ConstAccessor<std::vector<int> > n_tracksAcc("n_tracks");
  if (!n_tracksAcc.isAvailable(xDiTau))
  {
    ATH_MSG_DEBUG("n_tracks decoration not available. Try with track links.");

    static const SG::ConstAccessor<TrackParticleLinks_t> trackLinksAcc("trackLinks");
    if (!trackLinksAcc.isAvailable(xDiTau) )
    {
      ATH_MSG_WARNING("Track links not available. Return 0.");
      return (int)m_dDefault;
    } 

    TrackParticleLinks_t xTracks = xDiTau.trackLinks();

    TLorentzVector tlvSubjet;
    tlvSubjet.SetPtEtaPhiE( xDiTau.subjetPt(iSubjet),
                            xDiTau.subjetEta(iSubjet),
                            xDiTau.subjetPhi(iSubjet),
                            xDiTau.subjetE(iSubjet) );
        
    TLorentzVector tlvTrack;
    int nTracks = 0;
    for (const auto &xTrack: xTracks) 
    { 
      tlvTrack.SetPtEtaPhiE( (*xTrack)->pt(),
                             (*xTrack)->eta(),
                             (*xTrack)->phi(),
                             (*xTrack)->e() );
      if ( tlvSubjet.DeltaR(tlvTrack) < 0.2 ) nTracks++;
    }

    return nTracks;
  }

  return n_tracksAcc(xDiTau).at(iSubjet);

}

//______________________________________________________________________________;
int DiTauIDVarCalculator::n_isotrack(const xAOD::DiTauJet& xDiTau) 
{ 
  return xDiTau.nIsoTracks();
}

//______________________________________________________________________________;
float DiTauIDVarCalculator::R_tracks(const xAOD::DiTauJet& xDiTau, int iSubjet) const
{
  double R_sum = 0;
  double pt = 0;

  static const SG::ConstAccessor<TrackParticleLinks_t> trackLinksAcc("trackLinks");
  if (!trackLinksAcc.isAvailable(xDiTau) )
  {
    ATH_MSG_WARNING("Link not available");
  }

  static const SG::ConstAccessor<int> n_subjetsAcc("n_subjets");
  if (iSubjet < 0 || iSubjet >= n_subjetsAcc(xDiTau)) {
    return m_dDefault;
  }
 
  TrackParticleLinks_t xTracks = xDiTau.trackLinks();

  TLorentzVector tlvSubjet;
  tlvSubjet.SetPtEtaPhiE( xDiTau.subjetPt(iSubjet),
                          xDiTau.subjetEta(iSubjet),
                          xDiTau.subjetPhi(iSubjet),
                          xDiTau.subjetE(iSubjet) );
    
  TLorentzVector tlvTrack;

  for (const auto& xTrack: xTracks) 
  { 
    tlvTrack.SetPtEtaPhiE( (*xTrack)->pt(),
                           (*xTrack)->eta(),
                           (*xTrack)->phi(),
                           (*xTrack)->e() );

    if ( tlvSubjet.DeltaR(tlvTrack) < 0.2 )
    {
      //ATH_MSG_DEBUG("smaller");
      R_sum += tlvSubjet.DeltaR(tlvTrack)*tlvTrack.Pt();
      pt += tlvTrack.Pt();
    }
  }
  
  if (pt == 0)
  {
    return m_dDefault;
  }

  return R_sum / pt;
}

//______________________________________________________________________________;
float DiTauIDVarCalculator::R_core(const xAOD::DiTauJet& xDiTau, int iSubjet) const
{
  double R_sum = 0;
  double pt = 0;

  static const SG::ConstAccessor<TrackParticleLinks_t> trackLinksAcc("trackLinks");
  if (!trackLinksAcc.isAvailable(xDiTau) )
  {
    ATH_MSG_WARNING("Link not available");
  }
  static const SG::ConstAccessor<int> n_subjetsAcc("n_subjets");
  if (iSubjet < 0 || iSubjet >= n_subjetsAcc(xDiTau)) {
    return m_dDefault;
  }
 
  TrackParticleLinks_t xTracks = xDiTau.trackLinks();

  TLorentzVector tlvSubjet;
  tlvSubjet.SetPtEtaPhiE( xDiTau.subjetPt(iSubjet),
                          xDiTau.subjetEta(iSubjet),
                          xDiTau.subjetPhi(iSubjet),
                          xDiTau.subjetE(iSubjet) );
    
  TLorentzVector tlvTrack;

  static const SG::ConstAccessor<float> R_coreAcc("R_core");
  for (const auto& xTrack: xTracks) 
  { 
    tlvTrack.SetPtEtaPhiE( (*xTrack)->pt(),
                           (*xTrack)->eta(),
                           (*xTrack)->phi(),
                           (*xTrack)->e() );

    if ( tlvSubjet.DeltaR(tlvTrack) < R_coreAcc(xDiTau) )
    {
      R_sum += tlvSubjet.DeltaR(tlvTrack)*tlvTrack.Pt();
      pt += tlvTrack.Pt();
    }
  }
  
  if (pt == 0)
  {
    return m_dDefault;
  }

  return R_sum / pt;
}

//______________________________________________________________________________;
float DiTauIDVarCalculator::R_track_core(const xAOD::DiTauJet& xDiTau) const
{
  double R_sum = 0;
  double pt = 0;

  static const SG::ConstAccessor<TrackParticleLinks_t> trackLinksAcc("trackLinks");
  if (!trackLinksAcc.isAvailable(xDiTau) )
  {
    ATH_MSG_WARNING("Link not available");
  }
  static const SG::ConstAccessor<int> n_subjetsAcc("n_subjets");
  if (n_subjetsAcc(xDiTau) < 2) {
    return m_dDefault;
  }
 

  for (int i = 0; i<=1; i++)
  {
  
    TrackParticleLinks_t xTracks = xDiTau.trackLinks();

    TLorentzVector tlvSubjet;
    tlvSubjet.SetPtEtaPhiE( xDiTau.subjetPt(i),
                            xDiTau.subjetEta(i),
                            xDiTau.subjetPhi(i),
                            xDiTau.subjetE(i) );
      
    TLorentzVector tlvTrack;

    static const SG::ConstAccessor<float> R_coreAcc("R_core");
    for (const auto& xTrack: xTracks) 
    { 
      tlvTrack.SetPtEtaPhiE( (*xTrack)->pt(),
                             (*xTrack)->eta(),
                             (*xTrack)->phi(),
                             (*xTrack)->e() );
      if ( tlvSubjet.DeltaR(tlvTrack) < R_coreAcc(xDiTau) )
      {
        //ATH_MSG_DEBUG("smaller");
        R_sum += tlvSubjet.DeltaR(tlvTrack)*tlvTrack.Pt();
        pt += tlvTrack.Pt();
      }
    }
  }
  if (pt == 0)
  {
    return m_dDefault;
  }

  return R_sum / pt;
}

//______________________________________________________________________________;
float DiTauIDVarCalculator::R_track(const xAOD::DiTauJet& xDiTau) const
{
  double R_sum = 0;
  double pt = 0;

  static const SG::ConstAccessor<TrackParticleLinks_t> trackLinksAcc("trackLinks");
  if (!trackLinksAcc.isAvailable(xDiTau) )
  {
    ATH_MSG_WARNING("Link not available");
  }
  static const SG::ConstAccessor<int> n_subjetsAcc("n_subjets");
  if (n_subjetsAcc(xDiTau) < 2) {
    return m_dDefault;
  }
 
  for (int i = 0; i<=1; i++)
  {
  
    TrackParticleLinks_t xTracks = xDiTau.trackLinks();

    TLorentzVector tlvSubjet;
    tlvSubjet.SetPtEtaPhiE( xDiTau.subjetPt(i),
                            xDiTau.subjetEta(i),
                            xDiTau.subjetPhi(i),
                            xDiTau.subjetE(i) );
      
    TLorentzVector tlvTrack;

    for (const auto& xTrack: xTracks) 
    { 
      tlvTrack.SetPtEtaPhiE( (*xTrack)->pt(),
                             (*xTrack)->eta(),
                             (*xTrack)->phi(),
                             (*xTrack)->e() );

      if (tlvSubjet.DeltaR(tlvTrack) < 0.2)
      {
        R_sum += tlvSubjet.DeltaR(tlvTrack)*tlvTrack.Pt();
        pt += tlvTrack.Pt();
      }
    }
  }
  if (pt == 0)
  {
    return m_dDefault;
  }

  return R_sum / pt;
}
//______________________________________________________________________________;
float DiTauIDVarCalculator::R_track_all(const xAOD::DiTauJet& xDiTau) const
{
  double R_sum = 0;
  double pt = 0;

  static const SG::ConstAccessor<TrackParticleLinks_t> trackLinksAcc("trackLinks");
  if (!trackLinksAcc.isAvailable(xDiTau) )
  {
    ATH_MSG_WARNING("Link not available");
  }

  static const SG::ConstAccessor<int> n_subjetsAcc("n_subjets");
  for (int i = 0; i<n_subjetsAcc(xDiTau); i++)
  {
  
    TrackParticleLinks_t xTracks = xDiTau.trackLinks();

    TLorentzVector tlvSubjet;
    tlvSubjet.SetPtEtaPhiE( xDiTau.subjetPt(i),
                            xDiTau.subjetEta(i),
                            xDiTau.subjetPhi(i),
                            xDiTau.subjetE(i) );
      
    TLorentzVector tlvTrack;

    for (const auto& xTrack: xTracks) 
    { 
      tlvTrack.SetPtEtaPhiE( (*xTrack)->pt(),
                             (*xTrack)->eta(),
                             (*xTrack)->phi(),
                             (*xTrack)->e() );

      if (tlvSubjet.DeltaR(tlvTrack) <= 0.2)
      {
        R_sum += tlvSubjet.DeltaR(tlvTrack)*tlvTrack.Pt();
        pt += tlvTrack.Pt();
      }
    }
  }

  if (pt == 0)
  {
    return m_dDefault;
  }

  return R_sum / pt;
}

//______________________________________________________________________________;
float DiTauIDVarCalculator::R_isotrack(const xAOD::DiTauJet& xDiTau) const
{
  double R_sum = 0;
  double pt = 0;

  static const SG::ConstAccessor<TrackParticleLinks_t> isoTrackLinksAcc("isoTrackLinks");
  if (!isoTrackLinksAcc.isAvailable(xDiTau) )
  {
    ATH_MSG_WARNING("Link not available");
  }
  
  static const SG::ConstAccessor<int> n_subjetsAcc("n_subjets");
  if (n_subjetsAcc(xDiTau) < 2) {
    return m_dDefault;
  }
 
  for (int i = 0; i<=1; i++)
  {
  
    TrackParticleLinks_t xIsoTracks = xDiTau.isoTrackLinks();

    TLorentzVector tlvSubjet;
    tlvSubjet.SetPtEtaPhiE( xDiTau.subjetPt(i),
                            xDiTau.subjetEta(i),
                            xDiTau.subjetPhi(i),
                            xDiTau.subjetE(i) );
      
    TLorentzVector tlvIsoTrack;

    for (const auto& xIsoTrack: xIsoTracks) 
    { 
      tlvIsoTrack.SetPtEtaPhiE( (*xIsoTrack)->pt(),
                                (*xIsoTrack)->eta(),
                                (*xIsoTrack)->phi(),
                                (*xIsoTrack)->e() );

      if (tlvSubjet.DeltaR(tlvIsoTrack) < 0.4)
      {
        R_sum += tlvSubjet.DeltaR(tlvIsoTrack)*tlvIsoTrack.Pt();
        pt += tlvIsoTrack.Pt();
      }
    }
  }

  if (pt == 0)
  {
    return m_dDefault;
  }

  return R_sum / pt;
}

//______________________________________________________________________________;
float DiTauIDVarCalculator::mass_track_core(const xAOD::DiTauJet& xDiTau) const
{

  static const SG::ConstAccessor<TrackParticleLinks_t> trackLinksAcc("trackLinks");
  if (!trackLinksAcc.isAvailable(xDiTau) )
  {
    ATH_MSG_WARNING("Link not available");
  } 
    
  static const SG::ConstAccessor<int> n_subjetsAcc("n_subjets");
  if (n_subjetsAcc(xDiTau) < 2) {
    return m_dDefault;
  }

  TLorentzVector tlvallTracks;

  static const SG::ConstAccessor<float> R_coreAcc("R_core");
  for (int i = 0; i<=1; i++)
  {

    TrackParticleLinks_t xTracks = xDiTau.trackLinks();

    TLorentzVector tlvSubjet;
    tlvSubjet.SetPtEtaPhiE( xDiTau.subjetPt(i),
                            xDiTau.subjetEta(i),
                            xDiTau.subjetPhi(i),
                            xDiTau.subjetE(i) );
    
    TLorentzVector tlvTrack;

    for (const auto& xTrack: xTracks) 
    { 
      tlvTrack.SetPtEtaPhiE( (*xTrack)->pt(),
                             (*xTrack)->eta(),
                             (*xTrack)->phi(),
                             (*xTrack)->e() );
      if ( tlvSubjet.DeltaR(tlvTrack) < R_coreAcc(xDiTau) )
      {
        //ATH_MSG_DEBUG("smaller");
        tlvallTracks += tlvTrack;
      }
    }
  }
  if (tlvallTracks.M() < 0)
  {
    return m_dDefault;
  }

  return tlvallTracks.M();
}

//______________________________________________________________________________;
float DiTauIDVarCalculator::mass_core(const xAOD::DiTauJet& xDiTau, int iSubjet) const
{

  static const SG::ConstAccessor<TrackParticleLinks_t> trackLinksAcc("trackLinks");
  if (!trackLinksAcc.isAvailable(xDiTau) )
  {
    ATH_MSG_WARNING("Link not available");
  } 
    
  static const SG::ConstAccessor<int> n_subjetsAcc("n_subjets");
  if ( iSubjet < 0 || iSubjet >= n_subjetsAcc(xDiTau)) {
    return m_dDefault;
  }

  TLorentzVector tlvallTracks;


  TrackParticleLinks_t xTracks = xDiTau.trackLinks();

  TLorentzVector tlvSubjet;
  tlvSubjet.SetPtEtaPhiE( xDiTau.subjetPt(iSubjet),
                          xDiTau.subjetEta(iSubjet),
                          xDiTau.subjetPhi(iSubjet),
                          xDiTau.subjetE(iSubjet) );
  
  TLorentzVector tlvTrack;

  static const SG::ConstAccessor<float> R_coreAcc("R_core");
  for (const auto& xTrack: xTracks) 
  { 
    tlvTrack.SetPtEtaPhiE( (*xTrack)->pt(),
                           (*xTrack)->eta(),
                           (*xTrack)->phi(),
                           (*xTrack)->e() );
    if ( tlvSubjet.DeltaR(tlvTrack) < R_coreAcc(xDiTau) )
    {
      //ATH_MSG_DEBUG("smaller");
      tlvallTracks += tlvTrack;
    }
  }
  
  if (tlvallTracks.M() < 0)
  {
    return m_dDefault;
  }

  return tlvallTracks.M();
}

//______________________________________________________________________________;
float DiTauIDVarCalculator::mass_tracks(const xAOD::DiTauJet& xDiTau, int iSubjet) const
{

  static const SG::ConstAccessor<TrackParticleLinks_t> trackLinksAcc("trackLinks");
  if (!trackLinksAcc.isAvailable(xDiTau) )
  {
    ATH_MSG_WARNING("Link not available");
  } 
    
  static const SG::ConstAccessor<int> n_subjetsAcc("n_subjets");
  if ( iSubjet < 0 || iSubjet >= n_subjetsAcc(xDiTau)) {
    return m_dDefault;
  }

  TLorentzVector tlvallTracks;

  TrackParticleLinks_t xTracks = xDiTau.trackLinks();

  TLorentzVector tlvSubjet;
  tlvSubjet.SetPtEtaPhiE( xDiTau.subjetPt(iSubjet),
                          xDiTau.subjetEta(iSubjet),
                          xDiTau.subjetPhi(iSubjet),
                          xDiTau.subjetE(iSubjet) );
  
  TLorentzVector tlvTrack;

  for (const auto& xTrack: xTracks) 
  { 
    tlvTrack.SetPtEtaPhiE( (*xTrack)->pt(),
                           (*xTrack)->eta(),
                           (*xTrack)->phi(),
                           (*xTrack)->e() );
    if ( tlvSubjet.DeltaR(tlvTrack) < 0.2 )
    {
      tlvallTracks += tlvTrack;
    }
  }
  
  if (tlvallTracks.M() < 0)
  {
    return m_dDefault;
  }

  return tlvallTracks.M();
}
//______________________________________________________________________________;
float DiTauIDVarCalculator::mass_track(const xAOD::DiTauJet& xDiTau) const
{

  static const SG::ConstAccessor<TrackParticleLinks_t> trackLinksAcc("trackLinks");
  if (!trackLinksAcc.isAvailable(xDiTau) )
  {
    ATH_MSG_WARNING("Link not available");
  } 

  TLorentzVector tlvallTracks;

  TrackParticleLinks_t xTracks = xDiTau.trackLinks();
    
  TLorentzVector tlvTrack;

  for (const auto& xTrack: xTracks) 
  { 
    tlvTrack.SetPtEtaPhiE( (*xTrack)->pt(),
                           (*xTrack)->eta(),
                           (*xTrack)->phi(),
                           (*xTrack)->e() );

    tlvallTracks += tlvTrack;
  }
  
  if (tlvallTracks.M() < 0)
  {
    return m_dDefault;
  }
  return tlvallTracks.M();
}
//______________________________________________________________________________;
float DiTauIDVarCalculator::mass_track_all(const xAOD::DiTauJet& xDiTau) const
{

  static const SG::ConstAccessor<TrackParticleLinks_t> trackLinksAcc("trackLinks");
  if (!trackLinksAcc.isAvailable(xDiTau) )
  {
    ATH_MSG_WARNING("Link not available");
  } 

  TLorentzVector tlvallTracks;

  TrackParticleLinks_t xTracks = xDiTau.trackLinks();
    
  TLorentzVector tlvTrack;

  for (const auto& xTrack: xTracks) 
  { 
    tlvTrack.SetPtEtaPhiE( (*xTrack)->pt(),
                           (*xTrack)->eta(),
                           (*xTrack)->phi(),
                           (*xTrack)->e() );

    tlvallTracks += tlvTrack;
  }

  
  TrackParticleLinks_t xIsoTracks = xDiTau.isoTrackLinks();

  TLorentzVector tlvIsoTrack;

  for (const auto& xIsoTrack: xIsoTracks) 
  { 
    tlvIsoTrack.SetPtEtaPhiE( (*xIsoTrack)->pt(),
                             (*xIsoTrack)->eta(),
                             (*xIsoTrack)->phi(),
                             (*xIsoTrack)->e() );

    tlvallTracks += tlvIsoTrack;
  }

  if (tlvallTracks.M() < 0)
  {
    return m_dDefault;
  }

  return tlvallTracks.M();
}

//______________________________________________________________________________;
float DiTauIDVarCalculator::E_frac(const xAOD::DiTauJet& xDiTau, int iSubjet) const
{ 
  static const SG::ConstAccessor<int> n_subjetsAcc("n_subjets");
  if ( iSubjet < 0 || iSubjet >= n_subjetsAcc(xDiTau)) {
    return m_dDefault;
  }

  return xDiTau.subjetE(iSubjet) / xDiTau.subjetE(0);
}

//______________________________________________________________________________;
float DiTauIDVarCalculator::R_subjets(const xAOD::DiTauJet& xDiTau, int iSubjet) const
{

  static const SG::ConstAccessor<int> n_subjetsAcc("n_subjets");
  if ( iSubjet < 0 || iSubjet >= n_subjetsAcc(xDiTau)) {
    return m_dDefault;
  }
  
  static const SG::ConstAccessor<TrackParticleLinks_t> trackLinksAcc("trackLinks");
  if (!trackLinksAcc.isAvailable(xDiTau) )
  {
    ATH_MSG_WARNING("Track links not available");
  }
  
  TLorentzVector tlvLeadSubjet;
  tlvLeadSubjet.SetPtEtaPhiE( xDiTau.subjetPt(0),
                              xDiTau.subjetEta(0),
                              xDiTau.subjetPhi(0),
                              xDiTau.subjetE(0) );

  TLorentzVector tlvSubjet;
  tlvSubjet.SetPtEtaPhiE( xDiTau.subjetPt(iSubjet),
                          xDiTau.subjetEta(iSubjet),
                          xDiTau.subjetPhi(iSubjet),
                          xDiTau.subjetE(iSubjet) );
  return tlvLeadSubjet.DeltaR(tlvSubjet);
}

//______________________________________________________________________________;
float DiTauIDVarCalculator::d0_leadtrack(const xAOD::DiTauJet& xDiTau, int iSubjet) const
{
  double pt_leadtrk = 0;
  double d0 = m_dDefault;
  static const SG::ConstAccessor<TrackParticleLinks_t> trackLinksAcc("trackLinks");
  if (!trackLinksAcc.isAvailable(xDiTau) )
  {
    ATH_MSG_WARNING("Track links not available");
  } 

  static const SG::ConstAccessor<int> n_subjetsAcc("n_subjets");
  if ( iSubjet < 0 || iSubjet >= n_subjetsAcc(xDiTau)) {
    return m_dDefault;
  }

  TLorentzVector tlvSubjet;
  tlvSubjet.SetPtEtaPhiE( xDiTau.subjetPt(iSubjet),
                          xDiTau.subjetEta(iSubjet),
                          xDiTau.subjetPhi(iSubjet),
                          xDiTau.subjetE(iSubjet) );
			  
  TrackParticleLinks_t xTracks = xDiTau.trackLinks();
    
  TLorentzVector tlvTrack;

  static const SG::ConstAccessor<float> R_coreAcc("R_core");
  for (auto &xTrack: xTracks) 
  { 
    tlvTrack.SetPtEtaPhiE( (*xTrack)->pt(),
                           (*xTrack)->eta(),
                           (*xTrack)->phi(),
                           (*xTrack)->e() );

    if (tlvTrack.DeltaR(tlvSubjet) < R_coreAcc(xDiTau)) 
    {
      if (tlvTrack.Pt() > pt_leadtrk)
      {
        pt_leadtrk = tlvTrack.Pt();
        d0 = (*xTrack)->d0();
      }
    }
  }
  return d0;
}

//______________________________________________________________________________;
float DiTauIDVarCalculator::f_isotracks(const xAOD::DiTauJet& xDiTau) const
{
  double iso_pt = 0;
  static const SG::ConstAccessor<TrackParticleLinks_t> isoTrackLinksAcc("isoTrackLinks");
  if (!isoTrackLinksAcc.isAvailable(xDiTau) )
  {
    ATH_MSG_WARNING("Track links not available");
  }
  
  TrackParticleLinks_t xIsoTracks = xDiTau.isoTrackLinks();

  TLorentzVector tlvIsoTrack;

  for (const auto& xIsoTrack: xIsoTracks) 
  { 
    tlvIsoTrack.SetPtEtaPhiE( (*xIsoTrack)->pt(),
                              (*xIsoTrack)->eta(),
                              (*xIsoTrack)->phi(),
                              (*xIsoTrack)->e() );

    iso_pt += tlvIsoTrack.Pt();
  }

  return iso_pt / xDiTau.pt();
}

//______________________________________________________________________________;
StatusCode DiTauIDVarCalculator::decorNtracks (const xAOD::DiTauJet& xDiTau)
{
  static const SG::ConstAccessor<TrackParticleLinks_t> trackLinksAcc("trackLinks");
  if (!trackLinksAcc.isAvailable(xDiTau) )
  {
    Warning("decorNtracks()", "Track links not available.");
    return StatusCode::FAILURE;
  } 

  static const SG::ConstAccessor<int> n_subjetsAcc("n_subjets");
  int nSubjets = n_subjetsAcc(xDiTau);

  static const SG::ConstAccessor<float> R_subjetAcc("R_subjet");
  float Rsubjet = R_subjetAcc(xDiTau);
  std::vector<int> nTracks(nSubjets, 0);

  TrackParticleLinks_t xTracks = xDiTau.trackLinks();
  for (const auto &xTrack: xTracks)
  {
    double dRmin = 1111;
    double itrmin = -1;

    for (int i=0; i<nSubjets; ++i)
    {
      TLorentzVector tlvSubjet = TLorentzVector();
      tlvSubjet.SetPtEtaPhiE(xDiTau.subjetPt(i),
                             xDiTau.subjetEta(i),
                             xDiTau.subjetPhi(i),
                             xDiTau.subjetE(i));
      double dR = tlvSubjet.DeltaR((*xTrack)->p4());


      if ((dR < Rsubjet) && (dR < dRmin))
      {
        dRmin = dR;
        itrmin = i;
      }
    } // loop over subjets
    if (itrmin > -1) nTracks[itrmin]++;
  } // loop over tracks

  static const SG::Decorator< std::vector<int> > n_tracksDec("n_tracks");
  n_tracksDec(xDiTau) = nTracks;

  return StatusCode::SUCCESS;
}
