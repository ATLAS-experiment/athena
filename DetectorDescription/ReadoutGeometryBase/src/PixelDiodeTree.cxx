/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
  */
#include "ReadoutGeometryBase/PixelDiodeTree.h"
#include <sstream>
#include <iomanip>
#include <utility>
#include <iostream>

namespace InDetDD {

unsigned int PixelDiodeTree::cloneSingleSplitsToUnusedHalf() {
   unsigned int errors=0;
   constexpr std::array<IndexType,2> axis_offset{1,2};
   for (std::array<IndexType,4> &a_split : m_subMatrixIndex) {
      unsigned int n_invalid=0;
      IndexType good_idx=s_invalid;
      std::array<std::array<unsigned int,2>,2> n_invalid_halves{};
      for (unsigned int element_i=0; element_i<a_split.size(); ++element_i) {
         bool is_invalid = a_split.at(element_i)==s_invalid;
         unsigned int axis0=element_i/2;
         unsigned int axis1=element_i%2;
         n_invalid_halves[0][axis0]+=is_invalid;
         n_invalid_halves[1][axis1]+=is_invalid;
         if (is_invalid) ++n_invalid;
         else { good_idx=a_split[element_i]; }
      }
      if (n_invalid==3) {
         assert( good_idx != s_invalid);
         for (unsigned int element_i=0; element_i<a_split.size(); ++element_i) {
            a_split.at(element_i)=good_idx;
         }
      }
      else if (n_invalid==2) {
         unsigned int element_i=std::numeric_limits<unsigned int>::max();
         for (unsigned int axis_i=0; axis_i<2; ++axis_i) {
            for (unsigned int side_i=0; side_i<2; ++side_i) {
               if (n_invalid_halves[axis_i][side_i]==2 && n_invalid_halves[axis_i][side_i^1]==0) {
                  element_i=axis_offset[axis_i^1]*side_i;
                  IndexType other_element_i = axis_offset[axis_i^1]*(side_i==1 ? -1 : 1);
                  unsigned int element_end_i=element_i+axis_offset[axis_i]*2;
                  for(; element_i<element_end_i; element_i+=axis_offset[axis_i] ) {
                     a_split[ element_i ] = a_split[ element_i + other_element_i] ;
                  }
                  break;
               }
               if (element_i != std::numeric_limits<unsigned int>::max()) break;
            }
         }
         if (element_i == std::numeric_limits<unsigned int>::max()) {
            errors += 2;
         }
      }
      else if (n_invalid!=0) {
         errors +=2;
      }
   }

   return errors;
}

void PixelDiodeTree::computeMatrixCorner(const std::array<PixelDiodeTree::CellIndexType,2> &matrix_dim) {
   assert( matrix_dim[0]>0 && matrix_dim[1]>0);
   m_matrixDim=matrix_dim;
   PixelDiodeTree::Vector2D tolerance = computeTolerance(matrix_dim);
   {
      DiodeProxyWithPosition lower_corner_proxy(diodeProxyFromIdxCachePosition(std::array<CellIndexType,2> {}));
      m_matrixCorner[0]=lower_corner_proxy.position() - lower_corner_proxy.width()*.5;
      for (unsigned int axis_i=0; axis_i<2; ++axis_i) {
         m_matrixCorner[0][axis_i] += tolerance[axis_i];
      }
   }

   {
      DiodeProxyWithPosition upper_corner_proxy(diodeProxyFromIdxCachePosition(std::array<PixelDiodeTree::CellIndexType,2> {matrix_dim[0]-1,
                                                                                                                            matrix_dim[1]-1}));
      m_matrixCorner[1]=upper_corner_proxy.position() + upper_corner_proxy.width()*.5 ;
      for (unsigned int axis_i=0; axis_i<2; ++axis_i) {
         m_matrixCorner[1][axis_i] -= tolerance[axis_i];
      }
   }
   if (!isInsideMatrix(findFromPos(m_matrixCorner[0])) || !isInsideMatrix(findFromPos(m_matrixCorner[1]))) {
      DiodeProxy upper_corner_proxy_from_idx(diodeProxyFromPos(m_matrixCorner[0]));
      DiodeProxy lower_corner_proxy_from_idx(diodeProxyFromPos(m_matrixCorner[1]));
      constexpr unsigned int n_secs=5;
      for (unsigned int i=20; i-->0; ) {
         std::cout <<  "wait " << (i *n_secs) << " : " << getpid() << std::endl;
         sleep(n_secs);
      }
      throw std::logic_error("Logic error! Matrix corner positions do not yield valid indices for this matrix!");
   }
}

PixelDiodeTree::Vector2D PixelDiodeTree::computeTolerance(const std::array<PixelDiodeTree::CellIndexType,2> &matrix_dim) const {
   PixelDiodeTree::Vector2D tolerance;
   for (unsigned int axis_i=0; axis_i<2; ++axis_i) {
      assert( matrix_dim[axis_i]>0 && std::abs(matrix_dim[axis_i]) < std::numeric_limits<unsigned int>::max());
      unsigned int bits_i=0;
      for (; bits_i<16 && 1u<<bits_i < static_cast<unsigned int>(matrix_dim[axis_i])*3; ++bits_i);
      tolerance[axis_i]=(1u<<bits_i) * std::numeric_limits<PixelDiodeTree::FloatType>::epsilon();
   }
   return tolerance;
}

std::string PixelDiodeTree::debugStringRepr() const {
   std::stringstream out;
   if (!m_diodeParam.m_width.empty()) {
      if (   m_idxSplit.size() != m_posSplit.size()
          || m_idxSplit.size() != m_subMatrixIndex.size()
          || m_idxSplit.size() != m_attribute.size()
          || m_diodeParam.m_width.size() != m_diodeParam.m_invWidth.size()
          || m_diodeParam.m_width.size() != m_diodeParam.m_attribute.size()) {
         // should never happer
         out << "PixelDiodeTree is inconsistent. Expected identical number of elements but has the following "
             << " container sizes:  index-split points : " << m_idxSplit.size()
             << " position split points : " << m_posSplit.size()
             << " submatrices : " << m_subMatrixIndex.size()
             << " attributes : " << m_attribute.size()
             << "; diodes : width : " << m_diodeParam.m_width.size()
             << " inverted width : " << m_diodeParam.m_invWidth.size()
             << " attributes : " << m_diodeParam.m_attribute.size()
             << ".\n";
      }
      else {
         out << "PixelDiodeTree total width : " << m_diodeParam.m_width[0][0] << "x" << m_diodeParam.m_width[0][1] << " mm^2" << "\n:";
         std::vector<std::pair<IndexType,unsigned int> > submatrix_stack;
         submatrix_stack.push_back(std::make_pair(0u,2u));
         while (!submatrix_stack.empty()) {
            auto [submatrix_idx, margin] = submatrix_stack.back();
            submatrix_stack.pop_back();
            out << std::setw(margin) << ' ';
            if (submatrix_idx < 0) {
               if (submatrix_idx!=s_invalid) {
                  unsigned int diode_idx=std::abs(submatrix_idx);
                  if (diode_idx<m_diodeParam.m_width.size()) {
                     out << " diode width " << m_diodeParam.m_width[diode_idx][0] << " x " << m_diodeParam.m_width[diode_idx][1]
                         << " attribute " << std::hex << m_diodeParam.m_attribute[diode_idx] << std::dec << "\n";
                  }
               }
            }
            else if (static_cast<std::size_t>(submatrix_idx) >= m_idxSplit.size()) {
               out << "Invalid sub-matrix index : " << submatrix_idx << "\n";
            }
            else {
               out << " split at (row,col) " << m_idxSplit[submatrix_idx][0] << " " << m_idxSplit[submatrix_idx][1]
                   << " , position (local-x/-y) " << m_posSplit[submatrix_idx][0] <<  " " << m_posSplit[submatrix_idx][1]
                   << " attribute " << std::hex << m_attribute[submatrix_idx] << std::dec
                   << " sub-matrices (diodes): ";
               for (unsigned int split_i=0; split_i<m_subMatrixIndex[submatrix_idx].size(); ++split_i) {
                  out << (split_i==2 ? " | " : " ");
                  if (m_subMatrixIndex[submatrix_idx][split_i] == s_invalid) {
                     out << "-";
                  }
                  else {
                     out << m_subMatrixIndex[submatrix_idx][split_i];
                  }
               }
               out << "\n";
               for (std::array<IndexType,4>::const_reverse_iterator iter = m_subMatrixIndex[submatrix_idx].rbegin();
                    iter != m_subMatrixIndex[submatrix_idx].rend();
                    ++iter) {
                  if (*iter != s_invalid) {
                     submatrix_stack.push_back( std::make_pair( *iter, margin+2));
                  }
               }
            }
         }
      }
   }
   else {
      out << "PixelDiodeTree with " << m_subMatrixIndex.size() << " has no diodes." << "\n";
   }
   return out.str();
}
}
