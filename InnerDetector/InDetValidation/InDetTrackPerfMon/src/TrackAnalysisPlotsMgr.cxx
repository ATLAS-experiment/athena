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
#include "OfflineObjectDecorHelper.h"

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
    if( !m_trkAnaDefSvc ) { // suppress cppcheck warning.
      return StatusCode::FAILURE;
    }
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
        this, "Tracks/Efficiencies", m_anaTag, m_trkAnaDefSvc->referenceTag(), true );
    if( m_trkAnaDefSvc->matchingType() == "EFTruthMatch" ) {
      m_plots_eff_vsTruth = std::make_unique< EfficiencyPlots >(
          this, "Tracks/Efficiencies", m_anaTag, "truth" );
    }
  }

  /// Resolution plots
  if( m_trkAnaDefSvc->plotResolutions() ) {
    m_plots_resolution = std::make_unique< ResolutionPlots >(
        this, "Tracks/Resolutions", m_anaTag, 
        m_trkAnaDefSvc->testTag(), m_trkAnaDefSvc->referenceTag(),
        m_trkAnaDefSvc->resolutionMethod() );
  }

  /// Fake Rate plots (only if reference is Truth)
  if( m_trkAnaDefSvc->plotFakeRates() and m_trkAnaDefSvc->isReferenceTruth() ) {
    m_plots_fakeRate = std::make_unique< FakeRatePlots >(
        this, "Tracks/FakeRates", m_anaTag, m_trkAnaDefSvc->testTag(), true );
    m_plots_missingTruth = std::make_unique< FakeRatePlots >(
        this, "Tracks/Unlinked/FakeRates", m_anaTag, m_trkAnaDefSvc->testTag(), true );
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
  float actualMu = trkAnaColls.eventInfo() ?
                   trkAnaColls.eventInfo()->actualInteractionsPerCrossing() : 0.;
  float truthMu = 0.; // TODO - do proper calculation

  /// Plots w.r.t. test tracks quantities
  if( m_trkAnaDefSvc->isTestTruth() ) {
    ATH_CHECK( fillPlotsTest(
        trkAnaColls.testTruthVec( TrackAnalysisCollections::InRoI ),
        trkAnaColls.matches(), truthMu, actualMu, weight ) );
  } else {
    ATH_CHECK( fillPlotsTest(
        trkAnaColls.testTrackVec( TrackAnalysisCollections::InRoI ),
        trkAnaColls.matches(), truthMu, actualMu, weight ) );
  } 

  /// Plots w.r.t. reference tracks quantities
  if( m_trkAnaDefSvc->isReferenceTruth() ) {
    ATH_CHECK( fillPlotsReference(
        trkAnaColls.refTruthVec( TrackAnalysisCollections::InRoI ),
        trkAnaColls.matches(), truthMu, actualMu, weight ) );
  } else {
    ATH_CHECK( fillPlotsReference(
        trkAnaColls.refTrackVec( TrackAnalysisCollections::InRoI ),
        trkAnaColls.matches(), truthMu, actualMu, weight ) );
  } 

  /// Plots w.r.t. truth quantities (for EFTruthMatch only)
  if( m_trkAnaDefSvc->matchingType() == "EFTruthMatch" ) {
    ATH_CHECK( fillPlotsTruth(
        trkAnaColls.testTrackVec( TrackAnalysisCollections::InRoI ),
        trkAnaColls.truthPartVec( TrackAnalysisCollections::InRoI ),
        trkAnaColls.matches(), truthMu, actualMu, weight ) );
  }

  return StatusCode::SUCCESS;
}


/// ------------------------------
/// --- Fill plots w.r.t. test ---
/// ------------------------------
template< typename PARTICLE >
StatusCode IDTPM::TrackAnalysisPlotsMgr::fillPlotsTest(
    const std::vector< const PARTICLE* >& particles,
    const ITrackMatchingLookup& matches,
    float truthMu, float actualMu, float weight )
{
  for( const PARTICLE* particle : particles ) {
    /// track parameters plots
    if( m_plots_trkParam_vsTest ) {
      ATH_CHECK( m_plots_trkParam_vsTest->fillPlots( *particle, weight ) );
    }

    bool isMatched = matches.isTestMatched( *particle );

    /// efficiency plots
    if( m_plots_eff_vsTest ) {
      ATH_CHECK( m_plots_eff_vsTest->fillPlots(
          *particle, isMatched, truthMu, actualMu, weight ) );
    }

    /// resolution plots
    if( m_plots_resolution ) {
      if( isMatched ) {
        if( m_trkAnaDefSvc->isReferenceTruth() ) {
	        ATH_CHECK( m_plots_resolution->fillPlots(
            *particle, *(matches.getMatchedRefTruth( *particle )), weight ) );
        } else {
	        ATH_CHECK( m_plots_resolution->fillPlots(
            *particle, *(matches.getMatchedRefTrack( *particle )), weight ) );
        }
      }
    }

    /// fake rate plots
    if( m_plots_missingTruth ) {
      bool isUnlinked = isUnlinkedTruth( *particle );
      ATH_CHECK( m_plots_missingTruth->fillPlots( *particle, isUnlinked, truthMu, actualMu, weight ) );
      if( not isUnlinked and m_plots_fakeRate ) {
        bool isFake = isFakeTruth( *particle, m_trkAnaDefSvc->truthProbCut() );
        ATH_CHECK( m_plots_fakeRate->fillPlots( *particle, isFake, truthMu, actualMu, weight ) );
      }
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
    const ITrackMatchingLookup& matches,
    float truthMu, float actualMu, float weight );

template StatusCode
IDTPM::TrackAnalysisPlotsMgr::fillPlotsTest< xAOD::TruthParticle >(
    const std::vector< const xAOD::TruthParticle* >& particles,
    const ITrackMatchingLookup& matches,
    float truthMu, float actualMu, float weight );


/// -----------------------------------
/// --- Fill plots w.r.t. reference ---
/// -----------------------------------
template< typename PARTICLE >
StatusCode IDTPM::TrackAnalysisPlotsMgr::fillPlotsReference(
    const std::vector< const PARTICLE* >& particles,
    const ITrackMatchingLookup& matches,
    float truthMu, float actualMu, float weight )
{
  for( const PARTICLE* particle : particles ) {
    /// track parameters plots
    if( m_plots_trkParam_vsRef ) {
      ATH_CHECK( m_plots_trkParam_vsRef->fillPlots( *particle, weight ) );
    }

    bool isMatched = matches.isRefMatched( *particle );

    /// efficiency plots
    if( m_plots_eff_vsRef ) {
      ATH_CHECK( m_plots_eff_vsRef->fillPlots(
          *particle, isMatched, truthMu, actualMu, weight ) );
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
    const ITrackMatchingLookup& matches,
    float truthMu, float actualMu, float weight );

template StatusCode
IDTPM::TrackAnalysisPlotsMgr::fillPlotsReference< xAOD::TruthParticle >(
    const std::vector< const xAOD::TruthParticle* >& particles,
    const ITrackMatchingLookup& matches,
    float truthMu, float actualMu, float weight );


/// ------------------------------
/// --- Fill plots w.r.t. truth ---
/// ------------------------------
StatusCode IDTPM::TrackAnalysisPlotsMgr::fillPlotsTruth(
    const std::vector< const xAOD::TrackParticle* >& tracks,
    const std::vector< const xAOD::TruthParticle* >& truths,
    const ITrackMatchingLookup& matches,
    float truthMu, float actualMu, float weight )
{
  for( const xAOD::TruthParticle* thisTruth : truths ) {

    /// Loop over tracks to find if truth is matched
    bool isMatched( false );
    for( const xAOD::TrackParticle* thisTrack : tracks ) {
      const xAOD::TruthParticle* linkedTruth = getLinkedTruth(
          *thisTrack, m_trkAnaDefSvc->truthProbCut() );
      if( not linkedTruth ) {
        ATH_MSG_WARNING( "Unlinked track!!" );
        continue;
      }
      if( thisTruth == linkedTruth ) {
        isMatched = matches.isTestMatched( *thisTrack );
        break;
      }
    } // close loop over tracks

    /// efficiency plots (for EFTruthMatch only)
    if( m_plots_eff_vsTruth ) {
      ATH_CHECK( m_plots_eff_vsTruth->fillPlots(
          *thisTruth, isMatched, truthMu, actualMu, weight ) );
    }
  } // close loop over truth particles

  return StatusCode::SUCCESS;
}
