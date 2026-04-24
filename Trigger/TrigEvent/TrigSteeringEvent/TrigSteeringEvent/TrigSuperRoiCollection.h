/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef  TRIGSTEERINGEVENT_TRIGSUPERROICOLLECTION_H
#define  TRIGSTEERINGEVENT_TRIGSUPERROICOLLECTION_H

#include "AthContainers/DataVector.h"
#include "AthenaKernel/BaseInfo.h"

#include "TrigSteeringEvent/TrigSuperRoi.h"


class TrigSuperRoiCollection : public DataVector<TrigSuperRoi> { };

CLASS_DEF( TrigSuperRoiCollection, 1078197961, 1 )

SG_BASE(TrigSuperRoiCollection, DataVector<TrigSuperRoi>);

#endif
