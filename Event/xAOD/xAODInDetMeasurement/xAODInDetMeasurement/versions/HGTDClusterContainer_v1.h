/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */

#ifndef XAODINDETMEASUREMENT_HGTDCLUSTERCONTAINER_V1_H
#define XAODINDETMEASUREMENT_HGTDCLUSTERCONTAINER_V1_H


// Core include(s):
#include "AthContainers/DataVector.h"
#include "xAODInDetMeasurement/versions/HGTDCluster_v1.h"

namespace xAOD {
    /// The container is a simple typedef for now
    typedef DataVector< xAOD::HGTDCluster_v1 > HGTDClusterContainer_v1;
}

#endif // XAODINDETMEASUREMENT_HGTDCLUSTERCONTAINER_V1_H
