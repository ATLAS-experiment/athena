/*
 Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "egammaCaloUtils/egammaClusterCookieCut.h"
#include "egammaCaloUtils/CookieCutterHelpers.h"

#include "CaloUtils/CaloClusterStoreHelper.h"
#include "CaloEvent/CaloClusterCellLinkContainer.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODCaloEvent/CaloClusterKineHelper.h"
#include "FourMomUtils/P4Helpers.h"

namespace {
  template <typename... T>
  void copyMoments(const xAOD::CaloCluster& src,
                   std::unique_ptr<xAOD::CaloCluster>& dest,
                   T... momentIds) {
    for (const auto& momentId : {momentIds...}) {
      double moment {};
      if (src.retrieveMoment(momentId, moment)) {
        dest->insertMoment(momentId, moment);
      }
    }
  }
}

std::unique_ptr<xAOD::CaloCluster> egammaClusterCookieCut::cookieCut(
  const xAOD::CaloCluster& cluster,
  const CaloDetDescrManager& mgr,
  const DataLink<CaloCellContainer>& cellCont,
  const egammaClusterCookieCut::CookieCutPars& pars
) {
  if (!cluster.hasSampling(CaloSampling::EME2) &&
      !cluster.hasSampling(CaloSampling::FCAL0)) {
    return nullptr;
  }

  CookieCutterHelpers::CentralPosition cp0({&cluster}, mgr);

  const bool isEC = cp0.emaxEC >= cp0.emaxF;
  const float eta = isEC ? cp0.etaEC : cp0.etaF;
  const float phi = isEC ? cp0.phiEC : cp0.phiF;

  auto newCluster = CaloClusterStoreHelper::makeCluster(cellCont);

  if (!newCluster) {
    return nullptr;
  }

  CaloClusterCellLink* newCellLinks = newCluster->getOwnCellLinks();
  if (!pars.recomputeMoments) {
    copyMoments(cluster,
		newCluster,
		xAOD::CaloCluster::CENTER_X,
		xAOD::CaloCluster::CENTER_Y,
		xAOD::CaloCluster::CENTER_Z,
		xAOD::CaloCluster::SECOND_LAMBDA,
		xAOD::CaloCluster::LATERAL,
		xAOD::CaloCluster::LONGITUDINAL,
		xAOD::CaloCluster::ENG_FRAC_MAX,
		xAOD::CaloCluster::SECOND_R,
		xAOD::CaloCluster::CENTER_LAMBDA,
		xAOD::CaloCluster::SECOND_ENG_DENS,
		xAOD::CaloCluster::SIGNIFICANCE);
  }

  const CaloClusterCellLink* cellLinks = cluster.getCellLinks();
  CaloClusterCellLink::const_iterator cellItr = cellLinks->begin();
  CaloClusterCellLink::const_iterator cellEnd = cellLinks->end();

  for (; cellItr != cellEnd; ++cellItr) {
    const float deltaEta = std::abs(eta - cellItr->eta());
    const float deltaPhi = std::abs(P4Helpers::deltaPhi(phi, cellItr->phi()));

    const float deltaEta2 = deltaEta * deltaEta;
    const float deltaPhi2 = deltaPhi * deltaPhi;

    const bool excludeCell = isEC ?
      (deltaEta >= pars.maxDelEta || deltaPhi >= pars.maxDelPhi) :
      (deltaEta2 + deltaPhi2 >= pars.maxDelR2);

    if (!excludeCell) {
      double w = pars.fixCellWeights ? 1. : cellItr.weight();
      newCellLinks->addCell(cellItr.index(), w);
    }
  }

  CaloClusterKineHelper::calculateKine(newCluster.get(), true, true);

  return newCluster;
}
