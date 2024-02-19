/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODINDETMEASUREMENT_HGTDCLUSTERAUXCONTAINER_H
#define XAODINDETMEASUREMENT_HGTDCLUSTERAUXCONTAINER_H

// Local include(s):
#include "xAODInDetMeasurement/versions/HGTDClusterAuxContainer_v1.h"

namespace xAOD {
   /// Definition of the current HGTD cluster auxiliary container
   ///
   typedef HGTDClusterAuxContainer_v1 HGTDClusterAuxContainer;
}

// Set up a CLID for the class:
#include "xAODCore/CLASS_DEF.h"
CLASS_DEF( xAOD::HGTDClusterAuxContainer, 1379454273, 1 )

#endif // XAODINDETMEASUREMENT_HGTDCLUSTERAUXCONTAINER_H