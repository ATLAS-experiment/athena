/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsGeometry/ActsDetectorElement.h"
#include "Acts/Surfaces/BoundaryTolerance.hpp"
#include "xAODMeasurementBase/MeasurementDefs.h"
#include "ActsGeometry/ATLASSourceLink.h"

#include "src/detail/HitSummaryDataUtils.h"

namespace ActsTrk::detail {

  void gatherTrackSummaryData(const ActsTrk::TrackContainer &tracksContainer,
                              const typename ActsTrk::TrackContainer::ConstTrackProxy &track,
                              const std::array<const InDetDD::SiDetectorElementCollection *,
			      to_underlying(xAOD::UncalibMeasType::nTypes)> &siDetEleColl,
                              const std::array<unsigned short, to_underlying(xAOD::UncalibMeasType::nTypes)>
                                       &measurement_to_summary_type,
                              SumOfValues &chi2_stat_out,
                              HitSummaryData &hit_info_out,
                              std::vector<ActsTrk::TrackStateBackend::ConstTrackStateProxy::IndexType > &param_state_idx_out,
                              std::array<std::array<uint8_t, to_underlying(HitCategory::N)>,
			      to_underlying(xAOD::UncalibMeasType::nTypes)> &special_hit_counts_out)
  {
     chi2_stat_out.reset();

     hit_info_out.reset();
     param_state_idx_out.clear();

     const auto lastMeasurementIndex = track.tipIndex();
     tracksContainer.trackStateContainer().visitBackwards(
          lastMeasurementIndex,
          [&siDetEleColl,
           &measurement_to_summary_type,
           &chi2_stat_out,
           &hit_info_out,
           &param_state_idx_out,
           &special_hit_counts_out
           ](const typename ActsTrk::TrackStateBackend::ConstTrackStateProxy &state) -> void
          {

             auto flag = state.typeFlags();
             if (flag.test(Acts::TrackStateFlag::HoleFlag)) {
                xAOD::UncalibMeasType det_type = xAOD::UncalibMeasType::Other;
                if (state.hasReferenceSurface()) {
                   if (state.referenceSurface().associatedDetectorElement()) {
                      const ActsDetectorElement *
                         actsDetEl = dynamic_cast<const ActsDetectorElement *>(state.referenceSurface().associatedDetectorElement());
                      if (actsDetEl) {
                         const InDetDD::SiDetectorElement *
                            detEl = dynamic_cast<const InDetDD::SiDetectorElement *>(actsDetEl->upstreamDetectorElement());
                         if (detEl) {
                            if (detEl->isPixel()) {
                               det_type = xAOD::UncalibMeasType::PixelClusterType;
                            }
                            else if (detEl->isSCT()) {
                               det_type = xAOD::UncalibMeasType::StripClusterType;
                         }
                         }
                      }
                   }
                }

                Acts::BoundaryTolerance tolerance = Acts::BoundaryTolerance::AbsoluteBound(
                  // local0 tolerance
                  (det_type == xAOD::UncalibMeasType::PixelClusterType
                   || state.referenceSurface().bounds().type() != Acts::SurfaceBounds::eAnnulus)
                  ? 0.0 : std::numeric_limits<double>::infinity(),
                  // local1 tolerance
                  (det_type == xAOD::UncalibMeasType::PixelClusterType
                   || state.referenceSurface().bounds().type() == Acts::SurfaceBounds::eAnnulus)
                  ? 0.0 : std::numeric_limits<double>::infinity()
                );
                Acts::Vector2 localPos(state.predicted()[Acts::eBoundLoc0],state.predicted()[Acts::eBoundLoc1]);
                if (state.referenceSurface().insideBounds(localPos,tolerance)) {
                   // @TODO check whether detector element is dead..
                   // if (dead) {
                   // ++specialHitCounts.at(to_underlying(det_type)).at(HitCategory::DeadSensor);
                   // } else {
                   ++special_hit_counts_out.at(to_underlying(det_type)).at(HitCategory::Hole);
                }

             }
             else if (flag.test(Acts::TrackStateFlag::MeasurementFlag)) {
                // do not consider material states
                param_state_idx_out.push_back(state.index());
             }
             // @TODO dead elements

             if (state.hasUncalibratedSourceLink()) {
                auto sl = state.getUncalibratedSourceLink().template get<ActsTrk::ATLASUncalibSourceLink>();
                assert( sl != nullptr );
                const xAOD::UncalibratedMeasurement &uncalibMeas = getUncalibratedMeasurement(sl);
                if (measurement_to_summary_type.at(to_underlying(uncalibMeas.type())) <  xAOD::numberOfTrackSummaryTypes ) {
                   if (static_cast<unsigned int>(to_underlying(uncalibMeas.type())) < siDetEleColl.size()) {
                     HitSummaryData::EHitSelection hit_selection = (flag.test(Acts::TrackStateFlag::OutlierFlag)
                                                                        ? HitSummaryData::Outlier
                                                                        : HitSummaryData::Hit);
                     if (flag.test(Acts::TrackStateFlag::SharedHitFlag)) {
                        hit_selection = HitSummaryData::EHitSelection(hit_selection | HitSummaryData::SharedHit);
                     }
                     hit_info_out.addHit(siDetEleColl[to_underlying(uncalibMeas.type())],
                                         uncalibMeas.identifierHash(),
                                         hit_selection);
                   }
                }
                if (state.calibratedSize()>0 && !flag.test(Acts::TrackStateFlag::OutlierFlag)) {
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
