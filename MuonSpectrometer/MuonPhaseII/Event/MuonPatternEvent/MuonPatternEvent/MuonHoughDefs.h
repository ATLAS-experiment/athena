/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONR4_MUONPATTERNEVENT_MUONHOUGHDEFS__H 
#define MUONR4_MUONPATTERNEVENT_MUONHOUGHDEFS__H

#include "Acts/Seeding/HoughTransformUtils.hpp"
#include "MuonPatternEvent/HoughMaximum.h"
#include "MuonPatternEvent/SegmentSeed.h"
#include "MuonPatternEvent/MuonSegmentFitterEventData.h"

/// This header ties the generic definitions in this package 
//  to concrete types for representations of the hit, 
/// the accumulator, and the peak finder. 

namespace MuonR4{
  // representation of hits in the hough via space points
  using HoughHitType = HoughMaximum::HitType;
 
  using MuonSegmentFitterEventData = MuonSegmentFitterEventData_impl<HoughHitType>;
  // ACTS representation of the hough accumulator
  using HoughPlane = Acts::HoughTransformUtils::HoughPlane<HoughHitType> ; 
  // configuration class for the accumulator
  using Acts::HoughTransformUtils::HoughPlaneConfig;
  // peak finder - use an existing ACTS one inspired by Run-2 ATLAS muon 
  using ActsPeakFinderForMuon = Acts::HoughTransformUtils::PeakFinders::IslandsAroundMax<HoughHitType>; 
  // config for the peak finder
  using ActsPeakFinderForMuonCfg = Acts::HoughTransformUtils::PeakFinders::IslandsAroundMaxConfig;

}


#endif
