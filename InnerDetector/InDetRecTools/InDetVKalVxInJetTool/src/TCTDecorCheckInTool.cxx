/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetVKalVxInJetTool/TCTDecorCheckInTool.h"
#include "PathResolver/PathResolver.h"
#include "TLorentzVector.h"

#include <cassert>
#include "TestTools/FLOATassert.h"
//
//-------------------------------------------------
//
//Constructor-------------------------------------------------------------- 
TCTDecorCheckInTool::TCTDecorCheckInTool( const std::string& name,
                            ISvcLocator* pSvcLocator):
  AthAlgorithm( name, pSvcLocator )
  { }

//Destructor---------------------------------------------------------------
  TCTDecorCheckInTool::~TCTDecorCheckInTool(){
    ATH_MSG_DEBUG("TCTDecorCheckInTool destructor called");
  }

//Initialize---------------------------------------------------------------
   StatusCode TCTDecorCheckInTool::initialize(){


    ATH_CHECK( m_particlesKey.initialize() );
    ATH_CHECK( m_verticesKey.initialize() );
    ATH_CHECK( m_jetsKey.initialize() );

    //from https://acode-browser1.usatlas.bnl.gov/lxr/source/athena/Event/xAOD/xAODTrackingCnv/src/TrackParticleCnvAlg.cxx
    m_trackReadDecorKeyTCTScore = "InDetTrackParticles.TCTScore_"+m_jetsKey.key();
    m_trackReadDecorKeyJetLink = "InDetTrackParticles.TCTJetLink_"+m_jetsKey.key();

    m_jetReadDecorKeyTCTScore = m_jetsKey.key()+ ".TCTScore";
    m_jetReadDecorKeyTrackLink = m_jetsKey.key() +".TCTTrackLink";

    ATH_CHECK( m_trackReadDecorKeyTCTScore.initialize());
    ATH_CHECK( m_trackReadDecorKeyJetLink.initialize());

    ATH_CHECK( m_jetReadDecorKeyTCTScore.initialize());
    ATH_CHECK( m_jetReadDecorKeyTrackLink.initialize());
    
     //-------
     //check that the TrackClassificationTool can be accessed
     if (m_trackClassificationTool.retrieve().isFailure()) {
        ATH_MSG_DEBUG("Could not find InDet::InDetTrkInJetType");
        return StatusCode::SUCCESS;
     } else {
        ATH_MSG_DEBUG("InDet::InDetTrkInJetType found");
     }
     
     return StatusCode::SUCCESS;
   }

   StatusCode TCTDecorCheckInTool::finalize()
   {
    ATH_MSG_DEBUG("TCTDecorCheck finalize()");
    return StatusCode::SUCCESS; 
   }

   StatusCode TCTDecorCheckInTool::execute() 
   {  
      ATH_MSG_DEBUG( "Executing..." );
      SG::ReadDecorHandle< xAOD::TrackParticleContainer, std::vector<float> > trackReadDecorHandleTCTScore (m_trackReadDecorKeyTCTScore);
      SG::ReadDecorHandle< xAOD::TrackParticleContainer, ElementLink<xAOD::JetContainer> > trackReadDecorHandleJetLink (m_trackReadDecorKeyJetLink);

      //JetRead handles
      SG::ReadDecorHandle< xAOD::JetContainer, std::vector<std::vector<float>> > jetReadDecorHandleTCTScore (m_jetReadDecorKeyTCTScore);
      SG::ReadDecorHandle< xAOD::JetContainer, std::vector<ElementLink<xAOD::TrackParticleContainer>> > jetReadDecorHandleTrackLink (m_jetReadDecorKeyTrackLink);

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

    xAOD::TrackParticleContainer::const_iterator trackItr  = trackTES->begin();
    xAOD::TrackParticleContainer::const_iterator trackItrE = trackTES->end();

    xAOD::JetContainer::const_iterator jetItr  = jetTES->begin();
    xAOD::JetContainer::const_iterator jetItrE = jetTES->end();

    //decorator methods for decorating the tracks
    if(m_decoratorMethod=="decorateTrack")
    {
        ATH_MSG_DEBUG("Using decorateTrack method ");
        for(trackItr = trackTES->begin(); trackItr != trackItrE; ++trackItr){
        const xAOD::TrackParticle* itrk = (*trackItr);
        //first decorate the track with the InDetTrkInJetType method
        //find closest jet to the track in Delta R 
        float minDeltaR=1.0; 
        const xAOD::Jet* closestJet = *(jetTES->begin());
        for(jetItr = jetTES->begin(); jetItr != jetItrE; ++jetItr){
          const xAOD::Jet* curJet = (*jetItr);
          float curDeltaR = (itrk)->p4().DeltaR(curJet->p4());
          if(curDeltaR < minDeltaR) {minDeltaR = curDeltaR; closestJet = curJet;}
        }
        m_trackClassificationTool->decorateTrack(itrk,*primVertex, *jetTES, closestJet);
        }
      
       //loop over tracks and check if decoration was correctly added (using either decorateTrack)
      for(trackItr = trackTES->begin(); trackItr != trackItrE; ++trackItr){
        const xAOD::TrackParticle* itrk = (*trackItr);
        const std::vector<float>& v_tctScoresDeco = trackReadDecorHandleTCTScore(*itrk);
        const ElementLink<xAOD::JetContainer>& v_jetLinks = trackReadDecorHandleJetLink(*itrk);

          ATH_MSG_DEBUG("TCT score from decoration: " << v_tctScoresDeco.at(0) << ", " << v_tctScoresDeco.at(1) << ", "<< v_tctScoresDeco.at(2));
          std::vector<float> v_tctScore = m_trackClassificationTool->trkTypeWgts(itrk,*primVertex,(*v_jetLinks)->p4());
          ATH_MSG_DEBUG("Calculated TCT score: " << v_tctScore.at(0) << ", " << v_tctScore.at(1) << ", " << v_tctScore.at(2));

          for(int j=0; j<=2 ; j++) {assert(Athena_test::isEqual(v_tctScore.at(j),v_tctScoresDeco.at(j)));}
        
      }//end track loop    
    }//end if decorator methods for decorating tracks
    else if(m_decoratorMethod=="decorateJet")
    {
      ATH_MSG_DEBUG("Using decorateJets method ");
      for(jetItr = jetTES->begin(); jetItr != jetItrE; ++jetItr){
        const xAOD::Jet* ijet = (*jetItr);
        std::vector<const xAOD::TrackParticle*> trkparticles(0);
        for(trackItr = trackTES->begin(); trackItr != trackItrE; ++trackItr){
          const xAOD::TrackParticle* itrk = (*trackItr);
          if((itrk)->p4().DeltaR(ijet->p4()) < 0.4) {trkparticles.push_back(itrk); }
        }
        m_trackClassificationTool->decorateJet(trkparticles,*trackTES,*primVertex, ijet);
      }
      
      //loop over jets and check if decoration was correctly added 
      for(jetItr = jetTES->begin(); jetItr != jetItrE; ++jetItr){
        const xAOD::Jet* ijet = (*jetItr);
        const std::vector<std::vector<float>>& v_tctScoresDeco = jetReadDecorHandleTCTScore(*ijet);
        const std::vector<ElementLink<xAOD::TrackParticleContainer>>& v_trackLinks = jetReadDecorHandleTrackLink(*ijet);

        for(unsigned int i=0; i<v_tctScoresDeco.size(); i++)
        {
          ATH_MSG_DEBUG("TCT score from decoration: " << v_tctScoresDeco.at(i).at(0) << ", " << v_tctScoresDeco.at(i).at(1) << ", "<< v_tctScoresDeco.at(i).at(2));
          std::vector<float> v_tctScore = m_trackClassificationTool->trkTypeWgts(*v_trackLinks.at(i),*primVertex,ijet->p4());
          ATH_MSG_DEBUG("Calculated TCT score: " << v_tctScore.at(0) << ", " << v_tctScore.at(1) << ", " << v_tctScore.at(2));

          for(int j=0; j<=2 ; j++) {assert(Athena_test::isEqual(v_tctScore.at(j),v_tctScoresDeco.at(i).at(j)));}
        }//end of tct vector loop
      }//end of jet loop
    }
    else
    {
      ATH_MSG_ERROR("Specified decorator method not implemented in InDetTrkInJetType: " << m_decoratorMethod );
    }


    return StatusCode::SUCCESS;   
  }
   
