/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file    TrackAnalysisPlotsMgr.cxx
 * @author  Marco Aparo <marco.aparo@cern.ch>
 * @date    19 June 2023
**/

/// local includes
#include "TrackAnalysisPlotsMgr.h"
#include "TrackAnalysisCollections.h"
#include "ITrackMatchingLookup.h"

/// Gaudi include(s)
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/Service.h"


/// -------------------
/// --- Constructor ---
/// -------------------
IDTPM::TrackAnalysisPlotsMgr::TrackAnalysisPlotsMgr(
    const std::string& dirName,
    const std::string& anaTag,
    const std::string& chain,
    PlotMgr* pParent ) :
        PlotMgr( dirName, anaTag, pParent ), 
        m_anaTag( anaTag ), m_chain( chain ),
        m_directory( dirName ), m_trkAnaDefSvc( nullptr ) { }


/// ------------------
/// --- initialize ---
/// ------------------
StatusCode IDTPM::TrackAnalysisPlotsMgr::initialize()
{
  ATH_MSG_DEBUG( "Initialising in directory: " << m_directory );

  /// load trkAnaDefSvc 
  if( not m_trkAnaDefSvc ) {
    ISvcLocator* svcLoc = Gaudi::svcLocator();
    ATH_CHECK( svcLoc->service( "TrkAnaDefSvc"+m_anaTag, m_trkAnaDefSvc ) );
  }

  /// Track parameters plots
  if( m_trkAnaDefSvc->plotTrackParameters() ) {
    m_plots_trkParam_vsTest = std::make_unique< TrackParametersPlots >(
        this, "Tracks/Parameters", m_anaTag, m_trkAnaDefSvc->testTag() );
    m_plots_trkParam_vsRef = std::make_unique< TrackParametersPlots >(
        this, "Tracks/Parameters", m_anaTag, m_trkAnaDefSvc->referenceTag() );
  } 

  /// Efficiency plots
  if( m_trkAnaDefSvc->plotEfficiencies() ) {
    m_plots_eff_vsTest = std::make_unique< EfficiencyPlots >(
        this, "Tracks/Efficiencies", m_anaTag, m_trkAnaDefSvc->testTag() );
    m_plots_eff_vsRef = std::make_unique< EfficiencyPlots >(
        this, "Tracks/Efficiencies", m_anaTag, m_trkAnaDefSvc->referenceTag() );
  }

  /// Offline electron plots
  if( m_trkAnaDefSvc->plotOfflineElectrons() ) {
    m_plots_offEle = std::make_unique< OfflineElectronPlots >(
        this, "Tracks/Parameters", m_anaTag );
    if( m_trkAnaDefSvc->plotEfficiencies() ) {
      m_plots_eff_vsOffEle = std::make_unique< OfflineElectronPlots >(
          this, "Tracks/Efficiencies", m_anaTag, true );
    }
  }

  /// intialize PlotBase
  ATH_CHECK( PlotMgr::initialize() );

  return StatusCode::SUCCESS;
}


/// --------------------------
/// ------ General fill ------
/// --------------------------
StatusCode IDTPM::TrackAnalysisPlotsMgr::fill(
    TrackAnalysisCollections& trkAnaColls, float weight )
{
  /// Plots w.r.t. test tracks quantities
  if( m_trkAnaDefSvc->isTestTruth() ) {
    ATH_CHECK( fillPlotsTest(
        trkAnaColls.testTruthVec( TrackAnalysisCollections::InRoI ),
        trkAnaColls.matches(), weight ) );
  } else {
    ATH_CHECK( fillPlotsTest(
        trkAnaColls.testTrackVec( TrackAnalysisCollections::InRoI ),
        trkAnaColls.matches(), weight ) );
  } 

  /// Plots w.r.t. reference tracks quantities
  if( m_trkAnaDefSvc->isReferenceTruth() ) {
    ATH_CHECK( fillPlotsReference(
        trkAnaColls.refTruthVec( TrackAnalysisCollections::InRoI ),
        trkAnaColls.matches(), weight ) );
  } else {
    ATH_CHECK( fillPlotsReference(
        trkAnaColls.refTrackVec( TrackAnalysisCollections::InRoI ),
        trkAnaColls.matches(), weight ) );
  } 

  return StatusCode::SUCCESS;
}


/// ------------------------------
/// --- Fill plots w.r.t. test ---
/// ------------------------------
template< typename PARTICLE >
StatusCode IDTPM::TrackAnalysisPlotsMgr::fillPlotsTest(
    const std::vector< const PARTICLE* >& particles,
    const ITrackMatchingLookup& matches, float weight )
{
  for( const PARTICLE* particle : particles ) {
    /// track parameters plots
    if( m_plots_trkParam_vsTest ) {
      ATH_CHECK( m_plots_trkParam_vsTest->fillPlots( *particle, weight ) );
    }

    bool isMatched = matches.isTestMatched( *particle );

    /// efficiency plots
    if( m_plots_eff_vsTest ) {
      ATH_CHECK( m_plots_eff_vsTest->fillPlots( *particle, isMatched, weight ) );
    }

    /// offline electron plots (Offline is always either test or reference)
    if( m_trkAnaDefSvc->isTestOffline() ) {
      if( m_plots_offEle ) {
        ATH_CHECK( m_plots_offEle->fillPlots( *particle, false, weight ) );
      }
      if( m_plots_eff_vsOffEle ) {
        ATH_CHECK( m_plots_eff_vsOffEle->fillPlots( *particle, isMatched, weight ) );
      }
    }

  } // close loop over particles

  return StatusCode::SUCCESS;
}

template StatusCode
IDTPM::TrackAnalysisPlotsMgr::fillPlotsTest< xAOD::TrackParticle >(
    const std::vector< const xAOD::TrackParticle* >& particles,
    const ITrackMatchingLookup& matches, float weight );

template StatusCode
IDTPM::TrackAnalysisPlotsMgr::fillPlotsTest< xAOD::TruthParticle >(
    const std::vector< const xAOD::TruthParticle* >& particles,
    const ITrackMatchingLookup& matches, float weight );


/// -----------------------------------
/// --- Fill plots w.r.t. reference ---
/// -----------------------------------
template< typename PARTICLE >
StatusCode IDTPM::TrackAnalysisPlotsMgr::fillPlotsReference(
    const std::vector< const PARTICLE* >& particles,
    const ITrackMatchingLookup& matches, float weight )
{
  for( const PARTICLE* particle : particles ) {
    /// track parameters plots
    if( m_plots_trkParam_vsRef ) {
      ATH_CHECK( m_plots_trkParam_vsRef->fillPlots( *particle, weight ) );
    }

    bool isMatched = matches.isRefMatched( *particle );

    /// efficiency plots
    if( m_plots_eff_vsRef ) {
      ATH_CHECK( m_plots_eff_vsRef->fillPlots( *particle, isMatched, weight ) );
    }

    /// offline electron plots (Offline is always either test or reference)
    if( m_trkAnaDefSvc->isReferenceOffline() ) {
      if( m_plots_offEle ) {
        ATH_CHECK( m_plots_offEle->fillPlots( *particle, false, weight ) );
      }
      if( m_plots_eff_vsOffEle ) {
        ATH_CHECK( m_plots_eff_vsOffEle->fillPlots( *particle, isMatched, weight ) );
      }
    }

  } // close loop over particles

  return StatusCode::SUCCESS;
}

template StatusCode
IDTPM::TrackAnalysisPlotsMgr::fillPlotsReference< xAOD::TrackParticle >(
    const std::vector< const xAOD::TrackParticle* >& particles,
    const ITrackMatchingLookup& matches, float weight );

template StatusCode
IDTPM::TrackAnalysisPlotsMgr::fillPlotsReference< xAOD::TruthParticle >(
    const std::vector< const xAOD::TruthParticle* >& particles,
    const ITrackMatchingLookup& matches, float weight );
