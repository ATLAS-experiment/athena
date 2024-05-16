/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file    TrackParametersPlots.cxx
 * @author  Marco Aparo <marco.aparo@cern.ch> 
 **/

/// local include(s)
#include "TrackParametersPlots.h"
#include "../TrackParmetersHelper.h"


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

  /// Fill the histograms
  ATH_CHECK( fill( m_pt,  ppt,   weight ) );
  ATH_CHECK( fill( m_eta, peta,  weight ) );

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
