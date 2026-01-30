/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPGATRACKSIMHITCOLLECTION_H
#define FPGATRACKSIMHITCOLLECTION_H

#include "AthContainers/DataVector.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"


typedef DataVector<FPGATrackSimHit> FPGATrackSimHitCollection;
CLASS_DEF( FPGATrackSimHitCollection , 1205121526 , 2 )



#endif // FPGATRACKSIMHITCOLLECTION_DEF_H
