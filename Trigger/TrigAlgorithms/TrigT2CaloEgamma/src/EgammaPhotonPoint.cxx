/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

// ********************************************************************
// 
// NAME:     EgammaPhotonPoint.cxx
// PACKAGE:  Trigger/TrigAlgorithms/TrigT2CaloEgamma
// 
// AUTHOR:   D.O. Damazio
// 
//
// ********************************************************************

#include "CaloIdentifier/LArEM_ID.h"

//#include "TrigCaloEvent/TrigEMCluster.h"
#include "xAODTrigCalo/TrigEMCluster.h"
#include "CaloGeoHelpers/CaloSampling.h"

#include "EgammaPhotonPoint.h"
#include "TrigT2CaloCommon/Calo_Def.h"

#include "TrigSteeringEvent/TrigRoiDescriptor.h"

EgammaPhotonPoint::EgammaPhotonPoint(const std::string & type, const std::string & name, 
                   const IInterface* parent): IReAlgToolCalo(type, name, parent)
		   {
#ifndef NDEBUG
	// Create Geometry object
  // 0 -> CaloType EM, 2 -> Second Layer
  // m_geometry[0] = new T2Geometry(0,2);
#endif
}

StatusCode EgammaPhotonPoint::initialize(){
      ATH_CHECK(IReAlgToolCalo::initialize());
      ATH_CHECK( m_noiseCDOKey.initialize() );
      return StatusCode::SUCCESS;
}

StatusCode EgammaPhotonPoint::execute(xAOD::TrigEMCluster &rtrigEmCluster,
				 const IRoiDescriptor& roi,
				 const CaloDetDescrElement*& /*caloDDE*/,
                                 const EventContext& context) const { 
  

  // noisy business
  SG::ReadCondHandle<CaloNoise> noiseHdl{m_noiseCDOKey,context};
  const CaloNoise* noiseCDO=*noiseHdl;

  int sampling=2;
  LArTT_Selector<LArCellCont> sel;
  ATH_CHECK( m_dataSvc->loadCollections(context, roi, TTEM, sampling, sel) );

  double energyEta = 0.;
  double energyPhi = 0.;

  // add these variables to take care of phi wrap-around
  double energyNegPhi = 0.;     // SRA
  double energyNegPhiConv = 0.; // SRA
  double energyPosPhi = 0.;     // SRA

  // 1. Find seed cell (highest Et in ROI .. layer 2)
  // 2. Find Et weighted eta, phi in 3*7 cell (layer 2) (photon + e id)
  // 3. Find Et in cells of sizes 3*3, 3*7, 7*7 (layer 2 + strips)
  //                                            (3*7 for photon + e id)
  // 4. Find cluster width in 3*5 cell, layer 2 (photon id, needs
  //                                             parabolic parametrisation)
  // 5. Find strip energies and eta (2*5 window)
  // 6. Find frac73 (photon id), (E1-E2)/(E1+E2) (e + photon id)

  double seedEnergy = 0.;
  double seedPhi = 999.;
  double seedEta = 999.;
  double hotPhi = 999.;
  double hotEta = 999.;
  int ncells = 0;
  // LVL1 positions
  float etaL1 = rtrigEmCluster.eta();
  float phiL1 = rtrigEmCluster.phi();

  const LArEM_ID* emID = m_larMgr->getEM_ID();
  const LArCell* seedCell = nullptr;
  const LArCell* hotCell = nullptr;
  for (const LArCell* larcell : sel) {
    float noiseSigma = noiseCDO->getNoise(larcell->ID(),larcell->gain());
    if (larcell->energy() < 2.0*noiseSigma) continue;
    if (larcell->energy() > seedEnergy) { // Hottest cell seach
      float deta = std::abs(etaL1 - larcell->eta());
      if (deta < m_maxHotCellDeta) { // Eta check is faster. Do it First
        float dphi = std::abs(phiL1 - larcell->phi());
        dphi = std::abs(M_PI - dphi);
        dphi = std::abs(M_PI - dphi);
        if (dphi < m_maxHotCellDphi) {
          seedEnergy = larcell->energy();
          seedCell = larcell;
        } // End of dphi check
      }   // End of deta check
    }     // End of if energy
    ncells++;
  }
  if (seedCell != nullptr) {
    seedEta = seedCell->eta();
    seedPhi = seedCell->phi();
    // For the S-shape correction, we store the caloDDE of the hottest cell
    hotCell = seedCell;
    hotEta = hotCell->eta();
    hotPhi = hotCell->phi();
  }
  else {
    return StatusCode::SUCCESS;
  }

  float sum=0.0;
  for (const LArCell* larcell : sel) {
    float noiseSigma = noiseCDO->getNoise(larcell->ID(),larcell->gain());
    if (larcell->energy() < 2.0*noiseSigma) continue;
    float deta = std::abs(seedEta - larcell->eta());
    if (deta < 0.025 + 0.002) { // Eta check is faster.
                                // Do it First 0.025 is cell, plus a little loose
      float dphi = std::abs(seedPhi - larcell->phi());
      dphi = std::abs(M_PI - dphi);
      dphi = std::abs(M_PI - dphi);
      if (dphi < 0.025 + 0.002) { // the same here (around 2*pi/64/4, but ok)
        sum+=larcell->et();
      }   // End of dphi check
    }     // End of deta check
    ncells++;
  }

  // for samp 1 things should be simpler
  sampling=1;
  LArTT_Selector<LArCellCont> sel1;
  ATH_CHECK( m_dataSvc->loadCollections(context, roi, TTEM, sampling, sel1) );

  // Just get me the maximum energy
  float energy1st=-999.0;
  const LArCell* maxLayer1=nullptr;
  for (const LArCell* larcell : sel1) {
     float noiseSigma = noiseCDO->getNoise(larcell->ID(),larcell->gain());
     if (larcell->energy() < 2.0*noiseSigma) continue;
     if ( larcell->et() > energy1st ){
       energy1st=larcell->et();
       maxLayer1=larcell;
     }
  }
  float etaLayer1 = -999.0;
  if ( maxLayer1 ) etaLayer1 = maxLayer1->eta();


  std::cout << "CHECK ENERGY : " << etaL1 << " " << phiL1 << " " << seedEta << " " << seedPhi << " " << ncells << " " << sum << " " << etaLayer1 << std::endl;

    
  // Update cluster Variables

  rtrigEmCluster.setEnergy(sum);
  rtrigEmCluster.setEta(etaL1);
  rtrigEmCluster.setPhi(phiL1);
  rtrigEmCluster.setEta1(etaLayer1);
  rtrigEmCluster.setNCells(ncells);
        
  // Finished save EMShowerMinimal time


  return StatusCode::SUCCESS;
}
