/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "egammaCaloUtils/CookieCutterHelpers.h"
#include "CaloGeoHelpers/proxim.h"

namespace CookieCutterHelpers
{
CentralPosition::CentralPosition(
  const std::vector<const xAOD::CaloCluster*>& clusters,
  const CaloDetDescrManager& mgr)
  : AthMessaging("CookieCutterHelpers::CentralPosition")
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

  if (emaxB > 0) {
    const CaloDetDescrElement* dde =
      mgr.get_element(CaloCell_ID::EMB2, etaB, phiB);
    if (dde) {
      etaB = dde->eta_raw();
      phiB = dde->phi_raw();
    } else {
      ATH_MSG_WARNING("Couldn't get CaloDetDescrElement from mgr for eta = "
                      << etaB << ", phi = " << phiB);
    }
  }
  if (emaxEC > 0) {
    const CaloDetDescrElement* dde =
      mgr.get_element(CaloCell_ID::EME2, etaEC, phiEC);
    if (dde) {
      etaEC = dde->eta_raw();
      phiEC = dde->phi_raw();
    } else {
      ATH_MSG_WARNING("Couldn't get CaloDetDescrElement from mgr for eta = "
                      << etaEC << ", phi = " << phiEC);
    }
  }
  if (emaxF > 0) {
    const CaloDetDescrElement* dde =
      mgr.get_element(CaloCell_ID::FCAL0, etaF, phiF);
    if (dde) {
      etaF = dde->eta_raw();
      phiF = dde->phi_raw();
    } else {
      ATH_MSG_WARNING("Couldn't get CaloDetDescrElement from mgr for eta = "
                      << etaF << ", phi = " << phiF);
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

