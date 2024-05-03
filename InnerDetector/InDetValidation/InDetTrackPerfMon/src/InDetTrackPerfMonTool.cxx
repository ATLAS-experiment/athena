/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
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
/// TODO - To be included in later MRs
//#include "xAODTruth/TruthVertex.h"
//#include "xAODTruth/TruthEventContainer.h"
//#include "xAODTruth/TruthPileupEvent.h"
//#include "xAODTruth/TruthPileupEventContainer.h"
//#include "xAODTruth/TruthPileupEventAuxContainer.h"
//#include "xAODJet/JetContainer.h"
//#include "TrkTrack/TrackCollection.h"

/// Athena
/// TODO - To be included in later MRs
//#include "AtlasDetDescr/AtlasDetectorID.h"
//#include "InDetIdentifier/PixelID.h"
//#include "InDetIdentifier/SCT_ID.h"
//#include "InDetIdentifier/TRT_ID.h"
//#include "TRT_ReadoutGeometry/TRT_DetectorManager.h"

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
  ManagedMonitorToolBase( type, name, parent ), 
  m_trkAnaDefSvc( nullptr ) { }


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
    ISvcLocator* svcLoc = Gaudi::svcLocator();
    ATH_CHECK( svcLoc->service( "TrkAnaDefSvc"+m_anaTag.value(), m_trkAnaDefSvc ) );
  }

  ATH_MSG_DEBUG( "Initializing sub-tools" );

  ATH_CHECK( m_trigDecTool.retrieve( EnableTool{ m_trkAnaDefSvc->useTrigger() } ) );
  ATH_CHECK( m_roiSelectionTool.retrieve( EnableTool{ m_trkAnaDefSvc->useTrigger() } ) );
  ATH_CHECK( m_trackRoiSelectionTool.retrieve( EnableTool{ m_trkAnaDefSvc->useTrigger() } ) );
  ATH_CHECK( m_trackMatchingTool.retrieve( EnableTool{ m_doMatch.value() } ) );

  ATH_CHECK( m_eventInfoContainerName.initialize() );

  ATH_CHECK( m_offlineTrkParticleName.initialize( 
      m_trkAnaDefSvc->useOffline() and not m_offlineTrkParticleName.key().empty() ) );
  ATH_CHECK( m_triggerTrkParticleName.initialize( 
      m_trkAnaDefSvc->useTrigger() and not m_triggerTrkParticleName.key().empty() ) );
  //ATH_CHECK( m_offlineVertexContainerName.initialize( not m_offlineVertexContainerName.empty() ) );

  ATH_CHECK( m_truthParticleName.initialize( 
      m_trkAnaDefSvc->useTruth() and not m_truthParticleName.key().empty() ) );
  /// TODO - To be included in later MRs
  //ATH_CHECK( m_truthVertexContainerName.initialize( not m_truthVertexContainerName.key().empty() ) );
  //ATH_CHECK( m_truthEventName.initialize( not m_truthEventName.key().empty() ) );
  //ATH_CHECK( m_truthPileUpEventName.initialize( not m_truthPileUpEventName.key().empty() ) );

  /// TODO - To be included in later MRs
  /// Retrieving list of configured chains
  /*std::vector<std::string> configuredChains = m_trkAnaDefSvc->configuredChains();
  m_trkAnaPlotsMgrVec.reserve( configuredChains.size() );

  /// booking analyses
  for( size_t ic=0 ; ic<configuredChains.size() ; ic++ ) { 

    ATH_MSG_INFO( "Booking TrkAnalysis/histograms for chain : " << configuredChains.at(ic) );

    /// Instantiating a different TrkAnalysis object (with corresponding histograms) 
    /// for every configured chain
    m_trkAnaPlotsMgrVec.emplace_back(
        std::make_unique< TrackAnalysisPlotsMgr >( nullptr, 
            m_dirName.value() + m_trkAnaDefSvc->subFolder(),
            configuredChains.at(ic), m_anaTag.value() ) );
  } // close m_configuredChains loop 
  */

  return StatusCode::SUCCESS;
}


///------------------------------
///------- bookHistograms -------
///------------------------------
StatusCode InDetTrackPerfMonTool::bookHistograms() {

  ATH_MSG_INFO( "Booking hists " << name() << " with detailed level: " << m_detailLevel );

  /// TODO - to be included in later MRs
  /*
  for( size_t iAna=0 ; iAna<m_trkAnaPlotsMgrVec.size() ; iAna++ ) {

    /// initialising/booking histograms
    m_trkAnaPlotsMgrVec.at(iAna)->initialize();

    /// Register booked histogram to corresponding monitoring group
    /// Register "plain" histograms (including TH1/2/3 and TProfiles)
    std::vector<HistData> hists = m_trkAnaPlotsMgrVec.at(iAna)->retrieveBookedHistograms();
    for ( size_t ih=0 ; ih<hists.size() ; ih++ ) {
      ATH_CHECK( regHist( hists.at(ih).first, hists.at(ih).second, all ) );
    }

    // do the same for Efficiencies, but there's a twist:
    std::vector<EfficiencyData> effs = m_trkAnaPlotsMgrVec.at(iAna)->retrieveBookedEfficiencies();
    for ( size_t ie=0 ; ie<effs.size() ; ie++ ) {
      ATH_CHECK( regEfficiency( effs.at(ie).first, MonGroup(this, effs.at(ie).second, all) ) );
      //ATH_CHECK( regGraph( reinterpret_cast<TGraph*>( effs.at(ie).first ), effs.at(ie).second, all));
    }
//    for (auto& eff : effs) {
//      // reg**** in the monitoring baseclass doesnt have a TEff version, but TGraph *
//      // pointers just get passed through, so we use that method after an ugly cast
//      ATH_CHECK(regGraph(reinterpret_cast<TGraph*>(eff.first), eff.second, all)); // ??
//    }

//    // register trees for ntuple writing
//    if (m_fillTruthToRecoNtuple) {
//      std::vector<TreeData> trees = m_trkAnaPlotsMgrVec.at(iAna)->retrieveBookedTrees();
//      for (auto& t : trees) {
//        ATH_CHECK(regTree(t.first, t.second, all));
//      }
//    }

  } // closing loop over TrkAnalyses
*/
  
  return StatusCode::SUCCESS;
}


/// ------------------------------
/// ------- fillHistograms -------
/// ------------------------------
StatusCode InDetTrackPerfMonTool::fillHistograms() {

  ATH_MSG_INFO( "Filling hists " << name() << " ..." );

  /// Defining TrackAnalysisCollections object
  /// to contain all collections for this event
  IDTPM::TrackAnalysisCollections thisTrkAnaCollections( m_anaTag.value() );
  ATH_CHECK( thisTrkAnaCollections.initialize() );

  SG::ReadHandle<xAOD::EventInfo> pie = SG::ReadHandle<xAOD::EventInfo>( m_eventInfoContainerName );

  /// filling TrackAnalysisCollections
  ATH_CHECK( loadCollections( thisTrkAnaCollections ) );

  /// FIXME - some debug printouts - to remove after R&D is done
  ATH_MSG_DEBUG( "Processing event = " << pie->eventNumber() <<
                 "\n==========================================" );
  ATH_MSG_DEBUG( "ALL Track Info: " << thisTrkAnaCollections.printInfo() );

  /// skip event if overall test/reference track vectors are empty
  if( thisTrkAnaCollections.empty() ) {
    ATH_MSG_DEBUG( "Some FULL collections are empty. Skipping event." );
    return StatusCode::SUCCESS;
  }

  /// ------------------------------
  /// --- Track quality selector ---
  /// ------------------------------
  /// Track-quality-based selection
  ATH_CHECK( m_trackQualitySelectionTool->selectTracks( thisTrkAnaCollections ) );

  /// skip event if overall test/reference track vectors are empty
  if( thisTrkAnaCollections.empty( IDTPM::TrackAnalysisCollections::FS ) ) {
    ATH_MSG_DEBUG( "Some collections are empty after quality selection. Skipping event." );
    return StatusCode::SUCCESS;
  }

  /// -------------------------------------------
  /// -- Main loop over configured TrkAnalyses --
  /// -------------------------------------------
  /// contains only the "dummy Offline" chain for offline analysis
  /// TODO - To be included in later MRs
  //for( size_t iAna=0 ; iAna<m_trkAnaPlotsMgrVec.size() ; iAna++ ) {
    //std::string thisChain = m_trkAnaPlotsMgrVec.at(iAna)->chain();

  for( const std::string& thisChain : m_trkAnaDefSvc->configuredChains() ) {

    ATH_MSG_DEBUG( "Processing chain = " << thisChain );

    /// ----------------------------------
    /// --------- Chain selector ---------
    /// ----------------------------------
    unsigned decisionType = TrigDefs::Physics; // TrigDefs::includeFailedDecisions;

    /// skipping TrkAnalysis if chain is not passed for this event
    if( !thisChain.empty() and thisChain != "Offline" and m_trkAnaDefSvc->useTrigger() ) {
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

    if( m_trkAnaDefSvc->useTrigger() ) {
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
      if( m_trkAnaDefSvc->useTrigger() ) thisRoiLink = selectedRois.at(ir).link;
      const TrigRoiDescriptor* const* thisRoi = m_trkAnaDefSvc->useTrigger() ? 
                                                thisRoiLink.cptr() : nullptr;
      std::string thisRoiStr = m_trkAnaDefSvc->useTrigger() ?
                               std::string( **thisRoi ) : "Full Scan";

      ATH_MSG_DEBUG( "Processing selected RoI : " << thisRoiStr );

      /// ----------------------------------
      /// --- Track selection within RoI ---
      /// ----------------------------------
      if( m_trkAnaDefSvc->useTrigger() ) {
        /// Tracks in RoI selection
        ATH_CHECK( m_trackRoiSelectionTool->selectTracksInRoI(
                                thisTrkAnaCollections, thisRoiLink ) );
      } else {
        /// No RoI selection required. Copying FullScan vectors
        thisTrkAnaCollections.copyFS();
      }

      /// checking if track collections are empty
      if( thisTrkAnaCollections.empty( IDTPM::TrackAnalysisCollections::InRoI ) ) {
        ATH_MSG_DEBUG( "Some collections are empty after RoI selection. Skipping event." );
        continue;
      }

      /// -------------------------------
      /// --- Test/Reference Matching ---
      /// -------------------------------
      std::string chainRoIName = thisChain;
      if( m_trkAnaDefSvc->useTrigger() ) chainRoIName += "_RoI_"+std::to_string(ir);

      if( m_doMatch.value() ) {
        ATH_MSG_DEBUG( "Doing Test-Reference matching..." );
        ATH_CHECK( m_trackMatchingTool->match( thisTrkAnaCollections,
                                               chainRoIName, thisRoiStr ) );
      }

      /// --------------------------
      /// --- Filling histograms ---
      /// --------------------------
      /// TODO - To be included in later MRs
      //ATH_CHECK( m_trkAnaPlotsMgrVec.at(iAna)->fill( thisTrkAnaCollections ) );

    } // close selectedRois loop

  } // close TrkAnalyses loop 

  return StatusCode::SUCCESS;
}


///------------------------------
///------- procHistograms -------
///------------------------------
StatusCode InDetTrackPerfMonTool::procHistograms() {

  ATH_MSG_INFO( "Finalizing hists " << name() << "..." );

  /// TODO - To be included in later MRs
  /*if( endOfRunFlag() ) {
    for( size_t iAna=0 ; iAna<m_trkAnaPlotsMgrVec.size() ; iAna++ ) {
      m_trkAnaPlotsMgrVec.at(iAna)->finalize();
    }
  }*/

  ATH_MSG_INFO( "Successfully finalized hists" );

  return StatusCode::SUCCESS;
}


///---------------------------
///----- loadCollections -----
///---------------------------
StatusCode InDetTrackPerfMonTool::loadCollections( IDTPM::TrackAnalysisCollections& trkAnaColls ) {

  ATH_MSG_INFO( "Loading collections" );
  ATH_CHECK( trkAnaColls.fillTruthPartContainer( m_truthParticleName ) );
  ATH_CHECK( trkAnaColls.fillOfflTrackContainer( m_offlineTrkParticleName ) );
  ATH_CHECK( trkAnaColls.fillTrigTrackContainer( m_triggerTrkParticleName ) );

  return StatusCode::SUCCESS;
}
