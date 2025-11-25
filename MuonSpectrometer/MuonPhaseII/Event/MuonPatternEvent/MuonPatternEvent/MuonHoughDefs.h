/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONR4_MUONPATTERNEVENT_MUONHOUGHDEFS__H 
#define MUONR4_MUONPATTERNEVENT_MUONHOUGHDEFS__H

#include "GeoPrimitives/GeoPrimitives.h"
///

#include "MuonPatternEvent/HoughMaximum.h"

#include "Acts/Seeding/detail/CompSpacePointAuxiliaries.hpp"
#include "Acts/Seeding/CompositeSpacePointLineFitter.hpp"
#include "Acts/Seeding/HoughTransformUtils.hpp"
#include "Acts/Utilities/Helpers.hpp"

/// This header ties the generic definitions in this package 
//  to concrete types for representations of the hit, 
/// the accumulator, and the peak finder. 

namespace MuonR4{
  // representation of hits in the hough via space points
  using HoughHitType = const SpacePoint*;
 // ACTS representation of the hough accumulator
  using HoughPlane = Acts::HoughTransformUtils::HoughPlane<HoughHitType> ; 
  // configuration class for the accumulator
  using Acts::HoughTransformUtils::HoughPlaneConfig;
  // peak finder - use an existing ACTS one inspired by Run-2 ATLAS muon 
  using ActsPeakFinderForMuon = Acts::HoughTransformUtils::PeakFinders::IslandsAroundMax<HoughHitType>; 
  // config for the peak finder
  using ActsPeakFinderForMuonCfg = Acts::HoughTransformUtils::PeakFinders::IslandsAroundMaxConfig;

  namespace SegmentFit {
        /** @brief Abrivation of the CompSpacePointAuxiliaries */
        using SeedingAux = SpacePoint::SeedingAux;
        /** @brief Use the same parameter indices as used by the CompSpacePointAuxiliaries*/
        using ParamDefs = SeedingAux::FitParIndex;
        /** @brief Abrivation of the line with partial derivatives */
        using Line_t = SeedingAux::Line_t;
        /** @brief Use the same mapping of the covariance space indicies as used by 
         *         the SpacePoint */
        using AxisDefs = SpacePoint::CovIdx;
        

        using Parameters = Acts::Experimental::CompositeSpacePointLineFitter::ParamVec_t;
        using Covariance = Acts::Experimental::CompositeSpacePointLineFitter::CovMat_t;
  }

}


#endif
