#ifndef STRIPCLUSTERAUXDATACACHE_H
#define STRIPCLUSTERAUXDATACACHE_H

#include "xAODInDetMeasurement/StripCluster.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "InDetClusterAuxDataCache.h"

#include <type_traits>

/// Aux data item cache to speed up access
/// The xAOD accessors are used once to get access to element vector,
/// which speeds up access if multiple elements need to be access e.g. in
/// a loop.

///@TODO in principle it would be sufficient to define 
// template<Utils::AccessPolicy accessPolicy=Utils::AccessPolicy::Const>
// using StripClusterAuxDataCache = InDetClusterAuxDataCache< typename Utils::ContainerAccessHelper<const xAOD::StripClusterContainerAlt, accessPolicy>::ContainerType ,1 >

template<Utils::AccessPolicy accessPolicy=Utils::AccessPolicy::Const>
struct StripClusterAuxDataCache
   : InDetClusterAuxDataCache< typename Utils::ContainerAccessHelper<const xAOD::StripClusterContainerAlt, accessPolicy>::ContainerType ,1 >  {
   using T_Container = typename Utils::ContainerAccessHelper<const xAOD::StripClusterContainerAlt, accessPolicy>::ContainerType;
   using BASE = InDetClusterAuxDataCache< T_Container ,1 >;

   template <Utils::AccessPolicy proxyAccessPolicy=Utils::AccessPolicy::Const>
   using Proxies = InDetClusterAuxDataCacheProxies< typename Utils::ContainerAccessHelper<StripClusterAuxDataCache,proxyAccessPolicy>::ContainerType, 1 >;

   /// Create the cache and reserve storage for the RDO items.
   /// The container will not be able to store more ROD data than the reserved storage.
   StripClusterAuxDataCache(T_Container &srcContainer, unsigned int n_cluster_rdos, bool is_initialized=true)
      : BASE(srcContainer,n_cluster_rdos, is_initialized)
   {
   }
   /// create the cache for a fully allocated container.
   StripClusterAuxDataCache(T_Container &srcContainer) requires(BASE::isConst) : StripClusterAuxDataCache(srcContainer, 0u, true) {}

   // /// Add new cluster RDO data to the container.
   // void emplace_back_rdos(Identifier::value_type compact_id)
   //    requires(!BASE::isConst)
   // {
   //    assert( this->m_rdoSizeInfo.m_initializedSize < this->m_rdoSizeInfo.m_size);

   //    BASE::emplace_back_rdos(compact_id);
   // }

   /// Add a new cluster to the container.
   /// the rdo data for this cluster will be the element range from
   /// the rdo_end_index of the preceding call (or zero) and the rdo_end_index
   /// (non-inclusive).
   void emplace_back(xAOD::DetectorIdentType identifier,
                     xAOD::DetectorIDHashType identifierHash,
                     const std::span<const float, 1> &localPosition,
                     const std::span<const float, 1> &localCovariance,
                     const std::span<const float,3> &globalPosition,
                     int channelsInPhi,
                     unsigned int rdo_end_index)
      requires(!BASE::isConst)
   {
      BASE::emplace_back(identifier,identifierHash, localPosition,localCovariance,globalPosition, channelsInPhi, rdo_end_index);
   }

   /// A proxy representing the Strip RDO associated to a cluster.
   template <Utils::AccessPolicy proxyAccessPolicy=Utils::AccessPolicy::Const>
   class StripClusterRDOProxy : public Proxies<proxyAccessPolicy>::InDetClusterRDOProxy
   {
   public:
      using BASE = typename Proxies<proxyAccessPolicy>::InDetClusterRDOProxy;
      using BASE::BASE;
   };

   /// A proxy representing a prixel cluster
   template <Utils::AccessPolicy proxyAccessPolicy=Utils::AccessPolicy::Const>
   class StripClusterProxy  : public Proxies<proxyAccessPolicy>::InDetClusterProxy
   {
   public:
      using BASE = typename Proxies<proxyAccessPolicy>::InDetClusterProxy;
      using BASE::BASE;
   };

   template <typename T>
   using access_controlled_t = BASE::template access_controlled_t<T>;
   template <typename T>
   using accessor_t = BASE::template accessor_t<T>;
};

namespace traits {
   template <>
   struct ElementProxies< StripClusterAuxDataCache<Utils::AccessPolicy::Mutable > > {
      template <Utils::AccessPolicy accessPolicy>
      using ClusterRDOProxy = typename StripClusterAuxDataCache<Utils::AccessPolicy::Mutable>::template StripClusterRDOProxy<accessPolicy>;
      template <Utils::AccessPolicy accessPolicy>
      using ClusterProxy =  typename StripClusterAuxDataCache<Utils::AccessPolicy::Mutable>::template StripClusterProxy<accessPolicy>;
      template <Utils::AccessPolicy accessPolicy>
      using ModuleProxy = typename StripClusterAuxDataCache<Utils::AccessPolicy::Mutable>::template Proxies<accessPolicy>::ModuleClusterProxy;
   };

   template <>
   struct ElementProxies< StripClusterAuxDataCache<Utils::AccessPolicy::Const> > {
      template <Utils::AccessPolicy accessPolicy>
      using ClusterRDOProxy = typename StripClusterAuxDataCache<Utils::AccessPolicy::Const>::template StripClusterRDOProxy<accessPolicy>;
      template <Utils::AccessPolicy accessPolicy>
      using ClusterProxy =  typename StripClusterAuxDataCache<Utils::AccessPolicy::Const>::template StripClusterProxy<accessPolicy>;
      template <Utils::AccessPolicy accessPolicy>
      using ModuleProxy = typename StripClusterAuxDataCache<Utils::AccessPolicy::Const>::template Proxies<accessPolicy>::ModuleClusterProxy;
      template <Utils::AccessPolicy accessPolicy>
      using AllClusterProxy = typename StripClusterAuxDataCache<Utils::AccessPolicy::Const>::template Proxies<accessPolicy>::AllClusterTopLevelProxy;
   };

}

// Create a top level proxy to access random clusters or iterate over clusters (read-only access).
// The proxy will provide read-only access, although the aux data cache would have provided read/write access if it was not const.
inline auto makeStripClusterAuxDataCacheProxy(const StripClusterAuxDataCache<Utils::AccessPolicy::Mutable> &container) {
   return StripClusterAuxDataCache<Utils::AccessPolicy::Mutable>::Proxies<Utils::AccessPolicy::Const>::TopLevelProxy(&container);
}
// Create a top level proxy to access random clusters or iterate over clusters (read/write access).
inline auto makeStripClusterAuxDataCacheProxy(StripClusterAuxDataCache<Utils::AccessPolicy::Mutable> &container) {
   return StripClusterAuxDataCache<Utils::AccessPolicy::Mutable>::Proxies<Utils::AccessPolicy::Mutable>::TopLevelProxy(&container);
}

// Create a top level proxy to access random clusters or iterate over clusters (read-only access).
inline auto makeStripClusterAuxDataCacheProxy(const StripClusterAuxDataCache<Utils::AccessPolicy::Const> &container) {
   return StripClusterAuxDataCache<Utils::AccessPolicy::Const>::Proxies<Utils::AccessPolicy::Const>::TopLevelProxy(&container);
}


#endif
