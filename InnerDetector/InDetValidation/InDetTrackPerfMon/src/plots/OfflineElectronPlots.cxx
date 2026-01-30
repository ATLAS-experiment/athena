/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file    OfflineElectronPlots.cxx
 * @author  Marco Aparo <marco.aparo@cern.ch> 
 **/


/// local include(s)
#include "InDetTrackPerfMon/OfflineElectronPlots.h"
#include "InDetTrackPerfMon/TrackParametersHelper.h"
#include "InDetTrackPerfMon/OfflineObjectDecorHelper.h"


/// -----------------------
/// ----- Constructor -----
/// -----------------------
IDTPM::OfflineElectronPlots::OfflineElectronPlots(
    PlotMgr* pParent, const std::string& dirName, 
    const std::string& anaTag, bool doEfficiency ) :
        PlotMgr( dirName, anaTag, pParent ), 
        m_doEfficiency( doEfficiency ) { }


/// ---------------------------
/// --- Book the histograms ---
/// ---------------------------
void IDTPM::OfflineElectronPlots::initializePlots()
{
  StatusCode sc = bookPlots();
  if( sc.isFailure() ) {
    ATH_MSG_ERROR( "Failed to book offline electron plots" );
  }
}


StatusCode IDTPM::OfflineElectronPlots::bookPlots()
{
  ATH_MSG_DEBUG( "Booking offline electron plots in " << getDirectory() ); 

  if( not m_doEfficiency ) {
    ATH_CHECK( retrieveAndBook( m_Et,       "offEle_Et" ) );
    ATH_CHECK( retrieveAndBook( m_EtOverPt, "offEle_EtOverPt" ) );
  } else {
    ATH_CHECK( retrieveAndBook( m_eff_vs_Et,        "eff_vs_offEle_Et" ) );
    ATH_CHECK( retrieveAndBook( m_eff_vs_EtOverPt,  "eff_vs_offEle_EtOverPt" ) );
  }

  return StatusCode::SUCCESS;
}


/// ------------------------------
/// --- Dedicated fill methods ---
/// ------------------------------
StatusCode IDTPM::OfflineElectronPlots::fillPlots(
    const xAOD::TrackParticle& track, 
    bool isMatched, float weight )
{
  const xAOD::Electron* ele = getLinkedElectron( track );

  if( not ele ) {
    ATH_MSG_ERROR( "Electron not found" );
    return StatusCode::FAILURE;
  }

  /// Compute track parameters - TODO: add more...
  float ppt = pT( track ) / Gaudi::Units::GeV;
  float pet = eT( *ele ) / Gaudi::Units::GeV;

  /// Fill the histograms
  if( not m_doEfficiency ) {
    ATH_CHECK( fill( m_Et,        pet,       weight ) );
    ATH_CHECK( fill( m_EtOverPt,  (pet/ppt), weight ) );
  } else {
    ATH_CHECK( fill( m_eff_vs_Et,        pet,       isMatched,  weight ) );
    ATH_CHECK( fill( m_eff_vs_EtOverPt,  (pet/ppt), isMatched,  weight ) );
  }

  return StatusCode::SUCCESS;
}


StatusCode IDTPM::OfflineElectronPlots::fillPlots(
  const xAOD::TruthParticle&, bool, float )
{
  ATH_MSG_ERROR( "Fill method disabled for truth particles" );
  return StatusCode::FAILURE;
}


/// -------------------------
/// ----- finalizePlots -----
/// -------------------------
void IDTPM::OfflineElectronPlots::finalizePlots()
{
  ATH_MSG_DEBUG( "Finalising offline electron plots" );
  /// print stat here if needed
}
