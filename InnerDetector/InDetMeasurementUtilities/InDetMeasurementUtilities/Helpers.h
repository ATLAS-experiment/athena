/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "InDetIdentifier/PixelID.h"
#include "xAODInDetMeasurement/PixelClusterAuxDataCacheCollection.h"

namespace TrackingUtilities {

  std::pair<float, float> computeOmegas(const xAOD::PixelCluster& cluster,
                                        const PixelID& pixelID);

  using PixelCluster_t = traits::ElementProxies<const PixelClusterAuxDataCacheCollection >::ClusterProxy<Utils::AccessPolicy::Const>;
  std::pair<float, float> computeOmegas(const PixelCluster_t& cluster,
                                        const PixelID& pixelID);

}

