/*
 Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#include "egammaCaloUtils/egammaClusterHelpers.h"

#include "CaloUtils/CaloLayerCalculator.h"
#include "CaloUtils/CaloCellDetPos.h"

namespace egammaClusterHelpers {

  std::pair<const double, const double>
  etaphi_range(const CaloDetDescrManager& mgr,
	       double eta,
	       double phi,
	       CaloCell_ID::CaloSample sampling,
	       const CaloDetDescrElement* elt)
  {
    // Should be smaller than the eta half-width of any cell.
    constexpr double eps = 0.001;
    
    // Now look in the negative eta direction, on the low (left) side.
    const CaloDetDescrElement* elt_l = mgr.get_element_raw(sampling, eta - elt->deta() - eps, phi);
    double deta_l = elt_l ? std::abs(eta - elt_l->eta_raw()) + eps : 0.;
    
    // Now look in the positive eta direction, on the high (right) side.
    const CaloDetDescrElement* elt_r = mgr.get_element_raw(sampling, eta + elt->deta() + eps, phi);
    double deta_r = elt_r ? std::abs(eta - elt_r->eta_raw()) + eps : 0.; 
    
    // Now for the phi variation.
    // The phi size can change as a function of eta, but not of phi.
    // Thus we have to look again at the adjacent eta cells, and
    // take the largest variation.
    
    // Now look in the negative eta direction, on the low-eta () side.
    elt_l = mgr.get_element_raw(sampling, eta - elt->deta() - eps, CaloPhiRange::fix(phi - elt->dphi() - eps));
    double dphi_l = elt_l ? std::abs(CaloPhiRange::fix(phi - elt_l->phi_raw())) + eps : 0.; 
    
    // Now look in the positive eta direction, on the positive (down) side.
    elt_r = mgr.get_element_raw(sampling, eta + elt->deta() + eps, CaloPhiRange::fix(phi - elt->dphi() - eps));
    double dphi_r = elt_r ? std::abs(CaloPhiRange::fix(phi - elt_r->phi_raw())) + eps : 0.; 
  
    // Total is twice the maximum.
    return {2 * std::max(deta_r, deta_l), 2 * std::max(dphi_l, dphi_r)};
}

  void
  fillPositionsInCalo(xAOD::CaloCluster* cluster, const CaloDetDescrManager& mgr)
  {
    bool isBarrel = cluster->inBarrel();
    const bool isEndcap = cluster->inEndcap();
    if (isBarrel && isEndcap) {
      isBarrel = cluster->eSample(CaloSampling::EMB2)
	>= cluster->eSample(CaloSampling::EME2);
    }
    CaloCell_ID::CaloSample sample =
      isBarrel ? CaloCell_ID::EMB2 : CaloCell_ID::EME2;
    // eta and phi of the cluster in the calorimeter frame
    double eta;
    double phi;
    CaloCellDetPos::getDetPosition(
				   mgr, sample, cluster->eta(), cluster->phi(), eta, phi);
    cluster->insertMoment(xAOD::CaloCluster::ETACALOFRAME, eta);
    cluster->insertMoment(xAOD::CaloCluster::PHICALOFRAME, phi);
    //  eta in the second sampling
    CaloCellDetPos::getDetPosition(
				   mgr, sample, cluster->etaBE(2), cluster->phiBE(2), eta, phi);
    cluster->insertMoment(xAOD::CaloCluster::ETA2CALOFRAME, eta);
    cluster->insertMoment(xAOD::CaloCluster::PHI2CALOFRAME, phi);
    //  eta in the first sampling
    sample = isBarrel ? CaloCell_ID::EMB1 : CaloCell_ID::EME1;
    CaloCellDetPos::getDetPosition(
				   mgr, sample, cluster->etaBE(1), cluster->phiBE(1), eta, phi);
    cluster->insertMoment(xAOD::CaloCluster::ETA1CALOFRAME, eta);
    cluster->insertMoment(xAOD::CaloCluster::PHI1CALOFRAME, phi);
  }
  
  void
  makeCorrection1(xAOD::CaloCluster* cluster,
		  const CaloDetDescrManager& mgr,
		  const CaloSampling::CaloSample sample)
  {
    const double clusterEtaMax = cluster->etamax(sample);
    const double clusterPhiMax = cluster->phimax(sample);
    
    // Protections.
    if (clusterEtaMax == -999. || clusterPhiMax == -999.) {
      return;
    }
    if (std::abs(clusterEtaMax) < 1E-6 && std::abs(clusterPhiMax) < 1E-6) {
      return;
    }
    
    // Get the hottest in raw co-ordinates
    // We have two kinds of enums ...
    const CaloCell_ID::CaloSample xsample = 
				  (sample == CaloSampling::EMB1) ? CaloCell_ID::EMB1 : CaloCell_ID::EME1;
    
    const CaloDetDescrElement* dde = mgr.get_element(xsample, 
                                                   clusterEtaMax, 
						     clusterPhiMax);

    if (!dde) {
      return;
    }
    
    double etamax = dde->eta_raw();
    double phimax = dde->phi_raw();
    
    const CaloDetDescrElement* elt = mgr.get_element_raw(xsample, etamax, phimax);
    if (!elt) {
      return;
    }
    
  // Now Locate the +-1 range, use raw co-ordinates here.
    auto [detastr, dphistr] = etaphi_range(mgr, etamax, phimax, xsample, elt);
    
    // Given the range refine the position employing the smaller window
    if (detastr > 0 && dphistr > 0) {
      CaloLayerCalculator helper;
      const auto* const cellLink = cluster->getCellLinks();
      helper.fill(cellLink->begin(),
		  cellLink->end(),
		  etamax,
		  phimax,
		  detastr,
		  dphistr,
		  sample);
      
      // Here is where we (re-)fill the eta in the 1st sampling
      if (helper.etam() != -999.) {
	// This is "real" atlas co-ordinates
      cluster->setEta(sample, helper.etam());
      }
    }
  }

  void
  refineEta1Position(xAOD::CaloCluster* cluster, const CaloDetDescrManager& mgr)
  {
    // This only makes sense if we have cells there
    if (!cluster->hasSampling(CaloSampling::EMB1) &&
	!cluster->hasSampling(CaloSampling::EME1)) {
      return;
    }
    // Now calculare the position using cells in barrel or endcap or both
    const double aeta = std::abs(cluster->etaBE(2));
    if (aeta < 1.6 && cluster->hasSampling(CaloSampling::EMB1)) {
      makeCorrection1(cluster, mgr, CaloSampling::EMB1);
    }
    if (aeta > 1.3 && cluster->hasSampling(CaloSampling::EME1)) {
      makeCorrection1(cluster, mgr, CaloSampling::EME1);
    }
  }
}

  
  
