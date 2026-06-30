/*
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigMuonEFIdtpHypoAlg.h"
#include "TrigMuonEFIdtpCommon.h"
#include "AthViews/ViewHelper.h"

using namespace TrigCompositeUtils; 

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

TrigMuonEFIdtpHypoAlg::TrigMuonEFIdtpHypoAlg( const std::string& name,
						  ISvcLocator* pSvcLocator ) :
   ::HypoBase( name, pSvcLocator ) { } 

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

StatusCode TrigMuonEFIdtpHypoAlg::initialize()
{
  ATH_CHECK(m_hypoTools.retrieve());

  renounce(m_PTTracksKey);
  ATH_CHECK(m_PTTracksKey.initialize());

  renounce(m_FTFTracksKey);
  ATH_CHECK(m_FTFTracksKey.initialize());

  return StatusCode::SUCCESS;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

StatusCode TrigMuonEFIdtpHypoAlg::execute( const EventContext& context ) const
{
   ATH_MSG_DEBUG("StatusCode TrigMuonEFIdtpHypoAlg::execute start");
   
   // common for all hypos, to move in the base class
   auto previousDecisionsHandle = SG::makeHandle( decisionInput(), context );
   ATH_CHECK( previousDecisionsHandle.isValid() );
   ATH_MSG_DEBUG("Running with "<< previousDecisionsHandle->size() <<" previous decisions");
   
   // new output decisions
   SG::WriteHandle<DecisionContainer> outputHandle = createAndStore(decisionOutput(), context ); 
   auto decisions = outputHandle.ptr();
  
   std::vector<TrigMuonEFIdtpHypoTool::MuonEFIdperfInfo> toolInput;
   int counter = -1;

   // loop over previous decisions
   for ( const auto previousDecision: *previousDecisionsHandle ) {

      ATH_MSG_VERBOSE("--- counter: "<<++counter<<" ---");
      
      // get view
      auto viewEL = previousDecision->objectLink<ViewContainer>( viewString() );
      ATH_CHECK( viewEL.isValid() );
      
      // get ID tracks
      auto ptTrkHandle  = ViewHelper::makeHandle( *viewEL, m_PTTracksKey,  context );
      ATH_CHECK( ptTrkHandle.isValid() );
      if( ptTrkHandle->size()==0  ) {
	 ATH_MSG_VERBOSE("No PT track handle, skipping this decision");
	 continue;
      }
      auto ftfTrkHandle = ViewHelper::makeHandle( *viewEL, m_FTFTracksKey, context );
      ATH_CHECK( ftfTrkHandle.isValid() );
      if( ftfTrkHandle->size()==0 ) {
	 ATH_MSG_VERBOSE("No FTF track handle, skipping this decision");
	 continue;
      }

      // get SA muon from the previous decision
      const xAOD::Muon *muonSA = nullptr;
      auto prevMuInfo = TrigCompositeUtils::findLinks<xAOD::MuonContainer>(context, previousDecision, TrigCompositeUtils::featureString(), TrigDefs::lastFeatureOfType);
      ATH_CHECK(prevMuInfo.size()==1);
      auto muonSALink = prevMuInfo.at(0).link;
      ATH_CHECK( muonSALink.isValid() );
      muonSA = *muonSALink;
      if( muonSA->muonType() != xAOD::Muon::MuonType::MuonStandAlone ) {
	 ATH_MSG_VERBOSE("previous decision muon is not SA, skipping this decision");
	 continue;
      }
      const xAOD::TrackParticle* metrack = muonSA->trackParticle( xAOD::Muon::TrackParticleType::ExtrapolatedMuonSpectrometerTrackParticle );
      ATH_MSG_VERBOSE("muonSA: muonType="<<muonSA->muonType()<<", pT="<<muonSA->pt()/1000.0<<", eta="<<muonSA->eta()<<", phi="<<muonSA->phi()<<", author="<<muonSA->author());

      // select PT track
      float dRmin = 999;
      const xAOD::TrackParticle* thePTTrack = nullptr;
      std::vector<ElementLink<xAOD::TrackParticleContainer>> thePTTrackLinks;
      const xAOD::TrackParticleContainer * ptTracks = ptTrkHandle.get();
      ATH_MSG_VERBOSE("PT  tracks size: " << ptTracks->size());
      for ( const xAOD::TrackParticle* idtrack : *ptTracks ) {
	 if (idtrack->pt()< 1*Gaudi::Units::GeV ) continue;
	 float dr = TrigMuonEFIdtpCommon::matchingMetric(metrack,idtrack);
	 if( dr < dRmin ) {
	    dRmin = dr;
	    thePTTrack = idtrack;
	 }
      }
      if( thePTTrack != nullptr ) {
	 auto idtrackLink = ElementLink<xAOD::TrackParticleContainer>(*ptTracks,thePTTrack->index());
	 ATH_CHECK( idtrackLink.isValid() );
	 thePTTrackLinks.push_back(idtrackLink);
	 ATH_MSG_VERBOSE("... best selected PT track pt / eta / phi / dr = " << thePTTrack->pt()/Gaudi::Units::GeV << " / " << thePTTrack->eta() << " / " << thePTTrack->phi() << " / " << dRmin << ", fitter=" << thePTTrack->trackFitter());
      }

      // FTF track
      dRmin = 999;
      const xAOD::TrackParticle* theFTFTrack = nullptr;
      std::vector<ElementLink<xAOD::TrackParticleContainer>> theFTFTrackLinks;
      const xAOD::TrackParticleContainer * ftfTracks = ftfTrkHandle.get();
      ATH_MSG_VERBOSE("FTF tracks size: " << ftfTracks->size());
      for ( const xAOD::TrackParticle* idtrack : *ftfTracks ) {
	 if (idtrack->pt()< 1*Gaudi::Units::GeV ) continue;
	 float dr = TrigMuonEFIdtpCommon::matchingMetric(metrack,idtrack);
	 if( dr < dRmin ) {
	    dRmin = dr;
	    theFTFTrack = idtrack;
	 }
      }
      if( theFTFTrack != nullptr ) {
	 auto idtrackLink = ElementLink<xAOD::TrackParticleContainer>(*ftfTracks,theFTFTrack->index());
	 ATH_CHECK( idtrackLink.isValid() );
	 theFTFTrackLinks.push_back(idtrackLink);
	 ATH_MSG_VERBOSE("... best selected FTF track pt / eta / phi / dr = " << theFTFTrack->pt()/Gaudi::Units::GeV << " / " << theFTFTrack->eta() << " / " << theFTFTrack->phi() << " / " << dRmin << ", fitter=" << theFTFTrack->trackFitter());
      }

      // create new decisions
      auto newd = newDecisionIn( decisions, hypoAlgNodeName() );
      
      // push_back to toolInput
      toolInput.emplace_back( newd, muonSA, thePTTrack, theFTFTrack, previousDecision );

      newd -> setObjectLink( featureString(), muonSALink );
      newd -> addObjectCollectionLinks( m_PTTracksKey.key(),  thePTTrackLinks );
      newd -> addObjectCollectionLinks( m_FTFTracksKey.key(), theFTFTrackLinks );
      ATH_MSG_VERBOSE("Setting object link for PT / FTF track: size="<<thePTTrackLinks.size()<<" / "<<theFTFTrackLinks.size());
      TrigCompositeUtils::linkToPrevious( newd, previousDecision, context );
   }

   ATH_MSG_DEBUG("Found "<<toolInput.size()<<" inputs to tools");

   // to TrigMuonEFIdtpHypoTool
   StatusCode sc = StatusCode::SUCCESS;
   for ( auto& tool: m_hypoTools ) {
      ATH_MSG_DEBUG("Go to " << tool );
      sc = tool->decide(toolInput);
      if (!sc.isSuccess()) {
	 ATH_MSG_ERROR("MuonHypoTool is failed");
	 return StatusCode::FAILURE;
      }
   } // End of tool algorithms */	
   
   ATH_CHECK(hypoBaseOutputProcessing(outputHandle));
   
   ATH_MSG_DEBUG("StatusCode TrigMuonEFIdtpHypoAlg::execute success");
   return StatusCode::SUCCESS;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
