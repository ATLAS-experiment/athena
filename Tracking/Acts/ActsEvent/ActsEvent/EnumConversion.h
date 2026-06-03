/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ActsEvent_EnumConversion_h
#define ActsEvent_EnumConversion_h

#include "ActsGeometryInterfaces/GeometryDefs.h"
#include "xAODMeasurementBase/MeasurementDefs.h"

namespace ActsTrk{
    /** @brief Converts the uncalibrated measurement type to a detector type */
    DetectorType toDetType(const xAOD::UncalibMeasType mType);
    /** @brief Conversts the detector type to an uncalibrated measurement type */
    xAOD::UncalibMeasType toMeasType(const DetectorType dType);
}

#endif