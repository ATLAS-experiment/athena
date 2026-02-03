/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsGeometry/ActsDetectorElement.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"

#include "ExpectedHitUtils.h"

namespace ActsTrk::detail {

  std::array<unsigned int,4> expectedLayerPattern(const EventContext& ctx,
                                                  const IExtrapolationTool &extrapolator,
                                                  const Acts::BoundTrackParameters& perigee_parameters,
                                                  double pathLimit) {
    std::array<unsigned int,4> expected_layer_pattern {0u,0u,0u,0u};
    auto result = extrapolator.propagationSteps(ctx,
                                                perigee_parameters,
                                                Acts::Direction::Forward(),
                                                pathLimit);
    if (result.ok()) {
      for (const Acts::detail::Step &step : result->first ) {
        // @TODO boundary check ?, does layer number match layer numbering in athena ?
        // @TODO filter out dead modules
        if (step.geoID.sensitive()) {
          if (step.surface && step.surface->surfacePlacement()) {
            const ActsDetectorElement *
              actsDetEl = dynamic_cast<const ActsDetectorElement *>(step.surface->surfacePlacement());
            if (actsDetEl) {
              addToExpectedLayerPattern(expected_layer_pattern, *actsDetEl);
            }
          }
        }
      }
    }

    return expected_layer_pattern;
  }

  void addToExpectedLayerPattern(std::array<unsigned int,4>& pattern, const ActsDetectorElement& detElement) {
    const InDetDD::SiDetectorElement *
      detEl = dynamic_cast<const InDetDD::SiDetectorElement *>(detElement.upstreamDetectorElement());
    if (detEl) {
      if (detEl->isPixel()) {
        InDetDD::DetectorType type = detEl->design().type();
        unsigned int pattern_idx = type==InDetDD::PixelBarrel ? 0 : 1;
        const PixelID* pixel_id = static_cast<const PixelID *>(detEl->getIdHelper());
        pattern[pattern_idx] |= (1<< pixel_id->layer_disk(detEl->identify()) );
      }
      else if (detEl->isSCT()) {
        // @TODO SCT/strip layer number should be shifted by total number of pixel layers ...
        const SCT_ID* sct_id = static_cast<const SCT_ID *>(detEl->getIdHelper());
        unsigned int pattern_idx = detEl->isBarrel() ? 2 : 3;
        pattern[pattern_idx] |= (1<< sct_id->layer_disk(detEl->identify()) );
      }
    }
  }
}
