/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file GlobalSimulation/eEmTOBContainer.h
 * @author P. Sherwood, peter@cern.ch
 * @date October 2025
 * @brief Interface container class to hold CommonTOBs
 */

#ifndef GLOBALSIM_COMMONTOBCONTAINER_H
#define GLOBALSIM_COMMONTOBCONTAINER_H

#include "CommonTOB.h"

#include "AthContainers/DataVector.h"

namespace GlobalSim {
  namespace IOBitwise {

    using CommonTOBContainer = DataVector<GlobalSim::IOBitwise::CommonTOB>;
  }
}

CLASS_DEF( GlobalSim::IOBitwise::CommonTOBContainer , 1123153720 , 1 )

#endif
