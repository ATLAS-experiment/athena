/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPGATRACKSIMHITCONTAINER_H
#define FPGATRACKSIMHITCONTAINER_H

#include "AthContainers/DataVector.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"


typedef std::vector<std::vector<FPGATrackSimHit>> FPGATrackSimHitContainer;
CLASS_DEF( FPGATrackSimHitContainer , 1074585532 , 1 )



#endif // FPGATRACKSIMHITCONTAINER_DEF_H