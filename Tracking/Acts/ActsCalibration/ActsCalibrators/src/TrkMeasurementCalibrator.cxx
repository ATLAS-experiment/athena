
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "ActsCalibrators/TrkMeasurementCalibrator.h"

namespace ActsTrk::detail {
    Acts::SourceLink TrkMeasurementCalibrator::pack(const Trk::MeasurementBase* meas) {
        return Acts::SourceLink{meas};
    }
    const Trk::MeasurementBase* TrkMeasurementCalibrator::unpack(const Acts::SourceLink& sl) {        
        SourceLink_t meas = sl.template get<SourceLink_t>();
        assert(meas != nullptr);
        return meas;
    }
}