/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file InDetTrackPerfMonTool.cxx
 * @author marco aparo
 **/

/// local include
#include "InDetTrackPerfMon/InDetTrackPerfMonTool.h"

#include "InDetTrackPerfMon/ITrackAnalysisDefinition.h"

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
  ManagedMonitorToolBase( type, name, parent ), m_trkAnaDef(0)
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

  std::cout << "SUTT IDTPM::anaTag " << m_anaTag.value() << std::endl;
  
  /// Retrievingm_trkAnaDefSvcSvc
  if( ! m_trkAnaDefSvc ) {
    ATH_MSG_DEBUG( "Retrieving TrkAnaDefSvc" << m_anaTag.value() );
    m_trkAnaDefSvc = Gaudi::svcLocator()->service( "TrkAnaDefSvc"+m_anaTag.value() );
    // not initialized ??
    ATH_CHECK( m_trkAnaDefSvc.isValid() );
  }

  m_trkAnaDef = m_trkAnaDefSvc->get();
  
  std::cout << "SUTT:m_trkAnaDefSvcSvc: " << m_trkAnaDefSvc << std::endl;
  
  ATH_MSG_INFO( "Initializing sub-tools" );

  std::cout << "SUTT " << name() << "\t nitiialising subtools" << std::endl; 
  
  ATH_CHECK( m_trigDecTool.retrieve( EnableTool{m_trkAnaDef->doTrigNavigation() } ) );
  ATH_CHECK( m_trackQualitySelectionTool.retrieve() );
  ATH_CHECK( m_vertexQualitySelectionTool.retrieve() );
  ATH_CHECK( m_roiSelectionTool.retrieve( EnableTool{m_trkAnaDef->doTrigNavigation() } ) );
  ATH_CHECK( m_trackRoiSelectionTool.retrieve( EnableTool{m_trkAnaDef->doTrigNavigation() } ) );
  ATH_CHECK( m_vertexRoiSelectionTool.retrieve( EnableTool{m_trkAnaDef->doTrigNavigation() } ) );
  ATH_CHECK( m_trackMatchingTool.retrieve( EnableTool{ m_doMatch.value() } ) );
  ATH_CHECK( m_trkAnaInfoWriteTool.retrieve( EnableTool{ m_writeOut.value() } ) );

  ATH_MSG_INFO( "Initializing collections" );

  std::cout << "SUTT " << name() << "\t nitiialising collections" << std::endl; 
  
  /// Events
  ATH_CHECK( m_eventInfoContainerName.initialize() );
  ATH_CHECK( m_truthEventName.initialize(
    m_trkAnaDef->useTruth() && ! m_truthEventName.key().empty() ) );
  ATH_CHECK( m_truthPileUpEventName.initialize(
    m_trkAnaDef->useTruth() && ! m_truthPileUpEventName.key().empty() and
    m_trkAnaDef->hasFullPileupTruth() ) );

  std::cout << "SUTT " << name() << "\t initiialising tracks" << std::endl; 

  /// Tracks

  std::cout << "SUTT useOffline " <<m_trkAnaDef->useOffline() << std::endl; 
  std::cout << "SUTT useTrigger " <<m_trkAnaDef->useTrigger() << std::endl; 
  std::cout << "SUTT useTruth   " <<m_trkAnaDef->useTruth()   << std::endl; 

  std::cout << "SUTT " << name() << "\t offline tracks:" << m_offlineTrkParticleName.key() << std::endl; 

  //  bool useOffline = m_trkAnaDefSvc->useOffline();;
  //  bool useOffline = t->useOffline();;
  
  //  std::cout << "useOffline bool: " << useOffline<< std::endl;
    
  //  if ( m_trkAnaDef->useOffline() && ! m_offlineTrkParticleName.key().empty() ) { 
    
  ATH_CHECK( m_offlineTrkParticleName.initialize(
    m_trkAnaDef->useOffline() && ! m_offlineTrkParticleName.key().empty() ) );

  std::cout << "SUTT " << name() << "\t offline tracks:" << m_triggerTrkParticleName.key() << std::endl; 
  
  ATH_CHECK( m_triggerTrkParticleName.initialize(
    m_trkAnaDef->useTrigger() && ! m_triggerTrkParticleName.key().empty() ) );

  std::cout << "SUTT " << name() << "\t offline tracks:" << m_truthParticleName.key() << std::endl; 

  ATH_CHECK( m_truthParticleName.initialize(
     m_trkAnaDef->useTruth() && ! m_truthParticleName.key().empty() ) );

  std::cout << "SUTT " << name() << "\t nitiialising vertices" << std::endl; 

  /// Vertex
  ATH_CHECK( m_offlineVertexContainerName.initialize( 
    m_trkAnaDef->useOffline() && ! m_offlineVertexContainerName.key().empty() ) );
  ATH_CHECK( m_triggerVertexContainerName.initialize(
    m_trkAnaDef->useTrigger() && ! m_triggerVertexContainerName.key().empty() ) );
  ATH_CHECK( m_truthVertexContainerName.initialize(
    m_trkAnaDef->useTruth() && ! m_truthVertexContainerName.key().empty() ) );

  std::cout << "SUTT " << name() << "\t infokey ..." << std::endl; 

  /// TrkAnaInfo for AOD_IDTPM output
  ATH_CHECK( m_trkAnaInfoKey.initialize() );

  /// Retrieving list of configured chains
  const std::vector< std::string >& configuredChains = m_trkAnaDefSvc->configuredChains();
  m_trkAnaPlotsMgrVec.reserve( configuredChains.size() );

  /// booking analyses
  for( const std::string& thisChain : configuredChains ) {
    std::cout <<  "SUTT " << name() << " Booking TrkAnalysis/histograms for chain : " << thisChain << std::endl;;

    ATH_MSG_DEBUG( "Booking TrkAnalysis/histograms for chain : " << thisChain );

    /// Instantiating a different TrkAnalysis object (with corresponding histograms) 
    /// for every configured chain
    m_trkAnaPlotsMgrVec.emplace_back(
        std::make_unique< IDTPM::TrackAnalysisPlotsMgr >(
            m_trkAnaDefSvc->plotsFullDir( thisChain ),
            m_anaTag.value(),
            thisChain ) );
  } // close m_configuredChains loop 

  std::cout << "SUTT " << name() << "\t done initialisation" << std::endl; 

  return StatusCode::SUCCESS;
}


///------------------------------
///------- bookHistograms -------
///------------------------------
StatusCode InDetTrackPerfMonTool::bookHistograms()
{
  ATH_MSG_DEBUG( "Booking plots" );

  for( size_t iAna=0 ; iAna < m_trkAnaPlotsMgrVec.size() ; iAna++ ) {

    /// initialising/booking histograms
    ATH_CHECK( m_trkAnaPlotsMgrVec[iAna]->initialize() );

    /// Register booked histogram to corresponding monitoring group
    /// Register "plain" histograms (including TH1/2/3 && TProfiles)
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

  ATH_MSG_DEBUG("Filling hists " << name() << " ...");

  /// Output TrackAnalysisInfo container writing
  SG::WriteHandle< xAOD::BaseContainer > outTrkAnaInfoContHandle( m_trkAnaInfoKey );
  if( m_writeOut ) {
    ATH_CHECK( outTrkAnaInfoContHandle.record(
                  std::make_unique< xAOD::BaseContainer >(),
                  std::make_unique< xAOD::AuxContainerBase >() ) );
  }

  /// Defining TrackAnalysisCollections object
  /// to contain all collections for this event
  IDTPM::TrackAnalysisCollections thisTrkAnaCollections( m_anaTag.value(), m_trkAnaDef );
  ATH_CHECK( thisTrkAnaCollections.initialize() );

  /// filling TrackAnalysisCollections
  ATH_CHECK( loadCollections( thisTrkAnaCollections ) );

  ATH_MSG_DEBUG( "Processing event = " <<
                 thisTrkAnaCollections.eventInfo()->eventNumber() <<
                 "\n==========================================" );

  ATH_MSG_DEBUG( "ALL Track Info: " << thisTrkAnaCollections.printInfo() );


  std::cout <<  "SUTT: ALL Track Info: " << thisTrkAnaCollections.printInfo() << std::endl;;

  
  /// Check if overall test/reference track vectors are empty
  if( thisTrkAnaCollections.empty() ) {
    ATH_MSG_WARNING( "Some FULL collections are empty." );
  }

  /// ------------------------------
  /// --- Track quality selector ---
  /// ------------------------------
  ATH_CHECK( m_trackQualitySelectionTool->selectTracks( thisTrkAnaCollections ) );

  
  /// Check if overall test/reference track vectors are empty
  if( thisTrkAnaCollections.empty( IDTPM::TrackAnalysisCollections::FS ) ) {
    ATH_MSG_WARNING( "Some collections are empty after quality selection." );
  }

  /// -------------------------------
  /// --- Vertex quality selector ---
  /// -------------------------------
  ATH_CHECK( m_vertexQualitySelectionTool->selectVertices( thisTrkAnaCollections ) );

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

    /// skipping TrkAnalysis if chain is ! passed for this event
    if( m_trkAnaDef->doTrigNavigation() && 
        ! thisChain.empty() && thisChain != "Offline" ) {

      unsigned decisionType = TrigDefs::Physics; // TrigDefs::includeFailedDecisions;

      if( ! m_trigDecTool->isPassed( thisChain, decisionType ) ) {
        ATH_MSG_DEBUG( "Trigger chain " << thisChain << " is ! fired. Skipping" );
        continue;
      }
    }

    /// ----------------------------------
    /// ------ RoI getter/selector -------
    /// ----------------------------------
    std::vector< TrigCompositeUtils::LinkInfo< TrigRoiDescriptorCollection > > selectedRois;
    size_t selectedRoisSize(1); // by default only one "dummy" RoI, i.e. for offline analysis

    if( m_trkAnaDef->doTrigNavigation() ) {
      selectedRois = m_roiSelectionTool->getRois( thisChain ); 
      selectedRoisSize = selectedRois.size();
    }

    /// ----------------------------------
    /// -- Main loop over selected RoIs --
    /// ----------------------------------
    /// Only one "dummy" RoI iteration for offline analysis

    /// Only one "dummy" RoI iteration for offline analysis

    std::cout << "SUTT: Roi size: " << selectedRoisSize << " " << selectedRois.size() << std::endl; 


    /// this isn is insane !! the selectedRoisize variable is NOT actually equal to the
    /// size of the selectRois vector. WTF ?
    for( size_t ir=selectedRoisSize ; ir-- ;  ) {
      //    for( size_t ir=selectedRois.size() ; ir-- ;  ) {

      /// clear collections in this RoI from previous iteration
      thisTrkAnaCollections.clear( IDTPM::TrackAnalysisCollections::InRoI );

      /// Getting RoI ElementLink
      ElementLink< TrigRoiDescriptorCollection > thisRoiLink;
      std::string thisRoiStr( "Full Scan" );
      if( m_trkAnaDef->doTrigNavigation() ) {
        thisRoiLink = selectedRois.at(ir).link;

        /// skip non-valid RoI link
        if( ! thisRoiLink.isValid() ) {
          ATH_MSG_WARNING( "Found non-valid RoI ElementLink" );
          continue;
        }

        /// Updating RoI string
        thisRoiStr = std::string( **thisRoiLink.cptr() );
      }

      ATH_MSG_DEBUG( "Processing selected RoI : " << thisRoiStr );

      /// ---------------------------------------------------
      /// --- Track (and Vertex) selection within the RoI ---
      /// ---------------------------------------------------
      if( m_trkAnaDef->doTrigNavigation() ) {
        /// Tracks in RoI selection
        ATH_CHECK( m_trackRoiSelectionTool->selectTracksInRoI(
                          thisTrkAnaCollections, thisRoiLink ) );

        /// Vertices in RoI selection
        ATH_CHECK( m_vertexRoiSelectionTool->selectVerticesInRoI(
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
      if( m_trkAnaDef->doTrigNavigation() ) {
        chainRoIName += "_RoI_"+std::to_string(ir);
      }

      if( m_doMatch.value() ) {
        ATH_MSG_DEBUG( "Doing Test-Reference matching..." );
        ATH_CHECK( m_trackMatchingTool->match( thisTrkAnaCollections, chainRoIName, thisRoiStr ) );
                                               
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

      thisTrkAnaCollections.newRoI();
    } // close selectedRois loop

    thisTrkAnaCollections.newChain();
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

  ATH_MSG_DEBUG( "Finalizing plots" );

  if( endOfRunFlag() ) {
    for( size_t iAna=0 ; iAna < m_trkAnaPlotsMgrVec.size() ; iAna++ ) {
      m_trkAnaPlotsMgrVec[iAna]->finalize();
    }
  }

  ATH_MSG_DEBUG( "Successfully finalized hists" );

  return StatusCode::SUCCESS;
}


///---------------------------
///----- loadCollections -----
///---------------------------
/// why is thuis a method of the InDetTrackPerfMonClass, rather than the
/// TrackAnalysisCollections class ?
/// If the parameters are being passed by the TrackAnalysisDefinitions(Svc)
/// then it makes sense for these *ContainerName (RHK ? or just strings ?)
/// to come from there also
StatusCode InDetTrackPerfMonTool::loadCollections( IDTPM::TrackAnalysisCollections& trkAnaColls ) {

  ATH_MSG_DEBUG( "Loading collections" );

  std::cout << "SUTT loadcollections: eventInfo " <<  m_eventInfoContainerName.key() << std::endl;

  std::cout << "SUTT loadcollections: truthPart " <<  m_truthParticleName.key() << std::endl;
  std::cout << "SUTT loadcollections: offTrack  " <<  m_offlineTrkParticleName.key() << std::endl;
  std::cout << "SUTT loadcollections: trigTrack " <<  m_triggerTrkParticleName.key() << std::endl;

  std::cout << "SUTT loadcollections: truthVertex " <<  m_truthVertexContainerName.key()  << std::endl;
  std::cout << "SUTT loadcollections: offVertex   " <<  m_offlineVertexContainerName.key() << std::endl;
  std::cout << "SUTT loadcollections: trigVertex  " <<  m_triggerVertexContainerName.key() << std::endl;

  std::cout << "SUTT: test: " << m_trkAnaDef->testType()      << "\t" << m_trkAnaDef->testTag()      << std::endl;
  std::cout << "SUTT: ref:  " << m_trkAnaDef->referenceType() << "\t" << m_trkAnaDef->referenceTag() << std::endl;

  std::cout << "SUTT: test coll: " << m_trkAnaDef->testCollection()      << std::endl;
  std::cout << "SUTT: ref coll:  " << m_trkAnaDef->referenceCollection() << std::endl;

  
  /// these track collections should perhaps be added to this configuration object - in fact all the
  /// configuration parameters that need to be passed around, should go there, then we don't need any
  /// class variables and the fetching could properly be moved to the TrackAnalysisCollection class
  /// where it arguably belongs, since that class fetches the collections already, so all they should
  /// need is the name of the collection

  /// Events
  ATH_CHECK( trkAnaColls.fillEventInfo( m_eventInfoContainerName, m_truthEventName, m_truthPileUpEventName ) );

  //// this is bad practice - trying to fetch truth, offline, and trigger even though
  ///  really only two should ever be defined (even i nthe case where we want
  ///  eg truth matched offline as the reference, then the truth matched part
  ///  should be in the offline selection, and not make it to the list of tracks
  ///  to be processed
  /// Tracks
#if 1
  ATH_CHECK( trkAnaColls.fillTruthPartContainer( m_truthParticleName ) );
  ATH_CHECK( trkAnaColls.fillOfflTrackContainer( m_offlineTrkParticleName ) );  /// why are some "Tracks" abreviated to "Trk" but not others ????
  ATH_CHECK( trkAnaColls.fillTrigTrackContainer( m_triggerTrkParticleName ) );
#else

  /// can get the names opf the track collections from the anaTrkDef, so really, should
  /// just be able to retrieve the test anmd reference collections directly,
  /// but can't be done like this yet

  /// notice here, the name we pass in is always the same, so the problem, it that we have different
  /// methods and different variables in the class for trigger, offline or truth particles
  /// if we more sensibly had only the test and reference collections, we would
  /// always only need
  ///
  ///     tc.fetchRefCollection( m_trkAnaDef->referenceCollection() );
  ///     tc.fetchTestCollection( m_trkAnaDef->testCollection() );
  ///
  /// and everything else afterwards would be a lot simpler
  
  if      ( m_trkAnaDef->referenceType() == "Offline" )  ATH_CHECK( trkAnaColls.fillOfflTrackContainer( m_trkAnaDef->referenceCollection() ) );
  else if ( m_trkAnaDef->referenceType() == "Trigger" )  ATH_CHECK( trkAnaColls.fillTrigTrackContainer( m_trkAnaDef->referenceCollection() ) ); 
  else if ( m_trkAnaDef->referenceType() == "Truth" )    ATH_CHECK( trkAnaColls.fillTruthPartContainer( m_trkAnaDef->referenceCollection() ) );
  
  if      ( m_trkAnaDef->testType() == "Offline" )  ATH_CHECK( trkAnaColls.fillOfflTrackContainer( m_trkAnaDef->testCollection() ) );
  else if ( m_trkAnaDef->testType() == "Trigger" )  ATH_CHECK( trkAnaColls.fillTrigTrackContainer( m_trkAnaDef->testCollection() ) ); 
  else if ( m_trkAnaDef->testType() == "Truth" )    ATH_CHECK( trkAnaColls.fillTruthPartContainer( m_trkAnaDef->testCollection() ) );

#endif

  /// Vertices
  ATH_CHECK( trkAnaColls.fillTruthVertexContainer( m_truthVertexContainerName ) );
  ATH_CHECK( trkAnaColls.fillOfflVertexContainer( m_offlineVertexContainerName ) );
  ATH_CHECK( trkAnaColls.fillTrigVertexContainer( m_triggerVertexContainerName ) );

  return StatusCode::SUCCESS;
}
