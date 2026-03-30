#ifndef PIXELCLUSTERAUXDATACACHECOLLECTION_H
#define PIXELCLUSTERAUXDATACACHECOLLECTION_H

#include "PixelClusterAuxDataCache.h"
#include "InDetClusterAuxDataCacheCollection.h"

//template<Utils::AccessPolicy accessPolicy=Utils::AccessPolicy::Const>
using PixelClusterAuxDataCacheCollection = InDetClusterAuxDataCacheCollection<PixelClusterAuxDataCache<>,
                                                                              typename PixelClusterAuxDataCache<>::T_Container>;

namespace traits {

   template <>
   struct ElementProxies< const PixelClusterAuxDataCacheCollection > {
      template <Utils::AccessPolicy accessPolicy>
      using AuxDataCache = std::conditional<accessPolicy == Utils::AccessPolicy::Const,
                                            const PixelClusterAuxDataCache<Utils::AccessPolicy::Const>,
                                            PixelClusterAuxDataCache<Utils::AccessPolicy::Mutable> >::type;

      template <Utils::AccessPolicy accessPolicy>
      using ClusterRDOProxy = typename PixelClusterAuxDataCache<Utils::AccessPolicy::Const>::template PixelClusterRDOProxy<accessPolicy>;
      template <Utils::AccessPolicy accessPolicy>
      using ClusterProxy =  typename PixelClusterAuxDataCache<Utils::AccessPolicy::Const>::template PixelClusterProxy<accessPolicy>;

      template <Utils::AccessPolicy accessPolicy>
      using ModuleProxy = ClusterContainerProxy<AuxDataCache<accessPolicy>, ClusterProxy<accessPolicy> >;
   };

}


inline auto makePixelClusterAuxDataCacheModuleProxy(const PixelClusterAuxDataCache<> &container, IndexWithSubsetRange &&index) {
   return ClusterContainerProxy< const PixelClusterAuxDataCache<>,
                                 PixelClusterAuxDataCache<Utils::AccessPolicy::Const>::PixelClusterProxy<Utils::AccessPolicy::Const> >(&container,
                                                                                                                                       std::move(index));
}


// Create a top level proxy to access random clusters or iterate over clusters (read-only access).
// The proxy will provide read-only access, although the aux data cache would have provided read/write access if it was not const.
inline auto makePixelClusterAuxDataCacheCollectionProxy(const PixelClusterAuxDataCacheCollection &collection) {
   using PixelClusterProxy_t = PixelClusterAuxDataCache<Utils::AccessPolicy::Const>::PixelClusterProxy<Utils::AccessPolicy::Const>;
   return ContainerCollectionProxy< const PixelClusterAuxDataCacheCollection, PixelClusterProxy_t >(&collection);
}

#endif
