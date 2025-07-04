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
#include "../OfflineObjectDecorHelper.h"


/// -----------------------
/// ----- Constructor -----
/// -----------------------
IDTPM::TrackParametersPlots::TrackParametersPlots(
    PlotMgr* pParent, const std::string& dirName, 
    const std::string& anaTag, const std::string& trackType,
    bool plotErrors, bool recomputeIP ) :
        PlotMgr( dirName, anaTag, pParent ), 
        m_trackType( trackType ),
        m_plotErrors( plotErrors ), m_recomputeIP( recomputeIP ) { }


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

  if( m_plotErrors ) {
    /// sigma plots - defined in ResPlotsDef.json
    ATH_CHECK( retrieveAndBook( m_sigma_pt, "sigma_"+m_trackType+"_pt" ) );
    ATH_CHECK( retrieveAndBook( m_sigma_eta, "sigma_"+m_trackType+"_eta" ) );
    ATH_CHECK( retrieveAndBook( m_sigma_phi, "sigma_"+m_trackType+"_phi" ) );
    ATH_CHECK( retrieveAndBook( m_sigma_d0, "sigma_"+m_trackType+"_d0" ) );
    ATH_CHECK( retrieveAndBook( m_sigma_z0, "sigma_"+m_trackType+"_z0" ) );
    ATH_CHECK( retrieveAndBook( m_sigma_z0sin, "sigma_"+m_trackType+"_z0sin" ) );
    ATH_CHECK( retrieveAndBook( m_sigma_theta, "sigma_"+m_trackType+"_theta" ) );

    /// significance plots - defined in ResPlotsDef.json
    ATH_CHECK( retrieveAndBook( m_significance_pt, "significance_"+m_trackType+"_pt" ) );
    ATH_CHECK( retrieveAndBook( m_significance_eta, "significance_"+m_trackType+"_eta" ) );
    ATH_CHECK( retrieveAndBook( m_significance_phi, "significance_"+m_trackType+"_phi" ) );
    ATH_CHECK( retrieveAndBook( m_significance_d0, "significance_"+m_trackType+"_d0" ) );
    ATH_CHECK( retrieveAndBook( m_significance_z0, "significance_"+m_trackType+"_z0" ) );
    ATH_CHECK( retrieveAndBook( m_significance_z0sin, "significance_"+m_trackType+"_z0sin" ) );
    ATH_CHECK( retrieveAndBook( m_significance_theta, "significance_"+m_trackType+"_theta" ) );
  }

  return StatusCode::SUCCESS;
}


/// -----------------------------
/// --- Dedicated fill method ---
/// -----------------------------
template< typename PARTICLE >
StatusCode IDTPM::TrackParametersPlots::fillPlots(
    const PARTICLE& particle, float weight )
{
  /// Compute track parameters
  float ppt    = pT( particle ) / Gaudi::Units::GeV;
  float peta   = eta( particle );
  float pphi   = phi( particle );
  float pd0    = d0( particle );
  if( m_recomputeIP ) pd0 = getD0TrackInJet( particle ); 
  float pz0    = z0( particle );
  float pz0sin = z0SinTheta( particle );
  float ptheta = theta( particle );
  float pqoverp = qOverP( particle ); // in MeV^-1
  float pprodR = prodR( particle );
  float pprodZ = prodZ( particle );
  float pnSiHits = nSiHits( particle );
  float pchi2 = chiSquared( particle );
  float pndof = ndof( particle );
  float pchi2OverNdof = ( pndof > 0 ) ? pchi2 / pndof : 0.;
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

  if( m_plotErrors ) {
    float ppt_err = pTError( particle ) / Gaudi::Units::GeV;
    float peta_err = etaError( particle );
    float pphi_err = error( particle, Trk::phi );
    float pd0_err = error( particle, Trk::d0 );
    float pz0_err = error( particle, Trk::z0 );
    float pz0sin_err = z0SinThetaError( particle );
    float ptheta_err = error( particle, Trk::theta );

    ATH_CHECK( fill( m_sigma_pt,    ppt_err,    weight ) );
    ATH_CHECK( fill( m_sigma_eta,   peta_err,   weight ) );
    ATH_CHECK( fill( m_sigma_phi,   pphi_err,   weight ) );
    ATH_CHECK( fill( m_sigma_d0,    pd0_err,    weight ) );
    ATH_CHECK( fill( m_sigma_z0,    pz0_err,    weight ) );
    ATH_CHECK( fill( m_sigma_z0sin, pz0sin_err, weight ) );
    ATH_CHECK( fill( m_sigma_theta, ptheta_err, weight ) );

    float ppt_sig = ppt_err > 0. ? ppt / ppt_err : -9999.;
    float peta_sig = peta_err > 0. ? peta / peta_err : -9999.;
    float pphi_sig = pphi_err > 0. ? pphi / pphi_err : -9999.;
    float pd0_sig = pd0_err > 0. ? pd0 / pd0_err : -9999.;
    float pz0_sig = pz0_err > 0. ? pz0 / pz0_err : -9999.;
    float pz0sin_sig = pz0sin_err > 0. ? pz0sin / pz0sin_err  : -9999.;
    float ptheta_sig = ptheta_err > 0. ? ptheta / ptheta_err : -9999.;

    ATH_CHECK( fill( m_significance_pt,     ppt_sig,    weight ) );
    ATH_CHECK( fill( m_significance_eta,    peta_sig,   weight ) );
    ATH_CHECK( fill( m_significance_phi,    pphi_sig,   weight ) );
    ATH_CHECK( fill( m_significance_d0,     pd0_sig,    weight ) );
    ATH_CHECK( fill( m_significance_z0,     pz0_sig,    weight ) );
    ATH_CHECK( fill( m_significance_z0sin,  pz0sin_sig, weight ) );
    ATH_CHECK( fill( m_significance_theta,  ptheta_sig, weight ) );
  }

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
