/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// EDM include(s):
#include "xAODCore/AddDVProxy.h"

// Local include(s):
#include "xAODInDetMeasurement/versions/PixelClusterContainer_v1.h"
#include "xAODInDetMeasurement/versions/StripClusterContainer_v1.h"
#include "xAODInDetMeasurement/versions/SpacePointContainer_v1.h"
#include "xAODInDetMeasurement/versions/HGTDClusterContainer_v1.h"

// Set up the collection proxies:
ADD_NS_DV_PROXY( xAOD, PixelClusterContainer_v1 );
ADD_NS_DV_PROXY( xAOD, StripClusterContainer_v1 );
ADD_NS_DV_PROXY( xAOD, SpacePointContainer_v1 );
ADD_NS_DV_PROXY( xAOD, HGTDClusterContainer_v1 );


