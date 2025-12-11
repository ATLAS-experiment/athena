/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file GlobalSimulation/eEmEg1BDTTOBContainer.h
 * @brief Container class to hold eEmEg1BDTTOBs
 */

#ifndef GLOBALSIM_EG1BDTTOBCONTAINER_H
#define GLOBALSIM_EG1BDTTOBCONTAINER_H

#include "eEmEg1BDTTOB.h"

#include "AthContainers/DataVector.h"

DATAVECTOR_BASE(GlobalSim::IOBitwise::eEmEg1BDTTOB,
		GlobalSim::IOBitwise::eEmTOB);

namespace GlobalSim {
  namespace IOBitwise {
    using eEmEg1BDTTOBContainer =
      DataVector<GlobalSim::IOBitwise::eEmEg1BDTTOB>;
  }
}

CLASS_DEF( GlobalSim::IOBitwise::eEmEg1BDTTOBContainer , 1270160788 , 1 )


#endif
