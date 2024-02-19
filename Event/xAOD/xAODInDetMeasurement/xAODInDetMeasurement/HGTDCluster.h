/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODINDETMEASUREMENT_HGTDCLUSTER_H
#define XAODINDETMEASUREMENT_HGTDCLUSTER_H

#include "xAODInDetMeasurement/versions/HGTDCluster_v1.h"

/// Namespace holding all the xAOD EDM classes
namespace xAOD {
    /// Define the version of the pixel cluster class
    typedef HGTDCluster_v1 HGTDCluster;
}

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::HGTDCluster, 239123881, 1 )

#endif // XAODINDETMEASUREMENT_HGTDCLUSTER_H