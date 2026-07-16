/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsGeometry/ActsDetectorElement.h"
#include "xAODMeasurementBase/MeasurementDefs.h"
#include "xAODTracking/TrackingPrimitives.h"
#include "src/detail/HitSummaryDataUtils.h"
#include "ActsCalibrators/xAODUncalibMeasCalibrator.h"
#include "ActsEvent/EnumConversion.h"

namespace ActsTrk::detail {

  void gatherTrackSummaryData(const typename ActsTrk::TrackContainer::ConstTrackProxy &track,
                              const std::array<unsigned short, Acts::toUnderlying(xAOD::UncalibMeasType::nTypes)>
                                       &measurement_to_summary_type,
                              SumOfValues &chi2_stat_out,
                              HitSummaryData &hit_info_out,
                              std::vector<ActsTrk::TrackStateBackend::ConstTrackStateProxy::IndexType > &param_state_idx_out,
                              std::array<std::array<uint8_t, Acts::toUnderlying(HitCategory::N)>,
			      Acts::toUnderlying(xAOD::UncalibMeasType::nTypes)> &special_hit_counts_out,
                              TimeInfo &time_info)
  {
     chi2_stat_out.reset();

     struct TimeInfoHelper { double sum{}; double sumInv2{}; double chi2{}; unsigned int n=0u;};
     TimeInfoHelper time_info_helper;

     hit_info_out.reset();
     param_state_idx_out.clear();

     const auto lastMeasurementIndex = track.tipIndex();
     track.container().trackStateContainer().visitBackwards(
          lastMeasurementIndex,
          [&measurement_to_summary_type,
           &chi2_stat_out,
           &hit_info_out,
           &param_state_idx_out,
           &special_hit_counts_out,
           &time_info_helper
           ](const typename ActsTrk::TrackStateBackend::ConstTrackStateProxy &state) -> void
          {

            auto flag = state.typeFlags();
            if (!state.hasReferenceSurface()) {
               return;
            }
            xAOD::UncalibMeasType det_type = xAOD::UncalibMeasType::Other;
            const auto* placement = dynamic_cast<const ISurfacePlacement*>(state.referenceSurface().surfacePlacement());
            if (placement != nullptr) {
               det_type = toMeasType(placement->detectorType());
            }

            if (flag.hasNoExpectedHit()) {
               // @TODO includes holes at sensor edges
               ++special_hit_counts_out.at(Acts::toUnderlying(det_type)).at(Acts::toUnderlying(HitCategory::DeadSensor));
               return;

            }
            if (flag.isHole()) {
               const Amg::Vector2D localPos{state.parameters()[Acts::eBoundLoc0],
                                            state.parameters()[Acts::eBoundLoc1]};
               if (state.referenceSurface().insideBounds(localPos)) {
                  // @TODO check whether detector element is dead..
                  ++special_hit_counts_out.at(Acts::toUnderlying(det_type)).at(Acts::toUnderlying(HitCategory::Hole));
               }
               return;

            }
            // do not consider material states       
            if (!flag.hasMeasurement() || !state.hasUncalibratedSourceLink()) {
               return;
            }
            param_state_idx_out.push_back(state.index());
       
            // @TODO dead elements
            auto uncalibMeas = detail::xAODUncalibMeasCalibrator::unpack(state.getUncalibratedSourceLink());
            assert( uncalibMeas != nullptr );
            assert( det_type == uncalibMeas->type());

            if (measurement_to_summary_type.at(Acts::toUnderlying(uncalibMeas->type())) <  
                xAOD::numberOfTrackSummaryTypes ) {
               HitSummaryData::EHitSelection hit_selection = (flag.isOutlier()
                                                           ? HitSummaryData::OutlierFlag
                                                           : HitSummaryData::HitFlag);
               if (flag.isSharedHit()) {
                  hit_selection = HitSummaryData::EHitSelection(hit_selection | HitSummaryData::SharedHitFlag);
               }
               if (flag.isSplitHit()) {
                  hit_selection = HitSummaryData::EHitSelection(hit_selection | HitSummaryData::SplitHitFlag);
               }
               if (const auto* idDetEl = getActsDetectorElement(state.referenceSurface()); idDetEl != nullptr) {
                  const auto* siDet = dynamic_cast<const InDetDD::SolidStateDetectorElementBase*>(idDetEl->upstreamDetectorElement());
                  hit_info_out.addHit(det_type, siDet, hit_selection);
               }

               if (det_type == xAOD::UncalibMeasType::HGTDClusterType) {
                  if (!flag.isOutlier() && state.hasCalibrated()) {
                     // compute HGTD calibrated
                     assert( state.calibratedSize()==3);
                     auto pos = state.calibrated<3>();
                     auto cov = state.calibratedCovariance<3>();
                     time_info_helper.sum += pos[2]; // Acts time
                     time_info_helper.sumInv2 += 1./cov(2,2);  // Acts time
                     time_info_helper.chi2 += state.chi2();
                     ++time_info_helper.n;
                  }
               }

               if (state.calibratedSize()>0 && !flag.isOutlier()) {
                   // from Tracking/TrkTools/TrkTrackSummaryTool/src/TrackSummaryTool.cxx
                   //       processTrackState
                   double chi2add = std::min(state.chi2(),1e5f) / state.calibratedSize();
                   chi2_stat_out.add(chi2add );
                }
             }

          });
     hit_info_out.computeSummaries();
     if (time_info_helper.n>1) {
        time_info.mean = static_cast<float>(ActsTrk::timeToAthena( time_info_helper.sum / time_info_helper.n));
        time_info.resolution = static_cast<float>(ActsTrk::timeToAthena(std::sqrt(1./time_info_helper.sumInv2)));
        time_info.chi2 = time_info_helper.chi2;
     }
     else {
        time_info.mean = static_cast<float>(ActsTrk::timeToAthena(time_info_helper.sum));
        time_info.resolution = time_info_helper.n>0 ? static_cast<float>(ActsTrk::timeToAthena(std::sqrt(1./time_info_helper.sumInv2))) : 0.f;
        time_info.chi2 = time_info_helper.chi2;
     }
  }

}
