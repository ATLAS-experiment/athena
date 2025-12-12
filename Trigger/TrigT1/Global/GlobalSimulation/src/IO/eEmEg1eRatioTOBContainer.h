/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file GlobalSimulation/eEmEg1BDTTOBContainer.h
 * @brief Container class to hold eEmEg1BDTTOBs
 */

#ifndef GLOBALSIM_EG1ERATIOTOBCONTAINER_H
#define GLOBALSIM_EG1ERATIOTOBCONTAINER_H

#include "eEmEg1eRatioTOB.h"

#include "AthContainers/DataVector.h"

DATAVECTOR_BASE(GlobalSim::IOBitwise::eEmEg1eRatioTOB,
		GlobalSim::IOBitwise::eEmTOB);

namespace GlobalSim {
  namespace IOBitwise {
    using eEmEg1eRatioTOBContainer =
      DataVector<GlobalSim::IOBitwise::eEmEg1eRatioTOB>;
  }
}


CLASS_DEF( GlobalSim::IOBitwise::eEmEg1eRatioTOBContainer , 1328416019 , 1 )


#endif
