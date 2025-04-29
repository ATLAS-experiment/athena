/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file    SummaryPlots.cxx
 * @author  Marco Aparo <marco.aparo@cern.ch>
 **/

/// local include(s)
#include "SummaryPlots.h"


/// -----------------------
/// ----- Constructor -----
/// -----------------------
IDTPM::SummaryPlots::SummaryPlots(
    PlotMgr* pParent, const std::string& dirName, 
    const std::string& anaTag,
    bool doTrigger ) :
        PlotMgr( dirName, anaTag, pParent ), 
        m_doTrigger( doTrigger ) { }


/// ---------------------------
/// --- Book the histograms ---
/// ---------------------------
void IDTPM::SummaryPlots::initializePlots()
{
  StatusCode sc = bookPlots();
  if( sc.isFailure() ) {
    ATH_MSG_ERROR( "Failed to book summary plots" );
  }
}


StatusCode IDTPM::SummaryPlots::bookPlots()
{
  ATH_MSG_DEBUG( "Booking Summary plots in " << getDirectory() ); 

  if( m_doTrigger ) {
    ATH_CHECK( retrieveAndBook( m_summary_trig, "summary_trigNav" ) );
  } else {
    ATH_CHECK( retrieveAndBook( m_summary, "summary" ) );
  }

  return StatusCode::SUCCESS;
}


/// -----------------------------
/// --- Dedicated fill method ---
/// -----------------------------
StatusCode IDTPM::SummaryPlots::fillPlots(
    const std::vector< size_t >& testTrackCounts,
    const std::vector< size_t >& refTrackCounts,
    bool isNewEvent,
    float weight )
{
  if( m_doTrigger ) {
    /// Filling summary plot bins (trigger navigation case)
    if( isNewEvent ) {
      /// bins only filled once per events, to avoid double-counting
      ATH_CHECK( fill( m_summary_trig, Nevents_trig+0.5,  weight ) );
      ATH_CHECK( fill( m_summary_trig, NtotTest_trig+0.5, weight * testTrackCounts[ ALL ] ) );
      ATH_CHECK( fill( m_summary_trig, NselTest_trig+0.5, weight * testTrackCounts[ SELECTED ] ) );
      ATH_CHECK( fill( m_summary_trig, NtotRef_trig+0.5,  weight * refTrackCounts[ ALL ] ) );
      ATH_CHECK( fill( m_summary_trig, NselRef_trig+0.5,  weight * refTrackCounts[ SELECTED ] ) );
    }
    /// bins filled for every RoI
    ATH_CHECK( fill( m_summary_trig, Nrois_trig+0.5,      weight ) );
    ATH_CHECK( fill( m_summary_trig, NinRoiTest_trig+0.5, weight * testTrackCounts[ INROI ] ) );
    ATH_CHECK( fill( m_summary_trig, NmatchTest_trig+0.5, weight * testTrackCounts[ MATCHED ] ) );
    ATH_CHECK( fill( m_summary_trig, NinRoiRef_trig+0.5,  weight * refTrackCounts[ INROI ] ) );
    ATH_CHECK( fill( m_summary_trig, NmatchRef_trig+0.5,  weight * refTrackCounts[ MATCHED ] ) );
  } else {
    /// Filling summary plot bins (full-scan/offline-like case)
    if( isNewEvent ) {
      /// For offline analysis isNewEvent, is always true by construction,
      /// so this condition is redundant. Still kept it for consistency/robustness
      ATH_CHECK( fill( m_summary, Nevents+0.5,  weight ) );
      ATH_CHECK( fill( m_summary, NtotTest+0.5, weight * testTrackCounts[ ALL ] ) );
      ATH_CHECK( fill( m_summary, NselTest+0.5, weight * testTrackCounts[ SELECTED ] ) );
      ATH_CHECK( fill( m_summary, NtotRef+0.5,  weight * refTrackCounts[ ALL ] ) );
      ATH_CHECK( fill( m_summary, NselRef+0.5,  weight * refTrackCounts[ SELECTED ] ) );
    }
    ATH_CHECK( fill( m_summary, NmatchTest+0.5, weight * testTrackCounts[ MATCHED ] ) );
    ATH_CHECK( fill( m_summary, NmatchRef+0.5,  weight * refTrackCounts[ MATCHED ] ) );
  }

  return StatusCode::SUCCESS;
}


/// -------------------------
/// ----- finalizePlots -----
/// -------------------------
void IDTPM::SummaryPlots::finalizePlots()
{
  ATH_MSG_DEBUG( "Finalising Summary plots" );
  /// print stat here if needed
}
