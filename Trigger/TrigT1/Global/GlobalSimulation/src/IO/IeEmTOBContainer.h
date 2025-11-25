/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file GlobalSimulation/IeEmNbhoodTOBContainer.h
 * @author P. Sherwood, peter@cern.ch
 * @date October 2025
 * @brief Interface container class to hold IeEmTOBs
 */

#ifndef GLOBALSIM_IEEMTOBCONTAINER_H
#define GLOBALSIM_IEEMTOBCONTAINER_H

#include "IeEmTOB.h"


#include "AthContainers/DataVector.h"

namespace GlobalSim {
  namespace IOBitwise {
    /// Property: Container to hold IeEmTOB objects
    using IeEmTOBContainer = DataVector<GlobalSim::IOBitwise::IeEmTOB>;
  }
}

CLASS_DEF( GlobalSim::IOBitwise::IeEmTOBContainer , 1157079861 , 1 )


#endif
