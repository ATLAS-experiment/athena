/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef GLOBALSIM_IEEMNEG1ERATIOTOBCONTAINER_H
#define GLOBALSIM_IEEMNEG1ERATIOTOBCONTAINER_H

#include "IeEmEg1eRatioTOB.h"

#include "AthContainers/DataVector.h"

namespace GlobalSim {
  namespace IOBitwise {
    using IeEmEg1eRatioTOBContainer = DataVector<GlobalSim::IOBitwise::IeEmEg1eRatioTOB>;
  }
}

CLASS_DEF( GlobalSim::IOBitwise::IeEmEg1eRatioTOBContainer , 1214466989 , 1 )

#endif //GLOBALSIM_IEEMNEG1ERATIOTOBCONTAINER_H
