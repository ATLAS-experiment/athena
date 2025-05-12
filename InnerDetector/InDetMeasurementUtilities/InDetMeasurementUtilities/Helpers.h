/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "InDetIdentifier/PixelID.h"

namespace TrackingUtilities {

  std::pair<float, float> computeOmegas(const xAOD::PixelCluster& cluster,
					 const PixelID& pixelID);

}

