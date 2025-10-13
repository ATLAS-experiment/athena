/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_LARSTRIPNEIGHBORHOODCONTAINER_H
#define GLOBALSIM_LARSTRIPNEIGHBORHOODCONTAINER_H

#include "LArStripNeighborhood.h"


#include "AthContainers/DataVector.h"

namespace GlobalSim {
  using LArStripNeighborhoodContainer = DataVector<LArStripNeighborhood>;
}

CLASS_DEF( GlobalSim::LArStripNeighborhoodContainer , 1220312500 , 1 )

#endif
