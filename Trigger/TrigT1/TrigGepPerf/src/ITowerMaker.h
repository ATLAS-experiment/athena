/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGGEPPERF_ITOWERMAKER_H
#define TRIGGEPPERF_ITOWERMAKER_H

#include "./Cluster.h"
#include "./GepCaloCell.h"
#include "GepCellMap.h"

#include "CaloEvent/CaloCellContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODEventInfo/EventInfo.h"

#include <map>
#include <string>
#include <memory>

namespace Gep{
  class ITowerMaker
  {
  public:
    
    virtual std::vector<Gep::Cluster> makeTowers(const xAOD::CaloClusterContainer&, const CaloCellContainer&) const = 0;

    virtual std::string getName() const = 0;
    
    virtual ~ITowerMaker() = default;
    
  };
}

#endif
