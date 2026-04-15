#ifndef PIXELCLUSTERAUXDATACACHE_H
#define PIXELCLUSTERAUXDATACACHE_H

#include "xAODInDetMeasurement/PixelCluster.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "InDetClusterAuxDataCache.h"

#include <type_traits>

/// Aux data item cache to speed up access
/// The xAOD accessors are used once to get access to element vector,
/// which speeds up access if multiple elements need to be access e.g. in
/// a loop.
template<Utils::AccessPolicy accessPolicy=Utils::AccessPolicy::Const>
struct PixelClusterAuxDataCache
   : InDetClusterAuxDataCache< typename Utils::ContainerAccessHelper<const xAOD::PixelClusterContainer, accessPolicy>::ContainerType ,2 >  {
   using T_Container = typename Utils::ContainerAccessHelper<const xAOD::PixelClusterContainer, accessPolicy>::ContainerType;
   using BASE = InDetClusterAuxDataCache< T_Container ,2 >;

   template <Utils::AccessPolicy proxyAccessPolicy=Utils::AccessPolicy::Const>
   using Proxies = InDetClusterAuxDataCacheProxies< typename Utils::ContainerAccessHelper<PixelClusterAuxDataCache,proxyAccessPolicy>::ContainerType, 2 >;

   /// Create the cache and reserve storage for the RDO items.
   /// The container will not be able to store more ROD data than the reserved storage.
   PixelClusterAuxDataCache(T_Container &srcContainer, unsigned int n_cluster_rdos, bool is_initialized=true)
      : BASE(srcContainer,n_cluster_rdos, is_initialized),
        m_channelsInEta( BASE:: getData(this->m_srcContainer, this->m_sizeInfo.m_size, s_channelsInEtaAcc) ),
        m_widthInEta( BASE:: getData(this->m_srcContainer, this->m_sizeInfo.m_size, s_widthInEtaAcc) )
   {
      auto *store = srcContainer.getStore();
      assert(store);
      this->setJaggedVectorData(*store,s_chargeListAcc, n_cluster_rdos, is_initialized, m_chargeList, m_chargeListPayload);
      this->setJaggedVectorData(*store,s_totListAcc, n_cluster_rdos, is_initialized, m_totList, m_totListPayload);
   }
   /// create the cache for a fully allocated container.
   PixelClusterAuxDataCache(T_Container &srcContainer) requires(BASE::isConst) : PixelClusterAuxDataCache(srcContainer, 0u, true) {}

   /// Add new cluster RDO data to the container.
   void emplace_back_rdos(Identifier::value_type compact_id, float charge, int tot)
      requires(!BASE::isConst)
   {
      assert( this->m_rdoSizeInfo.m_initializedSize < this->m_rdoSizeInfo.m_size);

      m_chargeListPayload[this->m_rdoSizeInfo.m_initializedSize]=charge;
      m_totListPayload[this->m_rdoSizeInfo.m_initializedSize]=tot;
      BASE::emplace_back_rdos(compact_id);
   }

   /// Add a new cluster to the container.
   /// the rdo data for this cluster will be the element range from
   /// the rdo_end_index of the preceding call (or zero) and the rdo_end_index
   /// (non-inclusive).
   void emplace_back(xAOD::DetectorIdentType identifier,
                     xAOD::DetectorIDHashType identifierHash,
                     const std::span<const float, 2> &localPosition,
                     const std::span<const float, 2*2> &localCovariance,
                     const std::span<const float,3> &globalPosition,
                     int channelsInPhi,
                     int channelsInEta,
                     float widthInEta,
                     unsigned int rdo_end_index)
      requires(!BASE::isConst)
   {
      m_channelsInEta[this->m_sizeInfo.m_initializedSize]=channelsInEta;
      m_widthInEta[this->m_sizeInfo.m_initializedSize]=widthInEta;
      m_chargeList[this->m_sizeInfo.m_initializedSize]=rdo_end_index;
      m_totList[this->m_sizeInfo.m_initializedSize]=rdo_end_index;
      BASE::emplace_back(identifier,identifierHash, localPosition,localCovariance,globalPosition, channelsInPhi, rdo_end_index);
   }

   /// A proxy representing the Pixel RDO associated to a cluster.
   template <Utils::AccessPolicy proxyAccessPolicy=Utils::AccessPolicy::Const>
   class PixelClusterRDOProxy : public Proxies<proxyAccessPolicy>::InDetClusterRDOProxy
   {
   public:
      using BASE = typename Proxies<proxyAccessPolicy>::InDetClusterRDOProxy;
      using BASE::BASE;
      auto charge() const  {
         assert( this->index() < this->container().m_rdoSizeInfo.initializedSize());
         return this->container().m_chargeListPayload[this->index()];
      }
      auto tot() const {
         assert( this->index() < this->container().m_rdoSizeInfo.initializedSize());
         return this->container().m_totListPayload[this->index()];
      }
   };

   /// A proxy representing a prixel cluster
   template <Utils::AccessPolicy proxyAccessPolicy=Utils::AccessPolicy::Const>
   class PixelClusterProxy  : public Proxies<proxyAccessPolicy>::InDetClusterProxy
   {
   public:
      using BASE = typename Proxies<proxyAccessPolicy>::InDetClusterProxy;
      using BASE::BASE;
      float widthInEta() const {
         assert( this->index() < this->container().m_sizeInfo.initializedSize());
         return this->container().m_widthInEta[this->index()];
      }
      float channelsInEta() const {
         assert( this->index() < this->container().m_sizeInfo.initializedSize());
         return this->container().m_channelsInEta[this->index()];
      }
   };

   template <typename T>
   using access_controlled_t = BASE::template access_controlled_t<T>;
   template <typename T>
   using accessor_t = BASE::template accessor_t<T>;

   access_controlled_t<int>  m_channelsInEta;
   access_controlled_t<float> m_widthInEta;
   access_controlled_t<SG::Accessor<SG::JaggedVecElt<float> >::Elt_t> m_chargeList;
   access_controlled_t<SG::Accessor<SG::JaggedVecElt<float> >::Payload_t> m_chargeListPayload;
   access_controlled_t<SG::Accessor<SG::JaggedVecElt<int> >::Elt_t> m_totList;
   access_controlled_t<SG::Accessor<SG::JaggedVecElt<int> >::Payload_t> m_totListPayload;

   static const accessor_t<int>  s_channelsInEtaAcc;
   static const accessor_t< float > s_widthInEtaAcc;
   static const accessor_t<SG::JaggedVecElt<float> > s_chargeListAcc;
   static const accessor_t<SG::JaggedVecElt<int> > s_totListAcc;

};

namespace traits {
   template <>
   struct ElementProxies< PixelClusterAuxDataCache<Utils::AccessPolicy::Mutable > > {
      template <Utils::AccessPolicy accessPolicy>
      using ClusterRDOProxy = typename PixelClusterAuxDataCache<Utils::AccessPolicy::Mutable>::template PixelClusterRDOProxy<accessPolicy>;
      template <Utils::AccessPolicy accessPolicy>
      using ClusterProxy =  typename PixelClusterAuxDataCache<Utils::AccessPolicy::Mutable>::template PixelClusterProxy<accessPolicy>;
      template <Utils::AccessPolicy accessPolicy>
      using ModuleProxy = typename PixelClusterAuxDataCache<Utils::AccessPolicy::Mutable>::template Proxies<accessPolicy>::ModuleClusterProxy;
   };

   template <>
   struct ElementProxies< PixelClusterAuxDataCache<Utils::AccessPolicy::Const> > {
      template <Utils::AccessPolicy accessPolicy>
      using ClusterRDOProxy = typename PixelClusterAuxDataCache<Utils::AccessPolicy::Const>::template PixelClusterRDOProxy<accessPolicy>;
      template <Utils::AccessPolicy accessPolicy>
      using ClusterProxy =  typename PixelClusterAuxDataCache<Utils::AccessPolicy::Const>::template PixelClusterProxy<accessPolicy>;
      template <Utils::AccessPolicy accessPolicy>
      using ModuleProxy = typename PixelClusterAuxDataCache<Utils::AccessPolicy::Const>::template Proxies<accessPolicy>::ModuleClusterProxy;
      template <Utils::AccessPolicy accessPolicy>
      using AllClusterProxy = typename PixelClusterAuxDataCache<Utils::AccessPolicy::Const>::template Proxies<accessPolicy>::AllClusterTopLevelProxy;
   };

}

// Create a top level proxy to access random clusters or iterate over clusters (read-only access).
// The proxy will provide read-only access, although the aux data cache would have provided read/write access if it was not const.
inline auto makePixelClusterAuxDataCacheProxy(const PixelClusterAuxDataCache<Utils::AccessPolicy::Mutable> &container) {
   return PixelClusterAuxDataCache<Utils::AccessPolicy::Mutable>::Proxies<Utils::AccessPolicy::Const>::TopLevelProxy(&container);
}
// Create a top level proxy to access random clusters or iterate over clusters (read/write access).
inline auto makePixelClusterAuxDataCacheProxy(PixelClusterAuxDataCache<Utils::AccessPolicy::Mutable> &container) {
   return PixelClusterAuxDataCache<Utils::AccessPolicy::Mutable>::Proxies<Utils::AccessPolicy::Mutable>::TopLevelProxy(&container);
}

// Create a top level proxy to access random clusters or iterate over clusters (read-only access).
inline auto makePixelClusterAuxDataCacheProxy(const PixelClusterAuxDataCache<Utils::AccessPolicy::Const> &container) {
   return PixelClusterAuxDataCache<Utils::AccessPolicy::Const>::Proxies<Utils::AccessPolicy::Const>::TopLevelProxy(&container);
}


#endif
