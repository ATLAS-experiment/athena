/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * Tau ID input variable calculator tool
 *
 * Author: Lorenz Hauswald
 */

#include "tauRecTools/TauIDVarCalculator.h"
#include "tauRecTools/HelperFunctions.h"

#include "CaloGeoHelpers/CaloSampling.h"
#include "FourMomUtils/xAODP4Helpers.h"

#include "TLorentzVector.h"

const float TauIDVarCalculator::LOW_NUMBER = -1111.;

TauIDVarCalculator::TauIDVarCalculator(const std::string& name):
  TauRecToolBase(name) {
}



StatusCode TauIDVarCalculator::execute(xAOD::TauJet& tau) const {
  static const SG::Accessor<float> acc_absipSigLeadTrk("absipSigLeadTrk");
  acc_absipSigLeadTrk(tau) = (tau.nTracks()>0) ? std::abs(tau.track(0)->d0SigTJVA()) : 0.;
  
  if(inTrigger()) return StatusCode::SUCCESS;
  
  //everything below is just for EleBDT!
  static const SG::Accessor<float> acc_absEtaLead("ABS_ETA_LEAD_TRACK"); 
  static const SG::ConstAccessor<float> acc_centFrac("centFrac");

  // Will: Fixed variables for R21
  static const SG::Accessor<float> acc_EMFracFixed("EMFracFixed");
  static const SG::Accessor<float> acc_hadLeakFracFixed("hadLeakFracFixed");
  static const SG::Accessor<float> acc_etHotShotWin("etHotShotWin"); 
  static const SG::Accessor<float> acc_etHotShotWinOverPtLeadTrk("etHotShotWinOverPtLeadTrk"); 


  // EMFracFixed and eHad1AtEMScaleFixed (for acc_hadLeakFracFixed)
  // --------------------------------------------------------------
  // ECAL Layers: 0, 1, 2. Don't include 3 because it is mismodelled!
  std::vector<CaloSampling::CaloSample> EMSamps = { 
        CaloSampling::PreSamplerB, CaloSampling::PreSamplerE, 
        CaloSampling::EMB1, CaloSampling::EME1, 
        CaloSampling::EMB2, CaloSampling::EME2 };
  // All HCAL Layers
  std::vector<CaloSampling::CaloSample> HadSamps = { 
        CaloSampling::HEC0, CaloSampling::HEC1, CaloSampling::HEC2, CaloSampling::HEC3, 
        CaloSampling::TileBar0, CaloSampling::TileBar1, CaloSampling::TileBar2,
        CaloSampling::TileGap1, CaloSampling::TileGap2, CaloSampling::TileGap3,
        CaloSampling::TileExt0, CaloSampling::TileExt1, CaloSampling::TileExt2};
  // First HCal Layer
  std::vector<CaloSampling::CaloSample> Had1Samps = { 
        CaloSampling::HEC0, CaloSampling::TileBar0, CaloSampling::TileGap1, CaloSampling::TileExt0};

  float eEMAtEMScaleFixed = 0.;
  float eHadAtEMScaleFixed = 0.;
  float eHad1AtEMScaleFixed = 0.;

  TLorentzVector tauAxis = tauRecTools::getTauAxis(tau, m_doVertexCorrection);

  std::vector<xAOD::CaloVertexedTopoCluster> vertexedClusterList = tau.vertexedClusters();
  for (const xAOD::CaloVertexedTopoCluster& vertexedCluster : vertexedClusterList){
    TLorentzVector clusterP4 = vertexedCluster.p4();
    
    if( clusterP4.DeltaR(tauAxis) > 0.2 ) continue;
   
    const xAOD::CaloCluster& cluster = vertexedCluster.clust(); 
    for( auto samp : EMSamps )
      eEMAtEMScaleFixed += cluster.eSample(samp);
    for( auto samp : HadSamps )
      eHadAtEMScaleFixed += cluster.eSample(samp);
    for( auto samp : Had1Samps )
      eHad1AtEMScaleFixed += cluster.eSample(samp);  
  }
  acc_EMFracFixed(tau) = ( eEMAtEMScaleFixed + eHadAtEMScaleFixed ) != 0. ? 
      eEMAtEMScaleFixed / ( eEMAtEMScaleFixed + eHadAtEMScaleFixed ) : LOW_NUMBER;
 
  if(tau.nTracks() > 0){
    const xAOD::TrackParticle* track = tau.track(0)->track();
    acc_absEtaLead(tau) = std::abs( track->eta() );

    // hadLeakFracFixed
    acc_hadLeakFracFixed(tau) = (track->p4().P() != 0.) ? eHad1AtEMScaleFixed / track->p4().P() : LOW_NUMBER;

    // HOT SHOTS!!!!!
    // --------------
    // Get track position extrapolated to EM1
    const xAOD::TauTrack* tauTrack = tau.track(0);
    float etaCalo = -10.0;
    float phiCalo = -10.0;
    if( not tauTrack->detail(xAOD::TauJetParameters::CaloSamplingEtaEM, etaCalo))
        ATH_MSG_WARNING("Failed to retrieve extrapolated chargedPFO eta");
    if( not tauTrack->detail(xAOD::TauJetParameters::CaloSamplingPhiEM, phiCalo))
        ATH_MSG_WARNING("Failed to retrieve extrapolated chargedPFO phi");
    ATH_MSG_DEBUG("track EM " << ", eta: " << etaCalo << ", phi: " << phiCalo );
    
    // Get hottest shot in dR<0.1 and in 0.05 x 0.1 window
    float etHotShotWin = 0.;
    for( const auto& shotLink : tau.shotPFOLinks() ){
        if( not shotLink.isValid() ){
            ATH_MSG_WARNING("Invalid shotLink");
            continue;
        }
        const xAOD::PFO* shot = *shotLink;
        float etShot = 0.;
        shot->attribute(xAOD::PFODetails::tauShots_pt3, etShot);
       
        // In 0.012 x 0.1 window
        if(std::abs(shot->eta() - etaCalo) > 0.012 ) continue;
        if(std::abs(xAOD::P4Helpers::deltaPhi(shot->phi(), phiCalo)) > 0.1 ) continue;
        if(etShot > etHotShotWin) etHotShotWin = etShot;
    }
    acc_etHotShotWin(tau) = etHotShotWin;
    acc_etHotShotWinOverPtLeadTrk(tau) = (track->pt() != 0.) ? etHotShotWin / track->pt() : LOW_NUMBER;

  }
  else{
    acc_absEtaLead(tau) = LOW_NUMBER;
    acc_hadLeakFracFixed(tau) = LOW_NUMBER;
    acc_etHotShotWin(tau) = LOW_NUMBER;
    acc_etHotShotWinOverPtLeadTrk(tau) = LOW_NUMBER; 
  }
 
  return StatusCode::SUCCESS;
}
