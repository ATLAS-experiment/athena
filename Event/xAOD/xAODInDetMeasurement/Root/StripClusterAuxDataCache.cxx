#include  "xAODInDetMeasurement/StripClusterAuxDataCache.h"
#include  "xAODInDetMeasurement/InDetClusterAuxDataCache.h"

#include <iostream>
#include <map>
#include <set>

#include  "xAODInDetMeasurement/InDetClusterAuxDataCacheAccessor.icc"


template struct StripClusterAuxDataCache<Utils::AccessPolicy::Const>;
template struct StripClusterAuxDataCache<Utils::AccessPolicy::Mutable>;


