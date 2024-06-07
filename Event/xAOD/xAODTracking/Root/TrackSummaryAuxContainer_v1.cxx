/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODTracking/versions/TrackSummaryAuxContainer_v1.h"
#include "xAODCore/tools/AuxVariable.h"

namespace xAOD {
  TrackSummaryAuxContainer_v1::TrackSummaryAuxContainer_v1()
    : AuxContainerBase() {
    AUX_VARIABLE(params);
    AUX_VARIABLE(covParams);
    AUX_VARIABLE(nMeasurements);
    AUX_VARIABLE(nHoles);
    AUX_VARIABLE(chi2f);
    AUX_VARIABLE(ndf);
    AUX_VARIABLE(nOutliers);
    AUX_VARIABLE(nSharedHits);
    AUX_VARIABLE(tipIndex);
    AUX_VARIABLE(stemIndex);
    AUX_VARIABLE(particleHypothesis);
    AUX_VARIABLE(surfaceIndex);

  }
}
