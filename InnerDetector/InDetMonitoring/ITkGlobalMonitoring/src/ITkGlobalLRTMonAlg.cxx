/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


/** @file ITkGlobalLRTMonAlg.h
 * Implementation of inner detector global LRT  monitoring tool
 *
 *@author
 * Hamza Hanif  <hamza.hanif@cern.ch> 
 *
 * based on InDetGlobalTrackMonTool.cxx (Leonid Serkin  and  Per Johansson)
 *
 ****************************************************************************/

//main header
#include "ITkGlobalLRTMonAlg.h"

//Standard c++
#include <vector>
#include <memory>


ITkGlobalLRTMonAlg::ITkGlobalLRTMonAlg( const std::string& name, ISvcLocator* pSvcLocator ) : 
  AthMonitorAlgorithm(name, pSvcLocator),
  m_trackSelTool      ( "InDet::InDetTrackSelectionTool/TrackSelectionTool", this)
{
  //jo flags go here, keys and some tools -> in class
  declareProperty( "TrackSelectionTool", m_trackSelTool);
}


ITkGlobalLRTMonAlg::~ITkGlobalLRTMonAlg() {}


StatusCode ITkGlobalLRTMonAlg::initialize() {

  if (!m_trackSelTool.empty() )      ATH_CHECK( m_trackSelTool.retrieve() );

  ATH_CHECK( m_trackParticleName.initialize() );
  
  return AthMonitorAlgorithm::initialize();
}


StatusCode ITkGlobalLRTMonAlg::fillHistograms( const EventContext& ctx ) const {
  using namespace Monitored;
  
  //*******************************************************************************
  //************************** Begin of filling Track Histograms ******************
  //*******************************************************************************

  ATH_MSG_DEBUG("Filling ITkGlobalLRTMonAlg");
  
  // For histogram naming
  const auto & lrtGroup = getGroup("LRT");
  
  int lb       = GetEventInfo(ctx)->lumiBlock();
  auto lb_m    = Monitored::Scalar<int>( "m_lb", lb );
  
  
  float lumiPerBCID       = lbAverageInteractionsPerCrossing(ctx);
  auto lumiPerBCID_m    = Monitored::Scalar<float>( "m_lumiPerBCID", lumiPerBCID );


  // retrieving tracks
  auto LRT_trackParticles = SG::makeHandle(m_trackParticleName, ctx);
  
  // check for tracks
  if ( !(LRT_trackParticles.isValid()) ) {
    ATH_MSG_ERROR("ITkGlobalMonitoring: Track container "<< m_trackParticleName.key() << " could not be found.");
    return StatusCode::RECOVERABLE;
  } else {
    ATH_MSG_DEBUG("ITkGlobalMonitoring: Track container "<< LRT_trackParticles.name() <<" is found.");
  }
  
  // counters
  int nBase = 0;
  int nNoBL = 0;
  
  uint8_t iSummaryValue(0); // Dummy counter to retrieve summary values

  for (const auto trackPart: *LRT_trackParticles) {
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

    // Loose primary tracks
    if ( !m_trackSelTool->accept(*track) )
      continue;
    
    nBase++;
    
    
    // =================================== //
    // Fill hits BEGINS

    float eta_perigee = perigee->eta();
    float phi_perigee = perigee->parameters()[Trk::phi0];
    
    auto eta_perigee_m   = Monitored::Scalar<float>( "m_eta_perigee", eta_perigee);
    auto phi_perigee_m   = Monitored::Scalar<float>( "m_phi_perigee", phi_perigee);
    fill(lrtGroup, eta_perigee_m, phi_perigee_m); // Trk_Base_eta_phi
    fill(lrtGroup, eta_perigee_m);
    fill(lrtGroup, phi_perigee_m);

    float d0_perigee = perigee->parameters()[Trk::d0];    
    auto d0_perigee_m   = Monitored::Scalar<float>( "m_d0_perigee", d0_perigee);
    fill(lrtGroup, d0_perigee_m);

    float z0_perigee = perigee->parameters()[Trk::z0];
    auto z0_perigee_m   = Monitored::Scalar<float>( "m_z0_perigee", z0_perigee);
    fill(lrtGroup, z0_perigee_m); 

    float theta = perigee->parameters()[Trk::theta];
    float qOverPt = perigee->parameters()[Trk::qOverP]/sin(theta);
    float charge = perigee->charge();
    float pT = 0;
    if ( qOverPt != 0 ) {
      pT = (1/qOverPt)*(charge);    
      auto pT_m = Monitored::Scalar<float>("m_trkPt", pT/1000); 
      fill(lrtGroup, pT_m);
    }

    int numberOfPixelHits = trackPart->summaryValue(iSummaryValue, xAOD::numberOfPixelHits) ? iSummaryValue : 0;
    int numberOfPixelDeadSensors = trackPart->summaryValue(iSummaryValue, xAOD::numberOfPixelDeadSensors) ? iSummaryValue : 0;
    int pixHits = numberOfPixelHits + numberOfPixelDeadSensors;
    auto pixHits_m  = Monitored::Scalar<int>( "m_pixHits", pixHits ); 
    fill(lrtGroup, eta_perigee_m, phi_perigee_m, pixHits_m);
    fill(lrtGroup, lb_m, pixHits_m);
    
    auto numberOfPixelDeadSensors_m = Monitored::Scalar<int>( "m_numberOfPixelDeadSensors", numberOfPixelDeadSensors );
    fill(lrtGroup, eta_perigee_m, phi_perigee_m, numberOfPixelDeadSensors_m);

    int numberOfPixelSharedHits = trackPart->summaryValue(iSummaryValue, xAOD::numberOfPixelSharedHits) ? iSummaryValue : 0;
    auto numberOfPixelSharedHits_m = Monitored::Scalar<int>( "m_numberOfPixelSharedHits", numberOfPixelSharedHits);
    fill(lrtGroup, eta_perigee_m, phi_perigee_m, numberOfPixelSharedHits_m);

    int numberOfPixelHoles = trackPart->summaryValue(iSummaryValue, xAOD::numberOfPixelHoles) ? iSummaryValue : 0;
    auto numberOfPixelHoles_m = Monitored::Scalar<int>( "m_numberOfPixelHoles", numberOfPixelHoles); 
    fill(lrtGroup, eta_perigee_m, phi_perigee_m, numberOfPixelHoles_m);

    int numberOfPixelSplitHits = trackPart->summaryValue(iSummaryValue, xAOD::numberOfPixelSplitHits) ? iSummaryValue : 0;
    auto numberOfPixelSplitHits_m = Monitored::Scalar<int>( "m_numberOfPixelSplitHits", numberOfPixelSplitHits);
    fill(lrtGroup, eta_perigee_m, phi_perigee_m, numberOfPixelSplitHits_m);

    int numberOfStripHits = trackPart->summaryValue(iSummaryValue, xAOD::numberOfSCTHits) ? iSummaryValue : 0;
    int numberOfStripDeadSensors = trackPart->summaryValue(iSummaryValue, xAOD::numberOfSCTDeadSensors) ? iSummaryValue : 0;
    int stripHits = numberOfStripHits + numberOfStripDeadSensors;
    auto stripHits_m  = Monitored::Scalar<int>( "m_stripHits", stripHits ); 
    fill(lrtGroup, eta_perigee_m, phi_perigee_m, stripHits_m);
    fill(lrtGroup, lb_m, stripHits_m);
    
    auto numberOfStripDeadSensors_m = Monitored::Scalar<int>( "m_numberOfStripDeadSensors", numberOfStripDeadSensors );
    fill(lrtGroup, eta_perigee_m, phi_perigee_m, numberOfStripDeadSensors_m);
    
    int numberOfStripSharedHits = trackPart->summaryValue(iSummaryValue, xAOD::numberOfSCTSharedHits) ? iSummaryValue : 0;
    auto numberOfStripSharedHits_m = Monitored::Scalar<int>( "m_numberOfStripSharedHits", numberOfStripSharedHits);
    fill(lrtGroup, eta_perigee_m, phi_perigee_m, numberOfStripSharedHits_m);
    
    int numberOfStripHoles   = trackPart->summaryValue(iSummaryValue, xAOD::numberOfSCTHoles) ? iSummaryValue : 0;
    auto numberOfStripHoles_m   = Monitored::Scalar<int>( "m_numberOfStripHoles", numberOfStripHoles);
    fill(lrtGroup, eta_perigee_m, phi_perigee_m, numberOfStripHoles_m);

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
    fill(lrtGroup, eta_perigee_m, phi_perigee_m, NextToInnermostPixelLayerHit_m);
    
// =================================== //
    // FillEtaPhi ENDS    
    
    int NoBL = 0;
    if ( expNInHit==1 && nNInHits==0 ) NoBL = 1;
    if (NoBL == 1) nNoBL++;
    auto NoBL_m = Monitored::Scalar<int>( "m_NoBL_LB", NoBL);
    fill(lrtGroup, lb_m, NoBL_m);
    
    // FillHitMaps is false for now
    // FillHoles is false for now
    
    
  } // end of track loop
  
  // Filling per-event histograms
  auto nBase_m   = Monitored::Scalar<int>( "m_nBase", nBase);
  fill(lrtGroup, nBase_m);
  
  auto nBaseLB_m   = Monitored::Scalar<int>( "m_nBase_LB", nBase);
  fill(lrtGroup, lb_m, nBaseLB_m);

  fill(lrtGroup, lumiPerBCID_m, nBaseLB_m);

  
  auto nNoBL_m   = Monitored::Scalar<int>( "m_nNoBL_LB", nNoBL);
  fill(lrtGroup, lb_m, nNoBL_m);
  
  
  //*******************************************************************************
  //**************************** End of filling Track Histograms ******************
  //*******************************************************************************
  
  return StatusCode::SUCCESS;
}
