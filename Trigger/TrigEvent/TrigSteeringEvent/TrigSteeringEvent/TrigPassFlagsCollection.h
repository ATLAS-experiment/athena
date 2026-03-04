/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGSTEERINGEVENT_TRIGPASSFLAGSCOLLECTION_H
#define TRIGSTEERINGEVENT_TRIGPASSFLAGSCOLLECTION_H

#include "AthContainers/DataVector.h"
#include "xAODCore/BaseInfo.h"

#include "TrigSteeringEvent/TrigPassFlags.h"


class TrigPassFlagsCollection : public DataVector<TrigPassFlags> {
};

CLASS_DEF( TrigPassFlagsCollection , 1210268481 , 1 )

SG_BASE(TrigPassFlagsCollection, DataVector<TrigPassFlags>);

#endif
