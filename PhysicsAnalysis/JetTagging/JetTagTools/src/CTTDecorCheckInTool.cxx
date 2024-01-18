/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "PathResolver/PathResolver.h"
#include "TLorentzVector.h"
//#include "StoreGate/ReadDecorHandle.h"

#include <cassert>
#include "TestTools/FLOATassert.h"
#include <iostream>

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

//Destructor---------------------------------------------------------------
  CTTDecorCheckInTool::~CTTDecorCheckInTool(){
    ATH_MSG_DEBUG("ClassifiedTrackTaggerTool destructor called");
  }

//Initialize---------------------------------------------------------------
   StatusCode CTTDecorCheckInTool::initialize(){


    ATH_CHECK( m_particlesKey.initialize() );
    ATH_CHECK( m_verticesKey.initialize() );
    ATH_CHECK( m_jetsKey.initialize() );

    if(m_jetCollection!="")
    {
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

   StatusCode CTTDecorCheckInTool::finalize()
   {
    ATH_MSG_DEBUG("CTTDecorCheckInTool finalize()");
    return StatusCode::SUCCESS; 
   }

   StatusCode CTTDecorCheckInTool::execute()
   {  
      ATH_MSG_DEBUG( "Executing..." );
      if(m_jetCollection=="") {ATH_MSG_FATAL("No JetCollection specified! ");}

      SG::ReadDecorHandle< xAOD::JetContainer, float > jetReadDecorHandle (m_jetReadDecorKey);

      // Retrieve the track particles:
      SG::ReadHandle<xAOD::TrackParticleContainer> trackTES(m_particlesKey);
      if ( !trackTES.isValid() ) {
      ATH_MSG_WARNING( "No TrackParticle container found in TDS" );
      return StatusCode::SUCCESS; }
      ATH_MSG_DEBUG( "TrackParticleContainer successfully retrieved" );

      SG::ReadHandle<xAOD::VertexContainer> pvTES(m_verticesKey);
      if ( !pvTES.isValid() ) {
      ATH_MSG_WARNING( "No Primary Vertices container found in TDS" );
      return StatusCode::SUCCESS; }
      ATH_MSG_DEBUG( "Primary Vertices container successfully retrieved" );
      const xAOD::Vertex *primVertex=*(pvTES->begin());



    //==========================================================================
    SG::ReadHandle<xAOD::JetContainer> jetTES(m_jetsKey);
    if ( !jetTES.isValid() ) {
      ATH_MSG_WARNING( "No AntiKt4EMPflow jet container found in TDS" );
      return StatusCode::SUCCESS;  }
    ATH_MSG_DEBUG( "AntiKt4EMPflow jet container successfully retrieved" );

    //save all TrackParticles in an std::vector (needed as input to the CTT tool)
    xAOD::TrackParticleContainer::const_iterator trackItr  = trackTES->begin();
    xAOD::TrackParticleContainer::const_iterator trackItrE = trackTES->end();
    std::vector<const xAOD::TrackParticle*> trkparticles(0);

    // Loop over all track particles:
    for( ; trackItr != trackItrE; ++trackItr ) {
      const xAOD::TrackParticle* trackParticle = ( *trackItr );
      trkparticles.push_back(trackParticle);
    }
    //first decorate all jets with the CTT method
    m_classifiedTrackTagger->decorateJets(trkparticles,*primVertex,*jetTES);

    xAOD::JetContainer::const_iterator jetItr  = jetTES->begin();
    xAOD::JetContainer::const_iterator jetItrE = jetTES->end();

    for(jetItr  = jetTES->begin(); jetItr != jetItrE;  ++jetItr) {
     /// this Jet
      const xAOD::Jet* curjet = ( *jetItr );
      ATH_MSG_DEBUG( " Jet  pt: " << curjet->pt()<<" eta: "<<curjet->eta()<<" phi: "<< curjet->phi() );
      float CTTScore = m_classifiedTrackTagger->bJetWgts(trkparticles, *primVertex, curjet->p4()); 
      ATH_MSG_DEBUG ("Retrieved CTT score from CTT tool: " << CTTScore);
      ATH_MSG_DEBUG("CTT score of decorated xAOD::Jet : " << jetReadDecorHandle(*curjet));
      assert(Athena_test::isEqual(CTTScore,jetReadDecorHandle(*curjet)));

    }

    /**
    for (const xAOD::Jet* & decoJet : *jetReadDecorHandle) {  // Access the container.
      ATH_MSG_INFO("CTT score of decorated xAOD::Jet : " << jetReadDecorHandle (decoJet); ) // Access the decoration.
    }**/

    return StatusCode::SUCCESS;   
  }

