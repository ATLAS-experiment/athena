/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <cassert>
#include "TestTools/FLOATassert.h"

#include "JetTagTools/CTTDecorCheckInTool.h"
//
//-------------------------------------------------

//Constructor-------------------------------------------------------------- 
CTTDecorCheckInTool::CTTDecorCheckInTool( const std::string& name,
                            ISvcLocator* pSvcLocator):
  AthAlgorithm( name, pSvcLocator ),
  m_classifiedTrackTagger("Analysis::ClassifiedTrackTaggerTool",this),
  m_jetCollection("AntiKt4EMPFlowJets")
  {
    declareProperty("ClassifiedTrackTaggerTool",  m_classifiedTrackTagger);
    declareProperty("JetCollection",m_jetCollection);
  }

//Initialize---------------------------------------------------------------
StatusCode CTTDecorCheckInTool::initialize(){

  ATH_CHECK( m_particlesKey.initialize() );
  ATH_CHECK( m_verticesKey.initialize() );
  ATH_CHECK( m_jetsKey.initialize() );

  if(m_jetCollection.empty()){
    ATH_MSG_FATAL("No JetCollection specified! ");
  } else {
    m_jetReadDecorKey = m_jetCollection+".CTTScore";
    ATH_CHECK( m_jetReadDecorKey.initialize());
  }

  //-------
  //check that the ClassifiedTrackTaggerTool can be accessed
  if (m_classifiedTrackTagger.retrieve().isFailure()) {
    ATH_MSG_DEBUG("Could not find Analysis::ClassifiedTrackTaggerTool");
    return StatusCode::SUCCESS;
  } else {
    ATH_MSG_DEBUG("Analysis::ClassifiedTrackTaggerTool found");
  }

  return StatusCode::SUCCESS;
}

StatusCode CTTDecorCheckInTool::execute()
{
  ATH_MSG_DEBUG( "Executing..." );

  SG::ReadDecorHandle< xAOD::JetContainer, float > jetReadDecorHandle (m_jetReadDecorKey);

  // Retrieve the track particles:
  SG::ReadHandle<xAOD::TrackParticleContainer> trackTES(m_particlesKey);
  if ( !trackTES.isValid() ) {
    ATH_MSG_WARNING( "No TrackParticle container found in TDS" );
    return StatusCode::SUCCESS;
  }
  ATH_MSG_DEBUG( "TrackParticleContainer successfully retrieved" );

  SG::ReadHandle<xAOD::VertexContainer> pvTES(m_verticesKey);
  if ( !pvTES.isValid() ) {
    ATH_MSG_WARNING( "No Primary Vertices container found in TDS" );
    return StatusCode::SUCCESS;
  }
  ATH_MSG_DEBUG( "Primary Vertices container successfully retrieved" );
  const xAOD::Vertex *primVertex=*(pvTES->begin());

  //==========================================================================
  SG::ReadHandle<xAOD::JetContainer> jetTES(m_jetsKey);
  if ( !jetTES.isValid() ) {
    ATH_MSG_WARNING( "No AntiKt4EMPflow jet container found in TDS" );
    return StatusCode::SUCCESS;
  }
  ATH_MSG_DEBUG( "AntiKt4EMPflow jet container successfully retrieved" );

  //save all TrackParticles in an std::vector (needed as input to the CTT tool)
  std::vector<const xAOD::TrackParticle*> trkparticles(0);
  for(const auto& trkPart : *trackTES){
    trkparticles.emplace_back(trkPart);
  }

  //first decorate all jets with the CTT method
  m_classifiedTrackTagger->decorateJets(trkparticles, *primVertex, *jetTES);

  for(const auto& curjet : *jetTES){
    ATH_MSG_DEBUG( " Jet  pt: " << curjet->pt()<<" eta: "<<curjet->eta()<<" phi: "<< curjet->phi() );
    float CTTScore = m_classifiedTrackTagger->bJetWgts(trkparticles, *primVertex, curjet->p4());
    ATH_MSG_DEBUG ("Retrieved CTT score from CTT tool: " << CTTScore);
    ATH_MSG_DEBUG("CTT score of decorated xAOD::Jet : " << jetReadDecorHandle(*curjet));
    assert(Athena_test::isEqual(CTTScore,jetReadDecorHandle(*curjet)));
  }

  return StatusCode::SUCCESS;
}

