#ifndef STRIPCLUSTERAUXDATACACHECOLLECTION_H
#define STRIPCLUSTERAUXDATACACHECOLLECTION_H

#include "StripClusterAuxDataCache.h"
#include "InDetClusterAuxDataCacheCollection.h"

//template<Utils::AccessPolicy accessPolicy=Utils::AccessPolicy::Const>
using StripClusterAuxDataCacheCollection = InDetClusterAuxDataCacheCollection<StripClusterAuxDataCache<>,
                                                                              typename StripClusterAuxDataCache<>::T_Container>;

namespace traits {

   template <>
   struct ElementProxies< const StripClusterAuxDataCacheCollection > {
      template <Utils::AccessPolicy accessPolicy>
      using AuxDataCache = std::conditional<accessPolicy == Utils::AccessPolicy::Const,
                                            const StripClusterAuxDataCache<Utils::AccessPolicy::Const>,
                                            StripClusterAuxDataCache<Utils::AccessPolicy::Mutable> >::type;
      
      template <Utils::AccessPolicy accessPolicy>
      using ClusterRDOProxy = typename StripClusterAuxDataCache<Utils::AccessPolicy::Const>::template StripClusterRDOProxy<accessPolicy>;
      template <Utils::AccessPolicy accessPolicy>
      using ClusterProxy =  typename StripClusterAuxDataCache<Utils::AccessPolicy::Const>::template StripClusterProxy<accessPolicy>;

      template <Utils::AccessPolicy accessPolicy>
      using ModuleProxy = ClusterContainerProxy<AuxDataCache<accessPolicy>, ClusterProxy<accessPolicy> >;
   };

}


inline auto makeStripClusterAuxDataCacheModuleProxy(const StripClusterAuxDataCache<> &container, IndexWithSubsetRange &&index) {
   return ClusterContainerProxy< const StripClusterAuxDataCache<>,
                                 StripClusterAuxDataCache<Utils::AccessPolicy::Const>::StripClusterProxy<Utils::AccessPolicy::Const> >(&container,
                                                                                                                                       std::move(index));
}


// Create a top level proxy to access random clusters or iterate over clusters (read-only access).
// The proxy will provide read-only access, although the aux data cache would have provided read/write access if it was not const.
inline auto makeStripClusterAuxDataCacheCollectionProxy(const StripClusterAuxDataCacheCollection &collection) {
   using StripClusterProxy_t = StripClusterAuxDataCache<Utils::AccessPolicy::Const>::StripClusterProxy<Utils::AccessPolicy::Const>;
   return ContainerCollectionProxy< const StripClusterAuxDataCacheCollection, StripClusterProxy_t >(&collection);
}

#endif
