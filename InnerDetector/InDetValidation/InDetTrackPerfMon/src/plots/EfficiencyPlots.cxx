/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file    EfficiencyPlots.cxx
 * @author  Marco Aparo <marco.aparo@cern.ch>
 **/

/// local include(s)
#include "EfficiencyPlots.h"
#include "../TrackParametersHelper.h"


/// -----------------------
/// ----- Constructor -----
/// -----------------------
IDTPM::EfficiencyPlots::EfficiencyPlots(
    PlotMgr* pParent, const std::string& dirName, 
    const std::string& anaTag, const std::string& trackType ) :
        PlotMgr( dirName, anaTag, pParent ), 
        m_trackType( trackType ) { }


/// ---------------------------
/// --- Book the histograms ---
/// ---------------------------
void IDTPM::EfficiencyPlots::initializePlots()
{
  StatusCode sc = bookPlots();
  if( sc.isFailure() ) {
    ATH_MSG_ERROR( "Failed to book efficiency plots" );
  }
}


StatusCode IDTPM::EfficiencyPlots::bookPlots()
{
  ATH_MSG_DEBUG( "Booking efficiency plots in " << getDirectory() ); 

  ATH_CHECK( retrieveAndBook( m_eff_vs_pt,  "eff_vs_"+m_trackType+"_pt" ) );
  ATH_CHECK( retrieveAndBook( m_eff_vs_eta, "eff_vs_"+m_trackType+"_eta" ) );
  ATH_CHECK( retrieveAndBook( m_eff_vs_phi, "eff_vs_"+m_trackType+"_phi" ) );
  ATH_CHECK( retrieveAndBook( m_eff_vs_d0, "eff_vs_"+m_trackType+"_d0" ) );
  ATH_CHECK( retrieveAndBook( m_eff_vs_z0, "eff_vs_"+m_trackType+"_z0" ) );
  ATH_CHECK( retrieveAndBook( m_eff_vs_prodR, "eff_vs_"+m_trackType+"_prodR" ) );
  ATH_CHECK( retrieveAndBook( m_eff_vs_prodZ, "eff_vs_"+m_trackType+"_prodZ" ) );
  ATH_CHECK( retrieveAndBook( m_eff_vs_R, "eff_vs_"+m_trackType+"_R" ) );
  ATH_CHECK( retrieveAndBook( m_eff_vs_Z, "eff_vs_"+m_trackType+"_Z" ) );
  return StatusCode::SUCCESS;
}


/// -----------------------------
/// --- Dedicated fill method ---
/// -----------------------------
template< typename PARTICLE >
StatusCode IDTPM::EfficiencyPlots::fillPlots(
    const PARTICLE& particle, bool isMatched, float weight )
{
  /// Compute track parameters - TODO: add more...
  float ppt    = pT( particle ) / Gaudi::Units::GeV;
  float peta   = eta( particle );
  float pphi   = phi( particle );
  float pd0    = d0( particle );
  float pz0    = z0( particle );
  float pR     = R( particle );
  float pZ     = Z( particle );
  float pprodR = prodR( particle );
  float pprodZ = prodZ( particle );

  /// Fill the histograms
  ATH_CHECK( fill( m_eff_vs_pt,  ppt,  isMatched, weight ) );
  ATH_CHECK( fill( m_eff_vs_eta, peta, isMatched, weight ) );
  ATH_CHECK( fill( m_eff_vs_phi, pphi, isMatched, weight ) );
  ATH_CHECK( fill( m_eff_vs_d0, pd0, isMatched, weight ) );
  ATH_CHECK( fill( m_eff_vs_z0, pz0, isMatched, weight ) );
  ATH_CHECK( fill( m_eff_vs_R, pR, isMatched, weight ) );
  ATH_CHECK( fill( m_eff_vs_Z, pZ, isMatched, weight ) );
  ATH_CHECK( fill( m_eff_vs_prodR, pprodR, isMatched, weight ) );
  ATH_CHECK( fill( m_eff_vs_prodZ, pprodZ, isMatched, weight ) );

  return StatusCode::SUCCESS;
}

template StatusCode IDTPM::EfficiencyPlots::fillPlots< xAOD::TrackParticle >(
    const xAOD::TrackParticle&, bool isMatched, float weight );

template StatusCode IDTPM::EfficiencyPlots::fillPlots< xAOD::TruthParticle >(
    const xAOD::TruthParticle&, bool isMatched, float weight );


/// -------------------------
/// ----- finalizePlots -----
/// -------------------------
void IDTPM::EfficiencyPlots::finalizePlots()
{
  ATH_MSG_DEBUG( "Finalising efficiency plots" );
  /// print stat here if needed
}
