/*
   Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include <algorithm>
#include "TrigJetCROldHypoAlg.h"
#include "TrigJetCROldHypoTool.h"
#include "TrigCompositeUtils/HLTIdentifier.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"

using namespace TrigCompositeUtils;


TrigJetCROldHypoAlg::TrigJetCROldHypoAlg( const std::string& name, 
				    ISvcLocator* pSvcLocator ) :
  ::HypoBase( name, pSvcLocator ) { }


StatusCode TrigJetCROldHypoAlg::initialize() {

  ATH_CHECK(m_hypoTools.retrieve() );
  ATH_CHECK(m_trackParticleKey.initialize());
  ATH_CHECK(m_cellKey.initialize());

  return StatusCode::SUCCESS;
}


StatusCode TrigJetCROldHypoAlg::execute( const EventContext& context ) const {

  ATH_MSG_DEBUG( "Executing " << name() << "..." );

  ATH_MSG_DEBUG("Retrieving HLT decision \"" << decisionInput().key() << "\"");
  auto previousDecisionsHandle = SG::makeHandle( decisionInput(), context );
  ATH_CHECK( previousDecisionsHandle.isValid() );
  const DecisionContainer* prevDecisions = previousDecisionsHandle.get();

  // Creates the mutable output DecisionContainer and registers it to StoreGate.
  SG::WriteHandle<DecisionContainer> outputHandle = createAndStore(decisionOutput(), context ); 
  DecisionContainer* decisions = outputHandle.ptr();
  

  ATH_MSG_DEBUG( "Creating new output decision handle"); 

  // prepare imput for tools
  std::vector<TrigJetCROldHypoTool::JetInfo> hypoToolInput;
 
  // get tracks from the key :
  ATH_MSG_DEBUG( "Getting Track Handle "<<m_trackParticleKey);
  auto trackHandle = SG::makeHandle(m_trackParticleKey, context );

  ATH_CHECK( trackHandle.isValid() );
  const xAOD::TrackParticleContainer* allTracks = trackHandle.get();
  ATH_MSG_DEBUG ( allTracks->size() << " tracks found" );

  // get cells from the key :
  ATH_MSG_DEBUG( "Getting Cells Handle "<<m_cellKey);
  auto cellHandle = SG::makeHandle(m_cellKey, context );

  ATH_CHECK( cellHandle.isValid() );
  const CaloConstCellContainer* cells = cellHandle.get();
  ATH_MSG_DEBUG ( cells->size() << " cells found" );

  for (const Decision* previousDecision : *prevDecisions) {
    // Create a new output Decision object, d, backed by the 'decisions' container.
    // Links previousDecision as the parent of d.
    
    Decision* d = newDecisionIn( decisions, previousDecision, hypoAlgNodeName(), context );
    

   const std::vector<LinkInfo< xAOD::JetContainer > >jetELs = 
      findLinks< xAOD::JetContainer >( previousDecision, featureString().c_str(), TrigDefs::lastFeatureOfType);
      d->setObjectLink<xAOD::JetContainer>( featureString(), jetELs.at(0).link );
      DecisionIDContainer previousDecisionIDs;
      decisionIDs(previousDecision, previousDecisionIDs);

        for (const LinkInfo< xAOD::JetContainer> & jetLinkInfo: jetELs){
    // Collect all the required information for the tool together in a handy struct 
              hypoToolInput.emplace_back( TrigJetCROldHypoTool::JetInfo{previousDecisionIDs, *(jetLinkInfo.link), allTracks, cells, d} );
        }
    // Obtain the set of chains which are active in previousDecision
   }


  //Loop over all hypoToolinputs and get their decisions
  for ( auto & tool: m_hypoTools ) {
    ATH_CHECK( tool->decide( hypoToolInput ) );    
  } 
  
  ATH_CHECK( hypoBaseOutputProcessing(outputHandle) );

  return StatusCode::SUCCESS;

}

