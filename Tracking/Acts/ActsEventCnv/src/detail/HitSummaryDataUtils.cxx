/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsGeometry/ActsDetectorElement.h"
#include "Acts/Surfaces/BoundaryTolerance.hpp"
#include "xAODMeasurementBase/MeasurementDefs.h"
#include "ActsGeometry/ATLASSourceLink.h"

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
			      Acts::toUnderlying(xAOD::UncalibMeasType::nTypes)> &special_hit_counts_out)
  {
     chi2_stat_out.reset();

     hit_info_out.reset();
     param_state_idx_out.clear();

     const auto lastMeasurementIndex = track.tipIndex();
     track.container().trackStateContainer().visitBackwards(
          lastMeasurementIndex,
          [&measurement_to_summary_type,
           &chi2_stat_out,
           &hit_info_out,
           &param_state_idx_out,
           &special_hit_counts_out
           ](const typename ActsTrk::TrackStateBackend::ConstTrackStateProxy &state) -> void
          {

            auto flag = state.typeFlags();
            if (!state.hasReferenceSurface()) {
               return;
            }
            xAOD::UncalibMeasType det_type = xAOD::UncalibMeasType::Other;
            const auto* detEl = dynamic_cast<const IDetectorElementBase*>(state.referenceSurface().surfacePlacement());
            if (detEl != nullptr) {
               det_type = toMeasType(detEl->detectorType());
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
            assert( det_type == toMeasType(uncalibMeas->type()));

            if (measurement_to_summary_type.at(Acts::toUnderlying(uncalibMeas->type())) <  
                xAOD::numberOfTrackSummaryTypes ) {
               HitSummaryData::EHitSelection hit_selection = (flag.isOutlier()
                                                           ? HitSummaryData::Outlier
                                                           : HitSummaryData::Hit);
               if (flag.isSharedHit()) {
                  hit_selection = HitSummaryData::EHitSelection(hit_selection | HitSummaryData::SharedHit);
               }
               const InDetDD::SiDetectorElement* siDet{nullptr};
               if (const auto* idDetEl = dynamic_cast<const ActsDetectorElement*>(detEl); idDetEl != nullptr) {
                  siDet = dynamic_cast<const InDetDD::SiDetectorElement*>(idDetEl->upstreamDetectorElement());
               }
               hit_info_out.addHit(siDet, hit_selection);
                   
                
               if (state.calibratedSize()>0 && !flag.isOutlier()) {
                   // from Tracking/TrkTools/TrkTrackSummaryTool/src/TrackSummaryTool.cxx
                   //       processTrackState
                   double chi2add = std::min(state.chi2(),1e5f) / state.calibratedSize();
                   chi2_stat_out.add(chi2add );
                }
             }

          });
     hit_info_out.computeSummaries();
  }

}
