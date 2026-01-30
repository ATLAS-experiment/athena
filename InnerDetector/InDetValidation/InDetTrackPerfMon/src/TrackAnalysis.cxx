/**
 **   @file    TrackAnalysis.cxx         
 ** 
 **   @author sutt
 **   @date   Mon 17 Nov 2025 21:09:16 GMT
 **
 **   $Id: TrackAnalysis.cxx, v0.0   Mon 17 Nov 2025 21:09:16 GMT sutt $
 **
 **   Copyright (C) 2025 sutt (sutt@cern.ch)    
 **   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 **/


/// local include
#include "InDetTrackPerfMon/TrackAnalysis.h"

/// gaudi includes
#include "GaudiKernel/SystemOfUnits.h"

/// EDM includes
#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTruth/TruthParticleContainer.h"

#include "TrigInDetAnalysisExample/ChainString.h"

/// STL includes
#include <algorithm>
#include <limits>
#include <cmath> // to get std::isnan(), std::abs etc.
#include <utility>
#include <cstdlib> // to getenv


///----------------------------------------
///------- Parametrized constructor -------
///----------------------------------------
IDTPM::TrackAnalysis::TrackAnalysis( const std::string& type, const std::string& name, const IInterface* parent ) :
  AthAlgTool( type, name, parent )
{ }


 


///----------------------------------
///------- Default destructor -------
///----------------------------------
IDTPM::TrackAnalysis::~TrackAnalysis() = default;


///--------------------------
///------- Initialize -------
///--------------------------
StatusCode IDTPM::TrackAnalysis::initialize() {

  std::cout << "name: " << name() << std::endl;

  std::string selectChain = name().substr(name().find("."),name().size());
  
  ChainString chainName = selectChain; //chainName.head();
  
  if ( chainName.tail()!="" )     m_testTracks = chainName.tail();
  if ( chainName.roi()!="" )      m_rois       = chainName.roi();
  if ( chainName.vtx()!="" )      m_vertices   = chainName.vtx();
  if ( chainName.element()!="" )  m_leg        = chainName.element();
  if ( chainName.extra()!="" )    m_extra      = chainName.extra();


  std::cout << "\ttrigger: " << m_trigger << " " << chainName.head() << std::endl; 
  std::cout << "\ttracks:  " << m_testTracks << std::endl;
  std::cout << "\trois:    " << m_rois       << std::endl;
  std::cout << "\tvtx:     " << m_vertices   << std::endl;
  std::cout << "\tleg:     " << m_leg     << std::endl;
  std::cout << "\textra:   " << m_extra   << std::endl;

  
  ATH_CHECK( m_trackQualitySelectionTool.retrieve() );

  ATH_CHECK( m_vertexQualitySelectionTool.retrieve() );

  ATH_CHECK( m_roiSelectionTool.retrieve() );

  // ATH_CHECK( m_roiSelectionTool.retrieve( EnableTool{ m_trkAnaDefSvc->doTrigNavigation() } ) );
 
  
  //  ATH_CHECK( AthAlgtool::initialize() );
 
  // /// Retrieving trkAnaDefSvc
  // if( ! m_trkAnaDefSvc ) {
  //   ATH_MSG_DEBUG( "Retrieving TrkAnaDefSvc" << m_anaTag.value() );
  //   m_trkAnaDefSvc = Gaudi::svcLocator()->service( "TrkAnaDefSvc"+m_anaTag.value() );
  //   ATH_CHECK( m_trkAnaDefSvc.isValid() );
  // }

  // ATH_MSG_DEBUG( "Initializing sub-tools" );

  // ATH_CHECK( m_trigDecTool.retrieve( EnableTool{ m_trkAnaDefSvc->doTrigNavigation() } ) );
  // ATH_CHECK( m_trackQualitySelectionTool.retrieve() );
  // ATH_CHECK( m_vertexQualitySelectionTool.retrieve() );
  // ATH_CHECK( m_roiSelectionTool.retrieve( EnableTool{ m_trkAnaDefSvc->doTrigNavigation() } ) );
  // ATH_CHECK( m_trackRoiSelectionTool.retrieve( EnableTool{ m_trkAnaDefSvc->doTrigNavigation() } ) );
  // ATH_CHECK( m_vertexRoiSelectionTool.retrieve( EnableTool{ m_trkAnaDefSvc->doTrigNavigation() } ) );
  // ATH_CHECK( m_trackMatchingTool.retrieve( EnableTool{ m_doMatch.value() } ) );
  // ATH_CHECK( m_trkAnaInfoWriteTool.retrieve( EnableTool{ m_writeOut.value() } ) );

  // ATH_MSG_DEBUG( "Initializing collections" );

  // /// Events
  // ATH_CHECK( m_eventInfoContainerName.initialize() );
  // ATH_CHECK( m_truthEventName.initialize(
  //     m_trkAnaDefSvc->useTruth() && ! m_truthEventName.key().empty() ) );
  // ATH_CHECK( m_truthPileUpEventName.initialize(
  //     m_trkAnaDefSvc->useTruth() && ! m_truthPileUpEventName.key().empty() and
  //     m_trkAnaDefSvc->hasFullPileupTruth() ) );

  // /// Tracks
  // ATH_CHECK( m_offlineTrkParticleName.initialize(
  //     m_trkAnaDefSvc->useOffline() && ! m_offlineTrkParticleName.key().empty() ) );
  // ATH_CHECK( m_triggerTrkParticleName.initialize(
  //     m_trkAnaDefSvc->useTrigger() && ! m_triggerTrkParticleName.key().empty() ) );
  // ATH_CHECK( m_truthParticleName.initialize(
  //     m_trkAnaDefSvc->useTruth() && ! m_truthParticleName.key().empty() ) );

  // /// Vertex
  // ATH_CHECK( m_offlineVertexContainerName.initialize( 
  //     m_trkAnaDefSvc->useOffline() && ! m_offlineVertexContainerName.key().empty() ) );
  // ATH_CHECK( m_triggerVertexContainerName.initialize(
  //     m_trkAnaDefSvc->useTrigger() && ! m_triggerVertexContainerName.key().empty() ) );
  // ATH_CHECK( m_truthVertexContainerName.initialize(
  //     m_trkAnaDefSvc->useTruth() && ! m_truthVertexContainerName.key().empty() ) );

  // /// TrkAnaInfo for AOD_IDTPM output
  // ATH_CHECK( m_trkAnaInfoKey.initialize() );

  // /// Retrieving list of configured chains
  // const std::vector< std::string >& configuredChains = m_trkAnaDefSvc->configuredChains();
  // m_trkAnaPlotsMgrVec.reserve( configuredChains.size() );

  // /// booking analyses
  // for( const std::string& thisChain : configuredChains ) {
  //   ATH_MSG_DEBUG( "Booking TrkAnalysis/histograms for chain : " << thisChain );

  //   /// Instantiating a different TrkAnalysis object (with corresponding histograms) 
  //   /// for every configured chain
  //   m_trkAnaPlotsMgrVec.emplace_back(
  //       std::make_unique< IDTPM::TrackAnalysisPlotsMgr >(
  //           m_trkAnaDefSvc->plotsFullDir( thisChain ),
  //           m_anaTag.value(),
  //           thisChain ) );
  // } // close m_configuredChains loop 

  return StatusCode::SUCCESS;
}


///------------------------------
///------- bookHistograms -------
///------------------------------
StatusCode IDTPM::TrackAnalysis::bookHistograms()
{
  // ATH_MSG_DEBUG( "Booking plots" );

  // for( size_t iAna=0 ; iAna < m_trkAnaPlotsMgrVec.size() ; iAna++ ) {

  //   /// initialising/booking histograms
  //   ATH_CHECK( m_trkAnaPlotsMgrVec[iAna]->initialize() );

  //   /// Register booked histogram to corresponding monitoring group
  //   /// Register "plain" histograms (including TH1/2/3 && TProfiles)
  //   std::vector< HistData > hists = m_trkAnaPlotsMgrVec[iAna]->retrieveBookedHistograms();
  //   for ( size_t ih=0 ; ih<hists.size() ; ih++ ) {
  //     ATH_CHECK( regHist( hists[ih].first, hists[ih].second, all ) );
  //   }

  //   // do the same for Efficiencies, but there's a twist:
  //   std::vector< EfficiencyData > effs = m_trkAnaPlotsMgrVec[iAna]->retrieveBookedEfficiencies();
  //   for ( size_t ie=0 ; ie<effs.size() ; ie++ ) {
  //     ATH_CHECK( regEfficiency( effs[ie].first, MonGroup( this, effs[ie].second, all ) ) );
  //   }

  // } // closing loop over TrkAnalyses
  
  return StatusCode::SUCCESS;
}


/// ------------------------------
/// ------- fillHistograms -------
/// ------------------------------
StatusCode IDTPM::TrackAnalysis::fillHistograms() {
  execute();
  return StatusCode::SUCCESS;
}


void IDTPM::TrackAnalysis::execute() {

  //  ATH_MSG_INFO("Filling hists " << name() << "\ttrigger: " << m_trigger.value() << " ...");

  //   ATH_MSG_INFO("Filling hists " << name() << "\ttrigger: " << m_tool.name() << " ...");

  std::cout << "TrackAnalysis:execute() " << name() << std::endl;
  
  IDTPM::TrackAnalysisCollections thisTrkAnaCollections("cock");

  //  ATH_CHECK( thisTrkAnaCollections.initialize() );

#if 0
  
  if ( ! thisTrkAnaCollections.initialize().isSuccess() ) return;

  /// filling TrackAnalysisCollections
  // ATH_CHECK( loadCollections( thisTrkAnaCollections ) );
  if ( ! loadCollections( thisTrkAnaCollections ).isSuccess() ) return;

  ATH_MSG_DEBUG( "Processing event = " << thisTrkAnaCollections.eventInfo()->eventNumber() << "\n==========================================" );
  ATH_MSG_DEBUG( "ALL Track Info: " << thisTrkAnaCollections.printInfo() );

  /// Check if overall test/reference track vectors are empty
  if( thisTrkAnaCollections.empty() ) {
    ATH_MSG_DEBUG( "Some FULL collections are empty." );
  }


  /// ------------------------------
  /// --- Track quality selector ---
  /// ------------------------------
  //  ATH_CHECK( m_trackQualitySelectionTool->selectTracks( thisTrkAnaCollections ) );
  if ( !m_trackQualitySelectionTool->selectTracks( thisTrkAnaCollections ).isSuccess() ) return;

  /// Check if overall test/reference track vectors are empty
  if( thisTrkAnaCollections.empty( IDTPM::TrackAnalysisCollections::FS ) ) {
    ATH_MSG_DEBUG( "Some collections are empty after quality selection." );
  }

#endif
  
  //  loadCollections( trkAnaColls ).isSuccess();
  
  // /// Output TrackAnalysisInfo container writing
  // SG::WriteHandle< xAOD::BaseContainer > outTrkAnaInfoContHandle( m_trkAnaInfoKey );
  // if( m_writeOut ) {
  //   ATH_CHECK( outTrkAnaInfoContHandle.record(
  //                 std::make_unique< xAOD::BaseContainer >(),
  //                 std::make_unique< xAOD::AuxContainerBase >() ) );
  // }

  // /// Defining TrackAnalysisCollections object
  // /// to contain all collections for this event
  // IDTPM::TrackAnalysisCollections thisTrkAnaCollections( m_anaTag.value() );
  // ATH_CHECK( thisTrkAnaCollections.initialize() );

  // /// filling TrackAnalysisCollections
  // ATH_CHECK( loadCollections( thisTrkAnaCollections ) );

  // ATH_MSG_DEBUG( "Processing event = " <<
  //                thisTrkAnaCollections.eventInfo()->eventNumber() <<
  //                "\n==========================================" );
  // ATH_MSG_DEBUG( "ALL Track Info: " << thisTrkAnaCollections.printInfo() );

  // /// Check if overall test/reference track vectors are empty
  // if( thisTrkAnaCollections.empty() ) {
  //   ATH_MSG_DEBUG( "Some FULL collections are empty." );
  // }

  // /// ------------------------------
  // /// --- Track quality selector ---
  // /// ------------------------------
  // /// does this retrieve the tracks && store in the m_trackQualitySelectionTool ??
  // /// of does it store them in the thisTrakAnaCollections ? 
  // /// if the former, this will ! be thread safe, we will need to
  // /// clone this m_trackQualitySelectionTool to be local to this call of the
  // /// algorithm, so that we do ! update any class variables that might be
  // /// over written by another thread 
  // ATH_CHECK( m_trackQualitySelectionTool->selectTracks( thisTrkAnaCollections ) );

  // /// Check if overall test/reference track vectors are empty
  // if( thisTrkAnaCollections.empty( IDTPM::TrackAnalysisCollections::FS ) ) {
  //   ATH_MSG_DEBUG( "Some collections are empty after quality selection." );
  // }

  // /// -------------------------------
  // /// --- Vertex quality selector ---
  // /// -------------------------------
  // ATH_CHECK( m_vertexQualitySelectionTool->selectVertices( thisTrkAnaCollections ) );

  // /// -------------------------------------------
  // /// -- Main loop over configured TrkAnalyses --
  // /// -------------------------------------------
  // /// one TrkAnalysis per configured chain (trigger only)
  // /// only one dummy chain (named "Offline") for offline analysis

  // /// Shouldn't we loop over the analyses, && then call 
  // /// eg    analysis->Fill(...);
  // /// && the Fill() method internally loops over the different sets
  // /// of histograms internally, rather than expose that here ?

  // /// where is the matching done ? In the selectTracks() method ? 
  
  // for( size_t iAna=0 ; iAna < m_trkAnaPlotsMgrVec.size() ; iAna++ ) {

  //   const std::string& thisChain = m_trkAnaPlotsMgrVec[iAna]->chain();
  //   ATH_MSG_DEBUG( "Processing chain = " << thisChain );

  //   /// ----------------------------------
  //   /// --------- Chain selector ---------
  //   /// ----------------------------------

  //   /// don't understand "doTrigNavigation()" as the navigation isn't "done" ir is
  //   /// used - should it really be called "useTrigNavigation()" ? does it just return
  //   /// a flag ?
    
  //   /// skipping TrkAnalysis if chain is ! passed for this event
  //   if( m_trkAnaDefSvc->doTrigNavigation() &&
  //       ! thisChain.empty() && thisChain != "Offline" ) {

  //     unsigned decisionType = TrigDefs::Physics; // TrigDefs::includeFailedDecisions;

  //     if( ! m_trigDecTool->isPassed( thisChain, decisionType ) ) {
  //       ATH_MSG_DEBUG( "Trigger chain " << thisChain << " is ! fired. Skipping" );
  //       continue;
  //     }
  //   }

  //   /// ----------------------------------
  //   /// ------ RoI getter/selector -------
  //   /// ----------------------------------
  //   std::vector< TrigCompositeUtils::LinkInfo< TrigRoiDescriptorCollection > > selectedRois;
  //   size_t selectedRoisSize(1); // by default only one "dummy" RoI, i.e. for offline analysis

  //   if( m_trkAnaDefSvc->doTrigNavigation() ) {
  //     selectedRois = m_roiSelectionTool->getRois( thisChain ); 
  //     selectedRoisSize = selectedRois.size();
  //   }

  //   /// ----------------------------------
  //   /// -- Main loop over selected RoIs --
  //   /// ----------------------------------
  //   /// Only one "dummy" RoI iteration for offline analysis
  //   for( size_t ir=0 ; ir<selectedRoisSize ; ir++ ) {

  //     /// clear collections in this RoI from previous iteration
  //     thisTrkAnaCollections.clear( IDTPM::TrackAnalysisCollections::InRoI );

  //     /// Getting RoI ElementLink
  //     ElementLink< TrigRoiDescriptorCollection > thisRoiLink;
  //     std::string thisRoiStr( "Full Scan" );
  //     if( m_trkAnaDefSvc->doTrigNavigation() ) {
  //       thisRoiLink = selectedRois.at(ir).link;

  //       /// skip non-valid RoI link
  //       if( ! thisRoiLink.isValid() ) {
  //         ATH_MSG_WARNING( "Found non-valid RoI ElementLink" );
  //         continue;
  //       }

  //       /// Updating RoI string
  //       thisRoiStr = std::string( **thisRoiLink.cptr() );
  //     }

  //     ATH_MSG_DEBUG( "Processing selected RoI : " << thisRoiStr );

  //     /// ---------------------------------------------------
  //     /// --- Track (and Vertex) selection within the RoI ---
  //     /// ---------------------------------------------------
  //     if( m_trkAnaDefSvc->doTrigNavigation() ) {
  //       /// Tracks in RoI selection
  //       ATH_CHECK( m_trackRoiSelectionTool->selectTracksInRoI(
  //                         thisTrkAnaCollections, thisRoiLink ) );

  //       /// Vertices in RoI selection
  //       ATH_CHECK( m_vertexRoiSelectionTool->selectVerticesInRoI(
  //                         thisTrkAnaCollections, thisRoiLink ) );
  //     } else {
  //       /// No RoI selection required. Copying FullScan vectors
  //       thisTrkAnaCollections.copyFS();
  //     }

  //     /// checking if track collections are empty
  //     if( thisTrkAnaCollections.empty( IDTPM::TrackAnalysisCollections::InRoI ) ) {
  //       ATH_MSG_DEBUG( "Some collections are empty after RoI selection." );
  //     }

  //     /// -------------------------------
  //     /// --- Test/Reference Matching ---
  //     /// -------------------------------
  //     std::string chainRoIName = thisChain;
  //     if( m_trkAnaDefSvc->doTrigNavigation() ) {
  //       chainRoIName += "_RoI_"+std::to_string(ir);
  //     }

  //     if( m_doMatch.value() ) {
  //       ATH_MSG_DEBUG( "Doing Test-Reference matching..." );
  //       ATH_CHECK( m_trackMatchingTool->match( thisTrkAnaCollections,
  //                                              chainRoIName, thisRoiStr ) );
  //     }

  //     /// --------------------------
  //     /// --- Filling histograms ---
  //     /// --------------------------
  //     ATH_CHECK( m_trkAnaPlotsMgrVec[iAna]->fill( thisTrkAnaCollections ) );

  //     /// ---------------------------------------
  //     /// --- Writing trkAnaInfo to StoreGate ---
  //     /// ---------------------------------------
  //     if( m_writeOut ) {
  //       ATH_CHECK( m_trkAnaInfoWriteTool->write( outTrkAnaInfoContHandle,
  //                                                thisTrkAnaCollections,
  //                                                thisChain, ir, thisRoiStr ) );
  //     }

  //     thisTrkAnaCollections.newRoI();
  //   } // close selectedRois loop

  //   thisTrkAnaCollections.newChain();
  // } // close TrkAnalyses loop 

  // if( m_writeOut ) {
  //   ATH_MSG_DEBUG( m_trkAnaInfoWriteTool->printInfo( outTrkAnaInfoContHandle ) );
  // }

}


///------------------------------
///------- procHistograms -------
///------------------------------
StatusCode IDTPM::TrackAnalysis::procHistograms() {

  ATH_MSG_DEBUG( "Finalizing plots" );

  // if( endOfRunFlag() ) {
  //   for( size_t iAna=0 ; iAna < m_trkAnaPlotsMgrVec.size() ; iAna++ ) {
  //     m_trkAnaPlotsMgrVec[iAna]->finalize();
  //   }
  // }

  ATH_MSG_DEBUG( "Successfully finalized hists" );

  return StatusCode::SUCCESS;
}


template<typename T, typename S=T>
void loadCollections( TrackCollections<T,S>& trackCollections ) { }



///---------------------------
///----- loadCollections -----
///---------------------------
StatusCode IDTPM::TrackAnalysis::loadCollections( IDTPM::TrackAnalysisCollections& trkAnaColls ) {

  ATH_MSG_INFO( "Loading collections " << name() << "\ttrigger: " << m_trigger );

  
  // /// Events
  // ATH_CHECK( trkAnaColls.fillEventInfo(
  //     m_eventInfoContainerName, m_truthEventName, m_truthPileUpEventName ) );

// trkAnaTest
// trkAnaRef

// trkAnaRef->AddSelectopr(); 


  // /// Tracks
  // ATH_CHECK( trkAnaTest.fillTruthPartContainer( m_truthParticleName ) );
  // ATH_CHECK( trkAnaColls.fillOfflTrackContainer( m_offlineTrkParticleName ) );
  // ATH_CHECK( trkAnaColls.fillTrigTrackContainer( m_triggerTrkParticleName ) );

  // ATH_CHECK( trkAnaColls.fillTruthPartContainer( m_truthParticleName ) );
  // ATH_CHECK( trkAnaColls.fillOfflTrackContainer( m_offlineTrkParticleName ) );
  // ATH_CHECK( trkAnaColls.fillTrigTrackContainer( m_triggerTrkParticleName ) );

  // /// Vertices
  // ATH_CHECK( trkAnaColls.fillTruthVertexContainer( m_truthVertexContainerName ) );
  // ATH_CHECK( trkAnaColls.fillOfflVertexContainer( m_offlineVertexContainerName ) );
  // ATH_CHECK( trkAnaColls.fillTrigVertexContainer( m_triggerVertexContainerName ) );

  return StatusCode::SUCCESS;
}
