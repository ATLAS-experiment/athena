/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
  */
/* Dear emacs, this is -*-c++-*- */
#ifndef ACTSTRK_SURFACEOFMEASUREMENTHELPER_H
#define ACTSTRK_SURFACEOFMEASUREMENTHELPER_H

#include "Acts/Geometry/TrackingGeometry.hpp"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "DetectorElementToActsGeometryIdMap.h"

namespace ActsTrk {
  inline Acts::GeometryIdentifier getSurfaceGeometryIdOfMeasurement(
                   const DetectorElementToActsGeometryIdMap &detector_element_to_geoid,
                   const xAOD::UncalibratedMeasurement &measurement)
  {
     DetectorElementToActsGeometryIdMap::const_iterator
        geoid_iter = detector_element_to_geoid.find( makeDetectorElementKey(measurement.type(), measurement.identifierHash()) );
     return  geoid_iter != detector_element_to_geoid.end()
        ? DetectorElementToActsGeometryIdMap::getValue( *geoid_iter)
        : Acts::GeometryIdentifier{};
  }
}

#endif
