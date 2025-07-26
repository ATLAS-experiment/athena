/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigBjetBtagHypoAlg.h"
#include "EventPrimitives/EventPrimitivesHelpers.h"
#include "AthContainers/ConstAccessor.h"
#include "TrigBjetHypo/safeLogRatio.h"

#include <cmath>

TrigBjetBtagHypoAlg::TrigBjetBtagHypoAlg( const std::string& name, 
						ISvcLocator* pSvcLocator ) : 
  TrigBjetHypoAlgBase( name, pSvcLocator ) {}


StatusCode TrigBjetBtagHypoAlg::initialize() {

  ATH_CHECK( m_hypoTools.retrieve() );
  if ( !m_monTool.empty() ) CHECK( m_monTool.retrieve() );

  ATH_CHECK( m_bTaggedJetKey.initialize() );
  ATH_CHECK( m_bTagKey.initialize() );
  ATH_CHECK( m_trackKey.initialize() );
  ATH_CHECK( m_inputPrmVtx.initialize() );

  renounce( m_bTaggedJetKey );
  renounce( m_bTagKey );
  renounce( m_trackKey );
  renounce( m_inputPrmVtx );

  ATH_CHECK(m_beamSpotKey.initialize());

  return StatusCode::SUCCESS;
}


StatusCode TrigBjetBtagHypoAlg::execute( const EventContext& context ) const {
  ATH_MSG_DEBUG ( "Executing " << name() << "..." );

  // ========================================================================================================================== //    ** Retrieve Ingredients 
  // ========================================================================================================================== 

  // Read in previous Decisions made before running this Hypo Alg.
  const TrigCompositeUtils::DecisionContainer *prevDecisionContainer = nullptr;
  CHECK( retrievePreviousDecisionContainer( context,prevDecisionContainer ) );
  ATH_MSG_DEBUG( "Running with "<< prevDecisionContainer->size() <<" previous decisions");

  // Retrive Precision tracks from Event Views. We get them all in this way!
  ElementLinkVector< xAOD::TrackParticleContainer > trackELs;
  CHECK( retrieveObjectFromEventView( context,trackELs,m_trackKey,prevDecisionContainer ) );
  ATH_MSG_DEBUG( "Retrieved " << trackELs.size() << " precision tracks..." );


  for ( const ElementLink< xAOD::TrackParticleContainer > trackLink : trackELs )
    ATH_MSG_DEBUG( "   * pt=" << (*trackLink)->p4().Et() << 
		   " eta=" << (*trackLink)->eta() <<
		   " phi=" << (*trackLink)->phi() );


  // Retrieve Jets
  ElementLinkVector< xAOD::JetContainer > jetELs;
  CHECK( retrieveCollectionFromNavigation( TrigCompositeUtils::featureString(), jetELs, prevDecisionContainer ) );  
  ATH_MSG_DEBUG( "Retrieved " << jetELs.size() << " Jets of key " << TrigCompositeUtils::featureString() );

  // online monitoring for tracks, per decision
  CHECK( monitor_tracks( context, prevDecisionContainer ) );

  // Retrieve Vertices
  ElementLinkVector< xAOD::VertexContainer > vertexELs;
  CHECK( retrieveObjectFromStoreGate( context,vertexELs,m_inputPrmVtx ) );
  ATH_MSG_DEBUG( "Retrieved " << vertexELs.size() <<" vertices..." );

  // opnline monitoring for vertex
  auto monitor_for_vertex_count = Monitored::Scalar( "vertex_count", vertexELs.size() );  


  // ==========================================================================================================================
  //    ** Prepare Outputs
  // ==========================================================================================================================

  // Decisions
  SG::WriteHandle< TrigCompositeUtils::DecisionContainer > handle = TrigCompositeUtils::createAndStore( decisionOutput(), context );
  TrigCompositeUtils::DecisionContainer *outputDecisions = handle.ptr();

  // ========================================================================================================================== 
  //    ** Prepare Decisions
  // ========================================================================================================================== 
  
  const unsigned int nDecisions = prevDecisionContainer->size();

  // Create output decisions
  ATH_MSG_DEBUG("Creating Output Decisions and Linking Stuff to it");
  std::vector< TrigCompositeUtils::Decision* > newDecisions;
  ElementLinkVector< xAOD::JetContainer > all_bTaggedJetELs;
  for ( unsigned int index(0); index<nDecisions; index++ ) {
    TrigCompositeUtils::Decision *toAdd = TrigCompositeUtils::newDecisionIn( outputDecisions,
									     prevDecisionContainer->at(index),
									     TrigCompositeUtils::hypoAlgNodeName(), context );

    // Attaching links to the output decisions
    // Retrieved jets from view on which we have run flavour tagging
    ElementLinkVector< xAOD::JetContainer > bTaggedJetEL;
    CHECK( retrieveCollectionFromView( context,
				       bTaggedJetEL,
				       m_bTaggedJetKey,
				       prevDecisionContainer->at(index) ) );

    if ( bTaggedJetEL.size() != 1 ) {
      ATH_MSG_ERROR( "Did not find only 1 b-tagged jet object from View!" );
      return StatusCode::FAILURE;
    }

    toAdd->setObjectLink< xAOD::JetContainer >( TrigCompositeUtils::featureString(),bTaggedJetEL.front() );     
    all_bTaggedJetELs.push_back( bTaggedJetEL.front() );



    // Retrieve Flavour Tagging object from view
    ElementLinkVector< xAOD::BTaggingContainer > bTaggingEL;
    CHECK( retrieveCollectionFromView( context,
				       bTaggingEL,
				       m_bTagKey,
				       prevDecisionContainer->at(index) ) );
    
    if ( bTaggingEL.size() != 1 ) {
      ATH_MSG_ERROR( "Did not find only 1 b-tagging object from View!" );
      return StatusCode::FAILURE;
    }

    toAdd->setObjectLink< xAOD::BTaggingContainer >(m_btaggingLinkName,bTaggingEL.front() ); // TM 2021-10-30

    // online monitoring for btagging, with a check to ensure the PV is marked
    ElementLink< xAOD::VertexContainer > vertexEL;
    CHECK( retrieveObjectFromNavigation(  m_prmVtxLink.value(), vertexEL, prevDecisionContainer->at(index) ) );
        
    if ( (*vertexEL)->vertexType() == xAOD::VxType::VertexType::PriVtx ) {
      CHECK( monitor_primary_vertex( vertexEL ) );
      CHECK( monitor_btagging( bTaggingEL ) );
    }




    // Add to Decision collection
    newDecisions.push_back( toAdd );
  }

  // online monitoring for jets
  auto monitor_for_jet_count = Monitored::Scalar( "jet_count", jetELs.size() );
  CHECK( monitor_jets( jetELs, all_bTaggedJetELs ) );

  auto monitor_group_for_events = Monitored::Group( m_monTool, monitor_for_jet_count, monitor_for_vertex_count );



  //// online monitoring for jets
  //auto monitor_for_jet_count = Monitored::Scalar( "jet_count", jetELs.size() );
  //CHECK( monitor_jets( jetELs ) );


  // ==========================================================================================================================
  //    ** Prepare input to Hypo Tools  
  // ==========================================================================================================================

  SG::ReadCondHandle< InDet::BeamSpotData > beamSpotHandle(
    m_beamSpotKey, context);
  const InDet::BeamSpotData* beamSpot = beamSpotHandle.retrieve();

  std::vector< TrigBjetBtagHypoTool::TrigBjetBtagHypoToolInfo > bTagHypoInputs;

  for ( unsigned int index(0); index<nDecisions; index++ ) {

    // Extract the IDs of the b-jet chains which are active.
    // Previous decision IDs.
    const TrigCompositeUtils::Decision *previousDecision = prevDecisionContainer->at(index);

    TrigCompositeUtils::DecisionIDContainer previousDecisionIDs {
      TrigCompositeUtils::decisionIDs( previousDecision ).begin(),
      TrigCompositeUtils::decisionIDs( previousDecision ).end() };

    // Retrieve PV from navigation
    ElementLink< xAOD::VertexContainer > vertexEL;
    CHECK( retrieveObjectFromNavigation(  m_prmVtxLink.value(), vertexEL, previousDecision ) );

    // Retrieve b-tagging code
    ElementLinkVector< xAOD::BTaggingContainer > bTaggingELs;
    CHECK( retrieveCollectionFromView< xAOD::BTaggingContainer >( context,
								  bTaggingELs,
								  m_bTagKey,
								  previousDecision ) );
    CHECK( bTaggingELs.size() == 1 );

    // Put everything in place
    TrigBjetBtagHypoTool::TrigBjetBtagHypoToolInfo infoToAdd;
    infoToAdd.previousDecisionIDs = previousDecisionIDs;
    infoToAdd.btaggingEL = bTaggingELs.front();
    infoToAdd.vertexEL = vertexEL;
    infoToAdd.decision = newDecisions.at( index );
    infoToAdd.beamSpot = beamSpot;
    bTagHypoInputs.push_back( infoToAdd );
  }

  // ==========================================================================================================================

  // Run on Trigger Chains
  for ( const ToolHandle< TrigBjetBtagHypoTool >& tool : m_hypoTools ) 
    CHECK( tool->decide( bTagHypoInputs ) ); 
  
  ATH_MSG_DEBUG( "Exiting with " << handle->size() << " decisions" );
  ATH_MSG_DEBUG( "CHECKING OUTPUT DECISION HANDLE" );
  CHECK( hypoBaseOutputProcessing(handle) );

  // ==========================================================================================================================  

  return StatusCode::SUCCESS;
}

StatusCode TrigBjetBtagHypoAlg::monitor_jets( const ElementLinkVector<xAOD::JetContainer >& jetELs, const ElementLinkVector<xAOD::JetContainer >& all_bTaggedJetELs ) const {

  auto monitor_for_jet_pt = Monitored::Collection( "jet_pt", jetELs,
    [](const ElementLink< xAOD::JetContainer >& jetLink) { return (*jetLink)->pt() / 1000.0 /*Gev*/; } );
  auto monitor_for_jet_eta = Monitored::Collection( "jet_eta", jetELs,
    [](const ElementLink< xAOD::JetContainer >& jetLink) { return (*jetLink)->eta(); } );
  auto monitor_for_jet_phi = Monitored::Collection( "jet_phi", jetELs,
    [](const ElementLink< xAOD::JetContainer >& jetLink) { return (*jetLink)->phi(); } );

  auto monitor_for_Bjet_pt = Monitored::Collection( "Bjet_pt", all_bTaggedJetELs,
    [](const ElementLink< xAOD::JetContainer >& jetLink) { return (*jetLink)->pt() / 1000.0 /*Gev*/; } );
  auto monitor_for_Bjet_eta = Monitored::Collection( "Bjet_eta", all_bTaggedJetELs,
    [](const ElementLink< xAOD::JetContainer >& jetLink) { return (*jetLink)->eta(); } );
  auto monitor_for_Bjet_phi = Monitored::Collection( "Bjet_phi", all_bTaggedJetELs,
    [](const ElementLink< xAOD::JetContainer >& jetLink) { return (*jetLink)->phi(); } );

  for( unsigned int i = 0; i < jetELs.size(); i++ ) {
    const ElementLink< xAOD::JetContainer >& jetLink = jetELs[i];
    const ElementLink< xAOD::JetContainer >& BjetLink = all_bTaggedJetELs[i];
    auto monitor_for_jet_delta_pt = Monitored::Scalar( "jet_bjet_delta_pt", (*jetLink)->pt() - (*BjetLink)->pt() );
    auto monitor_for_jet_delta_eta = Monitored::Scalar( "jet_bjet_delta_eta", (*jetLink)->eta() - (*BjetLink)->eta() );
    auto monitor_for_jet_delta_phi = Monitored::Scalar( "jet_bjet_delta_phi", (*jetLink)->phi() - (*BjetLink)->phi() );
    auto monitor_group_for_jet_deltas = Monitored::Group( m_monTool, 
       monitor_for_jet_delta_pt, monitor_for_jet_delta_eta, monitor_for_jet_delta_phi
    );

  }

  auto monitor_group_for_jets = Monitored::Group( m_monTool, 
     monitor_for_jet_pt, monitor_for_jet_eta, monitor_for_jet_phi,
     monitor_for_Bjet_pt, monitor_for_Bjet_eta, monitor_for_Bjet_phi
  );

  return StatusCode::SUCCESS;
}

StatusCode TrigBjetBtagHypoAlg::monitor_tracks( const EventContext& context, const TrigCompositeUtils::DecisionContainer* prevDecisionContainer ) const {
  // This vector is for checking we are not reading more than once from the same View, thus retrieving the same objects multiple times!
  std::vector< ElementLink< ViewContainer > > readViews;
  for ( const TrigCompositeUtils::Decision* previousDecision: *prevDecisionContainer ) {
    // get View
    ElementLink< ViewContainer > viewEL = previousDecision->objectLink< ViewContainer >( TrigCompositeUtils::viewString() );
    ATH_CHECK( viewEL.isValid() );
    ATH_MSG_DEBUG( "Retrieved View" );

    bool alreadyUsed = false;
    for ( const ElementLink< ViewContainer >& storedViews : readViews ) {
      if ( viewEL == storedViews ) {
        ATH_MSG_DEBUG( "We have already used this view!" );
	alreadyUsed = true;    
	break;
      } 
    }
    if ( alreadyUsed ) continue;

    readViews.push_back( viewEL );

    // online monitoring for tracks
    SG::ReadHandle< xAOD::TrackParticleContainer > trackHandle = ViewHelper::makeHandle( *viewEL, m_trackKey, context );
    ATH_CHECK( trackHandle.isValid() );
    size_t Counter = 0;
    ElementLinkVector< xAOD::TrackParticleContainer > trackELs;
    for ( auto it = trackHandle->begin(); it != trackHandle->end(); ++it, ++Counter ) {
      ElementLink< xAOD::TrackParticleContainer > tracks = ViewHelper::makeLink< xAOD::TrackParticleContainer >( *viewEL, trackHandle, Counter );
      ATH_CHECK( tracks.isValid() );
      trackELs.push_back( tracks );
    }
    auto monitor_for_track_count = Monitored::Scalar( "track_count", trackELs.size() );
    auto monitor_group_for_decision = Monitored::Group( m_monTool, monitor_for_track_count);

    auto monitor_for_track_Et = Monitored::Collection( "track_Et", trackELs,
      [](const ElementLink< xAOD::TrackParticleContainer >& trackLink) { return (*trackLink)->p4().Et() / 1000.0 /*Gev*/; } );
    auto monitor_for_track_eta = Monitored::Collection( "track_eta", trackELs,
      [](const ElementLink< xAOD::TrackParticleContainer >& trackLink) { return (*trackLink)->eta(); } ); 
    auto monitor_for_track_phi = Monitored::Collection( "track_phi", trackELs,
      [](const ElementLink< xAOD::TrackParticleContainer >& trackLink) { return (*trackLink)->phi(); } );

    // Monitors for d0 and z0 track impact parameter variables
    auto monitor_for_track_d0 = Monitored::Collection( "track_d0", trackELs,
      [](const ElementLink< xAOD::TrackParticleContainer >& trackLink) { return (*trackLink)->d0(); } );

    auto monitor_for_track_d0err = Monitored::Collection( "track_d0err", trackELs,
      [](const ElementLink< xAOD::TrackParticleContainer >& trackLink) {
	      return Amg::error( (*trackLink)->definingParametersCovMatrix(), 0);
      } );

    auto monitor_for_track_d0sig = Monitored::Collection( "track_d0sig", trackELs,
      [](const ElementLink< xAOD::TrackParticleContainer >& trackLink) {
	return (*trackLink)->d0() / Amg::error( (*trackLink)->definingParametersCovMatrix(), 0);
      } );

    auto monitor_for_track_z0 = Monitored::Collection( "track_z0", trackELs,
      [](const ElementLink< xAOD::TrackParticleContainer >& trackLink) { return (*trackLink)->z0(); } );

    auto monitor_for_track_z0err = Monitored::Collection( "track_z0err", trackELs,
      [](const ElementLink< xAOD::TrackParticleContainer >& trackLink) {
	      return Amg::error( (*trackLink)->definingParametersCovMatrix(), 1);
      } );

    auto monitor_for_track_z0sig = Monitored::Collection( "track_z0sig", trackELs,
      [](const ElementLink< xAOD::TrackParticleContainer >& trackLink) {
	return (*trackLink)->z0() / Amg::error( (*trackLink)->definingParametersCovMatrix(), 1);
      } );

    auto monitor_group_for_tracks = Monitored::Group( m_monTool, 
      monitor_for_track_Et, monitor_for_track_eta, monitor_for_track_phi,
      monitor_for_track_d0, monitor_for_track_d0err, monitor_for_track_d0sig,
      monitor_for_track_z0, monitor_for_track_z0err, monitor_for_track_z0sig
    );

  }
  return StatusCode::SUCCESS;
}


StatusCode TrigBjetBtagHypoAlg::monitor_flavor_probabilities( const ElementLinkVector< xAOD::BTaggingContainer >& bTaggingEL, const std::string& var_name ) const {
  auto monitor_pu = Monitored::Collection( "btag_"+var_name+"_pu", bTaggingEL,
    [var_name](const ElementLink< xAOD::BTaggingContainer >& bTagLink) { 
      auto jet = getJetFromBTagLink(**bTagLink);
      double pu = -1; 
      SG::ConstAccessor<float> acc(var_name+"_pu");
      pu = (acc.isAvailable(*jet) ? acc(*jet) : acc(**bTagLink));
      return pu; 
    } );

  auto monitor_pb = Monitored::Collection( "btag_"+var_name+"_pb", bTaggingEL,
    [var_name](const ElementLink< xAOD::BTaggingContainer >& bTagLink) { 
      auto jet = getJetFromBTagLink(**bTagLink);
      double pb = -1; 
      SG::ConstAccessor<float> acc(var_name+"_pb");
      pb = (acc.isAvailable(*jet) ? acc(*jet) : acc(**bTagLink));
      return pb;
    } );

  auto monitor_pc = Monitored::Collection( "btag_"+var_name+"_pc", bTaggingEL,
    [var_name](const ElementLink< xAOD::BTaggingContainer >& bTagLink) { 
      auto jet = getJetFromBTagLink(**bTagLink);
      double pc = -1; 
      SG::ConstAccessor<float> acc(var_name+"_pc");
      pc = (acc.isAvailable(*jet) ? acc(*jet) : acc(**bTagLink));
      return pc;
    } );

  auto monitor_llr = Monitored::Collection( "btag_"+var_name+"_llr", bTaggingEL,
    [var_name](const ElementLink< xAOD::BTaggingContainer >& bTagLink) { 
      auto jet = getJetFromBTagLink(**bTagLink);
      SG::ConstAccessor<float> acc_pb(var_name+"_pb");
      double pb = -1.0;
      pb = (acc_pb.isAvailable(*jet) ? acc_pb(*jet) : acc_pb(**bTagLink));
      SG::ConstAccessor<float> acc_pu(var_name+"_pu");
      double pu = -1.0;
      pu = (acc_pu.isAvailable(*jet) ? acc_pu(*jet) : acc_pu(**bTagLink));
      if( !pb || !pu )  return -1.0f;
      float llr = 0.;
      if(pb<=0.) {
        llr = -30.;
      } else if(pu<=0.) {
        llr = +100.;
      } else {
        llr = std::log(pb/pu);
      }
      return llr;
    } );

  auto monitor_group_for_flavor_tag_var = Monitored::Group( m_monTool, monitor_pu, monitor_pb, monitor_pc, monitor_llr );

  return StatusCode::SUCCESS;
}

StatusCode TrigBjetBtagHypoAlg::monitor_flavor_bb_probabilities( const ElementLinkVector< xAOD::BTaggingContainer >& bTaggingEL, const std::string& var_name ) const {

  auto monitor_pb = Monitored::Collection( "bbtag_"+var_name+"_pb", bTaggingEL,
    [var_name](const ElementLink< xAOD::BTaggingContainer >& bTagLink) { 
      auto jet = getJetFromBTagLink(**bTagLink);
      double pb = -1;
      SG::ConstAccessor<float> acc(var_name+"_pb");
      pb = (acc.isAvailable(*jet) ? acc(*jet) : acc(**bTagLink));
      return pb; 
    } );

  auto monitor_pbb = Monitored::Collection( "bbtag_"+var_name+"_pbb", bTaggingEL,
    [var_name](const ElementLink< xAOD::BTaggingContainer >& bTagLink) { 
      auto jet = getJetFromBTagLink(**bTagLink);
      double pbb = -1; 
      SG::ConstAccessor<float> acc(var_name+"_pbb");
      pbb = (acc.isAvailable(*jet) ? acc(*jet) : acc(**bTagLink));
      return pbb;
    } );

  auto monitor_group_for_flavor_bb_tag_var = Monitored::Group( m_monTool, monitor_pb, monitor_pbb );

  return StatusCode::SUCCESS;
}


StatusCode TrigBjetBtagHypoAlg::monitor_primary_vertex( const ElementLink< xAOD::VertexContainer >& primVertexEL ) const {
  auto monitor_for_primVtx_x = Monitored::Scalar( "primVtx_x", (*primVertexEL)->x() );  
  auto monitor_for_primVtx_y = Monitored::Scalar( "primVtx_y", (*primVertexEL)->y() );  
  auto monitor_for_primVtx_z = Monitored::Scalar( "primVtx_z", (*primVertexEL)->z() );  

  auto monitor_group_for_primary_vertex = Monitored::Group( m_monTool, monitor_for_primVtx_x, monitor_for_primVtx_y, monitor_for_primVtx_z );
  return StatusCode::SUCCESS;
}


ElementLinkVector<xAOD::BTaggingContainer> TrigBjetBtagHypoAlg::collect_valid_links(
    const ElementLinkVector< xAOD::BTaggingContainer >& bTaggingEL, std::string tagger ) const {

  ElementLinkVector<xAOD::BTaggingContainer> valid_bTaggingEL;
  for (const ElementLink< xAOD::BTaggingContainer > bTagLink : bTaggingEL) {
    SG::ConstAccessor<char> acc(tagger+"_isDefaults");
    if ( not acc(**bTagLink) ) { valid_bTaggingEL.push_back( bTagLink ); }
  }
  return valid_bTaggingEL;
}


StatusCode TrigBjetBtagHypoAlg::monitor_btagging( const ElementLinkVector< xAOD::BTaggingContainer >& bTaggingEL ) const {
  // Monitor high-level tagger flavor probabilites
  CHECK( monitor_flavor_probabilities(bTaggingEL, "dips20211116") );
  CHECK( monitor_flavor_probabilities(bTaggingEL, "GN120220813") );
  CHECK( monitor_flavor_probabilities(bTaggingEL, "GN220240122") );

  return StatusCode::SUCCESS;
}

