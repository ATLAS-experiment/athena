/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file GlobalSimulation/eEmTOBContainer.h
 * @author P. Sherwood, peter@cern.ch
 * @date October 2025
 * @brief Interface container class to hold IeEmTOBs
 */

#ifndef GLOBALSIM_EMETOBCONTAINER_H
#define GLOBALSIM_EMETOBCONTAINER_H

#include "eEmTOB.h"


#include "AthContainers/DataVector.h"

DATAVECTOR_BASE(GlobalSim::IOBitwise::eEmTOB, GlobalSim::IOBitwise::CommonTOB);

namespace GlobalSim {
  namespace IOBitwise {

    using eEmTOBContainer = DataVector<GlobalSim::IOBitwise::eEmTOB>;
  }
}

CLASS_DEF( GlobalSim::IOBitwise::eEmTOBContainer , 1271357431 , 1 )


#endif
