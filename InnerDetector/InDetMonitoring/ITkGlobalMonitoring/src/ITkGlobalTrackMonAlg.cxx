/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


/** @file ITkGlobalTrackMonAlg.h
 * Implementation of inner detector global track monitoring tool
 *
 *@author
 * Leonid Serkin <lserkin@cern.ch> @n
 * Per Johansson <Per.Johansson@cern.ch> @n
 *
 * based on InDetGlobalTrackMonTool.cxx
 *
 ****************************************************************************/

//main header
#include "ITkGlobalTrackMonAlg.h"

//Standard c++
#include <vector>
#include <memory>


ITkGlobalTrackMonAlg::ITkGlobalTrackMonAlg( const std::string& name, ISvcLocator* pSvcLocator ) : 
  AthMonitorAlgorithm(name, pSvcLocator) {}


ITkGlobalTrackMonAlg::~ITkGlobalTrackMonAlg() {}


StatusCode ITkGlobalTrackMonAlg::initialize() {
  ATH_CHECK( m_trackToVertexIPEstimator.retrieve() );

  
  if (!m_trackSelTool.empty() )      ATH_CHECK( m_trackSelTool.retrieve() );
  if (!m_tight_trackSelTool.empty()) ATH_CHECK( m_tight_trackSelTool.retrieve() );
  if (!m_loose_trackSelTool.empty()) ATH_CHECK( m_loose_trackSelTool.retrieve() );

  ATH_CHECK( m_trackParticleName.initialize() );
  ATH_CHECK( m_vxContainerName.initialize() );
  ATH_CHECK( m_jetContainerName.initialize() );
  
  return AthMonitorAlgorithm::initialize();
}


StatusCode ITkGlobalTrackMonAlg::fillHistograms( const EventContext& ctx ) const {
  using namespace Monitored;
  
  //*******************************************************************************
  //************************** Begin of filling Track Histograms ******************
  //*******************************************************************************

  ATH_MSG_DEBUG("Filling ITkGlobalTrackMonAlg");
  
  // For histogram naming
  const auto & trackGroup = getGroup("Track");
  
  // m_manager->lumiBlockNumber() // not used anymore, now use
  int lb       = GetEventInfo(ctx)->lumiBlock();
  auto lb_m    = Monitored::Scalar<int>( "m_lb", lb );
  
  // retrieving tracks
  auto trackParticles = SG::makeHandle(m_trackParticleName, ctx);
  
  // check for tracks
  if ( !(trackParticles.isValid()) ) {
    ATH_MSG_ERROR("ITkGlobalMonitoring: Track container "<< m_trackParticleName.key() << " could not be found.");
    return StatusCode::RECOVERABLE;
  } else {
    ATH_MSG_DEBUG("ITkGlobalMonitoring: Track container "<< trackParticles.name() <<" is found.");
  }
  
  // counters
  int nBase = 0;
  int nTight = 0;
  int nNoBL = 0;

  uint8_t iSummaryValue(0); // Dummy counter to retrieve summary values
  
  for (const auto trackPart: *trackParticles) {
    const Trk::Track * track = trackPart->track();
    if ( !track )
      {
	ATH_MSG_DEBUG( "ITkGlobalMonitoring: NULL track pointer in collection" );
	continue;
      }

    const Trk::Perigee *perigee = track->perigeeParameters();
    if ( !perigee )
      {
	ATH_MSG_DEBUG( "ITkGlobalMonitoring: NULL track->perigeeParameters pointer " );
	continue;
      }
    
    // =================================== //
    // Fill hits BEGINS
    
    float eta_perigee = perigee->eta();
    float phi_perigee = perigee->parameters()[Trk::phi0];
    
    auto eta_perigee_m   = Monitored::Scalar<float>( "m_eta_perigee", eta_perigee);
    auto phi_perigee_m   = Monitored::Scalar<float>( "m_phi_perigee", phi_perigee);

    auto eta_perigee_loose_m   = Monitored::Scalar<float>( "m_eta_perigee_loose", eta_perigee);
    auto phi_perigee_loose_m   = Monitored::Scalar<float>( "m_phi_perigee_loose", phi_perigee);

    // Loose tracks
    if ( m_loose_trackSelTool->accept(*track) ){
      fill(trackGroup, eta_perigee_loose_m, phi_perigee_loose_m);
    }

    // Base tracks
    if ( !m_trackSelTool->accept(*track) )
      continue;

    nBase++;

    fill(trackGroup, eta_perigee_m, phi_perigee_m); // Trk_Base_eta_phi
    
    int numberOfPixelHits = trackPart->summaryValue(iSummaryValue, xAOD::numberOfPixelHits) ? iSummaryValue : 0;
    int numberOfPixelDeadSensors = trackPart->summaryValue(iSummaryValue, xAOD::numberOfPixelDeadSensors) ? iSummaryValue : 0;
    int pixHits = numberOfPixelHits + numberOfPixelDeadSensors;
    auto pixHits_m  = Monitored::Scalar<int>( "m_pixHits", pixHits ); 
    fill(trackGroup, eta_perigee_m, phi_perigee_m, pixHits_m);
    fill(trackGroup, lb_m, pixHits_m);
    
    auto numberOfPixelDeadSensors_m = Monitored::Scalar<int>( "m_numberOfPixelDeadSensors", numberOfPixelDeadSensors );
    fill(trackGroup, eta_perigee_m, phi_perigee_m, numberOfPixelDeadSensors_m);

    int numberOfPixelSharedHits = trackPart->summaryValue(iSummaryValue, xAOD::numberOfPixelSharedHits) ? iSummaryValue : 0;
    auto numberOfPixelSharedHits_m = Monitored::Scalar<int>( "m_numberOfPixelSharedHits", numberOfPixelSharedHits);
    fill(trackGroup, eta_perigee_m, phi_perigee_m, numberOfPixelSharedHits_m);

    int numberOfPixelHoles = trackPart->summaryValue(iSummaryValue, xAOD::numberOfPixelHoles) ? iSummaryValue : 0;
    auto numberOfPixelHoles_m = Monitored::Scalar<int>( "m_numberOfPixelHoles", numberOfPixelHoles); 
    fill(trackGroup, eta_perigee_m, phi_perigee_m, numberOfPixelHoles_m);

    int numberOfPixelSplitHits = trackPart->summaryValue(iSummaryValue, xAOD::numberOfPixelSplitHits) ? iSummaryValue : 0;
    auto numberOfPixelSplitHits_m = Monitored::Scalar<int>( "m_numberOfPixelSplitHits", numberOfPixelSplitHits);
    fill(trackGroup, eta_perigee_m, phi_perigee_m, numberOfPixelSplitHits_m);

    int numberOfStripHits = trackPart->summaryValue(iSummaryValue, xAOD::numberOfSCTHits) ? iSummaryValue : 0;
    int numberOfStripDeadSensors = trackPart->summaryValue(iSummaryValue, xAOD::numberOfSCTDeadSensors) ? iSummaryValue : 0;
    int stripHits = numberOfStripHits + numberOfStripDeadSensors;
    auto stripHits_m  = Monitored::Scalar<int>( "m_stripHits", stripHits ); 
    fill(trackGroup, eta_perigee_m, phi_perigee_m, stripHits_m);
    fill(trackGroup, lb_m, stripHits_m);
    
    auto numberOfStripDeadSensors_m = Monitored::Scalar<int>( "m_numberOfStripDeadSensors", numberOfStripDeadSensors );
    fill(trackGroup, eta_perigee_m, phi_perigee_m, numberOfStripDeadSensors_m);

    int numberOfStripSharedHits = trackPart->summaryValue(iSummaryValue, xAOD::numberOfSCTSharedHits) ? iSummaryValue : 0;
    auto numberOfStripSharedHits_m = Monitored::Scalar<int>( "m_numberOfStripSharedHits", numberOfStripSharedHits);
    fill(trackGroup, eta_perigee_m, phi_perigee_m, numberOfStripSharedHits_m);

    int numberOfStripHoles   = trackPart->summaryValue(iSummaryValue, xAOD::numberOfSCTHoles) ? iSummaryValue : 0;
    auto numberOfStripHoles_m   = Monitored::Scalar<int>( "m_numberOfStripHoles", numberOfStripHoles);
    fill(trackGroup, eta_perigee_m, phi_perigee_m, numberOfStripHoles_m);

    // Fill hits ENDS 
    // =================================== //
    
    // =================================== //
    // FillEtaPhi BEGINS
    
    int NextToInnermostPixelLayerHit = 0;

    const xAOD::SummaryType expNInHitField = xAOD::expectInnermostPixelLayerHit;
    const xAOD::SummaryType nNInHitField = xAOD::numberOfInnermostPixelLayerHits;
    int expNInHit = trackPart->summaryValue(iSummaryValue, expNInHitField) ? iSummaryValue : 0;
    int nNInHits = trackPart->summaryValue(iSummaryValue, nNInHitField) ? iSummaryValue : 0;

    // no innermost pixel layer hit but a hit is expected
    if ( expNInHit==1 && nNInHits==0 ) NextToInnermostPixelLayerHit = 1 ;
    auto NextToInnermostPixelLayerHit_m = Monitored::Scalar<int>( "m_NextToInnermostPixelLayerHit", NextToInnermostPixelLayerHit);
    fill(trackGroup, eta_perigee_m, phi_perigee_m, NextToInnermostPixelLayerHit_m);
    
    // Tight track selection
    int track_pass_tight = 0;
    if ( m_tight_trackSelTool -> accept(*track) ){
      track_pass_tight = 1; // tight selection
      nTight++;
    }
    auto track_pass_tight_m = Monitored::Scalar<int>( "m_track_pass_tight", track_pass_tight);
    fill(trackGroup, eta_perigee_m, phi_perigee_m, track_pass_tight_m);
    
    // =================================== //
    // FillEtaPhi ENDS    
    
    int NoBL = 0;
    if ( expNInHit==1 && nNInHits==0 ) NoBL = 1;
    if (NoBL == 1) nNoBL++;
    auto NoBL_m = Monitored::Scalar<int>( "m_NoBL_LB", NoBL);
    fill(trackGroup, lb_m, NoBL_m);
    
    // FillHitMaps is false for now
    // FillHoles is false for now
    
    
  } // end of track loop
  
  // =================================== //
  // FillTide BEGINS
  if ( m_doTide )
    {
      // retrieving vertices
      auto handle_vxContainer = SG::makeHandle(m_vxContainerName, ctx);
      const xAOD::VertexContainer* vertexContainer = nullptr;

      if (!handle_vxContainer.isPresent()) {
	ATH_MSG_DEBUG ("ITkGlobalTrackMonAlg: StoreGate doesn't contain primary vertex container with key "+m_vxContainerName.key()+",may not be able to produce TIDE histograms");
      }
      if (!handle_vxContainer.isValid()) {
	ATH_MSG_DEBUG ("ITkGlobalTrackMonAlg: Could not retrieve primary vertex container with key "+m_vxContainerName.key()+",may not be able to produce TIDE histograms");
      }
      else {
        vertexContainer = handle_vxContainer.cptr();
      }

      // retrieving jets

      auto handle_jetContainer = SG::makeHandle(m_jetContainerName, ctx);

      if (!handle_jetContainer.isPresent()) {
	ATH_MSG_DEBUG ("ITkGlobalTrackMonAlg: StoreGate doesn't contain jet container with key "+m_jetContainerName.key()+",may not be able to produce TIDE histograms");
      }
      if (!handle_jetContainer.isValid()) {
	ATH_MSG_DEBUG ("ITkGlobalTrackMonAlg: Could not retrieve jet container with key "+m_jetContainerName.key()+",may not be able to produce TIDE histograms");
      }

      auto jetContainer = handle_jetContainer.cptr();

      if ( handle_jetContainer.isValid() ) {
	for ( auto jetItr = jetContainer->begin(); jetItr != jetContainer->end(); ++jetItr )
	  {
	    if ( (*jetItr)->pt() < 20000. ){
	      continue;
	    } 
	    std::vector<const xAOD::IParticle*> trackVector;
	    if ( !(*jetItr)->getAssociatedObjects<xAOD::IParticle>(xAOD::JetAttribute::GhostTrack, trackVector) ){
	      continue;
	    } 

	    for ( std::vector<const xAOD::IParticle*>::const_iterator trkItr = trackVector.begin(); trkItr != trackVector.end() ; ++trkItr )
	      {
		const xAOD::TrackParticle* trackPart = dynamic_cast<const xAOD::TrackParticle*>(*trkItr);
		if ( !trackPart ){
		  continue;
		} 
		uint8_t split;
		uint8_t shared;
		uint8_t pix;
		if ( trackPart->summaryValue(pix, xAOD::numberOfPixelHits) && pix )
		  {
		    const Trk::Perigee perigeeTIDE = trackPart->perigeeParameters();
		    const xAOD::Vertex* foundVertex { nullptr };
		    if ( handle_vxContainer.isValid() ){
		      for ( const auto *const vx : *vertexContainer )
			{
			  for ( const auto& tpLink : vx->trackParticleLinks() )
			    {
			      if ( *tpLink == trackPart )
				{
				  foundVertex = vx;
				  break;
				} 
			    } 
			  if (foundVertex) break;
			} 
		    } 
		    if ( foundVertex )
		      {
			std::unique_ptr<const Trk::ImpactParametersAndSigma>myIPandSigma(m_trackToVertexIPEstimator->estimate(trackPart,foundVertex));
			
			if ( myIPandSigma )
			  {
			    float jetassocdR = trackPart->p4().DeltaR( (*jetItr)->p4());
			    float jetassocd0Reso = std::abs( myIPandSigma->IPd0 / std::sqrt( myIPandSigma->sigmad0*myIPandSigma->sigmad0 + myIPandSigma->PVsigmad0*myIPandSigma->PVsigmad0 ) );
			    float jetassocz0Reso = std::abs( myIPandSigma->IPz0 / std::sqrt( myIPandSigma->sigmaz0*myIPandSigma->sigmaz0 + myIPandSigma->PVsigmaz0*myIPandSigma->PVsigmaz0 ) );
			    float jetassocIPReso = std::abs( myIPandSigma->IPd0 / std::sqrt( myIPandSigma->sigmad0*myIPandSigma->sigmad0 + myIPandSigma->PVsigmad0*myIPandSigma->PVsigmad0 ) );
			    auto jetassocdR_m = Monitored::Scalar<float>("m_jetassocdR", jetassocdR);
			    auto jetassocd0Reso_m = Monitored::Scalar<float>("m_jetassocd0Reso", jetassocd0Reso);
			    auto jetassocz0Reso_m = Monitored::Scalar<float>("m_jetassocz0Reso", jetassocz0Reso);
			    auto jetassocIPReso_m = Monitored::Scalar<float>("m_jetassocd0Reso", jetassocIPReso);

			    fill(trackGroup, jetassocdR_m, jetassocd0Reso_m);
			    fill(trackGroup, jetassocdR_m, jetassocz0Reso_m);
			    fill(trackGroup, lb_m, jetassocIPReso_m);    
			  } 
			
		      } 
		    if ( trackPart->summaryValue( split, xAOD::numberOfPixelSplitHits) ){
		      float frac = (double)split / pix;
		      float pixSplitdR = trackPart->p4().DeltaR( (*jetItr)->p4() );
		      auto pixSplitFrac_m = Monitored::Scalar<float>("m_pixSplitFrac", frac);
		      auto pixSplitdR_m =   Monitored::Scalar<float>("m_pixSplitdR", pixSplitdR);

		      fill(trackGroup, pixSplitdR_m, pixSplitFrac_m);
		      fill(trackGroup, lb_m, pixSplitFrac_m);
		    } 

		    if ( trackPart->summaryValue( shared, xAOD::numberOfPixelSharedHits) ){
                      float frac = (float)shared / pix;
                      float pixShareddR = trackPart->p4().DeltaR( (*jetItr)->p4() );
                      auto pixSharedFrac_m = Monitored::Scalar<float>("m_pixSharedFrac", frac);
                      auto pixShareddR_m =   Monitored::Scalar<float>("m_pixShareddR", pixShareddR);

                      fill(trackGroup, pixShareddR_m, pixSharedFrac_m);
                      fill(trackGroup, lb_m, pixSharedFrac_m);
		    } 
		  } 
	      } 
	  } 
      } 
    } 

  // FillTide ENDS
  // =================================== //

  // Filling per-event histograms
  auto nBase_m   = Monitored::Scalar<int>( "m_nBase", nBase);
  fill(trackGroup, nBase_m);
  
  auto nBaseLB_m   = Monitored::Scalar<int>( "m_nBase_LB", nBase);
  fill(trackGroup, lb_m, nBaseLB_m);
  
  auto nTight_m   = Monitored::Scalar<int>( "m_nTight_LB", nTight);
  fill(trackGroup, lb_m, nTight_m);
  
  
  auto nNoBL_m   = Monitored::Scalar<int>( "m_nNoBL_LB", nNoBL);
  fill(trackGroup, lb_m, nNoBL_m);
  


  
  
  //*******************************************************************************
  //**************************** End of filling Track Histograms ******************
  //*******************************************************************************
  
  return StatusCode::SUCCESS;
}
