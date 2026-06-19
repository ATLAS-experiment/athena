/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRACK_ACTSPROTOTRACK__H
#define ACTSTRACK_ACTSPROTOTRACK__H

#include "xAODMeasurementBase/UncalibratedMeasurementFwd.h"
#include "ActsEvent/TrackParameters.h"

/// @brief small non-persistent data class to wrap the output 
/// of the EF-tracking development pattern finding placeholder
namespace ActsTrk{
  struct ProtoTrack{
    /// set of measurements assigned to the same pattern
    std::vector<const xAOD::UncalibratedMeasurement*> measurements = {};
    /// estimate of initial track parameters for this pattern
    std::unique_ptr<Acts::BoundTrackParameters> parameters = nullptr; 
  };
};



#endif //ACTSTRACK_ACTSPROTOTRACK__H
