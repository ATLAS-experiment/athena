/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "egammaCaloUtils/egammaPreSamplerShape.h"
#include "egammaCaloUtils/egammaEnergyPositionAllSamples.h"
//
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "CaloUtils/CaloLayerCalculator.h"
#include "xAODCaloEvent/CaloCluster.h"

StatusCode
egammaPreSamplerShape::execute(const xAOD::CaloCluster& cluster,
                               const CaloDetDescrManager& cmgr,
                               const CaloCellContainer& cell_container,
                               Info& info)
{
  //
  // Estimate shower shapes in pre sampler
  // based on hottest cell and deta,dphi windows
  // defined from the pre sampler granularity
  // with eta  = cluster->etaSample(sam)
  //      phi  = cluster->phiSample(sam)
  //      deta = 0.025
  //      dphi = 10.*0.1*(2.*M_PI/64)
  //

  // check if cluster is in barrel or in the end-cap
  if (!cluster.inBarrel() && !cluster.inEndcap()) {
    return StatusCode::SUCCESS;
  }
  // check if cluster is in barrel or end-cap
  const bool in_barrel = egammaEnergyPositionAllSamples::inBarrel(cluster, 0);

  // define accordingly the position of CaloSampling
  const CaloSampling::CaloSample sam = in_barrel ? CaloSampling::PreSamplerB : CaloSampling::PreSamplerE;
  const CaloSampling::CaloSample sam2 = in_barrel ? CaloSampling::EMB2 : CaloSampling::EME2;

  //
  // From the original (eta,phi) position, find the location
  // (sampling, barrel/end-cap, granularity)
  // For this we use the tool egammaEnergyAllSamples
  // which uses the CaloCluster method inBarrel() and inEndcap()
  // but also, in case close to the crack region where both
  // boolean can be true, the energy reconstructed in the sampling
  //

  // Fetch eta and phi of the sampling
  // Note that we use m_sam2 in the 2nd sampling, not in presampler
  double eta = cluster.etamax(sam2);
  double phi = cluster.phimax(sam2);

  // bad patch to avoid crash with dde
  if ((eta == 0. && phi == 0.) || std::abs(eta) > 100) {
    return StatusCode::SUCCESS;
  }
  // granularity in (eta,phi) in the pre sampler
  CaloCell_ID::SUBCALO subcalo = CaloCell_ID::LAREM;
  bool barrel = false;
  int sampling_or_module = 0;
  // CaloCellList needs both enums: subCalo and CaloSample
  CaloDetDescrManager::decode_sample(subcalo, barrel, sampling_or_module, sam);

  // Get the corresponding grannularities : needs to know where you are
  //                  the easiest is to look for the CaloDetDescrElement
  const CaloDetDescrElement* dde =
    cmgr.get_element(subcalo, sampling_or_module, barrel, eta, phi);
  // if object does not exist then return
  if (!dde) {
    return StatusCode::SUCCESS;
  }
  // local granularity
  double deta = dde->deta();
  double dphi = dde->dphi();
  // change eta,phi values
  eta = dde->eta_raw();
  phi = dde->phi_raw();
  // estimate the relevant quantities around the hottest cell
  // in the following eta X phi windows
  CaloLayerCalculator calc;
  StatusCode sc = StatusCode::SUCCESS;
  // 1X1
  sc = calc.fill(cmgr, &cell_container, eta, phi, deta, dphi, sam);
  if (sc.isFailure()) {
    return sc;
  }
  info.e011 = calc.em();
  // 3X3
  sc = calc.fill(cmgr, &cell_container, eta, phi, 3. * deta, 3. * dphi, sam);
  if (sc.isFailure()) {
    return sc;
  }
  info.e033 = calc.em();
  return StatusCode::SUCCESS;
}

