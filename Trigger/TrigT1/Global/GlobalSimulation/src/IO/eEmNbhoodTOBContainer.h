/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file GlobalSimulation/eEmNbhoodTOBContainer.h
 * @author A. Martynwood, martyniu@cern.ch
 * @date October 2025
 * @brief Container class to hold eEmNbhoodTOBs
 */

#ifndef GLOBALSIM_EEMNBHOODTOBCONTAINER_H
#define GLOBALSIM_EEMNBHOODTOBCONTAINER_H

#include "eEmNbhoodTOB.h"

#include "AthContainers/DataVector.h"

namespace GlobalSim {
  namespace IOBitwise {
    /// Property: Defining the container object
    using eEmNbhoodTOBContainer = DataVector<GlobalSim::IOBitwise::eEmNbhoodTOB>;
  }
}

CLASS_DEF( GlobalSim::IOBitwise::eEmNbhoodTOBContainer , 1103580971 , 1 )

#endif //GLOBALSIM_EEMNBHOODTOBCONTAINER_H
