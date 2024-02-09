/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "egammaCaloUtils/CookieCutterHelpers.h"
#include "CaloGeoHelpers/proxim.h"

namespace CookieCutterHelpers
{
CentralPosition::CentralPosition(const std::vector<const xAOD::CaloCluster*>& clusters)
{
  for (const auto* cluster : clusters) {
    if (cluster->hasSampling(CaloSampling::EMB2)) {
      const float thisEmax = cluster->energy_max(CaloSampling::EMB2);
      if (thisEmax > emaxB) {
        emaxB = thisEmax;
        etaB = cluster->etamax(CaloSampling::EMB2);
        phiB = cluster->phimax(CaloSampling::EMB2);
      }
    }
    if (cluster->hasSampling(CaloSampling::EME2)) {
      const float thisEmax = cluster->energy_max(CaloSampling::EME2);
      if (thisEmax > emaxEC) {
        emaxEC = thisEmax;
        etaEC = cluster->etamax(CaloSampling::EME2);
        phiEC = cluster->phimax(CaloSampling::EME2);
      }
    }
    if (cluster->hasSampling(CaloSampling::FCAL0)) {
      const float thisEmax = cluster->energy_max(CaloSampling::FCAL0);
      if (thisEmax > emaxF) {
        emaxF = thisEmax;
        etaF = cluster->etamax(CaloSampling::FCAL0);
        phiF = cluster->phimax(CaloSampling::FCAL0);
      }
    }
  }
}

PhiSize::PhiSize(const CentralPosition& cp0, const xAOD::CaloCluster& cluster)
{
  auto cell_itr = cluster.cell_cbegin();
  auto cell_end = cluster.cell_cend();
  for (; cell_itr != cell_end; ++cell_itr) {

    const CaloCell* cell = *cell_itr;
    if (!cell) {
      continue;
    }

    const CaloDetDescrElement* dde = cell->caloDDE();
    if (!dde) {
      continue;
    }

    if (cp0.emaxB > 0 && CaloCell_ID::EMB2 == dde->getSampling()) {
      const float phi0 = cp0.phiB;
      double cell_phi = proxim(dde->phi_raw(), phi0);
      if (cell_phi > phi0) {
        auto diff = cell_phi - phi0;
        if (diff > plusB) {
          plusB = diff;
        }
      } else {
        auto diff = phi0 - cell_phi;
        if (diff > minusB) {
          minusB = diff;
        }
      }
    } else if (cp0.emaxEC > 0 && CaloCell_ID::EME2 == dde->getSampling()) {
      const float phi0 = cp0.phiEC;
      double cell_phi = proxim(dde->phi_raw(), phi0);
      if (cell_phi > phi0) {
        auto diff = cell_phi - phi0;
        if (diff > plusEC) {
          plusEC = diff;
        }
      } else {
        auto diff = phi0 - cell_phi;
        if (diff > minusEC) {
          minusEC = diff;
        }
      }
    }
  }
}
}

