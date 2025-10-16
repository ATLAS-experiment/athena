/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef GLOBALSIM_IEEMNEG1BDTTOBCONTAINER_H
#define GLOBALSIM_IEEMNEG1BDTTOBCONTAINER_H

#include "IeEmEg1BDTTOB.h"

#include "AthContainers/DataVector.h"

namespace GlobalSim {
  namespace IOBitwise {
    using IeEmEg1BDTTOBContainer = DataVector<GlobalSim::IOBitwise::IeEmEg1BDTTOB>;
  }
}

CLASS_DEF( GlobalSim::IOBitwise::IeEmEg1BDTTOBContainer , 1213716786 , 1 )

#endif //GLOBALSIM_IEEMNEG1BDTTOBCONTAINER_H
