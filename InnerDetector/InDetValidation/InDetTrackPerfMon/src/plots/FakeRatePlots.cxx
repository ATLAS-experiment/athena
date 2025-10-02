/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file    FakeRatePlots.cxx
 * @author  Marco Aparo <marco.aparo@cern.ch>
 **/

/// local include(s)
#include "FakeRatePlots.h"
#include "../TrackParametersHelper.h"


/// -----------------------
/// ----- Constructor -----
/// -----------------------
IDTPM::FakeRatePlots::FakeRatePlots(
    PlotMgr* pParent, const std::string& dirName, 
    const std::string& anaTag, const std::string& trackType,
    bool doGlobalPlots, bool doTruthMuPlots ) :
        PlotMgr( dirName, anaTag, pParent ), 
        m_trackType( trackType ),
        m_doGlobalPlots( doGlobalPlots ),
        m_doTruthMuPlots( doTruthMuPlots ) { }


/// ---------------------------
/// --- Book the histograms ---
/// ---------------------------
void IDTPM::FakeRatePlots::initializePlots()
{
  StatusCode sc = bookPlots();
  if( sc.isFailure() ) {
    ATH_MSG_ERROR( "Failed to book fake rate plots" );
  }
}


StatusCode IDTPM::FakeRatePlots::bookPlots()
{
  ATH_MSG_DEBUG( "Booking efficiency plots in " << getDirectory() ); 

  ATH_CHECK( retrieveAndBook( m_fakerate_vs_incl, "fakerate_vs_"+m_trackType+"_inclusive" ) );
  ATH_CHECK( retrieveAndBook( m_fakerate_vs_pt,   "fakerate_vs_"+m_trackType+"_pt" ) );
  ATH_CHECK( retrieveAndBook( m_fakerate_vs_logPt,"fakerate_vs_"+m_trackType+"_logPt" ) );
  ATH_CHECK( retrieveAndBook( m_fakerate_vs_eta,  "fakerate_vs_"+m_trackType+"_eta" ) );
  ATH_CHECK( retrieveAndBook( m_fakerate_vs_phi,  "fakerate_vs_"+m_trackType+"_phi" ) );
  ATH_CHECK( retrieveAndBook( m_fakerate_vs_d0,   "fakerate_vs_"+m_trackType+"_d0" ) );
  ATH_CHECK( retrieveAndBook( m_fakerate_vs_z0,   "fakerate_vs_"+m_trackType+"_z0" ) );
  if( m_doGlobalPlots ) {
    ATH_CHECK( retrieveAndBook( m_fakerate_vs_actualMu, "fakerate_vs_actualMu" ) );
    if( m_doTruthMuPlots ) ATH_CHECK( retrieveAndBook( m_fakerate_vs_truthMu, "fakerate_vs_truthMu" ) );
  }
  return StatusCode::SUCCESS;
}


/// -----------------------------
/// --- Dedicated fill method ---
/// -----------------------------
template< typename PARTICLE >
StatusCode IDTPM::FakeRatePlots::fillPlots(
    const PARTICLE& particle, bool isFake, float truthMu, float actualMu, float weight )
{
  /// Compute track parameters
  float ppt    = pT( particle ) / Gaudi::Units::GeV;
  float peta   = eta( particle );
  float pphi   = phi( particle );
  float pd0    = d0( particle );
  float pz0    = z0( particle );

  /// Fill the histograms
  ATH_CHECK( fill( m_fakerate_vs_incl,  1,  isFake, weight ) );
  ATH_CHECK( fill( m_fakerate_vs_pt,  ppt,  isFake, weight ) );
  ATH_CHECK( fill( m_fakerate_vs_logPt,  ppt,  isFake, weight ) );
  ATH_CHECK( fill( m_fakerate_vs_eta, peta, isFake, weight ) );
  ATH_CHECK( fill( m_fakerate_vs_phi, pphi, isFake, weight ) );
  ATH_CHECK( fill( m_fakerate_vs_d0,  pd0,  isFake, weight ) );
  ATH_CHECK( fill( m_fakerate_vs_z0,  pz0,  isFake, weight ) );

  if( m_doGlobalPlots ) {
    ATH_CHECK( fill( m_fakerate_vs_actualMu, actualMu, isFake, weight ) );
    if( m_doTruthMuPlots ) ATH_CHECK( fill( m_fakerate_vs_truthMu, truthMu, isFake, weight ) );
  }

  return StatusCode::SUCCESS;
}

template StatusCode IDTPM::FakeRatePlots::fillPlots< xAOD::TrackParticle >(
    const xAOD::TrackParticle&, bool isFake, float truthMu, float actualMu, float weight );

template StatusCode IDTPM::FakeRatePlots::fillPlots< xAOD::TruthParticle >(
    const xAOD::TruthParticle&, bool isFake, float truthMu, float actualMu, float weight );


/// -------------------------
/// ----- finalizePlots -----
/// -------------------------
void IDTPM::FakeRatePlots::finalizePlots()
{
  ATH_MSG_DEBUG( "Finalising fake rate plots" );
  /// print stat here if needed
}
