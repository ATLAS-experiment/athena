#ifndef INDETCLUSTERAUXDATACACHE_H
#define INDETCLUSTERAUXDATACACHE_H

#include <type_traits>
#include "InDetRawData/ProxyContainer.h"
#include "Identifier/Identifier.h"
#include "AthContainers/JaggedVecAccessor.h"
#include "ProxyUtils.h"

/// The proxies to access the data of a certain container.
namespace traits {
   template <typename T_Container>
   struct ElementProxies;
}

/// Aux data item cache to speed up access of inner detector xAOD clusters.
template<typename T_Container, std::size_t NDim>
struct InDetClusterAuxDataCache  {
   template <typename T>
   using access_controlled_t = std::conditional< std::is_const_v<T_Container>, const T *,  T *>::type;
   template <typename T>
   using accessor_t = SG::Accessor<T>;
   static constexpr bool isConst = std::is_const_v<T_Container>;

protected:
   template <typename T> static auto getData(T_Container *src_container, [[maybe_unused]] std::size_t container_size, const SG::Accessor<T> &accessor)
      -> std::conditional< std::is_const_v<T_Container>, const T *,  T *>::type
   {
      if (container_size==0u) return {};
#ifndef NDEBUG
      auto data = accessor.getDataSpan(*src_container);
      assert (data.size() == container_size);
      return data.data();
#else
      return accessor.getDataArray(*src_container);
#endif
   }
   template <typename T_Span>
   static auto getData(T_Span data, [[maybe_unused]] std::size_t expected_size) {
      assert (data.size() == expected_size || expected_size==0);
      return data.data();
   }
   template <typename T_StoreType, typename T_JaggedVecAccessor, typename T_ArrayElt, typename T_ArrayPayload>
   void setJaggedVectorData([[maybe_unused]] T_StoreType &store,
                            const T_JaggedVecAccessor &accessor,
                            unsigned int n_cluster_rdos,
                            bool is_initialized,
                            T_ArrayElt *&elt,
                            T_ArrayPayload *&payload) {
      if constexpr(!std::is_const_v<T_Container>) {
         if (n_cluster_rdos>0u || is_initialized) {
            store.getData(accessor.linkedAuxid(),n_cluster_rdos, n_cluster_rdos);
         }
      }
      if (!is_initialized || m_sizeInfo.m_size>0 ) {
         elt = getData(accessor.getEltSpan(*m_srcContainer), m_sizeInfo.m_size );
         payload = getData(accessor.getPayloadSpan(*m_srcContainer), n_cluster_rdos);
      }
      else {
         elt=nullptr;
         payload=nullptr;
      }
   }
   template <typename T_StoreType, typename T_JaggedVecAccessor, typename T_ArrayElt, typename T_ArrayPayload>
   void setJaggedVectorData([[maybe_unused]] T_StoreType &store,
                            const T_JaggedVecAccessor &accessor,
                            unsigned int n_cluster_rdos,
                            bool is_initialized,
                            T_ArrayElt *&elt,
                            T_ArrayPayload *&payload,
                            std::size_t &set_payload_size) {
      if constexpr(!std::is_const_v<T_Container>) {
         store.getData(accessor.linkedAuxid(),n_cluster_rdos, n_cluster_rdos);
      }
      if (!is_initialized || m_sizeInfo.m_size>0) {
         elt = getData(accessor.getEltSpan(*m_srcContainer), m_sizeInfo.m_size );
         auto payload_span =accessor.getPayloadSpan(*m_srcContainer);
         if (payload_span.size() != set_payload_size && set_payload_size == 0ul) {
            set_payload_size=payload_span.size();
         }
         payload = getData(payload_span, n_cluster_rdos);
      }
      else {
         elt=nullptr;
         payload=nullptr;
      }
   }
   template <typename T_StoreType, typename T_JaggedVecAccessor>
   auto setupPayload([[maybe_unused]] T_StoreType &store,
                     const T_JaggedVecAccessor &accessor,
                     unsigned int the_size) {
      if (the_size==0) {
      }
      else {
         if constexpr(!std::is_const_v<T_Container>) {
            store.getData(accessor.linkedAuxid(),the_size, the_size);
         }
         return getData(accessor.getPayloadSpan(*m_srcContainer), the_size);
      }
   }
   template <typename T_StoreType, typename T_JaggedVecAccessor>
   auto setupPayload([[maybe_unused]] T_StoreType &store,
                     const T_JaggedVecAccessor &accessor,
                     unsigned int the_size,
                     std::size_t &set_size)
      -> std::conditional< std::is_const_v<T_Container>,
                           const typename T_JaggedVecAccessor::Payload_t *,
                           typename T_JaggedVecAccessor::Payload_t *>::type
   {
      if (the_size==0) {
         return nullptr;
      }
      else {
         if constexpr(!std::is_const_v<T_Container>) {
            store.getData(accessor.linkedAuxid(),the_size, the_size);
         }
         auto data_span = accessor.getPayloadSpan(*m_srcContainer);
         if (data_span.size()!=set_size && set_size==0ul) {
            set_size=data_span.size();
         }
         return getData(data_span, the_size);
      }
   }

public:
   /// Create an aux data cache and reserve storage for RDO data which is stored in jagged vectors.
   InDetClusterAuxDataCache(T_Container &srcContainer, unsigned int n_cluster_rdos, bool is_initialized=true)
      : m_srcContainer(&srcContainer),
        m_sizeInfo(m_srcContainer->size(), is_initialized),
        m_rdoSizeInfo(n_cluster_rdos, is_initialized),
        m_identifier( getData(m_srcContainer, m_sizeInfo.m_size, s_identifierAcc) ),
        m_identifierHash( getData(m_srcContainer, m_sizeInfo.m_size, s_identifierHashAcc) ),
        m_localPosition( getData(m_srcContainer, m_sizeInfo.m_size, s_localPositionAcc) ),
        m_localCovariance( getData(m_srcContainer, m_sizeInfo.m_size, s_localCovarianceAcc) ),
        m_globalPosition( getData(m_srcContainer, m_sizeInfo.m_size, s_globalPositionAcc) ),
        m_channelsInPhi( getData(m_srcContainer, m_sizeInfo.m_size, s_channelsInPhiAcc) )
   {
      auto *store = srcContainer.getStore();
      assert(store);
      setJaggedVectorData(*store,s_rdoListAcc, n_cluster_rdos, is_initialized, m_rdoList, m_rdoListPayload, m_rdoSizeInfo.m_size);
   }

   /// Create an aux data cache for a fully allocated container.
   InDetClusterAuxDataCache(T_Container &srcContainer) requires(std::is_const_v<T_Container>)
   : InDetClusterAuxDataCache(srcContainer,0u,0u,true) {}

   /// Add the identifiers of the associated RDOs to the container.
   void emplace_back_rdos(Identifier::value_type compact_id)
      requires(!std::is_const_v<T_Container>)
   {
      assert( m_rdoSizeInfo.m_initializedSize < m_rdoSizeInfo.m_size);
      m_rdoListPayload[m_rdoSizeInfo.m_initializedSize]=compact_id;
      ++m_rdoSizeInfo.m_initializedSize;
   }

   /// Get the index after the last added RDO.
   unsigned int currentRdoEndIndex() const {
      std::size_t sz = this->m_rdoSizeInfo.initializedSize();
      assert( sz == static_cast<unsigned int>(sz) );
      return static_cast<unsigned int>(sz);
   }

   /// Get the number of added clusters.
   unsigned int initializedSize() const {
      return this->m_sizeInfo.initializedSize();
   }

   /// Add a cluster to the container.
   /// the rdo data for this cluster will be the element range from
   /// the rdo_end_index of the preceding call (or zero) and the rdo_end_index
   /// (non-inclusive).
   void emplace_back(xAOD::DetectorIdentType identifier,
                     xAOD::DetectorIDHashType identifierHash,
                     const std::span<const float, NDim> &localPosition,
                     const std::span<const float, NDim*NDim> &localCovariance,
                     const std::span<const float,3> &globalPosition,
                     int channelsInPhi,
                     unsigned int rdo_end_index)
      requires(!std::is_const_v<T_Container>)
   {
      assert( m_sizeInfo.m_initializedSize < m_sizeInfo.m_size);
      m_identifier[m_sizeInfo.m_initializedSize]=identifier;
      m_identifierHash[m_sizeInfo.m_initializedSize]=identifierHash;
      std::copy(localPosition.begin(), localPosition.end(), m_localPosition[m_sizeInfo.m_initializedSize].begin());
      std::copy(localCovariance.begin(), localCovariance.end(), m_localCovariance[m_sizeInfo.m_initializedSize].begin());
      std::copy(globalPosition.begin(),globalPosition.end(),m_globalPosition[m_sizeInfo.m_initializedSize].begin());
      m_channelsInPhi[m_sizeInfo.m_initializedSize]=channelsInPhi;
      m_rdoList[this->m_sizeInfo.m_initializedSize]=rdo_end_index;
      ++m_sizeInfo.m_initializedSize;
   }

   /// the maximum capacity of the pre-allocated container
   std::size_t capacity() const { return m_sizeInfo.m_size; }

   /// The number of clusters added to the container.
   std::size_t size() const {
      if constexpr(!std::is_const_v<T_Container>) {
         return m_sizeInfo.m_initializedSize;
      }
      else {
         return capacity();
      }
   }

   /// Get the xAOD object for cluster with the given index.
   const auto *getInterfaceObject(unsigned int index) const {
      assert (m_srcContainer);
      assert(index < m_sizeInfo.initializedSize());
      const T_Container &const_container = *m_srcContainer;
      return const_container[index];
   }

   T_Container *m_srcContainer;
   // provides the capacity and number of clusters, RDOs etc.
   // for const container it is assumed that the number of
   // clusters, RDOs, etc. is equal to the capacity.
   struct SizeInfo {
      SizeInfo(std::size_t container_size, [[maybe_unused]] bool is_initialized) : m_size(container_size) {
         if constexpr(!std::is_const_v<T_Container>) {
            m_initializedSize = is_initialized ? container_size : 0ul;
         }
      }
      std::size_t initializedSize() const {
         if constexpr(std::is_const_v<T_Container>) {
            return m_size;
         }
         else {
            return m_initializedSize;
         }
      }
      std::size_t m_size;
      using EmptyStruct = struct{};
      std::conditional< std::is_const_v<T_Container>, EmptyStruct, std::size_t >::type m_initializedSize;
   };

   SizeInfo m_sizeInfo;
   SizeInfo m_rdoSizeInfo;

   access_controlled_t<xAOD::DetectorIdentType>  m_identifier;
   access_controlled_t<xAOD::DetectorIDHashType> m_identifierHash;
   access_controlled_t<typename xAOD::PosAccessor<NDim>::element_type > m_localPosition;
   access_controlled_t<typename xAOD::CovAccessor<NDim>::element_type > m_localCovariance;
   access_controlled_t<std::array<float, 3> > m_globalPosition;
   access_controlled_t<int>  m_channelsInPhi;
   access_controlled_t<SG::Accessor<SG::JaggedVecElt<Identifier::value_type> >::Elt_t> m_rdoList;
   access_controlled_t<SG::Accessor<SG::JaggedVecElt<Identifier::value_type> >::Payload_t> m_rdoListPayload;

   static const accessor_t<xAOD::DetectorIdentType>  s_identifierAcc;
   static const accessor_t<xAOD::DetectorIDHashType> s_identifierHashAcc;
   static const accessor_t<typename xAOD::PosAccessor<NDim>::element_type > s_localPositionAcc;
   static const accessor_t<typename xAOD::CovAccessor<NDim>::element_type > s_localCovarianceAcc;
   static const accessor_t<std::array<float, 3> > s_globalPositionAcc;
   static const accessor_t<int>  s_channelsInPhiAcc;
   static const accessor_t<SG::JaggedVecElt<Identifier::value_type> > s_rdoListAcc;
};

/// Proxies to represent the container, the clusters and RDOs provided by an InDetClusterAuxDataCache
template<typename T_AuxDataCache, std::size_t NDim>
struct InDetClusterAuxDataCacheProxies {
   static constexpr bool isConst = std::is_const_v<T_AuxDataCache>;
   static constexpr Utils::AccessPolicy containerAccessPolicy = Utils::AccessPolicyHelper<isConst>::accessPolicy;
   using ContainerNonConst = std::remove_cvref_t<T_AuxDataCache>;

   /// A proxy representing an RDO
   class InDetClusterRDOProxy : public Utils::ElementProxyBase<T_AuxDataCache, unsigned int> {
   public:
      using BASE = Utils::ElementProxyBase<T_AuxDataCache, unsigned int>;
      using BASE::BASE;

      auto id() const  {
         assert( this->index() < this->container().m_rdoSizeInfo.initializedSize());
         return this->container().m_rdoListPayload[this->index()];
      }
   };

   using RDORangeProxy = Utils::ElementRangeProxy<T_AuxDataCache, InDetClusterRDOProxy >;

   /// A proxy representing a cluster
   class InDetClusterProxy : public Utils::ElementProxyBase<T_AuxDataCache, unsigned int> {
   public:
      using BASE = typename Utils::ElementProxyBase<T_AuxDataCache, unsigned int>;
      using BASE::BASE;

      template <Utils::AccessPolicy rdoAccessPolicy>
      using RDOProxy = typename traits::ElementProxies<typename BASE::ContainerNonConst>::template ClusterRDOProxy<rdoAccessPolicy>;

      template <Utils::AccessPolicy rdoAccessPolicy>
      using RDORangeProxy = Utils::ElementRangeProxy< typename Utils::ContainerAccessHelper<T_AuxDataCache, rdoAccessPolicy>::ContainerType,
                                                      RDOProxy<rdoAccessPolicy> >;

      // Get the corresponding xAOD object
      auto getInterfaceObject() const {
         return this->container().getInterfaceObject(this->index());
      }

      /// Get the local position.
      /// The result is undefined if N does not match the dimension associated to the container.
      template <int N>
      xAOD::ConstVectorMap<N> localPosition() const {
         using E_t = typename std::remove_cvref_t< decltype(this->container().s_localPositionAcc) >::element_type;
         static_assert( std::is_same_v<std::array<float,N>, E_t>);
         assert( this->index() < this->container().m_sizeInfo.initializedSize());
         return xAOD::ConstVectorMap<N>{this->container().m_localPosition[this->index()].data()};
      }
      /// Get the local covariance.
      /// The result is undefined if N does not match the dimension associated to the container.
      template <int N>
      xAOD::ConstMatrixMap<N> localCovariance() const {
         using E_t = typename std::remove_cvref_t< decltype(this->container().s_localCovarianceAcc) >::element_type;
         static_assert( std::is_same_v<std::array<float,N*N>, E_t>);
         assert( this->index() < this->container().m_sizeInfo.initializedSize());
         return xAOD::ConstMatrixMap<N>{this->container().m_localCovariance[this->index()].data()};
      }
      xAOD::ConstVectorMap<3> globalPosition() const {
         assert( this->index() < this->container().m_sizeInfo.initializedSize());
         return xAOD::ConstVectorMap<3>{this->container().m_globalPosition[this->index()].data()};
      }
      float channelsInPhi() const {
         assert( this->index() < this->container().m_sizeInfo.initializedSize());
         return this->container().m_channelsInPhi[this->index()];
      }
      xAOD::DetectorIDHashType identifierHash() const {
         assert( this->index() < this->container().m_sizeInfo.initializedSize());
         return this->container().m_identifierHash[this->index()];
      }
      // Get a proxy representing the associated RDO range (read/write)
      auto rdos() requires (!BASE::isConst ) {
         assert(this->index() < this->container().m_rdoSizeInfo.initializedSize() );
         const auto &rdo_end = this->container().m_rdoList;
         return RDORangeProxy<Utils::AccessPolicy::Mutable>( &this->container(),
                                                             Utils::IndexWithRange(this->index() > 0u ? rdo_end[this->index()-1].end() : 0u,
                                                                                   rdo_end[this->index()].end(),
                                                                                   this->index() ) );
      }
      // Get a proxy representing the associated RDO range (read only)
      auto rdos() const {
         assert(this->index() < this->container().m_rdoSizeInfo.initializedSize() );
         const auto &rdo_end = this->container().m_rdoList;
         Utils::IndexWithRange index(this->index() > 0u ? rdo_end[this->index()-1].end() : 0u,
                                      rdo_end[this->index()].end(),
                                      this->index() );
         return RDORangeProxy<Utils::AccessPolicy::Const>( &this->container(),
                                                            index);
      }
   };

   // Proxy representing the entire container.
   class TopLevelProxy : public Utils::ContainerProxy<T_AuxDataCache,
                                                      TopLevelProxy,
                                                      typename traits::ElementProxies<ContainerNonConst>::template ClusterProxy<containerAccessPolicy>,
                                                      Utils::RootNodeIndex> {
   public:
      using BASE=Utils::ContainerProxy<T_AuxDataCache,
                                       TopLevelProxy,
                                       typename traits::ElementProxies<ContainerNonConst>::template ClusterProxy<containerAccessPolicy>,
                                       Utils::RootNodeIndex>;
      using BASE::BASE;
   };

};

#endif
