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
    const std::string& anaTag, const std::string& trackType,
    bool isTechnical, bool doGlobalPlots, bool doTruthMuPlots ) :
        PlotMgr( dirName, anaTag, pParent ), 
        m_trackType( trackType ),
        m_isTechnical( isTechnical ),
        m_doGlobalPlots( doGlobalPlots ),
        m_doTruthMuPlots( doTruthMuPlots ) { }


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

  ATH_CHECK( retrieveAndBook( m_eff_vs_inclusive,  "eff_vs_"+m_trackType+"_inclusive" ) );
  ATH_CHECK( retrieveAndBook( m_eff_vs_pt,  "eff_vs_"+m_trackType+"_pt" ) );
  ATH_CHECK( retrieveAndBook( m_eff_vs_logPt,  "eff_vs_"+m_trackType+"_logPt" ) );
  ATH_CHECK( retrieveAndBook( m_eff_vs_lowPt,  "eff_vs_"+m_trackType+"_lowPt" ) );
  ATH_CHECK( retrieveAndBook( m_eff_vs_eta, "eff_vs_"+m_trackType+"_eta" ) );
  ATH_CHECK( retrieveAndBook( m_eff_vs_phi, "eff_vs_"+m_trackType+"_phi" ) );
  ATH_CHECK( retrieveAndBook( m_eff_vs_d0, "eff_vs_"+m_trackType+"_d0" ) );
  ATH_CHECK( retrieveAndBook( m_eff_vs_z0, "eff_vs_"+m_trackType+"_z0" ) );
  if( m_trackType == "truth" ) {
    ATH_CHECK( retrieveAndBook( m_eff_vs_prodR, "eff_vs_"+m_trackType+"_prodR" ) );
    ATH_CHECK( retrieveAndBook( m_eff_vs_prodZ, "eff_vs_"+m_trackType+"_prodZ" ) );
  }
  ATH_CHECK( retrieveAndBook( m_eff_vs_eta_vs_pt, "eff_vs_"+m_trackType+"_eta_vs_pt" ) );
  ATH_CHECK( retrieveAndBook( m_eff_vs_eta_vs_phi, "eff_vs_"+m_trackType+"_eta_vs_phi" ) );
  ATH_CHECK( retrieveAndBook( m_eff_vs_z0_vs_d0, "eff_vs_"+m_trackType+"_z0_vs_d0" ) );
  ATH_CHECK( retrieveAndBook( m_eff_vs_z0sin_vs_d0, "eff_vs_"+m_trackType+"_z0sin_vs_d0" ) );

  if( m_doGlobalPlots ) {
    ATH_CHECK( retrieveAndBook( m_eff_vs_actualMu, "eff_vs_actualMu" ) );
    if( m_doTruthMuPlots ) ATH_CHECK( retrieveAndBook( m_eff_vs_truthMu, "eff_vs_truthMu" ) );
  }

  /// updating technical efficiencies plot lables
  if( m_isTechnical ) {
    updateYaxisLabel( m_eff_vs_inclusive );
    updateYaxisLabel( m_eff_vs_pt );
    updateYaxisLabel( m_eff_vs_logPt );
    updateYaxisLabel( m_eff_vs_lowPt );
    updateYaxisLabel( m_eff_vs_eta );
    updateYaxisLabel( m_eff_vs_phi );
    updateYaxisLabel( m_eff_vs_d0 );
    updateYaxisLabel( m_eff_vs_z0 );
    if( m_trackType == "truth" ) {
      updateYaxisLabel( m_eff_vs_prodR );
      updateYaxisLabel( m_eff_vs_prodZ );
    }
    updateYaxisLabel( m_eff_vs_eta_vs_pt );
    updateYaxisLabel( m_eff_vs_eta_vs_phi );
    updateYaxisLabel( m_eff_vs_z0_vs_d0 );
    updateYaxisLabel( m_eff_vs_z0sin_vs_d0 );
    if( m_doGlobalPlots ) {
      updateYaxisLabel( m_eff_vs_actualMu );
      if( m_doTruthMuPlots ) updateYaxisLabel( m_eff_vs_truthMu );
    }
  }

  return StatusCode::SUCCESS;
}


/// -----------------------------
/// --- Dedicated fill method ---
/// -----------------------------
template< typename PARTICLE >
StatusCode IDTPM::EfficiencyPlots::fillPlots(
    const PARTICLE& particle, bool isMatched, float truthMu, float actualMu, float weight )
{
  /// Compute track parameters
  float ppt    = pT( particle ) / Gaudi::Units::GeV;
  float peta   = eta( particle );
  float pphi   = phi( particle );
  float ptheta = theta( particle );
  float pd0    = d0( particle );
  float pz0    = z0( particle );
  float pprodR = prodR( particle );
  float pprodZ = prodZ( particle );

  /// Fill the histograms
  ATH_CHECK( fill( m_eff_vs_inclusive,  1,  isMatched, weight ) );
  ATH_CHECK( fill( m_eff_vs_pt,  ppt,  isMatched, weight ) );
  ATH_CHECK( fill( m_eff_vs_logPt,  ppt,  isMatched, weight ) );
  ATH_CHECK( fill( m_eff_vs_lowPt,  ppt,  isMatched, weight ) );
  ATH_CHECK( fill( m_eff_vs_eta, peta, isMatched, weight ) );
  ATH_CHECK( fill( m_eff_vs_phi, pphi, isMatched, weight ) );
  ATH_CHECK( fill( m_eff_vs_d0, pd0, isMatched, weight ) );
  ATH_CHECK( fill( m_eff_vs_z0, pz0, isMatched, weight ) );
  if( m_trackType == "truth" ) {
    ATH_CHECK( fill( m_eff_vs_prodR, pprodR, isMatched, weight ) );
    ATH_CHECK( fill( m_eff_vs_prodZ, pprodZ, isMatched, weight ) );
  }
  ATH_CHECK( fill( m_eff_vs_eta_vs_pt, ppt, peta, isMatched, weight ) );
  ATH_CHECK( fill( m_eff_vs_eta_vs_phi, pphi, peta, isMatched, weight ) );
  ATH_CHECK( fill( m_eff_vs_z0_vs_d0, pd0, pz0, isMatched, weight ) );
  ATH_CHECK( fill( m_eff_vs_z0sin_vs_d0, pz0*std::sin(ptheta), pd0, isMatched, weight ) );

  if( m_doGlobalPlots ) {
    ATH_CHECK( fill( m_eff_vs_actualMu, actualMu, isMatched, weight ) );
    if( m_doTruthMuPlots ) ATH_CHECK( fill( m_eff_vs_truthMu, truthMu, isMatched, weight ) );
  }

  return StatusCode::SUCCESS;
}

template StatusCode IDTPM::EfficiencyPlots::fillPlots< xAOD::TrackParticle >(
    const xAOD::TrackParticle&, bool isMatched, float truthMu, float actualMu, float weight );

template StatusCode IDTPM::EfficiencyPlots::fillPlots< xAOD::TruthParticle >(
    const xAOD::TruthParticle&, bool isMatched, float truthMu, float actualMu, float weight );


/// -------------------------
/// ----- finalizePlots -----
/// -------------------------
void IDTPM::EfficiencyPlots::finalizePlots()
{
  ATH_MSG_DEBUG( "Finalising efficiency plots" );
  /// print stat here if needed
}


/// ----------------------------
/// ----- updateYaxisLabel -----
/// ----------------------------
void IDTPM::EfficiencyPlots::updateYaxisLabel( TEfficiency*& eff )
{
  std::string oldTID( eff->GetTitle() );
  TString oldTitle( retrieveDefinition( oldTID ).titleDigest() );
  TString newTitle = oldTitle.ReplaceAll( "Efficiency", "Technical Efficiency" );
  eff->SetTitle( newTitle.Data() );
}
