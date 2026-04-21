#include  "xAODInDetMeasurement/PixelClusterAuxDataCache.h"
#include  "xAODInDetMeasurement/InDetClusterAuxDataCache.h"

#include  "xAODInDetMeasurement/InDetClusterAuxDataCacheAccessor.icc"
#include  "xAODInDetMeasurement/PixelClusterAuxDataCacheAccessor.icc"

template struct PixelClusterAuxDataCache<Utils::AccessPolicy::Const>;
template struct PixelClusterAuxDataCache<Utils::AccessPolicy::Mutable>;


