/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//********************************************************************//
// NAME:     TauSubstructureVariables.cxx                             //
// AUTHORS:  M. Trottier-McDonald                                     //
// CREATED:  January 11 2010                                          //
//********************************************************************//

#include <algorithm> 
#include <cmath>
#include <sstream>

#include "xAODJet/Jet.h"
#include "xAODTau/TauJet.h"
#include "CxxUtils/trapping_fp.h"

#include "tauRecTools/TauSubstructureVariables.h"
#include "tauRecTools/HelperFunctions.h"

const float TauSubstructureVariables::DEFAULT = -1111.;

TauSubstructureVariables::TauSubstructureVariables( const std::string& name )
  : TauRecToolBase(name) {
}



StatusCode TauSubstructureVariables::execute(xAOD::TauJet& tau) const {

  //*****************************************************
  // calculate some new cluster based ID variables
  //*****************************************************
  // New cluster-based variables
  float totalEnergy(0.);
  float PSSEnergy(0.);
  float EMEnergy(0.);
  float HADEnergy(0.);

  TLorentzVector leadClusVec;
  TLorentzVector subLeadClusVec;
  TLorentzVector approxSubstructure4Vec;
  double clusELead = DEFAULT;
  double clusESubLead = DEFAULT;

  TLorentzVector tauAxis = tauRecTools::getTauAxis(tau, m_doVertexCorrection);

  // TODO: check which scale is needed here
  // p4 from cluster is at LC scale, p4 from vertexedCluster is at LC/EM scale for LC/EM seed jets
  std::vector<xAOD::CaloVertexedTopoCluster> vertexedClusterList = tau.vertexedClusters();

  tau.setDetail(xAOD::TauJetParameters::numTopoClusters, static_cast<int>(vertexedClusterList.size()));

  for (const xAOD::CaloVertexedTopoCluster& vertexedCluster : vertexedClusterList){
    // Tell clang to optimize assuming that FP operations may trap.
    CXXUTILS_TRAPPING_FP;
    // It is at EM/LC scale for EM/LC seed jets
    float clEnergy = vertexedCluster.e();

    const xAOD::CaloCluster& cluster = vertexedCluster.clust();

    // Calculate the fractions of energy in different calorimeter layers
    float PreSampler = cluster.eSample(CaloSampling::PreSamplerB) + cluster.eSample(CaloSampling::PreSamplerE);
    float EMLayer1   = cluster.eSample(CaloSampling::EMB1) + cluster.eSample(CaloSampling::EME1);
    float EMLayer2   = cluster.eSample(CaloSampling::EMB2) + cluster.eSample(CaloSampling::EME2);

    float Energy = cluster.rawE();
    float PSSF = (Energy != 0.) ? (PreSampler + EMLayer1) / Energy : 0.;
    float EM2F = (Energy != 0.) ? EMLayer2 / Energy : 0.;
    float EMF = PSSF + EM2F;

    PSSEnergy += PSSF * clEnergy;
    EMEnergy  += EMF * clEnergy;
    HADEnergy += (Energy != 0.) ? (1 - EMF) * clEnergy : 0.;

    TLorentzVector clusterP4 = vertexedCluster.p4();

    totalEnergy += clusterP4.E();
    
    if (tauAxis.DeltaR(clusterP4) < 0.2) {
      double clusEnergyBE = ( cluster.energyBE(0) + cluster.energyBE(1) + cluster.energyBE(2) );
		    
      if (clusEnergyBE > clusELead) {
	//change current leading cluster to subleading
	clusESubLead = clusELead;
	subLeadClusVec = leadClusVec;

	//set energy and 4-vector of leading cluster
	clusELead = clusEnergyBE;
	leadClusVec.SetPtEtaPhiM(clusELead/std::cosh(clusterP4.Eta()), clusterP4.Eta(), clusterP4.Phi(), 0.);
      }
      else if (clusEnergyBE > clusESubLead) {
	//set energy and 4-vector of subleading cluster only
	clusESubLead = clusEnergyBE;
	subLeadClusVec.SetPtEtaPhiM(clusESubLead/std::cosh(clusterP4.Eta()), clusterP4.Eta(), clusterP4.Phi(), 0.);
      }
    }	
  }

  if (clusELead > 0.) {
    approxSubstructure4Vec += leadClusVec;
  }
  if (clusESubLead > 0.) {
    approxSubstructure4Vec += subLeadClusVec;
  }

  // calculate trk momentum
  float trkSysMomentum(0.);
  for (size_t i=0; i < tau.nTracks(); ++i) {
    trkSysMomentum += tau.track(i)->pt() * std::cosh(tau.track(i)->eta());

    //adding the core tracks to the approximate substructure 4 vector
    approxSubstructure4Vec += tau.track(i)->p4();
  }

  // set new approximate energy flow variables for tau ID
  if(m_doVertexCorrection) {
    tau.setDetail(xAOD::TauJetParameters::ptRatioEflowApprox, static_cast<float>(approxSubstructure4Vec.Pt()/ tau.ptIntermediateAxis()) );
  }
  else {
    tau.setDetail(xAOD::TauJetParameters::ptRatioEflowApprox, static_cast<float>(approxSubstructure4Vec.Pt()/ tau.ptDetectorAxis()) );
  }
  tau.setDetail(xAOD::TauJetParameters::mEflowApprox, static_cast<float>(approxSubstructure4Vec.M()) );

  float fPSSFraction = (totalEnergy != 0.) ? PSSEnergy / totalEnergy : DEFAULT;
  float fChPIEMEOverCaloEME = (EMEnergy != 0.) ? (trkSysMomentum - HADEnergy) / EMEnergy : DEFAULT;
  float fEMPOverTrkSysP = DEFAULT;
  if (tau.nTracks() > 0) fEMPOverTrkSysP = (trkSysMomentum != 0.) ? EMEnergy / trkSysMomentum : DEFAULT;

  tau.setDetail(xAOD::TauJetParameters::PSSFraction, fPSSFraction);
  tau.setDetail(xAOD::TauJetParameters::ChPiEMEOverCaloEME, fChPIEMEOverCaloEME);
  tau.setDetail(xAOD::TauJetParameters::EMPOverTrkSysP, fEMPOverTrkSysP);

  // calculate dRMax
  size_t numTrack = tau.nTracks();
  if (numTrack > 0) {
    float dRmax = 0.;
    float dR = 0.;

    for (size_t i=0; i < numTrack; ++i) {
      dR = tau.track(i)->p4().DeltaR(tauAxis);
      if (dR > dRmax) dRmax = dR;
    }
    tau.setDetail(xAOD::TauJetParameters::dRmax, dRmax);
  }

  return StatusCode::SUCCESS;
}
