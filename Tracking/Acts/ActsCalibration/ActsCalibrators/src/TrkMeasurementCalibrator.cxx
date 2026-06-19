
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "ActsCalibrators/TrkMeasurementCalibrator.h"

namespace ActsTrk::detail {
    const Trk::MeasurementBase* TrkMeasurementCalibrator::unpack(const Acts::SourceLink& sl) {        
        const SourceLink_t& meas = sl.template get<SourceLink_t>();
        return std::holds_alternative<const Trk::MeasurementBase*>(meas) ? 
               std::get<const Trk::MeasurementBase*>(meas) : nullptr;
    }
}