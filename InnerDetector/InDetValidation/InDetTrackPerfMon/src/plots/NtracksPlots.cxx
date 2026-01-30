/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file    FakeRatePlots.cxx
 * @author  Marco Aparo <marco.aparo@cern.ch>
 **/

/// local include(s)
#include "InDetTrackPerfMon/NtracksPlots.h"
#include "InDetTrackPerfMon/TrackParametersHelper.h"


/// -----------------------
/// ----- Constructor -----
/// -----------------------
IDTPM::NtracksPlots::NtracksPlots(
    PlotMgr* pParent, const std::string& dirName, 
    const std::string& anaTag, const std::string& trackType,
    bool doTrigger, bool doTruthMuPlots ) :
        PlotMgr( dirName, anaTag, pParent ), 
        m_trackType( trackType ),
        m_doTrigger( doTrigger ),
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

    /// nTracks 1D distributions, e.g. "num_offl_selected"
    ATH_CHECK( retrieveAndBook( m_nTracks[i], "num_"+m_trackType+"_"+m_counterName[i] ) );

    /// nTracks vs nVtx 2D, e.g. "num_offl_selected_vs_num_vtx_offl_selected"
    ATH_CHECK( retrieveAndBook(
        m_nTracks_vs_nVertices[i],
        "num_"+m_trackType+"_"+m_counterName[i]+"_vs_num_vtx_"+m_trackType+"_"+m_counterName[i] ) );

    /// nTracks vs pileup 2D, e.g. "num_offl_selected_vs_actualMu"
    ATH_CHECK( retrieveAndBook(
        m_nTracks_vs_actualMu[i], "num_"+m_trackType+"_"+m_counterName[i]+"_vs_actualMu" ) );
    if( m_doTruthMuPlots ) ATH_CHECK( retrieveAndBook(
        m_nTracks_vs_truthMu[i],  "num_"+m_trackType+"_"+m_counterName[i]+"_vs_truthMu" ) );

    /// Average nTracks vs pileup TProfile, e.g. "avgNum_offl_selected_vs_actualMu"
    ATH_CHECK( retrieveAndBook(
        m_avg_nTracks_vs_actualMu[i], "avgNum_"+m_trackType+"_"+m_counterName[i]+"_vs_actualMu" ) );
    if( m_doTruthMuPlots ) ATH_CHECK( retrieveAndBook(
        m_avg_nTracks_vs_truthMu[i],  "avgNum_"+m_trackType+"_"+m_counterName[i]+"_vs_truthMu" ) );
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

    /// nTracks 1D distributions
    ATH_CHECK( fill( m_nTracks[i], trackCounts[i], weight ) );

    /// nTracks vs nVtx 2D
    ATH_CHECK( fill( m_nTracks_vs_nVertices[i], vertexCounts[i], trackCounts[i], weight ) );

    /// nTracks vs pileup 2D
    ATH_CHECK( fill( m_nTracks_vs_actualMu[i], actualMu, trackCounts[i], weight ) );
    if( m_doTruthMuPlots ) ATH_CHECK( fill( m_nTracks_vs_truthMu[i], truthMu, trackCounts[i], weight ) );

    /// average nTracks vs pileup TProfile
    ATH_CHECK( fill( m_avg_nTracks_vs_actualMu[i], actualMu, trackCounts[i], weight ) );
    if( m_doTruthMuPlots ) ATH_CHECK( fill( m_avg_nTracks_vs_truthMu[i], truthMu, trackCounts[i], weight ) );
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
