/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Header include
#include "VrtSecInclusive/VrtSecInclusive.h"
#include "VrtSecInclusive/NtupleVars.h"

#include "VxVertex/VxTrackAtVertex.h"
#include "GeneratorObjects/HepMcParticleLink.h"

// FrameWork include(s):
#include "StoreGate/WriteDecorHandle.h"

// ROOT Classes
#include "TMath.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TNtuple.h"
#include "TTree.h"
#include "TROOT.h"

#include <chrono>
#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <new>
#include <string>
#include <tuple>

using namespace std;


namespace VKalVrtAthena {

  //Constructor and destructor
  //__________________________________________________________________________
  VrtSecInclusive::VrtSecInclusive(const std::string& name, ISvcLocator* pSvcLocator):
    AthAlgorithm                   (name,pSvcLocator),
    m_ntupleVars                   ( nullptr )
  {
    m_patternStrategyFuncs["Classical"]           = &VrtSecInclusive::checkTrackHitPatternToVertex;
    m_patternStrategyFuncs["ClassicalOuter"]      = &VrtSecInclusive::checkTrackHitPatternToVertexOuterOnly;
    m_patternStrategyFuncs["Extrapolation"]       = &VrtSecInclusive::checkTrackHitPatternToVertexByExtrapolation;
    m_patternStrategyFuncs["ExtrapolationAssist"] = &VrtSecInclusive::checkTrackHitPatternToVertexByExtrapolationAssist;

    if( m_FillNtuple ) {
      m_ntupleVars = std::make_unique<NtupleVars>( );
    }
  }

  
  //__________________________________________________________________________
  VrtSecInclusive::~VrtSecInclusive()
  {
  }

  //__________________________________________________________________________
  StatusCode VrtSecInclusive::initialize()
  {
    ATH_MSG_INFO("initialize: begin");
    //
    // first instantiate tools

    //  VKalVrt vertex fitter
    if (m_fitSvc.retrieve().isFailure()) {
      ATH_MSG_ERROR("initialize: Can't find Trk::TrkVKalVrtFitter");
      return StatusCode::SUCCESS;
    } else {
      ATH_MSG_INFO("initialize: Trk::TrkVKalVrtFitter found");
    }

    //
    // retreive tool to get trackParameters of generated Particles
    if(m_truthToTrack.retrieve().isFailure()) {
      ATH_MSG_INFO("initialize: Cannot retrieve Trk::TruthToTrack Tool!");
      return StatusCode::FAILURE;
    }
    else {
      ATH_MSG_INFO("initialize: Retrieved Trk::TruthToTrack Tool" << m_truthToTrack);

    }
    // extract TrackToVertex extrapolator tool
    if ( m_trackToVertexTool.retrieve().isFailure() ) {
      ATH_MSG_ERROR("initialize: failed to retrieve trackToVertex tool ");
      return StatusCode::SUCCESS;
    }
    else {
      ATH_MSG_INFO("initialize: Retrieved Reco::TrackToVertex Tool" << m_trackToVertexTool);
    }
    // extract TrackToVertexIPEstimator extrapolator tool
    if ( m_trackToVertexIPEstimatorTool.retrieve().isFailure() ) {
      ATH_MSG_ERROR("initialize: failed to retrieve trackToVertexIPEstimator tool ");
      return StatusCode::SUCCESS;
    }
    else {
      ATH_MSG_INFO("initialize: Retrieved Trk::TrackToVertexIPEstimator Tool" << m_trackToVertexIPEstimatorTool);
    }

    if( detStore()->retrieve(m_atlasId, "AtlasID").isFailure() ) return StatusCode::FAILURE;
    if( detStore()->retrieve(m_pixelId, "PixelID").isFailure() ) return StatusCode::FAILURE;
    if( detStore()->retrieve(m_sctId,   "SCT_ID") .isFailure() ) return StatusCode::FAILURE;

    if ( m_pixelCondSummaryTool.retrieve().isFailure() ) {
      ATH_MSG_ERROR("initialize: failed to retrieve PixelConditionsSummaryTool");
      return StatusCode::SUCCESS;
    }
    else {
      ATH_MSG_INFO("initialize: Retrieved PixelConditionsSummaryTool" << m_pixelCondSummaryTool);
    }
    if ( m_sctCondSummaryTool.retrieve().isFailure() ) {
      ATH_MSG_ERROR("initialize: failed to retrieve SCTConditionsSummaryTool");
      return StatusCode::SUCCESS;
    }
    else {
      ATH_MSG_INFO("initialize: Retrieved SCTConditionsSummaryTool" << m_sctCondSummaryTool);
    }

    ATH_CHECK( m_extrapolator.retrieve() );

    // extract VertexMapper
    if( m_doMapToLocal ) {
      ATH_CHECK( m_vertexMapper.retrieve() );
    }

    // Track selection algorithm configuration
    if( m_doSelectTracksFromMuons )     { m_trackSelectionAlgs.emplace_back( &VrtSecInclusive::selectTracksFromMuons );     }
    if( m_doSelectTracksFromElectrons ) { m_trackSelectionAlgs.emplace_back( &VrtSecInclusive::selectTracksFromElectrons ); }
    if( m_doSelectIDAndGSFTracks )      { m_trackSelectionAlgs.emplace_back( &VrtSecInclusive::selectInDetAndGSFTracks );   }

    // if none of the above two flags are activated, use ID tracks (default)
    if( !m_doSelectTracksFromMuons && !m_doSelectTracksFromElectrons && !m_doSelectIDAndGSFTracks) {

      m_trackSelectionAlgs.emplace_back( &VrtSecInclusive::selectTracksInDet );

    }


    // Vertexing algorithm configuration
    m_vertexingAlgorithms.emplace_back( "extractIncompatibleTrackPairs", &VrtSecInclusive::extractIncompatibleTrackPairs     );
    m_vertexingAlgorithms.emplace_back( "findNtrackVertices",            &VrtSecInclusive::findNtrackVertices                );
    m_vertexingAlgorithms.emplace_back( "rearrangeTracks",               &VrtSecInclusive::rearrangeTracks                   );

    if( m_doReassembleVertices ) {
      m_vertexingAlgorithms.emplace_back( "reassembleVertices",          &VrtSecInclusive::reassembleVertices                );
    }

    if( m_doMergeByShuffling ) {
      m_vertexingAlgorithms.emplace_back( "mergeByShuffling",           &VrtSecInclusive::mergeByShuffling                   );
    }

    if ( m_doMergeFinalVerticesDistance ) {
      m_vertexingAlgorithms.emplace_back( "mergeFinalVertices",          &VrtSecInclusive::mergeFinalVertices                );
    }

    if( m_doAssociateNonSelectedTracks ) {
      m_vertexingAlgorithms.emplace_back( "associateNonSelectedTracks",  &VrtSecInclusive::associateNonSelectedTracks        );
    }

    m_vertexingAlgorithms.emplace_back( "refitAndSelect",                &VrtSecInclusive::refitAndSelectGoodQualityVertices );


    // now make histograms/ntuples

    ServiceHandle<ITHistSvc> hist_root("THistSvc", name());
    ATH_CHECK( hist_root.retrieve() );

    if( m_FillHist ) {

      std::vector<double> rbins = { 0.1, 0.3, 0.5, 1, 2, 3, 5, 7, 10, 14, 20, 28, 38, 50, 64, 80, 100, 130, 170, 220, 280, 350, 450, 600 };
      std::vector<double> nbins = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 16, 20, 24, 28, 38, 50, 70, 100, 150 };

      const size_t& nAlgs = m_vertexingAlgorithms.size();

      ATH_MSG_INFO("initialize: Filling Histograms");
      //
      m_hists["trkSelCuts"]        = new TH1F("trkSelCuts",        ";Cut Order;Tracks",                         10, -0.5, 10-0.5                                         );
      m_hists["selTracksDist"]     = new TH1F("selTracksDist",     ";Selected Tracks;Events",                   2000, -0.5, 2000-0.5                                     );
      m_hists["initVertexDispD0"]  = new TH2F("initVertexDispD0",  ";Rough d0 wrt init [mm];r [mm];Vertices",   1000, -100, 100, rbins.size()-1, &(rbins[0])             );
      m_hists["initVertexDispZ0"]  = new TH2F("initVertexDispZ0",  ";Rough z0 wrt init [mm];z [mm];Vertices",   1000, -100, 100, 100, -1000, 1000                        );
      m_hists["incompMonitor"]     = new TH1F("incompMonitor",     ";Setp;Track Pairs",                         10, -0.5, 10-0.5                                         );
      m_hists["2trkVerticesDist"]  = new TH1F("2trkVerticesDist",  ";2-track Vertices;Events",                  1000, -0.5, 1000-0.5                                     );
      m_hists["2trkChi2Dist"]      = new TH1F("2trkChi2Dist",      ";log10(#chi^{2}/N_{dof});Entries",          100, -3, 7                                               );
      m_hists["NtrkChi2Dist"]      = new TH1F("NtrkChi2Dist",      ";log10(#chi^{2}/N_{dof});Entries",          100, -3, 7                                               );
      m_hists["vPosDist"]          = new TH2F("vPosDist",          ";r;#vec{x}*#vec{p}/p_{T} [mm]",             rbins.size()-1, &(rbins[0]), 200, -1000, 1000            );
      m_hists["vPosMomAngTDist"]   = new TH2F("vPosMomAngDistT",   ";r;cos(#vec{r},#vec{p}_{T})",               rbins.size()-1, &(rbins[0]), 200, -1.0, 1.0              );
      m_hists["disabledCount"]     = new TH1F("disabledCount",     ";N_{modules};Tracks",                       20, -0.5, 10-0.5                                         );
      m_hists["vertexYield"]       = new TH1F("vertexYield",       ";Algorithm Step;Vertices",                  nAlgs, -0.5, nAlgs-0.5                                   );
      m_hists["vertexYieldNtrk"]   = new TH2F("vertexYieldNtrk",   ";Ntrk;Algorithm Step;Vertices",             100, 0, 100, nAlgs, -0.5, nAlgs-0.5                      );
      m_hists["vertexYieldChi2"]   = new TH2F("vertexYieldChi2",   ";#chi^{2}/N_{dof};Algorithm Step;Vertices", 100, 0, 100, nAlgs, -0.5, nAlgs-0.5                      );
      m_hists["mergeType"]         = new TH1F("mergeType",         ";Merge Algorithm Type;Entries",             10, -0.5, 10-0.5                                         );
      m_hists["associateMonitor"]  = new TH1F("associateMonitor",  ";Step;Vertices",                            10, -0.5, 10-0.5                                         );
      m_hists["shuffleMinSignif1"] = new TH1F("shuffleMinSignif1", ";Min( log_{10}( Significance ) );Vertices", 100, -3, 5                                               );
      m_hists["shuffleMinSignif2"] = new TH1F("shuffleMinSignif2", ";Min( log_{10}( Significance ) );Vertices", 100, -3, 5                                               );
      m_hists["shuffleMinSignif3"] = new TH1F("shuffleMinSignif3", ";Min( log_{10}( Significance ) );Vertices", 100, -3, 5                                               );
      m_hists["finalCutMonitor"]   = new TH1F("finalCutMonitor",   ";Step;Vertices",                            6, -0.5, 6-0.5                                           );
      m_hists["finalVtxNtrk"]      = new TH1F("finalVtxNtrk",      ";N_{trk};Vertices",                         nbins.size()-1, &(nbins[0])                              );
      m_hists["finalVtxR"]         = new TH1F("finalVtxR",         ";r [mm];Vertices",                          600, 0, 600                                              );
      m_hists["finalVtxNtrkR"]     = new TH2F("finalVtxNtrkR",     ";N_{trk};r [mm];Vertices",                  nbins.size()-1, &(nbins[0]), rbins.size()-1, &(rbins[0]) );
      m_hists["CPUTime"]           = new TH1F("CPUTime",           ";Step;Accum. CPU Time [s]",                 10, -0.5, 10-0.5                                         );
      m_hists["nMatchedTruths"]    = new TH2F("nMatchedTruths",    ";Step;;r [mm];Matched truth vertices",      11, -0.5, 11-0.5, rbins.size()-1, &(rbins[0])            );
      m_hists["vPosMomAngT"]       = new TH1F("vPosMomAngT",       ";cos(#vec{r},#vec{p}_{T}",                  200, -1.0, 1.0                                           );
      m_hists["vPosMomAng3D"]      = new TH1F("vPosMomAng3D",      ";cos(#vec{r},#vec{p})",                     200, -1.0, 1.0                                           );
      m_hists["2trkVtxDistFromPV"] = new TH1F("2trkVtDistFromPV",  ";2tr vertex distance from PV;Events",       100, 0, 3                                                );


      std::string histDir("/AANT/VrtSecInclusive" + m_augVerString + "/");

      for( auto& pair : m_hists ) {
        ATH_CHECK( hist_root->regHist( histDir + pair.first, pair.second ) );
      }
    }


    if( m_FillNtuple ) {

      ATH_CHECK( setupNtupleVariables() );

      m_tree_Vert = new TTree("tree_VrtSecInclusive","TTree of VrtSecInclusive");
      ATH_CHECK( hist_root->regTree("/AANT/tree_VrtSecInclusive", m_tree_Vert) );

      ATH_CHECK( setupNtuple() );

    }

    // initialize keys
    ATH_CHECK(m_eventInfoKey.initialize());

    // Instantiate and initialize our event info decorator write
    m_vertexingStatusKey = "VrtSecInclusive_" + m_secondaryVerticesContainerName + m_augVerString + "_status";
    ATH_CHECK( m_vertexingStatusKey.initialize() );
    
    m_vertexKey = "VrtSecInclusive_" + m_secondaryVerticesContainerName + m_augVerString;
    m_twoTrksVertexKey = "VrtSecInclusive_" + m_all2trksVerticesContainerName + m_augVerString;
    ATH_CHECK( m_vertexKey.initialize() );
    ATH_CHECK( m_twoTrksVertexKey.initialize(m_FillIntermediateVertices) );
    m_vertexCollectionsDefinitions[m_vertexKey.key()] = false;
    if (m_FillIntermediateVertices) m_vertexCollectionsDefinitions[m_twoTrksVertexKey.key()] = false;
    
    if ( m_FillIntermediateVertices ) {
      for( auto itr = m_vertexingAlgorithms.begin(); itr!=m_vertexingAlgorithms.end(); ++itr ) {
	const std::string& nameAlgo = itr->first;
	std::string fullName = "VrtSecInclusive_IntermediateVertices_" + nameAlgo + m_augVerString;
	m_intermediateVertexKey[nameAlgo] = SG::WriteHandleKey<xAOD::VertexContainer>( this, nameAlgo, fullName );
	ATH_CHECK( m_intermediateVertexKey[nameAlgo].initialize() );
	m_vertexCollectionsDefinitions[m_intermediateVertexKey[nameAlgo].key()] = false;
      }
    }

    
    ATH_CHECK( m_TrackLocation.initialize() );
    ATH_CHECK( m_MuonLocation.initialize() );
    ATH_CHECK( m_ElectronLocation.initialize() );
    ATH_CHECK( m_PrimVrtLocation.initialize() );
    
    //
    ATH_MSG_INFO("initialize: Exit VrtSecInclusive::initialize()");
    return StatusCode::SUCCESS;
  }


  StatusCode VrtSecInclusive::dummyVertexContainer(const EventContext& ctx,
						   const SG::WriteHandleKey<xAOD::VertexContainer>& handleKey)
  {
    SG::WriteHandle<xAOD::VertexContainer> vHandle = SG::makeHandle( handleKey, ctx );
    ATH_CHECK( vHandle.record( std::make_unique<xAOD::VertexContainer>(),
			       std::make_unique<xAOD::VertexAuxContainer>() ) );
    m_vertexCollectionsDefinitions.at(handleKey.key()) = true;
    return StatusCode::SUCCESS;
  }
  
  StatusCode VrtSecInclusive::defineDummyCollections(const EventContext& ctx)
  {
    if ( not m_vertexCollectionsDefinitions.at(m_vertexKey.key()) ) {
      ATH_CHECK( dummyVertexContainer(ctx, m_vertexKey) );
    }

    if (m_FillIntermediateVertices) {
      if (not m_vertexCollectionsDefinitions.at( m_twoTrksVertexKey.key() )) {
	ATH_CHECK( dummyVertexContainer(ctx, m_twoTrksVertexKey) );
      }

      for (const auto& [nameAlgo, handleKey] : m_intermediateVertexKey) {
	if (not m_vertexCollectionsDefinitions.at( handleKey.key() )) {
	  ATH_CHECK( dummyVertexContainer(ctx, handleKey) );
	}
      }
    }
    
    return StatusCode::SUCCESS;
  }


  //__________________________________________________________________________
  StatusCode VrtSecInclusive::initEvent()
  {

    ATH_MSG_DEBUG("initEvent: begin");

    // Clear all variables to be stored to the AANT
    if( m_FillNtuple ) {
      ATH_CHECK( clearNtupleVariables() );
    }


    ATH_MSG_DEBUG("initEvent: from initEvent ");
    return StatusCode::SUCCESS;

  }


  //__________________________________________________________________________
  StatusCode VrtSecInclusive::execute(const EventContext& ctx)
  {
    
    //
    ATH_MSG_DEBUG("VrtSecInclusive execute()");

    // Reset values
    for (auto& [key, val] : m_vertexCollectionsDefinitions) {
      val = false;
    }
    
    m_vertexingStatus = -1;

    SG::ReadHandle<xAOD::EventInfo> eventInfo = SG::makeHandle(m_eventInfoKey, ctx);
    if (!eventInfo.isValid()) {
      ATH_MSG_ERROR ("Could not retrieve EventInfo");
      return StatusCode::FAILURE;
    }

    SG::WriteDecorHandle<xAOD::EventInfo,int> vertexingStatusDecor(m_vertexingStatusKey, ctx);

    // clear ntuple variables
    StatusCode sc = this->initEvent();
    if(sc.isFailure()) {
      ATH_MSG_WARNING("Problem in initEvent ");
      vertexingStatusDecor(*eventInfo) = m_vertexingStatus;
      ATH_CHECK(defineDummyCollections(ctx));
      return StatusCode::SUCCESS;
    }

    // add event level info to ntuple
    if( m_FillNtuple ) sc = addEventInfo();

    if (sc.isFailure() ) {
      ATH_MSG_WARNING("Failure in getEventInfo() ");
      vertexingStatusDecor(*eventInfo) = m_vertexingStatus;
      ATH_CHECK(defineDummyCollections(ctx));
      return StatusCode::SUCCESS;
    }


    ///////////////////////////////////////////////////////////////////////////
    //
    // Setup StoreGate Variables
    //

    // Check Return StatusCode::Failure if the user-specified container names have duplication.
    {
      std::vector<std::string> userContainerNames { m_secondaryVerticesContainerName, m_all2trksVerticesContainerName };
      std::set<std::string> userContainerNamesSet;
      for( auto& name : userContainerNames ) userContainerNamesSet.insert( name );
      if( userContainerNamesSet.size() != userContainerNames.size() ) {
        ATH_MSG_ERROR( " > " << __FUNCTION__ << ": detected duplicated user-specified container name. Please check your job property" );
        return StatusCode::FAILURE;
      }
    }


    dumpTruthInformation();


    // Later use elsewhere in the algorithm
    m_selectedTracks.clear();
    m_associatedTracks.clear();
    m_leptonicTracks.clear();

    m_extrapolatedPatternBank.clear();

    ///////////////////////////////////////////////////////////////////////////
    //
    // now start algorithm
    //

    //--------------------------------------------------------
    //  Primary vertex processing
    //
    sc = this->processPrimaryVertices(); // fetch the 1st primary reconstructed vertex

    if( sc.isFailure() or !m_thePV ) {

      ATH_MSG_WARNING("processPrimaryVertices() failed");
      vertexingStatusDecor(*eventInfo) = m_vertexingStatus;
      ATH_CHECK(defineDummyCollections(ctx));
      return StatusCode::SUCCESS;
    }

    // Perform track selection and store it to selectedBaseTracks
    for( auto alg : m_trackSelectionAlgs ) {
      ATH_CHECK( (this->*alg)(ctx) );
    }

    if( m_FillNtuple )
      m_ntupleVars->get<unsigned int>( "NumSelTrks" ) = static_cast<int>( m_selectedTracks.size() );

    // fill information about selected tracks in AANT
    ATH_CHECK( fillAANT_SelectedBaseTracks() );

    //-------------------------------------------------------
    // Skip the event if the number of selected tracks is more than m_SelTrkMaxCutoff
    if( m_selectedTracks.size() < 2 ) {
      ATH_MSG_DEBUG( "execute: Too few (<2) selected reco tracks. Terminated reconstruction." );
      m_vertexingStatus = 1;
      vertexingStatusDecor(*eventInfo) = m_vertexingStatus;
      ATH_CHECK( lockTrackDecorations( true, ctx ) );
      ATH_CHECK(defineDummyCollections(ctx));
      return StatusCode::SUCCESS;
    }

    if( m_selectedTracks.size() > m_SelTrkMaxCutoff ) {
      ATH_MSG_INFO( "execute: Too many selected reco tracks. Terminated reconstruction." );
      m_vertexingStatus = 2;
      vertexingStatusDecor(*eventInfo) = m_vertexingStatus;
      ATH_CHECK( lockTrackDecorations( true, ctx ) );
      ATH_CHECK(defineDummyCollections(ctx));
      return StatusCode::SUCCESS;
    }

    //-------------------------------------------------------
    // Core part of Vertexing
    //

    {

      m_vertexingAlgorithmStep = 0;

      // set of vertices created in the following while loop.
      std::vector<WrkVrt> workVerticesContainer;

      // the main sequence of the main vertexing algorithms
      // see initialize() what kind of algorithms exist.
      for( auto itr = m_vertexingAlgorithms.begin(); itr!=m_vertexingAlgorithms.end(); ++itr ) {

        auto& name = itr->first;
        auto alg   = itr->second;

        auto t_start = std::chrono::system_clock::now();

        ATH_CHECK( (this->*alg)( ctx, &workVerticesContainer ) );

        auto t_end = std::chrono::system_clock::now();

        if( m_FillHist ) {
          auto sec = std::chrono::duration_cast<std::chrono::microseconds>( t_end - t_start ).count();
          m_hists["CPUTime"]->Fill( m_vertexingAlgorithmStep, sec/1.e6 );
        }

        std::erase_if( workVerticesContainer,
                       []( WrkVrt& wrkvrt ) {
                         return ( !wrkvrt.isGood || wrkvrt.nTracksTotal() < 2 ); }
                       );

        ATH_CHECK( monitorVertexingAlgorithmStep( ctx, &workVerticesContainer, name, std::next( itr ) == m_vertexingAlgorithms.end() ) );

        m_vertexingAlgorithmStep++;

      }
    }

    m_vertexingStatus = 0;
    vertexingStatusDecor(*eventInfo) = m_vertexingStatus;

    // Fill AANT
    if( m_FillNtuple ) {
      m_tree_Vert->Fill();
      ATH_CHECK( clearNtupleVariables() );
    }

    ATH_CHECK( lockTrackDecorations( false, ctx ) );

    ATH_MSG_VERBOSE( "execute: process done." );
    // end
    ATH_CHECK(defineDummyCollections(ctx));
    return StatusCode::SUCCESS;

  }

  void VrtSecInclusive::lockTrackDecorations( const xAOD::TrackParticle* trk, bool onlySelection ) const {
    SG::AuxVectorData* cont_nc ATLAS_THREAD_SAFE =
      const_cast<SG::AuxVectorData*> (trk->container());
    cont_nc->lockDecoration (m_decor_isSelected->auxid());

    if (onlySelection) return;

    if (m_decor_isAssociated && m_decor_isAssociated->isAvailable (*cont_nc)) {
      cont_nc->lockDecoration (m_decor_isAssociated->auxid());
    }
    if (m_decor_is_svtrk_final && m_decor_is_svtrk_final->isAvailable (*cont_nc)) {
      cont_nc->lockDecoration (m_decor_is_svtrk_final->auxid());
    }

    for (const auto& p : m_trkDecors) {
      cont_nc->lockDecoration (p.second.auxid());
    }
  }

  void VrtSecInclusive::lockLeptonDecorations( const SG::AuxVectorData* cont ) const {
    SG::AuxVectorData* cont_nc ATLAS_THREAD_SAFE =
      const_cast<SG::AuxVectorData*> (cont);
    for (const IPDecoratorType& dec : m_ipDecors) {
      if (dec.isAvailable (*cont)) {
        cont_nc->lockDecoration (dec.auxid());
      }
    }

    if (m_decor_svLink) {
      if (m_decor_svLink->isAvailable (*cont)) {
        cont_nc->lockDecoration (m_decor_svLink->auxid());
      }
    }
  }

  StatusCode VrtSecInclusive::lockTrackDecorations( bool onlySelection,
						    const EventContext& ctx ) const
  {
    SG::ReadHandle<xAOD::TrackParticleContainer> trackParticleHandle = SG::makeHandle( m_TrackLocation, ctx );
    ATH_CHECK( trackParticleHandle.isValid() );
    const xAOD::TrackParticleContainer* trackParticleContainer = trackParticleHandle.cptr();
    for( const xAOD::TrackParticle* trk : *trackParticleContainer ) {
      lockTrackDecorations( trk, onlySelection );
    }

    SG::ReadHandle<xAOD::MuonContainer> muonsHandle = SG::makeHandle( m_MuonLocation, ctx );
    ATH_CHECK( muonsHandle.isValid() );
    const xAOD::MuonContainer* muons = muonsHandle.cptr();
    if (muons->ownPolicy() != SG::VIEW_ELEMENTS) {
      lockLeptonDecorations (muons);
    }
    for( const xAOD::Muon* muon : *muons ) {
      if (muons->ownPolicy() == SG::VIEW_ELEMENTS) {
        lockLeptonDecorations (muon->container());
      }
      if ( const xAOD::TrackParticle* trk = muon->trackParticle( xAOD::Muon::InnerDetectorTrackParticle ) ) {
        lockTrackDecorations( trk, onlySelection );
      }
    }

    SG::ReadHandle<xAOD::ElectronContainer> electronsHandle = SG::makeHandle( m_ElectronLocation, ctx);
    ATH_CHECK( electronsHandle.isValid() );
    const xAOD::ElectronContainer *electrons = electronsHandle.cptr();
    if (electrons->ownPolicy() != SG::VIEW_ELEMENTS) {
      lockLeptonDecorations (electrons);
    }
    for( const xAOD::Electron* electron : *electrons ) {
      if (electrons->ownPolicy() == SG::VIEW_ELEMENTS) {
        lockLeptonDecorations (electron->container());
      }
      if( electron->nTrackParticles() > 0 ) {
        if (const xAOD::TrackParticle* trk = electron->trackParticle(0)) {
          lockTrackDecorations( trk, onlySelection );
        }
      }
    }

    SG::ReadHandle<xAOD::TrackParticleContainer> IDtracksHandle = SG::makeHandle( m_TrackLocation, ctx );
    ATH_CHECK( IDtracksHandle.isValid() );
    const xAOD::TrackParticleContainer* IDtracks = IDtracksHandle.cptr();
    for( const auto *trk : *IDtracks ) {
      lockTrackDecorations( trk, onlySelection );
    }

    return StatusCode::SUCCESS;
  }

} // end of namespace bracket
