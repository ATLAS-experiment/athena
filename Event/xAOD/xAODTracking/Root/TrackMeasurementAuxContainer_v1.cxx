/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODTracking/versions/TrackMeasurementAuxContainer_v1.h"

namespace xAOD {
  TrackMeasurementAuxContainer_v1::TrackMeasurementAuxContainer_v1()
    : AuxContainerBase() {
    AUX_VARIABLE(meas);
    AUX_VARIABLE(covMatrix);
    AUX_VARIABLE(projector);
  }
}
