/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file    FakeRatePlots.cxx
 * @author  Marco Aparo <marco.aparo@cern.ch>
 **/

/// local include(s)
#include "InDetTrackPerfMon/DuplicateRatePlots.h"
#include "InDetTrackPerfMon/TrackParametersHelper.h"


/// -----------------------
/// ----- Constructor -----
/// -----------------------
IDTPM::DuplicateRatePlots::DuplicateRatePlots(
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
void IDTPM::DuplicateRatePlots::initializePlots()
{
  StatusCode sc = bookPlots();
  if( sc.isFailure() ) {
    ATH_MSG_ERROR( "Failed to book duplicate rate plots" );
  }
}


StatusCode IDTPM::DuplicateRatePlots::bookPlots()
{
  ATH_MSG_DEBUG( "Booking duplicate rate plots in " << getDirectory() ); 

  ATH_CHECK( retrieveAndBook( m_duplrate_vs_incl, "duplrate_vs_"+m_trackType+"_inclusive" ) );
  ATH_CHECK( retrieveAndBook( m_duplrate_vs_pt,   "duplrate_vs_"+m_trackType+"_pt" ) );
  ATH_CHECK( retrieveAndBook( m_duplrate_vs_logPt,"duplrate_vs_"+m_trackType+"_logPt" ) );
  ATH_CHECK( retrieveAndBook( m_duplrate_vs_eta,  "duplrate_vs_"+m_trackType+"_eta" ) );
  ATH_CHECK( retrieveAndBook( m_duplrate_vs_phi,  "duplrate_vs_"+m_trackType+"_phi" ) );
  ATH_CHECK( retrieveAndBook( m_duplrate_vs_d0,   "duplrate_vs_"+m_trackType+"_d0" ) );
  ATH_CHECK( retrieveAndBook( m_duplrate_vs_z0,   "duplrate_vs_"+m_trackType+"_z0" ) );

  ATH_CHECK( retrieveAndBook( m_duplnum_vs_pt,   "duplnum_vs_"+m_trackType+"_pt" ) );
  ATH_CHECK( retrieveAndBook( m_duplnum_vs_eta,  "duplnum_vs_"+m_trackType+"_eta" ) );
  ATH_CHECK( retrieveAndBook( m_duplnum_vs_phi,  "duplnum_vs_"+m_trackType+"_phi" ) );
  ATH_CHECK( retrieveAndBook( m_duplnum_vs_d0,   "duplnum_vs_"+m_trackType+"_d0" ) );
  ATH_CHECK( retrieveAndBook( m_duplnum_vs_z0,   "duplnum_vs_"+m_trackType+"_z0" ) );

  ATH_CHECK( retrieveAndBook( m_duplnum_nonzero_vs_pt,   "duplnum_nonzero_vs_"+m_trackType+"_pt" ) );
  ATH_CHECK( retrieveAndBook( m_duplnum_nonzero_vs_eta,  "duplnum_nonzero_vs_"+m_trackType+"_eta" ) );
  ATH_CHECK( retrieveAndBook( m_duplnum_nonzero_vs_phi,  "duplnum_nonzero_vs_"+m_trackType+"_phi" ) );
  ATH_CHECK( retrieveAndBook( m_duplnum_nonzero_vs_d0,   "duplnum_nonzero_vs_"+m_trackType+"_d0" ) );
  ATH_CHECK( retrieveAndBook( m_duplnum_nonzero_vs_z0,   "duplnum_nonzero_vs_"+m_trackType+"_z0" ) );

  if( m_doGlobalPlots ) {
    ATH_CHECK( retrieveAndBook( m_duplrate_vs_actualMu,         "duplrate_vs_actualMu" ) );
    ATH_CHECK( retrieveAndBook( m_duplnum_vs_actualMu,          "duplnum_vs_actualMu" ) );
    ATH_CHECK( retrieveAndBook( m_duplnum_nonzero_vs_actualMu,  "duplnum_nonzero_vs_actualMu" ) );

    if( m_doTruthMuPlots ) {
      ATH_CHECK( retrieveAndBook( m_duplrate_vs_truthMu,          "duplrate_vs_truthMu" ) );
      ATH_CHECK( retrieveAndBook( m_duplnum_vs_truthMu,           "duplnum_vs_truthMu" ) );
      ATH_CHECK( retrieveAndBook( m_duplnum_nonzero_vs_truthMu,   "duplnum_nonzero_vs_truthMu" ) );
    }
  }

  return StatusCode::SUCCESS;
}


/// -----------------------------
/// --- Dedicated fill method ---
/// -----------------------------
template< typename PARTICLE >
StatusCode IDTPM::DuplicateRatePlots::fillPlots(
    const PARTICLE& particle,
    unsigned int nMatched,
    float truthMu,
    float actualMu,
    float weight )
{
  /// Compute track parameters
  float ppt   = pT( particle ) / Gaudi::Units::GeV;
  float peta  = eta( particle );
  float pphi  = phi( particle );
  float pd0   = d0( particle );
  float pz0   = z0( particle );

  /// Fill the histograms
  ATH_CHECK( fill( m_duplrate_vs_incl,  1,  (nMatched>1), weight ) );
  ATH_CHECK( fill( m_duplrate_vs_pt,  ppt,  (nMatched>1), weight ) );
  ATH_CHECK( fill( m_duplrate_vs_logPt,  ppt,  (nMatched>1), weight ) );
  ATH_CHECK( fill( m_duplrate_vs_eta, peta, (nMatched>1), weight ) );
  ATH_CHECK( fill( m_duplrate_vs_phi, pphi, (nMatched>1), weight ) );
  ATH_CHECK( fill( m_duplrate_vs_d0,  pd0,  (nMatched>1), weight ) );
  ATH_CHECK( fill( m_duplrate_vs_z0,  pz0,  (nMatched>1), weight ) );

  ATH_CHECK( fill( m_duplnum_vs_pt,  ppt,  nMatched, weight ) );
  ATH_CHECK( fill( m_duplnum_vs_eta, peta, nMatched, weight ) );
  ATH_CHECK( fill( m_duplnum_vs_phi, pphi, nMatched, weight ) );
  ATH_CHECK( fill( m_duplnum_vs_d0,  pd0,  nMatched, weight ) );
  ATH_CHECK( fill( m_duplnum_vs_z0,  pz0,  nMatched, weight ) );

  if( nMatched > 0 ) {
    ATH_CHECK( fill( m_duplnum_nonzero_vs_pt,  ppt,  nMatched, weight ) );
    ATH_CHECK( fill( m_duplnum_nonzero_vs_eta, peta, nMatched, weight ) );
    ATH_CHECK( fill( m_duplnum_nonzero_vs_phi, pphi, nMatched, weight ) );
    ATH_CHECK( fill( m_duplnum_nonzero_vs_d0,  pd0,  nMatched, weight ) );
    ATH_CHECK( fill( m_duplnum_nonzero_vs_z0,  pz0,  nMatched, weight ) );
  }

  if( m_doGlobalPlots ) {
    ATH_CHECK( fill( m_duplrate_vs_actualMu,  actualMu, (nMatched>1), weight ) );
    ATH_CHECK( fill( m_duplnum_vs_actualMu,   actualMu, nMatched, weight ) );
    if( nMatched > 0 ) ATH_CHECK( fill( m_duplnum_nonzero_vs_actualMu,  actualMu, nMatched, weight ) );

    if( m_doTruthMuPlots ) {
      ATH_CHECK( fill( m_duplrate_vs_truthMu,   truthMu,  (nMatched>1), weight ) );
      ATH_CHECK( fill( m_duplnum_vs_truthMu,    truthMu,  nMatched, weight ) );
      if( nMatched > 0 ) ATH_CHECK( fill( m_duplnum_nonzero_vs_truthMu,   truthMu,  nMatched, weight ) );
    }
  }

  return StatusCode::SUCCESS;
}

template StatusCode IDTPM::DuplicateRatePlots::fillPlots< xAOD::TrackParticle >(
    const xAOD::TrackParticle&, unsigned int nMatched, float truthMu, float actualMu, float weight );

template StatusCode IDTPM::DuplicateRatePlots::fillPlots< xAOD::TruthParticle >(
    const xAOD::TruthParticle&, unsigned int nMatched, float truthMu, float actualMu, float weight );


/// -------------------------
/// ----- finalizePlots -----
/// -------------------------
void IDTPM::DuplicateRatePlots::finalizePlots()
{
  ATH_MSG_DEBUG( "Finalising duplicate rate plots" );
  /// print stat here if needed
}
