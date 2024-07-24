/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONR4_MUONPATTERNRECOGNITIONEVENT_MUONHOUGHEVENTDATA__H 
#define MUONR4_MUONPATTERNRECOGNITIONEVENT_MUONHOUGHEVENTDATA__H

#include "Acts/Seeding/HoughTransformUtils.hpp"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "MuonPatternEvent/MuonHoughDefs.h"
#include <map> 
#include <memory> 

namespace MuonR4{ 

  /// @brief Templated event data class for the phase-2 muon hough transform. 
  /// @tparam peakFinder_t: Peak finder class to use 
  /// @tparam peakFinderConfig_t: Configuration object type for the peak finder 
  template <typename peakFinder_t, typename peakFinderConfig_t> 
      struct HoughEventData_impl{
            HoughEventData_impl(const ActsGeometryContext& _gctx):
                gctx{_gctx} {}
            
            /// @brief Updates a search space window to account for a value. 
            /// The window will adaptively grow so that it includes all of the user-supplied values
            /// @param searchWindow: The window to update
            /// @param value: The value to account for 
            inline static void updateSearchWindow(std::pair<double,double> & searchWindow, double value){
              searchWindow.first = std::min(searchWindow.first, value); 
              searchWindow.second = std::max(searchWindow.second, value); 
            }

            // cached ACTS geometry context 
            const ActsGeometryContext& gctx; 
            // Hough accumulator 
            std::unique_ptr<HoughPlane> houghPlane{nullptr}; 
            // peak finder instance
            std::unique_ptr<peakFinder_t> peakFinder{nullptr}; 
            /// @brief Hough transform configuration for one bucket 
            /// of the search space. 
            struct HoughSetupForBucket{
                // Bucket containing the input space points 
                const SpacePointBucket* bucket{nullptr};
                // placeholder boundary for the search space  
                static constexpr double dummyBoundary{1.e9};
                // search window for the intercept, optimised for the bucket
                std::pair<double, double> searchWindowIntercept{dummyBoundary, -dummyBoundary};
                // search window for the angle, optimised for the bucket
                std::pair<double, double> searchWindowTanAngle{dummyBoundary, -dummyBoundary};

                /// Update the hough space search window
                void updateHoughWindow(double interceptMeas, double TanAngleMeas) {
                    updateSearchWindow(searchWindowIntercept, interceptMeas); 
                    updateSearchWindow(searchWindowTanAngle, TanAngleMeas); 
                }
            };
            // the maxima found by the hough transform
            std::vector<HoughMaximum> maxima{};
            // the hough setup for each logical muon chamber
            std::map<const MuonGMR4::MuonChamber*, std::vector<HoughSetupForBucket>> houghSetups{}; 
            // the axis ranges currently mapped to the accumulator
            Acts::HoughTransformUtils::HoughAxisRanges currAxisRanges; 
            // current search window for the intercept
            std::pair<double,double> searchSpaceIntercept{10000000,-100000000.}; 
            // current search window for the angle
            std::pair<double,double> searchSpaceTanAngle{100000000.,-100000.}; 
            // hit counter for phi-measurements
            size_t phiHitsOnMax{0};
    };

    // for now, we use one common (default ACTS) peak finder for both the eta- and the phi-transforms. 
    using HoughEventData = HoughEventData_impl<ActsPeakFinderForMuon, ActsPeakFinderForMuonCfg>; 

}

#endif
