/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#ifndef ACTSTRK_CELLCONTAINER_H
#define ACTSTRK_CELLCONTAINER_H

#include <array>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <span>
#include <vector>

namespace ActsTrk {
/// @brief Definition of a cell to be used by the in-place clusterization
// @TODO should used Acts version once the InPlaceClusterization is part of Acts
template <typename coordinates_t, std::size_t NDIM, std::unsigned_integral index_t>
struct CellTmpl {
   CellTmpl(const std::array<coordinates_t, NDIM> &the_coordinates, index_t src_index)
      : coordinates(the_coordinates), label(index_t{}), srcIndex(src_index) {}
   std::array<coordinates_t, NDIM>
   coordinates;   ///< the coordinates of the cell on the regular grid
   index_t label;     ///< a label which will be assigned by the clusterization
   index_t srcIndex;  ///< the index to find the source cell
};

/// @brief temporary cell container suitable for the in-place clusterization
template <typename coordinates_t, std::size_t NDIM, std::unsigned_integral index_t>
struct CellContainer
{
public:
   CellContainer(unsigned int n_modules,
                     unsigned int n_cluster_total,
                     unsigned int n_cells_total)
   { reserve(n_modules, n_cluster_total, n_cells_total); }
   /// @brief reserve storage for the cell and cluster data
   /// @param n_modules the total number of modules i.e. independent invokations of the clusterization
   /// @param n_cluster_total the expected number of clusters of all modules (best guess likely, one cluster per cell)
   /// @param n_cells_total the total number of the cells of all modules to be clustered.
   void reserve(unsigned int n_modules,
                unsigned int n_cluster_total,
                unsigned int n_cells_total) {
      m_moduleClusterRange.reserve(n_modules);
      m_relativeClusterCellIndex.reserve(n_cluster_total+n_modules); // will contain per module i n_cluster_i+1 indices
      m_cells.reserve(n_cells_total);
   }

   std::size_t nClustersTotal() const {
      assert(m_relativeClusterCellIndex.size() >= nModules());
      return m_relativeClusterCellIndex.size()-nModules();
   }
   std::size_t nModules() const { return m_moduleClusterRange.size(); }
   std::size_t nCellsTotal() const { return m_cells.size(); }
   std::size_t size() const { return nModules(); }

   using Cell = CellTmpl<coordinates_t, NDIM, index_t>;

   // clusterization not necessarily executed in module order thus need cluster begin and end index.
   // the cells of a module are in one consecutive chunk. To keep the cell indices per cluster compact
   // store the full cell index per module and use relaive indices for cell indices per module.
   struct ClusterRange {
      unsigned int cellBeginIndex;          // index of first cell for this module; has to be >> uint16_t to fit all clusters of all modules
      unsigned int clusterRangeBeginIndex;  // index of first cluster of this module;  has to be >> uint16_t to fit all cells of all modules
      unsigned int clusterRangeEndIndex;    // index of first cluster not of this module; could be number of clusters instead which would fit in uint16_t
      unsigned int idHash;                  // identifier hash of the module this range of clusters is associated to
      unsigned int nClusters() const {
         return static_cast<unsigned int>(clusterRangeEndIndex - clusterRangeBeginIndex );
      }
   };

   struct ModuleRangeGuard {
      ModuleRangeGuard(CellContainer &cell_container, unsigned int id_hash)
      : m_cellContainer(&cell_container),
        m_firstCell(cell_container.m_cells.size()),
        m_clusterBegin(cell_container.m_relativeClusterCellIndex.size()),
        m_idHash(id_hash)
      {
         // The module guard should be constructed after startNewModule was called
         // and before any cluster was added.
         assert( m_clusterBegin>0u);
         assert( cell_container.m_relativeClusterCellIndex[m_clusterBegin-1u]==0u );
      }
      ClusterRange range() const {
         return ClusterRange{.cellBeginIndex = m_firstCell,
                             .clusterRangeBeginIndex = m_clusterBegin,
                             .clusterRangeEndIndex = static_cast<unsigned int>(m_cellContainer->m_relativeClusterCellIndex.size()),
                             .idHash = m_idHash };
      }
      std::span<Cell> moduleCellSpan() {
         assert( m_cellContainer->m_cells.size() >= m_firstCell);
         return std::span<Cell>(m_cellContainer->m_cells.begin()+m_firstCell,
                                m_cellContainer->m_cells.end());
      }
      IdentifierHash identifyHash() const {
         return m_idHash;
      }
   private:
      CellContainer *m_cellContainer;
      unsigned int m_firstCell;
      unsigned int m_clusterBegin;
      unsigned int m_idHash;
   };

   ModuleRangeGuard startNewModule(unsigned int id_hash) {
      // relative cluster cell indices will always start with 0, will contain n_cluster+1 indices, where
      // the last index will be the last relative index of the last cell.
      // below test size+1 because there should also be space for at least the end index
      assert( m_relativeClusterCellIndex.size()+1 < m_relativeClusterCellIndex.capacity());
      m_relativeClusterCellIndex.push_back(index_t{});
      return ModuleRangeGuard(*this, id_hash);
   }
   void emplace_back_cell(const std::array<coordinates_t, NDIM> &the_coordinates, index_t src_index) {
      m_cells.emplace_back(the_coordinates, src_index);
   }
   void registerNewCluster([[maybe_unused]] index_t cell_begin_idx, index_t cell_end_idx) {
      assert(!m_relativeClusterCellIndex.empty()); // startNewModule should have added a zero
      assert( m_relativeClusterCellIndex.back() == cell_begin_idx); // clusters must be added in order
      assert(cell_begin_idx < cell_end_idx); // a cluster must not be empty
      // @TODO for debugging
      assert( m_relativeClusterCellIndex.size() < m_relativeClusterCellIndex.capacity());

      m_relativeClusterCellIndex.push_back(cell_end_idx);
   }
   void registerClustersForNewModule(const ClusterRange &a_range) {
      // @TODO for debugging
      assert( m_moduleClusterRange.size() < m_moduleClusterRange.capacity());

      assert( a_range.clusterRangeEndIndex <= m_relativeClusterCellIndex.size() );
      assert( a_range.clusterRangeEndIndex >0u );
      assert( a_range.cellBeginIndex + m_relativeClusterCellIndex[a_range.clusterRangeEndIndex-1] <= m_cells.size() );

      m_moduleClusterRange.push_back(a_range);
   }
   const ClusterRange &moduleClusterRange(unsigned int module_i) const {
      assert(module_i<m_moduleClusterRange.size());
      return m_moduleClusterRange[module_i];
   }

   std::vector<Cell>  m_cells;                       // data of all cells of all clusters of all modules
   std::vector<index_t> m_relativeClusterCellIndex;  // cell index relative to the first cell index of a module, where two consecutive
                                                     // elements define the range of cells of a cluster.
   std::vector<ClusterRange> m_moduleClusterRange;   // cluster index range per module -> m_clusterCellRange
};
}
#endif
