/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FINDMAXCELL_H
#define FINDMAXCELL_H

#include "xAODCaloEvent/CaloCluster.h"
#include "AthenaBaseComps/AthMessaging.h"
#include "GaudiKernel/StatusCode.h"

namespace egammaCellUtils
{
  struct MaxECell : public AthMessaging
  {
    double etaCell = 999;
    double phiCell = 999;
    StatusCode sc = StatusCode::FAILURE;

    MaxECell() = delete;
    MaxECell(const xAOD::CaloCluster *clus,
	     const std::string &cellCKey = "AllCalo",
	     bool UseWeightForMaxCell = false);
  };
}

#endif
