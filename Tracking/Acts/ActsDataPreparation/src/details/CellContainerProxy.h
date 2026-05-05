/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#ifndef INPLACECLUSTERIZATION_CELLCONTAINERPROXY_H
#define INPLACECLUSTERIZATION_CELLCONTAINERPROXY_H
// helper classes to provide proxies to access hierarchical cell data:
//    cell_proxy = module_cluster_collection_proxy[module_index][cluster_index][cell_index]
// and provide the means to iterate over the cells using range based for loops:
//    for (auto module_proxy :  module_cluster_collection_proxy) {
//        for (auto cluster_proxy : module_proxy) {
//            for(auto cell_proxy : cluster_proxy) {
//                   .. cell_proxy.coordinates();
//                   .. rdo_index  =  cell_proxy.srcIndex();
//            }
//        }
//    }
#include "InDetRawData/ProxyContainer.h"
#include <cstdint>

namespace InPlaceClusterization {

using namespace Utils;

// Cell data proxy providing access to the coordinates of a cell, and the index of the rdo it is representing
// i.e. pixel colum and row, or strip number,
template <class T_CellContainer>
class CellProxy : public  Utils::ElementProxyBase<T_CellContainer,unsigned int> {

public:
   using BASE = Utils::ElementProxyBase<T_CellContainer,unsigned int>;
   using BASE::BASE;

   auto srcIndex() const {
      assert(this->index() <  this->container().m_cells.size());
      return this->container().m_cells[this->index()].srcIndex;
   }
   const auto &coordinates() const {
      assert(this->index() <  this->container().m_cells.size());
      return this->container().m_cells[this->index()].coordinates;
   }
   auto &coordinates() requires (!BASE::isConst)  {
      assert(this->index() <  this->container().m_cells.size());
      return this->container().m_cells[this->index()].coordinates;
   }
};

// An extended cluster index which also provides the module id hash (unused; for debugging)
// and the index of the first cell of all clusters.
struct IndexWithBeginIndexCache {
   IndexWithBeginIndexCache &operator++() { ++m_clusterIndex; return *this; }
   bool operator==(const IndexWithBeginIndexCache &other) const {
      // should only be executed if the indices refer to the same module,
      // thus the cached begin index should always be identical.
      assert(m_cellBeginIndex == other.m_cellBeginIndex);
      return m_clusterIndex == other.m_clusterIndex;
   }
   std::size_t operator-(const IndexWithBeginIndexCache &other) const {
      assert( m_clusterIndex >= other.m_clusterIndex);
      return m_clusterIndex - other.m_clusterIndex;
   }
   // @TODO ugly that "+" and "-" return differnt types
   //    but operator - is used to compute the size
   //    and operator + to create an element index for random access
   IndexWithBeginIndexCache operator+(std::size_t counter) const {
      IndexWithBeginIndexCache ret(*this);
      ret.m_clusterIndex += counter;
      return ret;
   }
   unsigned int m_clusterIndex;
   unsigned int m_cellBeginIndex;
   unsigned int m_idHash; // for debugging
};

// A proxy representing a cluster of a module which gives access to the cells this
// cluster is comprised of.
template <class T_CellContainer>
class ClusterProxy : public Utils::ContainerProxy<T_CellContainer,
                                                  ClusterProxy<T_CellContainer>,
                                                  CellProxy<T_CellContainer>,
                                                  IndexWithBeginIndexCache>
{
public:
   using BASE = Utils::ContainerProxy<T_CellContainer,
                                      ClusterProxy<T_CellContainer>,
                                      CellProxy<T_CellContainer>,
                                      IndexWithBeginIndexCache>;
   using BASE::BASE;

   unsigned int identifyHash() const {
      return this->index().m_idHash;
   }
   // return the index of the first cell of this cluster
   static unsigned int beginIndex(const T_CellContainer *container, IndexWithBeginIndexCache cluster_index)
   {
      assert(container != nullptr);
      // the cluster index (cluster_index.m_clusterIndex) should point at the end index of a cluster in container->m_relativeClusterCellIndex
      assert(cluster_index.m_clusterIndex>0);
      assert(cluster_index.m_clusterIndex < container->m_relativeClusterCellIndex.size());
      // a cluster must contain at least one cell, thus "<" not "<="
      assert(cluster_index.m_cellBeginIndex + container->m_relativeClusterCellIndex[cluster_index.m_clusterIndex-1] < container->m_cells.size());
      return static_cast<unsigned int> ( cluster_index.m_cellBeginIndex + container->m_relativeClusterCellIndex[cluster_index.m_clusterIndex-1]);
   }
   // return the index after the last cell of this cluster
   static unsigned int endIndex(const T_CellContainer *container,IndexWithBeginIndexCache cluster_index) {
      assert(container != nullptr);
      assert(cluster_index.m_clusterIndex < container->m_relativeClusterCellIndex.size());
      // the end cell index must not exceed the size of the cells but can be equal.
      assert(cluster_index.m_cellBeginIndex + container->m_relativeClusterCellIndex[cluster_index.m_clusterIndex] <= container->m_cells.size());
      return static_cast<unsigned int> ( cluster_index.m_cellBeginIndex + container->m_relativeClusterCellIndex[cluster_index.m_clusterIndex]);
   }
   // compute the index of the next cell of a cluster defined by the given cell index
   static unsigned int nextElementIndex([[maybe_unused]] const T_CellContainer *container, unsigned int element_index)
   {
      assert( element_index < container->m_cells.size());
      return ++element_index;
   }
};

// Proxy representing the clusters of a module
template <class T_CellContainer>
class ModuleProxy : public Utils::ContainerProxy<T_CellContainer,
                                                 ModuleProxy<T_CellContainer>,
                                                 ClusterProxy<T_CellContainer>,
                                                 unsigned int >  {
public:
   using BASE = Utils::ContainerProxy<T_CellContainer,
                                      ModuleProxy<T_CellContainer>,
                                      ClusterProxy<T_CellContainer>,
                                      unsigned int >;
   using BASE::BASE;

   // index referring to the first cluster of a module
   static IndexWithBeginIndexCache beginIndex(const T_CellContainer *container, unsigned int module_index)
   {
      assert(container != nullptr);
      assert(module_index < container->m_moduleClusterRange.size());
      const typename T_CellContainer::ClusterRange &cluster_range = container->m_moduleClusterRange[module_index];
      assert( cluster_range.clusterRangeBeginIndex <= container->m_relativeClusterCellIndex.size());
      assert( cluster_range.cellBeginIndex <= container->m_cells.size());
      return IndexWithBeginIndexCache{cluster_range.clusterRangeBeginIndex, cluster_range.cellBeginIndex, cluster_range.idHash/*for debugging*/};
   }
   // index after the last cluster of a module
   static IndexWithBeginIndexCache endIndex(const T_CellContainer *container,unsigned int module_index) {
      assert(container != nullptr);
      assert(module_index < container->m_moduleClusterRange.size());
      const typename T_CellContainer::ClusterRange &cluster_range = container->m_moduleClusterRange[module_index];
      assert( cluster_range.clusterRangeEndIndex <= container->m_relativeClusterCellIndex.size());
      assert( cluster_range.cellBeginIndex <= container->m_cells.size());
      return IndexWithBeginIndexCache{cluster_range.clusterRangeEndIndex, cluster_range.cellBeginIndex, cluster_range.idHash /*for debugging*/};
   }
   // compute the index of the next cluster, where the cluster is defined by the given cluster index
   static IndexWithBeginIndexCache nextElementIndex([[maybe_unused]] const T_CellContainer *container, IndexWithBeginIndexCache element_index)
   {
      assert( container != nullptr);
      assert( element_index.m_clusterIndex < container->m_relativeClusterCellIndex.size());
      return ++element_index;
   }

};

// Top level proxy representing all cells of all clusters of all modules.
// It provides access to all the modules which provide access to the clusters which provide access to the cells.
template <class T_CellContainer>
class CellContainerProxy : public Utils::ContainerProxy<T_CellContainer,
                                                        CellContainerProxy<T_CellContainer>,
                                                        ModuleProxy<T_CellContainer>,
                                                        RootNodeIndex >  {
   using BASE = Utils::ContainerProxy<T_CellContainer,
                                      CellContainerProxy<T_CellContainer>,
                                      ModuleProxy<T_CellContainer>,
                                      RootNodeIndex >;
   using BASE::BASE;
};

}
#endif
