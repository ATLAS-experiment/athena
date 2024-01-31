/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsGeometry/ActsDetectorElement.h"
#include "ActsGeometryInterfaces/IActsExtrapolationTool.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"

#include "ExpectedHitUtils.h"

namespace ActsTrk {
   std::array<unsigned int,4> expectedLayerPattern(const EventContext& ctx,
                                                   const IActsExtrapolationTool &extrapolator,
                                                   Acts::BoundTrackParameters perigee_parameters,
                                                   const Acts::CylinderSurface &extrapolation_volume) {

      ActsPropagationOutput result = extrapolator.propagationSteps(ctx,
                                                                   perigee_parameters,
                                                                   extrapolation_volume,
                                                                   Acts::Direction::Forward);
      std::array<unsigned int,4> expected_layer_pattern {0u,0u,0u,0u};
      for (const Acts::detail::Step &step : result.first ) {
         // @TODO boundary check ?, does layer number match layer numbering in athena ?
         // @TODO filter out dead modules
         if (step.geoID.sensitive()) {
            if (step.surface && step.surface->associatedDetectorElement()) {
               const ActsDetectorElement *
                  actsDetEl = dynamic_cast<const ActsDetectorElement *>(step.surface->associatedDetectorElement());
               if (actsDetEl) {
                  const InDetDD::SiDetectorElement *
                     detEl = dynamic_cast<const InDetDD::SiDetectorElement *>(actsDetEl->upstreamDetectorElement());
                  if (detEl) {
                     if (detEl->isPixel()) {
                        InDetDD::DetectorType type = detEl->design().type();
                        unsigned int pattern_idx = type==InDetDD::PixelBarrel ? 0 : 1;
                        const PixelID* pixel_id = static_cast<const PixelID *>(detEl->getIdHelper());
                        expected_layer_pattern[pattern_idx] |= (1<< pixel_id->layer_disk(detEl->identify()) );
                     }
                     else if (detEl->isSCT()) {
                        // @TODO SCT/strip layer number should be shifted by total number of pixel layers ...
                        const SCT_ID* sct_id = static_cast<const SCT_ID *>(detEl->getIdHelper());
                        unsigned int pattern_idx = detEl->isBarrel() ? 2 : 3;
                        expected_layer_pattern[pattern_idx] |= (1<< sct_id->layer_disk(detEl->identify()) );
                     }
                  }
               }
            }
         }
      }
      return expected_layer_pattern;
   }
}
