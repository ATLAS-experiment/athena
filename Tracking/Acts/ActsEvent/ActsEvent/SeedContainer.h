/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKEVENT_SEEDCONTAINER_H
#define ACTSTRKEVENT_SEEDCONTAINER_H 1

#include "ActsEvent/Seed.h"
#include "AthContainers/DataVector.h"

namespace ActsTrk {
typedef DataVector<ActsTrk::Seed> SeedContainer;
}

// Set up a CLID for the type:
#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF(ActsTrk::SeedContainer, 1261318102, 1)

#endif
