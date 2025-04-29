/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file    FakeRatePlots.cxx
 * @author  Marco Aparo <marco.aparo@cern.ch>
 **/

/// local include(s)
#include "NtracksPlots.h"
#include "../TrackParametersHelper.h"


/// -----------------------
/// ----- Constructor -----
/// -----------------------
IDTPM::NtracksPlots::NtracksPlots(
    PlotMgr* pParent, const std::string& dirName, 
    const std::string& anaTag, const std::string& trackType,
    bool doTrigger, bool doGlobalPlots, bool doTruthMuPlots ) :
        PlotMgr( dirName, anaTag, pParent ), 
        m_trackType( trackType ),
        m_doTrigger( doTrigger ),
        m_doGlobalPlots( doGlobalPlots ),
        m_doTruthMuPlots( doTruthMuPlots ) { }


/// ---------------------------
/// --- Book the histograms ---
/// ---------------------------
void IDTPM::NtracksPlots::initializePlots()
{
  StatusCode sc = bookPlots();
  if( sc.isFailure() ) {
    ATH_MSG_ERROR( "Failed to book track multiplicity plots" );
  }
}


StatusCode IDTPM::NtracksPlots::bookPlots()
{
  ATH_MSG_DEBUG( "Booking track multiplicity plots in " << getDirectory() ); 

  for( size_t i=0; i<NCOUNTERS; i++ ) {
    /// skip selected inRoI step for offline-like analysis
    if( not m_doTrigger and i == INROI ) continue;

    /// e.g. "num_offl_selected"
    ATH_CHECK( retrieveAndBook( m_nTracks[i], "num_"+m_trackType+"_"+m_counterName[i] ) );

    /// e.g. "num_offl_selected_vs_num_vtx_offl_selected"
    ATH_CHECK( retrieveAndBook(
        m_nTracks_vs_nVertices[i],
        "num_"+m_trackType+"_"+m_counterName[i]+"_vs_num_vtx_"+m_trackType+"_"+m_counterName[i] ) );
  }

  if( m_doGlobalPlots ) {
    /// e.g. "num_offl_selected_vs_actualMu"
    /// N.B.  These plots are filled always filled with counts[ INROI ] ,
    ///       which for offline-like analyses is by construction = counts[ SELECTED ] .
    ///       Regardless the name of these plots will always have m_counterName[ SELECTED ] = "selected"
    ATH_CHECK( retrieveAndBook(
        m_nTracks_vs_actualMu, "num_"+m_trackType+"_"+m_counterName[ SELECTED ]+"_vs_actualMu" ) );
    if( m_doTruthMuPlots ) ATH_CHECK( retrieveAndBook(
        m_nTracks_vs_truthMu,  "num_"+m_trackType+"_"+m_counterName[ SELECTED ]+"_vs_truthMu" ) );
  }

  return StatusCode::SUCCESS;
}


/// -----------------------------
/// --- Dedicated fill method ---
/// -----------------------------
StatusCode IDTPM::NtracksPlots::fillPlots(
    const std::vector< size_t >& trackCounts,
    const std::vector< size_t >& vertexCounts,
    float truthMu,
    float actualMu,
    float weight )
{
  /// check counts size
  if( trackCounts.size() != NCOUNTERS or vertexCounts.size() != NCOUNTERS ) {
    ATH_MSG_ERROR( "Counts vectors sizes are invalid" );
    return StatusCode::FAILURE;
  }

  /// Fill the histograms
  for( size_t i=0; i<NCOUNTERS; i++ ) {
    /// skip selected inRoI step for offline-like analysis
    if( not m_doTrigger and i == INROI ) continue;
    ATH_CHECK( fill( m_nTracks[i], trackCounts[i], weight ) );
    ATH_CHECK( fill( m_nTracks_vs_nVertices[i], vertexCounts[i], trackCounts[i], weight ) );
  }

  if( m_doGlobalPlots ) {
    ATH_CHECK( fill( m_nTracks_vs_actualMu, actualMu, trackCounts[ INROI ], weight ) );
    if( m_doTruthMuPlots ) ATH_CHECK( fill( m_nTracks_vs_truthMu, truthMu, trackCounts[ INROI ], weight ) );
  }

  return StatusCode::SUCCESS;
}


/// -------------------------
/// ----- finalizePlots -----
/// -------------------------
void IDTPM::NtracksPlots::finalizePlots()
{
  ATH_MSG_DEBUG( "Finalising track multiplicity plots" );
  /// print stat here if needed
}
