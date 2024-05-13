/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONR4_STATIONHOUGHMAXCONTAINER__H
#define MUONR4_STATIONHOUGHMAXCONTAINER__H

#include "StationHoughMaxima.h"
#include <set> 
#include "AthenaKernel/CLASS_DEF.h"
namespace MuonR4{
    using StationHoughMaxContainer = std::set<StationHoughMaxima> ; 
    using StationHoughSegmentSeedContainer = std::set<StationHoughSegmentSeeds> ; 
}
CLASS_DEF( MuonR4::StationHoughMaxContainer , 1158351533 , 1 )
CLASS_DEF( MuonR4::StationHoughSegmentSeedContainer , 1129863771 , 1 )

#endif
