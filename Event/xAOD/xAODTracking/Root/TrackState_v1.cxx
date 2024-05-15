/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODCore/AuxStoreAccessorMacros.h"
#include "xAODTracking/versions/TrackState_v1.h"
#include "xAODTracking/AuxAccessorMacro.h"


namespace xAOD {

    DEFINE_API(TrackState_v1, float, chi2, setChi2)    
    DEFINE_API(TrackState_v1, double, pathLength, setPathLength)
    DEFINE_API( TrackState_v1, uint64_t, typeFlags, setTypeFlags)
    DEFINE_API(TrackState_v1, TrackStateIndexType, previous, setPrevious)
    DEFINE_API(TrackState_v1, TrackStateIndexType, next, setNext)
    DEFINE_API(TrackState_v1, TrackStateIndexType, predicted, setPredicted)
    DEFINE_API(TrackState_v1, TrackStateIndexType, filtered, setFiltered)
    DEFINE_API(TrackState_v1, TrackStateIndexType, smoothed, setSmoothed)
    DEFINE_API(TrackState_v1, TrackStateIndexType, jacobian, setJacobian)
    DEFINE_API(TrackState_v1, TrackStateIndexType, calibrated, setCalibrated)
    DEFINE_API(TrackState_v1, TrackStateIndexType, measDim, setMeasDim)
    AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(TrackState_v1, const xAOD::UncalibratedMeasurement *, uncalibratedMeasurement, setUncalibratedMeasurement);
    AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(TrackState_v1,  uint64_t, geometryId, setGeometryId);
    AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(TrackState_v1, ElementLink<xAOD::TrackSurfaceContainer>, surfaceLink, setSurfaceLink);

}


#undef DEFINE_API
