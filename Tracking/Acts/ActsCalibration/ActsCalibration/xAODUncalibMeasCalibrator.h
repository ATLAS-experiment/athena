/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSCALIBRATION_DETAIL_XAODUNCALIBMEASCALIBRATOR_H
#define ACTSCALIBRATION_DETAIL_XAODUNCALIBMEASCALIBRATOR_H

#include "ActsCalibration/MeasurementCalibratorBase.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"


namespace ActsTrk::detail{
    /** @brief  */
    class xAODUncalibMeasCalibrator : public MeasurementCalibratorBase {
        public:
          /** @brief Underlying source link type of the uncalibrated measurement  */
          using SourceLink_t = const xAOD::UncalibratedMeasurement*;
          /** @brief Helper method to pack an uncalibrated measurement to an Acts source link
           *  @param meas: Pointer to the measurement to unpack */
          static Acts::SourceLink pack(const xAOD::UncalibratedMeasurement* meas);
          /** @brief Helper method to unpack an Acts source link to an uncalibrated measurement
           *  @param sl: Reference to the source link pointing to the uncalibrated measurement */
          static const xAOD::UncalibratedMeasurement* unpack(const Acts::SourceLink& sl);
        private:
          
    };
}

#endif