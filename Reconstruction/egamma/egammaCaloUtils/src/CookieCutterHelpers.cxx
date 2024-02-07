/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "egammaCaloUtils/CookieCutterHelpers.h"
#include "CaloGeoHelpers/proxim.h"

namespace CookieCutterHelpers
{
CentralPosition
findCentralPositionEM2(const std::vector<const xAOD::CaloCluster*>& clusters)
{
  CentralPosition cp;
  for (const auto* cluster : clusters) {
    if (cluster->hasSampling(CaloSampling::EMB2)) {
      const float thisEmax = cluster->energy_max(CaloSampling::EMB2);
      if (thisEmax > cp.emaxB) {
        cp.emaxB = thisEmax;
        cp.etaB = cluster->etamax(CaloSampling::EMB2);
        cp.phiB = cluster->phimax(CaloSampling::EMB2);
      }
    }
    if (cluster->hasSampling(CaloSampling::EME2)) {
      const float thisEmax = cluster->energy_max(CaloSampling::EME2);
      if (thisEmax > cp.emaxEC) {
        cp.emaxEC = thisEmax;
        cp.etaEC = cluster->etamax(CaloSampling::EME2);
        cp.phiEC = cluster->phimax(CaloSampling::EME2);
      }
    }
    if (cluster->hasSampling(CaloSampling::FCAL0)) {
      const float thisEmax = cluster->energy_max(CaloSampling::FCAL0);
      if (thisEmax > cp.emaxF) {
        cp.emaxF = thisEmax;
        cp.etaF = cluster->etamax(CaloSampling::FCAL0);
        cp.phiF = cluster->phimax(CaloSampling::FCAL0);
      }
    }
  }
  return cp;
}

/** Find the size of the cluster in phi using L2 cells.
 *
 * @param cp0: the reference position in calo-coordinates
 * @param cluster: the cluster filled with L2 and L3 cells
 *
 * The window is computed using only cells in the second layer.
 * Asymmetric sizes are computed for barrel and endcap. The size
 * is the maximum difference in phi between the center of a cell
 * and the refence, considering separately cells in the barrel
 * and in the endcap. The computation is done separately for the
 * cells with phi < reference phi or >=. A cutoff value of 1 is used.
 */
PhiSize
findPhiSize(const CentralPosition& cp0, const xAOD::CaloCluster& cluster) {

  PhiSize phiSize;
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
        if (diff > phiSize.plusB) {
          phiSize.plusB = diff;
        }
      } else {
        auto diff = phi0 - cell_phi;
        if (diff > phiSize.minusB) {
          phiSize.minusB = diff;
        }
      }
    } else if (cp0.emaxEC > 0 && CaloCell_ID::EME2 == dde->getSampling()) {
      const float phi0 = cp0.phiEC;
      double cell_phi = proxim(dde->phi_raw(), phi0);
      if (cell_phi > phi0) {
        auto diff = cell_phi - phi0;
        if (diff > phiSize.plusEC) {
          phiSize.plusEC = diff;
        }
      } else {
        auto diff = phi0 - cell_phi;
        if (diff > phiSize.minusEC) {
          phiSize.minusEC = diff;
        }
      }
    }
  }
  return phiSize;
}
}

