/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_IEEMTOBCONTAINER_H
#define GLOBALSIM_IEEMTOBCONTAINER_H

#include "IeEmTOB.h"


#include "AthContainers/DataVector.h"

namespace GlobalSim {
  namespace IOBitwise {
    using IeEmTOBContainer = DataVector<GlobalSim::IOBitwise::IeEmTOB>;
  }
}

CLASS_DEF( GlobalSim::IOBitwise::IeEmTOBContainer , 1157079861 , 1 )


#endif
