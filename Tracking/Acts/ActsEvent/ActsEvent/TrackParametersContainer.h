/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKEVENT_TRACKPARAMETERSCONTAINER_H
#define ACTSTRKEVENT_TRACKPARAMETERSCONTAINER_H 1

#include "Acts/EventData/TrackParameters.hpp"
#include "AthContainers/DataVector.h"

namespace ActsTrk {
  typedef DataVector< Acts::BoundTrackParameters > BoundTrackParametersContainer;
}

// Set up a CLID for the type:
#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( ActsTrk::BoundTrackParametersContainer, 1261498502, 1)

#endif
