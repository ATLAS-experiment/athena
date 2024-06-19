/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONR4__MUONHOUGHDEFS__H 
#define MUONR4__MUONHOUGHDEFS__H

#include "Acts/Seeding/HoughTransformUtils.hpp"
#include "MuonHoughEventData.h"
#include "HoughMaximum.h"
#include "HoughSegmentSeed.h"
#include "MuonSegmentFitterEventData.h"

/// This header ties the generic definitions in this package 
//  to concrete types for representations of the hit, 
/// the accumulator, and the peak finder. 

namespace MuonR4{
  // representation of hits in the hough via space points
  using HoughHitType = std::shared_ptr<MuonR4::MuonSpacePoint>;
  // representation of the hough maxiumum 
  using HoughMaximum = HoughMaximum_impl<HoughHitType>;
  // and of the segment seed
  using HoughSegmentSeed = HoughSegmentSeed_impl<HoughHitType>;
  using MuonSegmentFitterEventData = MuonSegmentFitterEventData_impl<HoughHitType>;
  // ACTS representation of the hough accumulator
  using HoughPlane = Acts::HoughTransformUtils::HoughPlane<HoughHitType> ; 
  // configuration class for the accumulator
  using Acts::HoughTransformUtils::HoughPlaneConfig;
  // peak finder - use an existing ACTS one inspired by Run-2 ATLAS muon 
  using ActsPeakFinderForMuon = Acts::HoughTransformUtils::PeakFinders::IslandsAroundMax<HoughHitType>; 
  // config for the peak finder
  using ActsPeakFinderForMuonCfg = Acts::HoughTransformUtils::PeakFinders::IslandsAroundMaxConfig;

  // for now, we use one common (default ACTS) peak finder for both the eta- and the phi-transforms. 
  using MuonHoughEventData = MuonHoughEventData_impl<HoughMaximum, HoughPlane, ActsPeakFinderForMuon, ActsPeakFinderForMuonCfg>; 


}


#endif
