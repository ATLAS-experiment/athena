/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file    TrackParametersPlots.cxx
 * @author  Marco Aparo <marco.aparo@cern.ch> 
 **/

/// local include(s)
#include "TrackParametersPlots.h"
#include "../TrackParametersHelper.h"


/// -----------------------
/// ----- Constructor -----
/// -----------------------
IDTPM::TrackParametersPlots::TrackParametersPlots(
    PlotMgr* pParent, const std::string& dirName, 
    const std::string& anaTag, const std::string& trackType ) :
        PlotMgr( dirName, anaTag, pParent ), 
        m_trackType( trackType ) { }


/// ---------------------------
/// --- Book the histograms ---
/// ---------------------------
void IDTPM::TrackParametersPlots::initializePlots()
{
  StatusCode sc = bookPlots();
  if( sc.isFailure() ) {
    ATH_MSG_ERROR( "Failed to book track parameters plots" );
  }
}


StatusCode IDTPM::TrackParametersPlots::bookPlots()
{
  ATH_MSG_DEBUG( "Booking track parameters plots in " << getDirectory() ); 

  ATH_CHECK( retrieveAndBook( m_pt,   m_trackType+"_pt" ) );
  ATH_CHECK( retrieveAndBook( m_eta,  m_trackType+"_eta" ) );
  ATH_CHECK( retrieveAndBook( m_phi,  m_trackType+"_phi" ) );
  ATH_CHECK( retrieveAndBook( m_d0,  m_trackType+"_d0" ) );
  ATH_CHECK( retrieveAndBook( m_z0,  m_trackType+"_z0" ) );
  ATH_CHECK( retrieveAndBook( m_z0sin,  m_trackType+"_z0sin" ) );
  ATH_CHECK( retrieveAndBook( m_theta,  m_trackType+"_theta" ) );
  ATH_CHECK( retrieveAndBook( m_qoverp,  m_trackType+"_qoverp" ) );
  if( m_trackType == "truth" ) {
    ATH_CHECK( retrieveAndBook( m_prodR, m_trackType+"_prodR" ) );
    ATH_CHECK( retrieveAndBook( m_prodZ, m_trackType+"_prodZ" ) );
    ATH_CHECK( retrieveAndBook( m_nSiHits, m_trackType+"_nSiHits" ) );
    ATH_CHECK( retrieveAndBook( m_nSiHits_vs_eta, m_trackType+"_nSiHits_vs_eta" ) );
  } else {
    ATH_CHECK( retrieveAndBook( m_chi2, m_trackType+"_chi2" ) );
    ATH_CHECK( retrieveAndBook( m_ndof, m_trackType+"_ndof" ) );
    ATH_CHECK( retrieveAndBook( m_chi2OverNdof, m_trackType+"_chi2OverNdof" ) );
    ATH_CHECK( retrieveAndBook( m_author, m_trackType+"_author" ) );
    ATH_CHECK( retrieveAndBook( m_time, m_trackType+"_time" ) );
    ATH_CHECK( retrieveAndBook( m_hasValidTime_eff_vs_eta, m_trackType+"_hasValidTime_eff_vs_eta" ) );
  }
  ATH_CHECK( retrieveAndBook( m_eta_vs_pt, m_trackType+"_eta_vs_pt" ) );
  ATH_CHECK( retrieveAndBook( m_eta_vs_phi, m_trackType+"_eta_vs_phi" ) );
  ATH_CHECK( retrieveAndBook( m_z0_vs_d0, m_trackType+"_z0_vs_d0" ) );
  ATH_CHECK( retrieveAndBook( m_z0sin_vs_d0, m_trackType+"_z0sin_vs_d0" ) );

  return StatusCode::SUCCESS;
}


/// -----------------------------
/// --- Dedicated fill method ---
/// -----------------------------
template< typename PARTICLE >
StatusCode IDTPM::TrackParametersPlots::fillPlots(
    const PARTICLE& particle, float weight )
{
  /// Compute track parameters - TODO: add more...
  float ppt    = pT( particle ) / Gaudi::Units::GeV;
  float peta   = eta( particle );
  float pphi   = phi( particle );
  float pd0    = d0( particle );
  float pz0    = z0( particle );
  float pz0sin = z0SinTheta( particle );
  float ptheta = theta( particle );
  float pqoverp = qOverP( particle ); // in MeV^-1
  float pprodR = prodR( particle );
  float pprodZ = prodZ( particle );
  float pnSiHits = nSiHits( particle );
  float pchi2 = chiSquared( particle );
  float pndof = ndof( particle );
  float pchi2OverNdof = pchi2 / pndof;
  std::vector< unsigned int > pauthor = author( particle );

  /// Fill the histograms
  ATH_CHECK( fill( m_pt,  ppt,   weight ) );
  ATH_CHECK( fill( m_eta, peta,  weight ) );
  ATH_CHECK( fill( m_phi, pphi,  weight ) );
  ATH_CHECK( fill( m_d0, pd0, weight ) );
  ATH_CHECK( fill( m_z0, pz0, weight ) );
  ATH_CHECK( fill( m_z0sin, pz0sin, weight ) );
  ATH_CHECK( fill( m_theta, ptheta, weight ) );
  ATH_CHECK( fill( m_qoverp, pqoverp, weight ) );
  if( m_trackType == "truth" ) {
    ATH_CHECK( fill( m_prodR, pprodR, weight ) );
    ATH_CHECK( fill( m_prodZ, pprodZ, weight ) );
    ATH_CHECK( fill( m_nSiHits, pnSiHits, weight ) );
    ATH_CHECK( fill( m_nSiHits_vs_eta, peta, pnSiHits, weight ) );
  } else {
    ATH_CHECK( fill( m_chi2, pchi2, weight ) );
    ATH_CHECK( fill( m_ndof, pndof, weight ) );
    ATH_CHECK( fill( m_chi2OverNdof, pchi2OverNdof, weight ) );
    for( unsigned int i=0; i<pauthor.size(); i++ ) {
      ATH_CHECK( fill( m_author, pauthor[i], weight ) );
    }
    static const SG::Accessor< uint8_t > accValidTime("hasValidTime");
    static const SG::Accessor< float > accTime("time");
    if( accValidTime.isAvailable( particle ) ) {
      bool phasValidTime = hasValidTime( particle );
      ATH_CHECK( fill( m_hasValidTime_eff_vs_eta, peta, phasValidTime, weight ) );
      if( accTime.isAvailable( particle ) and phasValidTime ) {
        ATH_CHECK( fill( m_time, time( particle ), weight ) );
      }
    }
  }
  ATH_CHECK( fill( m_eta_vs_pt, ppt, peta, weight ) );
  ATH_CHECK( fill( m_eta_vs_phi, pphi, peta, weight ) );
  ATH_CHECK( fill( m_z0_vs_d0, pd0, pz0, weight ) );
  ATH_CHECK( fill( m_z0sin_vs_d0, pz0*std::sin(ptheta), pd0, weight ) );

  return StatusCode::SUCCESS;
}

template StatusCode IDTPM::TrackParametersPlots::fillPlots< xAOD::TrackParticle >(
    const xAOD::TrackParticle&, float weight );

template StatusCode IDTPM::TrackParametersPlots::fillPlots< xAOD::TruthParticle >(
    const xAOD::TruthParticle&, float weight );


/// -------------------------
/// ----- finalizePlots -----
/// -------------------------
void IDTPM::TrackParametersPlots::finalizePlots()
{
  ATH_MSG_DEBUG( "Finalising track parameters plots" );
  /// print stat here if needed
}
