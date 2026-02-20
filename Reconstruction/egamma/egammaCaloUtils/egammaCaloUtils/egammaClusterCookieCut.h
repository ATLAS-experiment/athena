/*
 Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "xAODCaloEvent/CaloClusterFwd.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "CaloEvent/CaloCellContainer.h"

namespace egammaClusterCookieCut {

struct CookieCutPars {
  double maxDelEta;
  double maxDelPhi;
  double maxDelR2;
  bool recomputeMoments = false;
  bool fixCellWeights = false;
};

std::unique_ptr<xAOD::CaloCluster> cookieCut(
   const xAOD::CaloCluster& cluster,
   const CaloDetDescrManager& mgr,
   const DataLink<CaloCellContainer>& cellCont,
   const egammaClusterCookieCut::CookieCutPars& pars);

 
} // namespace egammaClusterCookieCut
