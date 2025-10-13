/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file GlobalSimulation/IeEmNbhoodTOBContainer.h
 * @author A. Martynwood, martyniu@cern.ch
 * @date October 2025
 * @brief Interface container class to hold IeEmNbhoodTOBs
 */

#ifndef GLOBALSIM_IEEMNBHOODTOBCONTAINER_H
#define GLOBALSIM_IEEMNBHOODTOBCONTAINER_H

#include "IeEmNbhoodTOB.h"

#include "AthContainers/DataVector.h"

namespace GlobalSim {
  namespace IOBitwise {
    /// Property: Defining the container object
    using IeEmNbhoodTOBContainer = DataVector<GlobalSim::IOBitwise::IeEmNbhoodTOB>;
  }
}

CLASS_DEF( GlobalSim::IOBitwise::IeEmNbhoodTOBContainer , 1285305937 , 1 )

#endif //GLOBALSIM_IEEMNBHOODTOBCONTAINER_H
