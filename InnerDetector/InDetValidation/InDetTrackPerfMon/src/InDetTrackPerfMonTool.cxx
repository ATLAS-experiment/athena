/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file InDetTrackPerfMonTool.cxx
 * @author marco aparo
 **/

/// local include
#include "InDetTrackPerfMonTool.h"

/// gaudi includes
#include "GaudiKernel/SystemOfUnits.h"

/// EDM includes
#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTruth/TruthParticleContainer.h"

/// STL includes
#include <algorithm>
#include <limits>
#include <cmath> // to get std::isnan(), std::abs etc.
#include <utility>
#include <cstdlib> // to getenv


///----------------------------------------
///------- Parametrized constructor -------
///----------------------------------------
InDetTrackPerfMonTool::InDetTrackPerfMonTool(
    const std::string& type,
    const std::string& name,
    const IInterface* parent ) :
        ManagedMonitorToolBase( type, name, parent )
{ }


///----------------------------------
///------- Default destructor -------
///----------------------------------
InDetTrackPerfMonTool::~InDetTrackPerfMonTool() = default;


///--------------------------
///------- Initialize -------
///--------------------------
StatusCode InDetTrackPerfMonTool::initialize() {

  ATH_CHECK( ManagedMonitorToolBase::initialize() );

  /// Retrieving trkAnaDefSvc
  if ( not m_trkAnaDefSvc ) {
    ATH_MSG_DEBUG( "Retrieving TrkAnaDefSvc" << m_anaTag.value() );
    m_trkAnaDefSvc = Gaudi::svcLocator()->service( "TrkAnaDefSvc"+m_anaTag.value() );
    ATH_CHECK( m_trkAnaDefSvc.isValid() );
  }

  ATH_MSG_DEBUG( "Initializing sub-tools" );

  ATH_CHECK( m_trigDecTool.retrieve(
      EnableTool{ m_trkAnaDefSvc->useTrigger() and not m_trkAnaDefSvc->useEFTrigger() } ) );
  ATH_CHECK( m_roiSelectionTool.retrieve(
      EnableTool{ m_trkAnaDefSvc->useTrigger() and not m_trkAnaDefSvc->useEFTrigger() } ) );
  ATH_CHECK( m_trackRoiSelectionTool.retrieve(
      EnableTool{ m_trkAnaDefSvc->useTrigger() and not m_trkAnaDefSvc->useEFTrigger()} ) );
  ATH_CHECK( m_trackMatchingTool.retrieve( EnableTool{ m_doMatch.value() } ) );
  ATH_CHECK( m_trkAnaInfoWriteTool.retrieve( EnableTool{ m_writeOut.value() } ) );

  ATH_MSG_DEBUG( "Initializing collections" );

  /// Events
  ATH_CHECK( m_eventInfoContainerName.initialize() );
  ATH_CHECK( m_truthEventName.initialize(
      m_trkAnaDefSvc->useTruth() and not m_truthEventName.key().empty() ) );
  ATH_CHECK( m_truthPileUpEventName.initialize(
      m_trkAnaDefSvc->useTruth() and not m_truthPileUpEventName.key().empty() and
      m_trkAnaDefSvc->hasFullPileupTruth() ) );

  /// Tracks
  ATH_CHECK( m_offlineTrkParticleName.initialize(
      m_trkAnaDefSvc->useOffline() and not m_offlineTrkParticleName.key().empty() ) );
  ATH_CHECK( m_triggerTrkParticleName.initialize(
      m_trkAnaDefSvc->useTrigger() and not m_triggerTrkParticleName.key().empty() ) );
  ATH_CHECK( m_truthParticleName.initialize(
      m_trkAnaDefSvc->useTruth() and not m_truthParticleName.key().empty() ) );

  /// Vertex
  ATH_CHECK( m_offlineVertexContainerName.initialize( 
      m_trkAnaDefSvc->useOffline() and not m_offlineVertexContainerName.key().empty() ) );
  ATH_CHECK( m_triggerVertexContainerName.initialize(
      m_trkAnaDefSvc->useTrigger() and not m_triggerVertexContainerName.key().empty() ) );
  ATH_CHECK( m_truthVertexContainerName.initialize(
      m_trkAnaDefSvc->useTruth() and not m_truthVertexContainerName.key().empty() ) );

  /// TrkAnaInfo for AOD_IDTPM output
  ATH_CHECK( m_trkAnaInfoKey.initialize() );

  /// Retrieving list of configured chains
  const std::vector< std::string >& configuredChains = m_trkAnaDefSvc->configuredChains();
  m_trkAnaPlotsMgrVec.reserve( configuredChains.size() );

  /// booking analyses
  for( const std::string& thisChain : configuredChains ) {
    ATH_MSG_DEBUG( "Booking TrkAnalysis/histograms for chain : " << thisChain );

    /// Instantiating a different TrkAnalysis object (with corresponding histograms) 
    /// for every configured chain
    m_trkAnaPlotsMgrVec.emplace_back(
        std::make_unique< IDTPM::TrackAnalysisPlotsMgr >(
            m_trkAnaDefSvc->plotsFullDir( thisChain ),
            m_anaTag.value(),
            thisChain ) );
  } // close m_configuredChains loop 

  return StatusCode::SUCCESS;
}


///------------------------------
///------- bookHistograms -------
///------------------------------
StatusCode InDetTrackPerfMonTool::bookHistograms()
{
  ATH_MSG_INFO( "Booking plots" );

  for( size_t iAna=0 ; iAna < m_trkAnaPlotsMgrVec.size() ; iAna++ ) {

    /// initialising/booking histograms
    ATH_CHECK( m_trkAnaPlotsMgrVec[iAna]->initialize() );

    /// Register booked histogram to corresponding monitoring group
    /// Register "plain" histograms (including TH1/2/3 and TProfiles)
    std::vector< HistData > hists = m_trkAnaPlotsMgrVec[iAna]->retrieveBookedHistograms();
    for ( size_t ih=0 ; ih<hists.size() ; ih++ ) {
      ATH_CHECK( regHist( hists[ih].first, hists[ih].second, all ) );
    }

    // do the same for Efficiencies, but there's a twist:
    std::vector< EfficiencyData > effs = m_trkAnaPlotsMgrVec[iAna]->retrieveBookedEfficiencies();
    for ( size_t ie=0 ; ie<effs.size() ; ie++ ) {
      ATH_CHECK( regEfficiency( effs[ie].first, MonGroup( this, effs[ie].second, all ) ) );
    }

  } // closing loop over TrkAnalyses
  
  return StatusCode::SUCCESS;
}


/// ------------------------------
/// ------- fillHistograms -------
/// ------------------------------
StatusCode InDetTrackPerfMonTool::fillHistograms() {

  ATH_MSG_INFO( "Filling hists " << name() << " ..." );

  /// Output TrackAnalysisInfo container writing
  SG::WriteHandle< xAOD::BaseContainer > outTrkAnaInfoContHandle( m_trkAnaInfoKey );
  if( m_writeOut ) {
    ATH_CHECK( outTrkAnaInfoContHandle.record(
                  std::make_unique< xAOD::BaseContainer >(),
                  std::make_unique< xAOD::AuxContainerBase >() ) );
  }

  /// Defining TrackAnalysisCollections object
  /// to contain all collections for this event
  IDTPM::TrackAnalysisCollections thisTrkAnaCollections( m_anaTag.value() );
  ATH_CHECK( thisTrkAnaCollections.initialize() );

  /// filling TrackAnalysisCollections
  ATH_CHECK( loadCollections( thisTrkAnaCollections ) );

  ATH_MSG_DEBUG( "Processing event = " <<
                 thisTrkAnaCollections.eventInfo()->eventNumber() <<
                 "\n==========================================" );
  ATH_MSG_DEBUG( "ALL Track Info: " << thisTrkAnaCollections.printInfo() );

  /// Check if overall test/reference track vectors are empty
  if( thisTrkAnaCollections.empty() ) {
    ATH_MSG_DEBUG( "Some FULL collections are empty." );
  }

  /// ------------------------------
  /// --- Track quality selector ---
  /// ------------------------------
  ATH_CHECK( m_trackQualitySelectionTool->selectTracks( thisTrkAnaCollections ) );

  /// Check if overall test/reference track vectors are empty
  if( thisTrkAnaCollections.empty( IDTPM::TrackAnalysisCollections::FS ) ) {
    ATH_MSG_DEBUG( "Some collections are empty after quality selection." );
  }

  /// -------------------------------
  /// --- Vertex quality selector ---
  /// -------------------------------
  /// FIXME - for the time being, only copy veritces from FULL
  /// will need selector tool, as for the tracks
  if( m_trkAnaDefSvc->useOffline() ) {
    ATH_CHECK( thisTrkAnaCollections.fillOfflVertexVec(
        thisTrkAnaCollections.offlVertexVec( IDTPM::TrackAnalysisCollections::FULL ),
        IDTPM::TrackAnalysisCollections::FS ) );
  }

  if( m_trkAnaDefSvc->useEFTrigger() ) {
    ATH_CHECK( thisTrkAnaCollections.fillTrigVertexVec(
        thisTrkAnaCollections.trigVertexVec( IDTPM::TrackAnalysisCollections::FULL ),
        IDTPM::TrackAnalysisCollections::FS ) );
  }

  if( m_trkAnaDefSvc->useTruth() ) {
    ATH_CHECK( thisTrkAnaCollections.fillTruthVertexVec(
        thisTrkAnaCollections.truthVertexVec( IDTPM::TrackAnalysisCollections::FULL ),
        IDTPM::TrackAnalysisCollections::FS ) );
  }

  /// Debug printout
  ATH_MSG_DEBUG( "Vertices after initial FullScan copy: " << 
      thisTrkAnaCollections.printVertexInfo( IDTPM::TrackAnalysisCollections::FS ) );

  /// -------------------------------------------
  /// -- Main loop over configured TrkAnalyses --
  /// -------------------------------------------
  /// one TrkAnalysis per configured chain (trigger only)
  /// only one dummy chain (named "Offline") for offline analysis
  for( size_t iAna=0 ; iAna < m_trkAnaPlotsMgrVec.size() ; iAna++ ) {

    const std::string& thisChain = m_trkAnaPlotsMgrVec[iAna]->chain();
    ATH_MSG_DEBUG( "Processing chain = " << thisChain );

    /// ----------------------------------
    /// --------- Chain selector ---------
    /// ----------------------------------
    unsigned decisionType = TrigDefs::Physics; // TrigDefs::includeFailedDecisions;

    /// skipping TrkAnalysis if chain is not passed for this event
    if( !thisChain.empty() and thisChain != "Offline" and m_trkAnaDefSvc->useTrigger() and not m_trkAnaDefSvc->useEFTrigger() ) {
      if( not m_trigDecTool->isPassed( thisChain, decisionType ) ) { 
        ATH_MSG_DEBUG( "Trigger chain " << thisChain << " is not fired. Skipping" );
        continue;
      }
    }

    /// ----------------------------------
    /// ------ RoI getter/selector -------
    /// ----------------------------------
    std::vector< TrigCompositeUtils::LinkInfo< TrigRoiDescriptorCollection > > selectedRois;
    size_t selectedRoisSize(1); // by default only one "dummy" RoI, i.e. for offline analysis

    if( m_trkAnaDefSvc->useTrigger() and not m_trkAnaDefSvc->useEFTrigger() ) {
      selectedRois = m_roiSelectionTool->getRois( thisChain ); 
      selectedRoisSize = selectedRois.size();
    }

    /// ----------------------------------
    /// -- Main loop over selected RoIs --
    /// ----------------------------------
    /// Only one "dummy" RoI iteration for offline analysis
    for( size_t ir=0 ; ir<selectedRoisSize ; ir++ ) {

      /// clear collections in this RoI from previous iteration
      thisTrkAnaCollections.clear( IDTPM::TrackAnalysisCollections::InRoI );

      ElementLink< TrigRoiDescriptorCollection > thisRoiLink;
      if( m_trkAnaDefSvc->useTrigger() and not m_trkAnaDefSvc->useEFTrigger() ) thisRoiLink = selectedRois.at(ir).link;
      const TrigRoiDescriptor* const* thisRoi = m_trkAnaDefSvc->useTrigger() and not m_trkAnaDefSvc->useEFTrigger() ? 
                                                thisRoiLink.cptr() : nullptr;
      std::string thisRoiStr = m_trkAnaDefSvc->useTrigger() and not m_trkAnaDefSvc->useEFTrigger()?
                               std::string( **thisRoi ) : "Full Scan";

      ATH_MSG_DEBUG( "Processing selected RoI : " << thisRoiStr );

      /// ----------------------------------
      /// --- Track selection within RoI ---
      /// ----------------------------------
      if( m_trkAnaDefSvc->useTrigger() and not m_trkAnaDefSvc->useEFTrigger() ) {
        /// Tracks in RoI selection
        ATH_CHECK( m_trackRoiSelectionTool->selectTracksInRoI(
                                thisTrkAnaCollections, thisRoiLink ) );
      } else {
        /// No RoI selection required. Copying FullScan vectors
        thisTrkAnaCollections.copyFS();
      }

      /// checking if track collections are empty
      if( thisTrkAnaCollections.empty( IDTPM::TrackAnalysisCollections::InRoI ) ) {
        ATH_MSG_DEBUG( "Some collections are empty after RoI selection." );
      }

      /// -------------------------------
      /// --- Test/Reference Matching ---
      /// -------------------------------
      std::string chainRoIName = thisChain;
      if( m_trkAnaDefSvc->useTrigger() and not m_trkAnaDefSvc->useEFTrigger() ) chainRoIName += "_RoI_"+std::to_string(ir);

      if( m_doMatch.value() ) {
        ATH_MSG_DEBUG( "Doing Test-Reference matching..." );
        ATH_CHECK( m_trackMatchingTool->match( thisTrkAnaCollections,
                                               chainRoIName, thisRoiStr ) );
      }

      /// --------------------------
      /// --- Filling histograms ---
      /// --------------------------
      ATH_CHECK( m_trkAnaPlotsMgrVec[iAna]->fill( thisTrkAnaCollections ) );

      /// ---------------------------------------
      /// --- Writing trkAnaInfo to StoreGate ---
      /// ---------------------------------------
      if( m_writeOut ) {
        ATH_CHECK( m_trkAnaInfoWriteTool->write( outTrkAnaInfoContHandle,
                                      thisTrkAnaCollections,
                                      thisChain, ir, thisRoiStr ) );
      }

    } // close selectedRois loop

  } // close TrkAnalyses loop 

  if( m_writeOut ) {
    ATH_MSG_DEBUG( m_trkAnaInfoWriteTool->printInfo( outTrkAnaInfoContHandle ) );
  }

  return StatusCode::SUCCESS;
}


///------------------------------
///------- procHistograms -------
///------------------------------
StatusCode InDetTrackPerfMonTool::procHistograms() {

  ATH_MSG_INFO( "Finalizing plots" );

  if( endOfRunFlag() ) {
    for( size_t iAna=0 ; iAna < m_trkAnaPlotsMgrVec.size() ; iAna++ ) {
      m_trkAnaPlotsMgrVec[iAna]->finalize();
    }
  }

  ATH_MSG_INFO( "Successfully finalized hists" );

  return StatusCode::SUCCESS;
}


///---------------------------
///----- loadCollections -----
///---------------------------
StatusCode InDetTrackPerfMonTool::loadCollections( IDTPM::TrackAnalysisCollections& trkAnaColls ) {

  ATH_MSG_INFO( "Loading collections" );

  /// Events
  ATH_CHECK( trkAnaColls.fillEventInfo(
      m_eventInfoContainerName, m_truthEventName, m_truthPileUpEventName ) );

  /// Tracks
  ATH_CHECK( trkAnaColls.fillTruthPartContainer( m_truthParticleName ) );
  ATH_CHECK( trkAnaColls.fillOfflTrackContainer( m_offlineTrkParticleName ) );
  ATH_CHECK( trkAnaColls.fillTrigTrackContainer( m_triggerTrkParticleName ) );

  /// Vertices
  ATH_CHECK( trkAnaColls.fillTruthVertexContainer( m_truthVertexContainerName ) );
  ATH_CHECK( trkAnaColls.fillOfflVertexContainer( m_offlineVertexContainerName ) );
  ATH_CHECK( trkAnaColls.fillTrigVertexContainer( m_triggerVertexContainerName ) );

  return StatusCode::SUCCESS;
}
