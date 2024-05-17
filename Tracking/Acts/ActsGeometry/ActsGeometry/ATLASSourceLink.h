/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRY_ATLASOURCELINK_H
#define ACTSGEOMETRY_ATLASOURCELINK_H

#include "AthContainers/DataVector.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"

#include "Acts/EventData/Measurement.hpp"
#include "Acts/EventData/MultiTrajectory.hpp"
#include "Acts/Utilities/CalibrationContext.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/Surfaces/Surface.hpp"
#include "Acts/Definitions/TrackParametrization.hpp"
#include "Acts/Geometry/TrackingGeometry.hpp"

#include <cassert>

namespace ActsTrk {
   using ATLASSourceLink = const Trk::MeasurementBase *;
   using ATLASUncalibSourceLink = const xAOD::UncalibratedMeasurement *;

   inline const xAOD::UncalibratedMeasurement &getUncalibratedMeasurement(const ATLASUncalibSourceLink &source_link) {
      assert(source_link);
      return *source_link;
   }
   inline ATLASUncalibSourceLink makeATLASUncalibSourceLink(const xAOD::UncalibratedMeasurementContainer *container,
                                                            std::size_t index,
                                                            [[maybe_unused]] const EventContext& ctx) {
      assert(container && index < container->size());
      return container->at(index);
   }
   inline ATLASUncalibSourceLink makeATLASUncalibSourceLink([[maybe_unused]] const xAOD::UncalibratedMeasurementContainer *container,
                                                            const xAOD::UncalibratedMeasurement *measurement,
                                                            [[maybe_unused]] const EventContext& ctx) {
      assert( container == measurement->container());
      assert( container && measurement->index() < container->size() );
      return measurement;
   }
   inline ATLASUncalibSourceLink makeATLASUncalibSourceLink(const xAOD::UncalibratedMeasurementContainer *container,
                                                            std::size_t index) {
      assert(container && index < container->size());
      return container->at(index);
   }
   // *dynamic_cast<const xAOD::UncalibratedMeasurementContainer*>(umeas->container()), umeas->index()
   inline ATLASUncalibSourceLink makeATLASUncalibSourceLink(const xAOD::UncalibratedMeasurement *measurement) {
      assert(measurement && measurement->container() && measurement->index() < measurement->container()->size_v());
      return measurement;
   }
   inline float localXFromSourceLink(const ATLASUncalibSourceLink &source_link) {
      const xAOD::UncalibratedMeasurement &uncalib_meas = getUncalibratedMeasurement(source_link);
      return uncalib_meas.type() == xAOD::UncalibMeasType::PixelClusterType
         ? uncalib_meas.localPosition<2>()[Trk::locX]
         : uncalib_meas.localPosition<1>()[Trk::locX];
   }

   inline float localYFromSourceLink(const ATLASUncalibSourceLink &source_link) {
      const xAOD::UncalibratedMeasurement &uncalib_meas = getUncalibratedMeasurement(source_link);
      assert(uncalib_meas.type() == xAOD::UncalibMeasType::PixelClusterType );
      return uncalib_meas.localPosition<2>()[Trk::locY];
   }

}

#endif
