/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGGEPPERF_TOPOTOWERMAKER_H
#define TRIGGEPPERF_TOPOTOWERMAKER_H

#include "./ITowerMaker.h"
#include "./Cluster.h"  // <-- Make sure this is included

#include <map>
#include <string>
#include <vector>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "CaloEvent/CaloCellContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "GepCellMap.h"

namespace Gep {

class TopoTowerMaker : virtual public ITowerMaker {

public:
    TopoTowerMaker() = default;
    ~TopoTowerMaker() = default;

    std::vector<Gep::Cluster>
    makeTowers(const xAOD::CaloClusterContainer& clusters,
               const CaloCellContainer& cells) const override;

    std::string getName() const override;

private:
    // add private members if needed
};

} // namespace Gep

#endif // TRIGGEPPERF_TOPOTOWERMAKER_H
