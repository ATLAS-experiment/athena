/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONR4_STATIONHOUGHMAXCONTAINER__H
#define MUONR4_STATIONHOUGHMAXCONTAINER__H

#include "MuonPatternEvent/SegmentSeed.h"
#include "MuonPatternEvent/HoughMaximum.h"
#include "MuonPatternEvent/Segment.h"
#include "AthContainers/DataVector.h"
#include <set> 
#include "AthenaKernel/CLASS_DEF.h"
namespace MuonR4{
    using EtaHoughMaxContainer = DataVector<HoughMaximum>; 
    using SegmentSeedContainer = DataVector<SegmentSeed>; 
    using SegmentContainer = DataVector<Segment>; 

}
CLASS_DEF( MuonR4::SegmentContainer , 1289070339 , 1 )
CLASS_DEF( MuonR4::EtaHoughMaxContainer , 1167732565 , 1 );
CLASS_DEF( MuonR4::SegmentSeedContainer , 1116604630 , 1 );

#endif
