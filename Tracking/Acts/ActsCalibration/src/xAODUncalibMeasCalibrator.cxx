/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "ActsCalibration/xAODUncalibMeasCalibrator.h"

namespace ActsTrk::detail{
    Acts::SourceLink xAODUncalibMeasCalibrator::pack(const xAOD::UncalibratedMeasurement* meas) {
        return Acts::SourceLink{meas};
    }
    const xAOD::UncalibratedMeasurement* xAODUncalibMeasCalibrator::unpack(const Acts::SourceLink& sl) {
        SourceLink_t meas = sl.template get<SourceLink_t>();
        assert(meas != nullptr);
        return meas;
    }
}