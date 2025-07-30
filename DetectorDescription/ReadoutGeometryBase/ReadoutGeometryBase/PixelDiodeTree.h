/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
  */
#ifndef INDETDD_PIXELDIODETREE_H
#define INDETDD_PIXELDIODETREE_H

#include "GeoPrimitives/GeoPrimitives.h"

#include <limits>
#include <vector>
#include <array>
#include <cassert>
#include <cmath>
#include <string>
#include <type_traits>

namespace InDetDD {

 namespace {
    // round to the integer which is smaller than or equal to val
    template <typename T_Int,typename T_Float>
    inline T_Int intFloor(T_Float val) {
       bool negative=val<static_cast<T_Float>(0.);
       // negative numbers should become int(val)-1 e.g. -0.1 should become -1 not 0
       return static_cast<T_Int>(val)-negative;
    }
 }

/// Tree structure to find the position, index or pitch of a pixel on a semi-regular grid
/// The grid is considered regular if sub grids resulting from consecutive splits in local-x
/// and local-y direction have identical pitch.
class PixelDiodeTree
{
public:
   using Vector2D = Amg::Vector2D;
   using FloatType = Amg::Vector2D::Scalar;
   using CellIndexType = int;
   using IndexType = int;      // must be signed
   using AttributeType = unsigned int;
   static constexpr IndexType s_invalid = std::numeric_limits<IndexType>::min(); // maximum negative number

   /// Parameters of a diode.
   struct DiodeParam {
      std::vector<Vector2D > m_width;
      std::vector<Vector2D > m_invWidth;
      std::vector<AttributeType>            m_attribute;

      // Add parameters of a new diode to the diode parameter lists.
      unsigned int addDiode(const Vector2D &width, AttributeType attribute=AttributeType{}) {
         assert( m_width.size() == m_invWidth.size());
         assert( m_width.size() == m_attribute.size());
         assert( m_width.size() < std::numeric_limits<unsigned int>::max());
         unsigned int idx = m_width.size();
         m_width.push_back(width);
         m_invWidth.push_back(Vector2D{static_cast<FloatType>(1.)/width[0], static_cast<FloatType>(1.)/width[1]});
         m_attribute.push_back(attribute);
         return idx;
      }
   };

   PixelDiodeTree(const Vector2D &total_width) {
      m_diodeParam.addDiode(total_width); // dummy diode representing the total width of the entire diode matrix
   }

   // add the parameters of a new diode to the diode parameter lists
   unsigned int addDiode(const Vector2D &width, AttributeType attribute) {
      assert( m_diodeParam.m_width.size()>0);
      return m_diodeParam.addDiode(width, attribute);
   }

   // set the diode type of all diodes in a certain split region of a sub-matrix
   // @param sub_matrix_idx the index of the sub-matrix
   // @param split_i index of the area of the split sub-matrix
   // @param diode_idx the index to get the diode parameters from the list of diode parameters
   // The split regions are numbered in the following way 2 | 3  ^  local-y (eta,column) direction
   //                                                     -----  |
   //                                                     0 | 1  |
   //                                                     --> local-x (phi, rows) direction
   void setDiodeForSubMatrix( unsigned int sub_matrix_idx, unsigned int split_i, unsigned int diode_idx) {
      assert( diode_idx > 0 && diode_idx < std::numeric_limits<IndexType>::max());
      assert( sub_matrix_idx < m_subMatrixIndex.size());
      assert( split_i <m_subMatrixIndex[sub_matrix_idx].size() );
      m_subMatrixIndex.at(sub_matrix_idx)[split_i]=-static_cast<IndexType>(diode_idx);
   }

   // Split a (sub-) matrix into 4 rectangular (sub-)sub-matrices.
   // @param idx_split the absolute index (row,column) at the center of split (lower edge of the upper-most sub-matrix)
   // @param pos_split the absolute position at indicating the center position of the split
   // @param parent_idx if this matrix is a sub-matrix, the index of the parent otherwise an invalid index.
   // @param split_i if this matrix is a sub-matrix this index indicates which of the split areas (0..3) this matrix represents.
   // @return the index of this matrix to provide access to the parameters of the split.
   unsigned int split( const std::array<CellIndexType, 2> &idx_split,
                       const Vector2D &pos_split,
                       AttributeType an_attribute,
                       unsigned int parent_idx=std::numeric_limits<unsigned int>::max(),
                       unsigned int split_i=0) {
      assert( m_idxSplit.size() == m_posSplit.size());
      assert( m_idxSplit.size() == m_subMatrixIndex.size());
      assert( m_idxSplit.size() == m_attribute.size());
      unsigned int this_submatrix_idx = m_idxSplit.size();
      m_idxSplit.push_back(idx_split);
      m_posSplit.push_back(pos_split);
      assert( PixelDiodeTree::s_invalid < 0);
      assert( -static_cast<IndexType>(m_diodeParam.m_width.size()) > PixelDiodeTree::s_invalid);
      m_subMatrixIndex.push_back(std::array<PixelDiodeTree::IndexType,4>{PixelDiodeTree::s_invalid, PixelDiodeTree::s_invalid,
                                                                         PixelDiodeTree::s_invalid, PixelDiodeTree::s_invalid});
      m_attribute.push_back(an_attribute);
      if (parent_idx != std::numeric_limits<unsigned int>::max()) {
         assert( split_i < m_subMatrixIndex[parent_idx].size());
         m_subMatrixIndex.at(parent_idx)[split_i]=this_submatrix_idx;
      }

      return this_submatrix_idx;
   }

   /// helper template to find the final node in a quad tree.
   template <typename T>
   std::tuple<IndexType,IndexType, IndexType>
   findFromT(const T &val,
             const std::vector< T > &split) const {
      IndexType sub_matrix_idx=0;
      IndexType last_sub_matrix_idx=0;
      unsigned int submatrix_i=0;
      while (sub_matrix_idx>=0) {
         assert( static_cast<std::size_t>(sub_matrix_idx) < split.size() );
         last_sub_matrix_idx=sub_matrix_idx;
         submatrix_i = (val[0] >= split[sub_matrix_idx][0]) + (val[1] >= split[sub_matrix_idx][1])*2;
         sub_matrix_idx=m_subMatrixIndex[sub_matrix_idx][submatrix_i];
      }
      assert( sub_matrix_idx != s_invalid);
      assert( std::abs(sub_matrix_idx) < m_diodeParam.m_width.size() );
      return std::make_tuple(last_sub_matrix_idx, std::abs(sub_matrix_idx), submatrix_i);
   }

   /// Compute the position of a certain diode in a certain sub-matrix
   /// @param sub_matrix_idx the index of the sub-matrix whose "split-areas" refer to this diode
   /// @param diode_idx the index of the diode which gives access to the parameters of the this diode.
   /// @param idx the 2D index of the diode (row,column)
   /// @return the position of the diode relative to the center of the full matrix (not sub-matrix)
   Vector2D computePosition(PixelDiodeTree::IndexType sub_matrix_idx,
                            PixelDiodeTree::IndexType diode_idx,
                            const std::array<CellIndexType,2> &idx) const {
      // pos = (idx - split-index) + 0.5) * pitch + split-position
      Vector2D pos;
      assert( diode_idx>0);
      assert( static_cast<unsigned int>(diode_idx) < m_diodeParam.m_width.size() ) ;
      assert( static_cast<unsigned int>(sub_matrix_idx) < m_idxSplit.size() );
      for (unsigned int axis_i=0; axis_i<2; ++axis_i) {
         assert( axis_i < m_diodeParam.m_width[diode_idx].size() );
         pos[axis_i] = (    m_posSplit[sub_matrix_idx][axis_i]
                        + ((idx[axis_i]-m_idxSplit[sub_matrix_idx][axis_i])
                        + static_cast<FloatType>(.5)) * m_diodeParam.m_width[diode_idx][axis_i]);
      };
      return pos;
   }

   /// Compute the 2D index (row, column) of a certain diode in a certain sub-matrix.
   /// @param sub_matrix_idx the index of the sub-matrix whose "split-areas" refer to this diode
   /// @param diode_idx the index of the diode which gives access to the parameters of the this diode.
   /// @param pos the position of the diode relative to the center of the full matrix (not sub-matrix).
   /// @return the 2D index of the diode (row,column)
   std::array<CellIndexType,2> computeIndex(PixelDiodeTree::IndexType sub_matrix_idx,
                                            PixelDiodeTree::IndexType diode_idx,
                                            const Vector2D &pos) const {
      // pos = (idx - split-index) + 0.5) * pitch + split-position
      // idx = inr( (pos - split-position)/pitch - 0.5 + split-index + 0.5 (rounding) )
      std::array<CellIndexType,2> idx;
      assert( diode_idx>0);
      assert( static_cast<unsigned int>(diode_idx) < m_diodeParam.m_invWidth.size() ) ;
      assert( static_cast<unsigned int>(sub_matrix_idx) < m_idxSplit.size() );
      assert( static_cast<unsigned int>(sub_matrix_idx) < m_posSplit.size() );
      for (unsigned int axis_i=0; axis_i<2; ++axis_i) {
         assert( axis_i < m_diodeParam.m_invWidth[diode_idx].size() );

         idx[axis_i] = intFloor<CellIndexType>(    m_idxSplit[sub_matrix_idx][axis_i]
                                               +  (pos[axis_i]-m_posSplit[sub_matrix_idx][axis_i])
                                                   * m_diodeParam.m_invWidth[diode_idx][axis_i]);
      };
      return idx;
   }

   /// Helper class to access parameters of a diode
   struct DiodeProxy {
      friend class PixelDiodeTree;
   protected:
      const PixelDiodeTree *m_diodeTree = nullptr;
      PixelDiodeTree::IndexType m_subMatrixIdx{};
      PixelDiodeTree::IndexType m_diodeIdx{};
      DiodeProxy() { }
      DiodeProxy(const PixelDiodeTree *diodeTree, PixelDiodeTree::IndexType subMatrixIdx, PixelDiodeTree::IndexType diodeIdx)
         : m_diodeTree(diodeTree), m_subMatrixIdx(subMatrixIdx), m_diodeIdx(diodeIdx) {}
      void setInvalid() { m_diodeTree=nullptr; }
   public:
      /// get the width stored for this diode.
      const PixelDiodeTree::Vector2D &width() const {
         assert( static_cast<unsigned int>(m_diodeIdx)<m_diodeTree->m_diodeParam.m_width.size());
         return m_diodeTree->m_diodeParam.m_width[m_diodeIdx];
      }
      /// get the inverse of the width of this diode.
      const PixelDiodeTree::Vector2D &invWidth() const {
         assert( static_cast<unsigned int>(m_diodeIdx)<m_diodeTree->m_diodeParam.m_invWidth.size());
         return m_diodeTree->m_diodeParam.m_invWidth[m_diodeIdx];
      }
      /// get the attribute associated to this diode (to be interpreted)
      unsigned int diodeAttribute() const {
         assert( static_cast<unsigned int>(m_diodeIdx)<m_diodeTree->m_diodeParam.m_attribute.size());
         return m_diodeTree->m_diodeParam.m_attribute[m_diodeIdx];
      }
      /// get the attribute associated to the sub-matrix of this diode (to be interpreted)
      unsigned int subMatrixAttribute() const {
         assert( static_cast<unsigned int>(m_subMatrixIdx)<m_diodeTree->m_attribute.size());
         return m_diodeTree->m_attribute[m_subMatrixIdx];
      }
      /// Compute the position of the diode.
      /// The origin is typically at the center of the pixel matrix.
      PixelDiodeTree::Vector2D computePosition(const std::array<CellIndexType,2> &idx) const {
         return m_diodeTree->computePosition(m_subMatrixIdx, m_diodeIdx, idx);
      }
      /// Compute the full 2D index (row, column) of the diode in the full diode matrix.
      std::array<PixelDiodeTree::CellIndexType,2> computeIndex(const Vector2D &pos) const {
         return m_diodeTree->computeIndex(m_subMatrixIdx, m_diodeIdx, pos);
      }
      /// return true if this proxy refers to a valide diode
      bool isValid() const { return m_diodeTree != nullptr; }
      /// return true if this proxy refers to a valide diode
      operator bool() const { return isValid(); }
   };

   /// A diode proxy which caches the position of a diode
   struct DiodeProxyWithPosition : public DiodeProxy {
      friend class PixelDiodeTree;
      DiodeProxyWithPosition() {}
      DiodeProxyWithPosition(const PixelDiodeTree *diodeTree,
                              PixelDiodeTree::IndexType subMatrixIdx,
                              PixelDiodeTree::IndexType diodeIdx,
                              Vector2D &&position)
          : DiodeProxy{diodeTree, subMatrixIdx, diodeIdx},
            m_position(std::move(position))
      {}
      /// get the cached position of this diode
      const Vector2D &position() const { return m_position; }
      /// for backward compatibility, return the pitch of a diode in local-x(phi, row) direction
      double phiWidth() const { return width()[0]; }
      /// for backward compatibility, return the pitch of a diode in local-y(eta, column) direction
      double etaWidth() const { return width()[1]; }
      /// for backward compatibility, return the position of the lower edge of the diode in local-x(phi, row) direction.
      /// The center postion is cached.
      double xPhiMin() const { return m_position[0]-width()[0]*.5; }
      /// for backward compatibility, return the position of the lower edge of the diode in local-y(phi, row) direction.
      /// The center postion is cached.
      double xPhiMax() const { return m_position[0]+width()[0]*.5; }
      /// for backward compatibility, return the position of the lower edge of the diode in local-y(eta, column) direction.
      /// The center postion is cached.
      double xEtaMin() const { return m_position[1]-width()[1]*.5; }
      /// for backward compatibility, return the position of the upper edge of the diode in local-y(eta, column) direction.
      /// The center postion is cached.
      double xEtaMax() const { return m_position[1]+width()[1]*.5; }

   protected:
      Vector2D m_position{};
   };

   /// find a diode by its 2D index (row, column) and compute the position relative to the center of the full diode matrix
   Vector2D findFromIdx(const std::array<CellIndexType,2> &idx) const {
      auto [sub_matrix_idx,diode_idx, submatrix_i] = findFromT(idx,m_idxSplit);
      return computePosition(sub_matrix_idx, diode_idx,idx);
   }
   /// find a diode by its 2D index (row, column)
   /// the returned proxy allows to compute the position and provides access to the parameters of the diode
   DiodeProxy diodeProxyFromIdx(const std::array<CellIndexType,2> &idx) const {
      auto [sub_matrix_idx,diode_idx, submatrix_i] = findFromT(idx,m_idxSplit);
      return DiodeProxy{this, sub_matrix_idx, diode_idx};
   }
   /// find a diode by its 2D index (row, column) and compute the position of the diode
   /// the returned proxy caches the diode position and provides access to its parameters.
   DiodeProxyWithPosition diodeProxyFromIdxCachePosition(const std::array<CellIndexType,2> &idx) const {
      auto [sub_matrix_idx,diode_idx, submatrix_i] = findFromT(idx,m_idxSplit);
      DiodeProxyWithPosition proxy(this, sub_matrix_idx, diode_idx, computePosition(sub_matrix_idx, diode_idx, idx));
      return proxy;
   }
   /// Find a diode by the position relative to the center of the full diode matrix and compute its 2D index (row, column)
   std::array<CellIndexType,2> findFromPos(const Vector2D &pos) const {
      auto [sub_matrix_idx,diode_idx, submatrix_i] = findFromT(pos,m_posSplit);
      return computeIndex(sub_matrix_idx,diode_idx,pos);
   }
   /// Find a diode by the position relative to the center of the full diode matrix.
   /// The returned proxy allows to compute the 2D index (row, column) and provides access to the parameters of the diode
   DiodeProxy diodeProxyFromPos(const Vector2D &pos) const {
      auto [sub_matrix_idx,diode_idx, submatrix_i] = findFromT(pos,m_posSplit);
      return DiodeProxy{this, sub_matrix_idx, diode_idx};
   }

   /// Return true if no sub-matrices are defined, indicates an invalid state.
   bool empty() const { return m_subMatrixIndex.empty(); }

   /// Return the total width of the diode matrix.
   const Vector2D &totalWidth() const {
      // constructor should store total width here, empty means that this diode tree is not properly constructed.
      assert(!m_diodeParam.m_width.empty());
      return m_diodeParam.m_width[0];
   }

   /// Dump the diode tree structure into a string.
   std::string debugStringRepr() const;

   /// Test whether the cell_index either row or column index could be valid
   /// The index may still be outside the matrix.
   template <typename T>
   static constexpr bool validCellIndex(T cell_index) {
      if constexpr( std::is_signed_v<T>) {
         return cell_index>= std::numeric_limits<CellIndexType>::min() && cell_index < std::numeric_limits<CellIndexType>::max();
      }
      else {
         return cell_index < std::numeric_limits<CellIndexType>::max();
      }
   }

   /// Create a 2D cell index from the indices in local-x (phi, row) and local-y (eta, column) direction
   template <typename T>
   static constexpr std::array<PixelDiodeTree::CellIndexType,2> makeCellIndex(T local_x_idx, T local_y_idx) {
      assert( validCellIndex(local_x_idx) && validCellIndex(local_y_idx) );
      return std::array<PixelDiodeTree::CellIndexType,2>{
         static_cast<PixelDiodeTree::CellIndexType>(local_x_idx),
         static_cast<PixelDiodeTree::CellIndexType>(local_y_idx)};
   }

   /// Get the attribute associated to a sub-matrix
   AttributeType attribute(IndexType idx) const {
      return m_attribute.at(idx);
   }
   /// Set the attribute associated to a sub-matrix
   /// @param idx the index of the sub-matrix
   /// @param new_attribute the new attribute for the sub-matrix
   void setAttribute(IndexType idx, AttributeType new_attribute) {
      m_attribute.at(idx)=new_attribute;
   }

   /// Clone half with valid split indices to "unused" half with invalid split indices.
   ///
   /// For splits which only split one of the two axes, half of the sub-matrix indices
   /// will be invalid, to ensure that the position and index computation can be carried
   /// out also for indices or positions which are not well within the bounds of the matrix,
   /// the invalid indices of that half will be overwritten with indices of the half for
   /// which the indices are set.
   unsigned int cloneSingleSplitsToUnusedHalf();

   /// Compute tolerance to ensure that a position is within the expected cell
   /// Due to limited floating point precision, positions may only yield the expected pixel
   /// index if they are not closer to the edge of the matrix or cell than this tolerance.
   PixelDiodeTree::Vector2D computeTolerance(const std::array<PixelDiodeTree::CellIndexType,2> &matrix_dim) const;

   /// Compute the effective maximum lower and upper corner positions of the matrix.
   /// The matrix will be shrunk to ensure that positions within the corners will yield valid indices.
   /// Note: needs to be called after the tree has been constructed, otherwise the isInsideMatrix calls
   /// will be undefined.
   void computeMatrixCorner(const std::array<PixelDiodeTree::CellIndexType,2> &matrix_dim);

   /// Return true if the given index describes a valid location inside the matrix.
   bool isInsideMatrix(const std::array<PixelDiodeTree::IndexType,2> &idx) const {
      return idx[0]>=0 && idx[1]>=0 && idx[0] < m_matrixDim[0] && idx[1] < m_matrixDim[1];
   }

   /// Test whether the given local position is well within the matrix.
   bool isInsideMatrix(const Amg::Vector2D &local_position) const {
      // corner positions are moved to the inside of the matrix to account for floating point
      // tolerances so >= && <= means "well" inside the matrix.
      return (   local_position[0]>=m_matrixCorner[0][0] && local_position[0]<=m_matrixCorner[1][0]
              && local_position[1]>=m_matrixCorner[0][1] && local_position[1]<=m_matrixCorner[1][1]);
   }

   /// Get indices of all adjacent cells.
   template <typename T_CellID>
   static void neighboursOfCell(const std::array<CellIndexType,2> &idx,
                                const std::array<CellIndexType,2> &max_idx,
                                std::vector<T_CellID > &neighbours);

protected:
   std::vector<std::array<CellIndexType, 2>>     m_idxSplit;
   std::vector<Vector2D >                        m_posSplit;
   std::vector<AttributeType>                    m_attribute;

   std::vector<std::array<IndexType, 4> >        m_subMatrixIndex;
   DiodeParam                                    m_diodeParam;
   std::array<Vector2D,2>                        m_matrixCorner{};
   std::array<CellIndexType,2>                   m_matrixDim{};
};

template <typename T_CellID>
void PixelDiodeTree::neighboursOfCell(const std::array<CellIndexType,2> &idx,
                                      const std::array<CellIndexType,2> &max_idx,
                                      std::vector<T_CellID > &neighbours) {
   neighbours.reserve(8);
   std::array<std::array<PixelDiodeTree::CellIndexType,2>,2> outer_idx {
      std::array<PixelDiodeTree::CellIndexType,2>{idx[0]-1,idx[0]+1},
      std::array<PixelDiodeTree::CellIndexType,2>{idx[1]-1,idx[1]+1}
   };

   if (outer_idx[0][0]>=0          &&  outer_idx[1][0]>=0)         neighbours.emplace_back(outer_idx[0][0],outer_idx[1][0]);
   if (                                outer_idx[1][0]>=0)         neighbours.emplace_back(idx[0],         outer_idx[1][0]);
   if (outer_idx[0][1]<max_idx[0]  &&  outer_idx[1][0]>=0)         neighbours.emplace_back(outer_idx[0][1],outer_idx[1][0]);
   if (outer_idx[0][1]<max_idx[0])                                 neighbours.emplace_back(outer_idx[0][1],idx[1]);
   if (outer_idx[0][1]<max_idx[0]  &&  outer_idx[1][1]<max_idx[1]) neighbours.emplace_back(outer_idx[0][1],outer_idx[1][1]);
   if (                                outer_idx[1][1]<max_idx[1]) neighbours.emplace_back(idx[0],         outer_idx[1][1]);
   if (outer_idx[0][0]>=0          &&  outer_idx[1][1]<max_idx[1]) neighbours.emplace_back(outer_idx[0][0],outer_idx[1][1]);
   if (outer_idx[0][0]>=0                                        ) neighbours.emplace_back(outer_idx[0][0],idx[1]);
}

}

#endif
