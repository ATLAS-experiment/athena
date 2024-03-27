/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONR4__MUONHOUGHEVENTDATA__H 
#define MUONR4__MUONHOUGHEVENTDATA__H

#include "Acts/Seeding/HoughTransformUtils.hpp"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "MuonPatternEvent/HoughMaximum.h"
#include "xAODMuonPrepData/MdtDriftCircleContainer.h"
#include "xAODMuonPrepData/RpcStripContainer.h"
#include "xAODMuonPrepData/TgcStripContainer.h"
#include "xAODMuonPrepData/MMClusterContainer.h"
#include "MuonSpacePoint/MuonSpacePointContainer.h"


#include <map> 
#include <memory> 

// specialise our hough plane to utilise uncalibrated measurements as "identifier" type
namespace MuonR4{
  using HoughPlane = Acts::HoughTransformUtils::HoughPlane<HoughHitType> ; 
  using Acts::HoughTransformUtils::HoughPlaneConfig; 
  using ActsPeakFinderForMuon = Acts::HoughTransformUtils::PeakFinders::IslandsAroundMax<HoughHitType>; 
  using ActsPeakFinderForMuonCfg = Acts::HoughTransformUtils::PeakFinders::IslandsAroundMaxConfig; 
  class HoughMaximum; 
  class StationIdentifier; 
    template <typename peakFinder_t, typename peakFinderConfig_t> 
    struct MuonHoughEventData_impl{
            MuonHoughEventData_impl(const ActsGeometryContext& _gctx):
                gctx{_gctx} {}
            const ActsGeometryContext& gctx; 
            std::unique_ptr<HoughPlane> houghPlane{nullptr}; 
            std::unique_ptr<peakFinder_t> peakFinder{nullptr}; 
            
            struct HoughSpaceInBucket{
                const MuonSpacePointBucket* bucket{nullptr}; 
                static constexpr double dummyBoundary{1.e9};
                std::pair<double, double> searchWindowZ{dummyBoundary, -dummyBoundary};
                std::pair<double, double> seachWindowTanTheta{dummyBoundary, -dummyBoundary};

                /// Update the hough space search window
                void updateHoughWindow(double zMeas, double tanThetaMeas) {
                    searchWindowZ.first = std::min(searchWindowZ.first, zMeas);
                    searchWindowZ.second = std::max(searchWindowZ.second, zMeas);

                    seachWindowTanTheta.first = std::min(seachWindowTanTheta.first, tanThetaMeas);
                    seachWindowTanTheta.second = std::max(seachWindowTanTheta.second, tanThetaMeas);
                }
            };
            std::vector<HoughMaximum> maxima{};

            std::map<const MuonGMR4::MuonChamber*, std::vector<HoughSpaceInBucket>> houghSpaces{}; 
            Acts::HoughTransformUtils::HoughAxisRanges currAxisRanges; 
            std::pair<double,double> searchSpaceZ{10000000,-100000000.}; 
            std::pair<double,double> searchSpaceTanTheta{100000000.,-100000.}; 
    };

    // for now, we use the same (default ACTS) peak finder for both the eta- and the phi-transforms 
    using MuonHoughEventData = MuonHoughEventData_impl<ActsPeakFinderForMuon, ActsPeakFinderForMuonCfg>; 
}

#endif
