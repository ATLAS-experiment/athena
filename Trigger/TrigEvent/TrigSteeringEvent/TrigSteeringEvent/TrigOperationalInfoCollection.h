/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGSTEERINGEVENT_TRIGOPERATIONALINFOCollection_H
#define TRIGSTEERINGEVENT_TRIGOPERATIONALINFOCollection_H

#include "AthContainers/DataVector.h"
#include "AthenaKernel/BaseInfo.h"

#include "TrigSteeringEvent/TrigOperationalInfo.h"

class TrigOperationalInfoCollection : public DataVector<TrigOperationalInfo> {

};
CLASS_DEF( TrigOperationalInfoCollection , 1320355091 , 1 )

SG_BASE(TrigOperationalInfoCollection, DataVector<TrigOperationalInfo>);

#endif
