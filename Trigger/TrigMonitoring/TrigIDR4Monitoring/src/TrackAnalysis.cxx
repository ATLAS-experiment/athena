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

#include "InDetTrackPerfMon/TrackAnalysisDefinition.h"
#include "InDetTrackPerfMon/TrackAnalysisCollections.h"
#include "InDetTrackPerfMon/TrackParametersHelper.h"

#include "AthenaMonitoringKernel/Monitored.h"

#include "GaudiKernel/SystemOfUnits.h"

#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODTruth/TruthParticleContainer.h"

#include "TrigInDetAnalysisExample/ChainString.h"

#include "TrackAnalysis.h"

#include <algorithm>
#include <limits>
#include <cmath>
#include <utility>
#include <cstdlib> 


///----------------------------------------
///------- Parametrized constructor -------
///----------------------------------------
IDTPM::TrackAnalysis::TrackAnalysis( const std::string& type, const std::string& name, const IInterface* parent ) :
  AthAlgTool( type, name, parent ),
  m_tdt("Trig::TrigDecisionTool/TrigDecisionTool"),
  m_anal(0)
{ }


 


///----------------------------------
///------- Default destructor -------
///----------------------------------
IDTPM::TrackAnalysis::~TrackAnalysis() = default;


///--------------------------
///------- Initialize -------
///--------------------------
StatusCode IDTPM::TrackAnalysis::initialize() {

  ATH_MSG_INFO( "TrackAnalysis::initialize() name: " << name() );

  std::string selectChain = name().substr(name().find(".")+1,name().size());

  ChainString chainName = selectChain; //chainName.head();

  if ( chainName.head()!="" )     m_triggerchain = chainName.head();
  if ( chainName.tail()!="" )     m_testTracks   = chainName.tail();
  if ( chainName.roi()!="" )      m_rois         = chainName.roi();
  if ( chainName.vtx()!="" )      m_vertices     = chainName.vtx();
  if ( chainName.element()!="" )  m_leg          = chainName.element();
  if ( chainName.extra()!="" )    m_extra        = chainName.extra();

  /// we probably want to set these in the initialise
  /// rather than set them for every eveny
  m_refTracks  = m_offlineTracks;

  ATH_MSG_INFO( "TrackAnalysis::initialize()"
    << " trigger=" << m_trigger
    << " testTracks=" << m_testTracks
    << " refTracks="  << m_refTracks
    << " rois=" << m_rois );

  std::cout << "SUTT: tes tracks: " << m_testTracks << "\t" << m_triggerTracks << std::endl;

  std::cout << "SUTT: tes tracks: " << m_testTracks << "\t" << m_triggerTracks << std::endl;



  //m_testTracks = m_triggerTracks;

  ATH_CHECK( m_eventInfoName.initialize() );
  
  /// are these even needed ???
  
  m_testContainerName = SG::ReadHandleKey<xAOD::TrackParticleContainer>(m_testTracks);
  m_refContainerName  = SG::ReadHandleKey<xAOD::TrackParticleContainer>(m_refTracks);

  ATH_CHECK( m_testContainerName.initialize() );
  ATH_CHECK( m_refContainerName.initialize() );
  
  
  //// No, no, no, no, no, the TDT is configured in the algorithm, and the
  ///  same instance is passed into all the tools
  //   ATH_CHECK( m_tdt.retrieve() );
  
  //  ATH_CHECK( m_trackQualitySelectionTool.retrieve() );

  //  ATH_CHECK( m_vertexQualitySelectionTool.retrieve() );

  //  ATH_CHECK( m_roiSelectionTool.retrieve() );

  //  ATH_CHECK( m_trackRoiSelectionTool.retrieve() );

  //  ATH_CHECK( m_trackMatchingTool.retrieve() );

  std::unique_ptr<TrackAnalysisDefinition> config = std::make_unique<TrackAnalysisDefinition>();

  config->setTestCollection( m_testTracks );
  config->setReferenceCollection( m_refTracks );

  config->setTestType( "Trigger" );
  config->setReferenceType( "Offline" );

  /// these useTrigger/Offline etc flags are redundant, and should all
  /// be inferred from the track collection names, although they are
  /// not needed at all, the only thing needed is to workout what sort
  /// of tracks they are and then call the ReadHandle with th relevant
  /// type 
  config->setUseTrigger( true );
  config->setUseOffline( true );
  config->setDoTrigNavigation( true );

  /// this should be safe, even though we are taking a pointer
  /// to an automatic objec t that will go out of scope when this
  /// returns, because we want the entire processing of this event
  /// to take place from within this function
  m_trkAnaDef = std::move( config );
  

  
  // ATH_CHECK( m_vertexRoiSelectionTool.retrieve( EnableTool{ m_trkAnaDefSvc->doTrigNavigation() } ) );
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

  std::cout << "TrackAnalysis::initialize() exitting" << std::endl;

  m_anal = new AnalysisR4(name());

  m_anal->set_monTool( &m_tool );

  m_anal->initialise();
  
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
StatusCode IDTPM::TrackAnalysis::fillHistograms() { // const EventContext& /*ctx*/) {
  if ( execute() )  return StatusCode::SUCCESS;
  return StatusCode::FAILURE;
}


bool IDTPM::TrackAnalysis::execute() {

  //  ATH_MSG_INFO("Filling hists " << name() << "\ttrigger: " << m_trigger.value() << " ...");

  //   ATH_MSG_INFO("Filling hists " << name() << "\ttrigger: " << m_tool.name() << " ...");


  //  std::set<std::string> chains;


 
  unsigned decisionType = TrigDefs::Physics; // TrigDefs::includeFailedDecisions;

  if( !m_tdt->isPassed( m_triggerchain, decisionType ) ) return true;

  std::cout << "[91;1m PROCESSING ------------------------------------------------------------------------------------[m" << std::endl;

  ATH_MSG_INFO( "TrackAnalysis::execute() " << name()
    << " refTracks=" << m_refTracks
    << " testTracks=" << m_testTracks );

  std::cout << "trigger chain: " << m_triggerchain << std::endl;
  

  const std::vector<std::string> configuredChains = m_tdt->getListOfTriggers("HLT_.*");

  std::cout << "[91;1m" << configuredChains.size() << " Configured Chains" << "[m" << std::endl;

#if 0															    
  for ( unsigned i=0 ; i<configuredChains.size() ; i++ ) {
    if( m_tdt->isPassed( configuredChains[i], decisionType ) ) std::cout << i << "\t" << "[91;1m" << "Chain " << configuredChains[i] << " pass   (ACN)[m" << std::endl;
    if ( configuredChains[i] == m_triggerchain ) std::cout << "Mutha ..." << configuredChains[i] << " " <<  m_triggerchain << "\n\n" << std::endl;
  }
#endif

  std::cout << "trigger chain: " << m_triggerchain << " :: " << m_tdt->isPassed( m_triggerchain, decisionType ) << "\n" << std::endl;
  
  
  IDTPM::TrackAnalysisCollections thisTrkAnaCollections( "duff", m_trkAnaDef.get() );

  //  bool anacollections = thisTrkAnaCollections.initialize().isSuccess();

  //  std::cout << "\t ana collections: " << anacollections << std::endl;
  
  //  if ( !anacollections )  return false;

  
  /// filling TrackAnalysisCollections
  // ATH_CHECK( loadCollections( thisTrkAnaCollections ) );

  if ( ! loadCollections( thisTrkAnaCollections ).isSuccess() ) return false;

  ATH_MSG_INFO( "TrackAnalysis::execute() after loadCollections:"
    << " trigTracks[FULL]="  << thisTrkAnaCollections.trigTrackVec( IDTPM::TrackAnalysisCollections::FULL ).size()
    << " offlTracks[FULL]="  << thisTrkAnaCollections.offlTrackVec( IDTPM::TrackAnalysisCollections::FULL ).size()
    << " truthParts[FULL]="  << thisTrkAnaCollections.truthPartVec( IDTPM::TrackAnalysisCollections::FULL ).size() );

  m_anal->execute( thisTrkAnaCollections );

#if 0  
  // --- Fill reference track distributions ---
  const auto& refTracks = thisTrkAnaCollections.offlTrackVec( IDTPM::TrackAnalysisCollections::FULL );

  {
    auto n = Monitored::Scalar<float>("reftrk_N", static_cast<float>(refTracks.size()));
    Monitored::Group(m_tool, n);
  }

  for ( const xAOD::TrackParticle* trk : refTracks ) {
    if ( !trk ) continue;
    auto pt  = Monitored::Scalar<float>("reftrk_pT",  IDTPM::pT(*trk)  * 1e-3f);
    auto eta = Monitored::Scalar<float>("reftrk_eta", IDTPM::eta(*trk));
    auto phi = Monitored::Scalar<float>("reftrk_phi", IDTPM::phi(*trk));
    auto d0  = Monitored::Scalar<float>("reftrk_d0",  IDTPM::d0(*trk));
    auto z0  = Monitored::Scalar<float>("reftrk_z0",  IDTPM::z0(*trk));
    Monitored::Group(m_tool, pt, eta, phi, d0, z0);
  }

  // --- Fill test (trigger) track distributions ---
  const auto& testTracks = thisTrkAnaCollections.trigTrackVec(
      IDTPM::TrackAnalysisCollections::FULL );

  {
    auto n = Monitored::Scalar<float>("testtrk_N", static_cast<float>(testTracks.size()));
    Monitored::Group(m_tool, n);
  }

  for ( const xAOD::TrackParticle* trk : testTracks ) {
    if ( !trk ) continue;
    auto pt  = Monitored::Scalar<float>("testtrk_pT",  IDTPM::pT(*trk)  * 1e-3f);
    auto eta = Monitored::Scalar<float>("testtrk_eta", IDTPM::eta(*trk));
    auto phi = Monitored::Scalar<float>("testtrk_phi", IDTPM::phi(*trk));
    auto d0  = Monitored::Scalar<float>("testtrk_d0",  IDTPM::d0(*trk));
    auto z0  = Monitored::Scalar<float>("testtrk_z0",  IDTPM::z0(*trk));
    Monitored::Group(m_tool, pt, eta, phi, d0, z0);
  }
#endif


  
#if 0
  
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

  return true;
  
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

  /// won't bother with the vertices just yet ...
  /// eventually we want to replace this with the appropriate templated stuff

  ATH_CHECK( trkAnaColls.fillEventInfo( m_eventInfoName.key() ) );

  
  ATH_MSG_INFO( "TrackAnalysis::loadCollections() "
    << " refType="  << m_trkAnaDef->referenceType()
    << " refColl="  << m_trkAnaDef->referenceCollection()
    << " testType=" << m_trkAnaDef->testType()
    << " testColl=" << m_trkAnaDef->testCollection() );

#if 0
  
  /// even this could be handled more automatically, from the pointer types passed
  /// into a rational template TrackAnalysisCollections class
  if ( m_trkAnaDef->referenceType() == "Truth" ) {
    const xAOD::TruthParticleContainer* duff = 0; 
    ATH_CHECK( trkAnaColls.fill( duff, m_trkAnaDef->referenceCollection() ) );
  }
  else  {
    const xAOD::TrackParticleContainer* duff = 0;
    ATH_CHECK( trkAnaColls.fill( duff, m_trkAnaDef->referenceCollection() ) );
  }
  
  if ( m_trkAnaDef->testType() == "Truth" ) {
    const xAOD::TruthParticleContainer* duff = 0; 
    ATH_CHECK( trkAnaColls.fill( duff, m_trkAnaDef->testCollection() ) );
  }
  else {
    const xAOD::TrackParticleContainer* duff = 0;
    ATH_CHECK( trkAnaColls.fill( duff, m_trkAnaDef->testCollection() ) );
  }

 
#else

  if      ( m_trkAnaDef->referenceType() == "Offline" ) {
    ATH_MSG_INFO( "TrackAnalysis::loadCollections() filling ref offline container: " << m_trkAnaDef->referenceCollection() );
    ATH_CHECK( trkAnaColls.fillOfflTrackContainer( m_trkAnaDef->referenceCollection() ) );
  }
  else if ( m_trkAnaDef->referenceType() == "Trigger" ) {
    ATH_MSG_INFO( "TrackAnalysis::loadCollections() filling ref trigger container: " << m_trkAnaDef->referenceCollection() );
    ATH_CHECK( trkAnaColls.fillTrigTrackContainer( m_trkAnaDef->referenceCollection() ) );
  }
  else if ( m_trkAnaDef->referenceType() == "Truth" ) {
    ATH_MSG_INFO( "TrackAnalysis::loadCollections() filling ref truth container: " << m_trkAnaDef->referenceCollection() );
    ATH_CHECK( trkAnaColls.fillTruthPartContainer( m_trkAnaDef->referenceCollection() ) );
  }
  else {
    ATH_MSG_WARNING( "TrackAnalysis::loadCollections() unknown referenceType: " << m_trkAnaDef->referenceType() );
  }

  if      ( m_trkAnaDef->testType() == "Offline" ) {
    ATH_MSG_INFO( "TrackAnalysis::loadCollections() filling test offline container: " << m_trkAnaDef->testCollection() );
    ATH_CHECK( trkAnaColls.fillOfflTrackContainer( m_trkAnaDef->testCollection() ) );
  }
  else if ( m_trkAnaDef->testType() == "Trigger" ) {
    ATH_MSG_INFO( "TrackAnalysis::loadCollections() filling test trigger container: " << m_trkAnaDef->testCollection() );
    ATH_CHECK( trkAnaColls.fillTrigTrackContainer( m_trkAnaDef->testCollection() ) );
  }
  else if ( m_trkAnaDef->testType() == "Truth" ) {
    ATH_MSG_INFO( "TrackAnalysis::loadCollections() filling test truth container: " << m_trkAnaDef->testCollection() );
    ATH_CHECK( trkAnaColls.fillTruthPartContainer( m_trkAnaDef->testCollection() ) );
  }
  else {
    ATH_MSG_WARNING( "TrackAnalysis::loadCollections() unknown testType: " << m_trkAnaDef->testType() );
  }
#endif
  
  std::cout << "SUTT: done and dusted" << std::endl;
  
  return StatusCode::SUCCESS;
}
