/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PFLOWUTILS_ACTSCALOPARAMETERSCHECKER_H
#define PFLOWUTILS_ACTSCALOPARAMETERSCHECKER_H

#include "CaloGeoHelpers/CaloSampling.h"
#include "GaudiKernel/MsgStream.h"

#include "Acts/EventData/BoundTrackParameters.hpp"
#include "Acts/Geometry/GeometryContext.hpp"

#include <vector>

/**
 Checks the consistency of the ACTS track parameters stored in an ActsTrk::CaloExtension
 for a single calorimeter sampling. When there is more than one parameter for the sampling, it reports this at DEBUG
 level and prints a WARNING for any parameter whose eta or phi differs from the first (nominal) one
 by more than half the smallest calorimeter cell width (0.025/2).
 Messages are written to the caller's message stream (e.g. msg() of an AthAlgTool), so they follow its OutputLevel.
*/
void checkMultipleActsCaloParameters(const std::vector<Acts::BoundTrackParameters>& caloPars,
                                     CaloSampling::CaloSample sample,
                                     const Acts::GeometryContext& gctx,
                                     MsgStream& msg);

#endif
