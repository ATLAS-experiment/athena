/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODINDETMEASUREMENT_HGTDCLUSTERCONTAINER_H
#define XAODINDETMEASUREMENT_HGTDCLUSTERCONTAINER_H

#include "xAODInDetMeasurement/HGTDCluster.h"
#include "xAODInDetMeasurement/versions/HGTDClusterContainer_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
    /// Define the version of the HGTD cluster container
    typedef HGTDClusterContainer_v1 HGTDClusterContainer;
}

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::HGTDClusterContainer, 1419540398, 1 )

#endif // XAODINDETMEASUREMENT_HGTDCLUSTERCONTAINER_H