/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGGEPPERF_TCTOWERMAKER_H
#define TRIGGEPPERF_TCTOWERMAKER_H

#include "./ITowerMaker.h"
#include <map>
#include <string>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODCaloEvent/CaloClusterContainer.h"

namespace Gep{
  class TCTowerMaker : virtual public ITowerMaker {

public:

    TCTowerMaker() = default;
    ~TCTowerMaker() = default;

    std::vector<Gep::Cluster>
    makeTowers(const xAOD::CaloClusterContainer& clusters, const CaloCellContainer& cells) const override;

    std::string getName() const override;

 private: 

  }; 
}
#endif //> !TRIGGEPPERF_TCTOWERMAKER_H
