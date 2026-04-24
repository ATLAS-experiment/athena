/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGSTEERINGEVENT_TRIGPASSBITSCOLLECTION_H
#define TRIGSTEERINGEVENT_TRIGPASSBITSCOLLECTION_H

#include "AthContainers/DataVector.h"
#include "xAODCore/BaseInfo.h"

#include "TrigSteeringEvent/TrigPassBits.h"


class TrigPassBitsCollection : public DataVector<TrigPassBits> {
};

CLASS_DEF( TrigPassBitsCollection , 1311696963 , 1 )

SG_BASE(TrigPassBitsCollection, DataVector<TrigPassBits>);

#endif
