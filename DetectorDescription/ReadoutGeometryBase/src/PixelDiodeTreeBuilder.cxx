/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
  */
#include "ReadoutGeometryBase/PixelDiodeTreeBuilder.h"
#include <stdexcept>
#include <iomanip>
#include <iostream>
#include <unordered_map>

namespace InDetDD {

namespace {
   enum ELocation {
      kOuter=0,
      kInner=1
   };

   struct SubMatrixData {
      enum EEdgeLocation {kXmin, kXmax, kYmin, kYmax, kNEdgeLocations};
      enum EEdgeType {kOuter,kInner,kInternal, kDeadZoneOuter, kDeadZoneInner};
      static double invOrZero(double a) { return a>0 ? 1./a : 0.; }
      static constexpr PixelDiodeTree::AttributeType s_defaultMatrixAttribute=0u;

      SubMatrixData(const PixelDiodeTree::Vector2D &pos,
                    const PixelDiodeTree::Vector2D &width,
                    const std::array<unsigned int,2> &idx,
                    const std::array<unsigned int,2> &dim,
                    const std::array<unsigned int,2> &chip_dim,
                    const std::array<SubMatrixData::EEdgeType, SubMatrixData::kNEdgeLocations> &edge_type,
                    PixelDiodeTree::AttributeType attribute,
                    unsigned int submatrix_idx,
                    unsigned int split_idx) :
         m_pos(pos),
         m_width(width),
         m_idx(idx),
         m_dim(dim),
         m_chipDim(chip_dim),
         m_edgeType(edge_type),
         m_attribute(attribute),
         m_subMatrixIdx(submatrix_idx),
         m_splitIdx(split_idx)
      { assert(m_splitIdx<4); }
      SubMatrixData(const PixelDiodeTree::Vector2D &width,
                    const std::array<unsigned int,2> &dim,
                    const std::array<unsigned int,2> &chip_dim)
         : SubMatrixData(-width*.5,width,
                         std::array<unsigned int,2>{0u,0u},dim,chip_dim,
                         std::array<SubMatrixData::EEdgeType, SubMatrixData::kNEdgeLocations>{kOuter,kOuter,kOuter,kOuter},
                         s_defaultMatrixAttribute,
                         std::numeric_limits<unsigned int>::max() /* no parent matrix*/,
                         0u /*split area index */ )
      {}

      PixelDiodeTree::Vector2D m_pos;
      PixelDiodeTree::Vector2D m_width;
      std::array<unsigned int,2> m_idx;
      std::array<unsigned int,2> m_dim;
      std::array<unsigned int,2> m_chipDim;
      std::array<EEdgeType,kNEdgeLocations> m_edgeType;
      PixelDiodeTree::AttributeType m_attribute;
      unsigned int m_subMatrixIdx;
      unsigned int m_splitIdx;
   };

   // helper function to split a matrix in 4 sub-matrices where a sub-matrix may be empty.
   void splitMatrix(const SubMatrixData &matrix_data,                                  // matrix data of the matrix to be split
                    std::array<unsigned int,2> split_idx,                              // the absolute pixel index (row, column) where the matrix is to be split
                    const std::array<PixelDiodeTree::Vector2D,2>  &width,              // physical width of the 4 sub-matrices
                    const std::array<std::array<unsigned int, 2>,2>  &split_chip_dim,  // number of circuits per sub-matrix
                    const std::array<bool,2> &dead_zone_split,                         // flag per axis whether the split splits off a dead zone from an inner or outer edge
                    const std::array<std::array<PixelDiodeTree::AttributeType, 2>,2>  &attribute,
                                                                                       // a special attribute to be associated to the sub-matrix (currently unused)
                    std::vector<SubMatrixData> &sub_matrix_list,                       // the new sub-matrices will be added to this list
                    PixelDiodeTree &diode_tree,                                        // the diode tree which to be developed further
                    std::ostream *debug_out,                                           // pointer to an output stream or nullptr
                    const std::array<std::string,SubMatrixData::kDeadZoneInner+1> *edgeName)  // pointer to edge names used in the debug output
   {
      std::array<std::array<unsigned int,2>,2> new_dim;
      for (unsigned int axis_i=0; axis_i<2; ++axis_i) {
         if (split_idx[axis_i] < matrix_data.m_idx[axis_i]+matrix_data.m_dim[axis_i]) {
            // if the split is not on the outer edge, compute the dimensions in the corresponding direction
            // for the new areas
            new_dim[0][axis_i]=split_idx[axis_i]-matrix_data.m_idx[axis_i];
            new_dim[1][axis_i]=matrix_data.m_idx[axis_i] + matrix_data.m_dim[axis_i] - split_idx[axis_i];
         }
         else {
            // if the split is on the outer edge. the first area inherits the entire width and the second
            // is empty.
            new_dim[0][axis_i]=matrix_data.m_dim[axis_i];
            new_dim[1][axis_i]=0;
         }
      }

      if (debug_out) {
         (*debug_out) << "Split (sub) matrix "
                      << matrix_data.m_idx[0] << " ,  " << matrix_data.m_idx[1] << " "
                      << matrix_data.m_dim[0] << "x" << matrix_data.m_dim[1] << " : "
                      << split_idx[0] << "  <  " << matrix_data.m_dim[0]+matrix_data.m_idx[0]
                      << " , " << split_idx[1] << "  <  " << matrix_data.m_dim[1]+matrix_data.m_idx[1]
                      << " -> " << (*edgeName)[matrix_data.m_edgeType[ 0 + 2*0]] <<  " | "
                      << new_dim[0][0] << " | " << new_dim[1][0] << " | " << (*edgeName)[matrix_data.m_edgeType[ 1 + 2*0]]
                      << " , " << (*edgeName)[matrix_data.m_edgeType[ 0 + 2*1]] <<  " | " << new_dim[0][1]
                      << " | " << new_dim[1][1] << " | " << (*edgeName)[matrix_data.m_edgeType[ 1 + 2*1]]
                      << std::endl;
      }

      unsigned int sub_matrix_idx = std::numeric_limits<unsigned int>::max();
      std::array<unsigned int,2> axis_split_i;
      // the matrix is split into sub-matrices which may have zero size
      // each axis is split into two regions side 0 is the "lower" and 1 the "upper side"
      for (axis_split_i[0]=0; axis_split_i[0]<2; ++axis_split_i[0]) {
         if (new_dim[axis_split_i[0]][0]==0) { continue; } //  index order: [side][axis]
         for (axis_split_i[1]=0; axis_split_i[1]<2; ++axis_split_i[1]) {
            if (new_dim[axis_split_i[1]][1]==0) continue;   // index order [side][axis]
            PixelDiodeTree::Vector2D pos(matrix_data.m_pos);  // width[0] is the width of the first sub-matrix
            std::array<unsigned int,2> idx(matrix_data.m_idx);
            std::array<SubMatrixData::EEdgeType, SubMatrixData::kNEdgeLocations> edge_type;
            for (unsigned int axis_i=0; axis_i<2; ++axis_i) {
               if (axis_split_i[axis_i]>0) {
                  pos[axis_i] += width[0][axis_i];
                  idx[axis_i] = split_idx[axis_i];
               }
               edge_type[axis_split_i[axis_i] + 2*axis_i] = matrix_data.m_edgeType[ axis_split_i[axis_i] + 2*axis_i];
               unsigned int other_side_i=axis_split_i[axis_i]^1;
               // the types of the outer edges of the sub-matrix are inherited from the matrix
               // the inner edges become "inner" for circuit splits, or "internal" for inner or outer edge splits.
               edge_type[other_side_i + 2*axis_i] = (new_dim[other_side_i][axis_i]>0
                                                     ? (split_chip_dim[axis_split_i[axis_i]][axis_i]>0
                                                        ?  SubMatrixData::kInner
                                                        :  SubMatrixData::kInternal )
                                                     : matrix_data.m_edgeType[ other_side_i + 2*axis_i] ); // if the other side has zero width inherit edge type

               // special handling if a "dead zone" is split off an inner or outer edge.
               if (dead_zone_split[axis_i]) {
                  for (unsigned int side_i=0; side_i<2; ++side_i) {
                     if (edge_type[side_i+2*axis_i]==SubMatrixData::kInternal) {
                        unsigned int other_side_i=(side_i^1);
                        edge_type[side_i+2*axis_i]=(  (   matrix_data.m_edgeType[side_i+2*axis_i] == SubMatrixData::kInner
                                                          || matrix_data.m_edgeType[other_side_i+2*axis_i] == SubMatrixData::kInner)
                                                      ?  SubMatrixData::kDeadZoneInner
                                                      : ((   matrix_data.m_edgeType[side_i+2*axis_i] == SubMatrixData::kOuter
                                                             || matrix_data.m_edgeType[other_side_i+2*axis_i] == SubMatrixData::kOuter)
                                                         ? SubMatrixData::kDeadZoneOuter
                                                         : SubMatrixData::kInternal));
                        if (edge_type[side_i+2*axis_i]==SubMatrixData::kInternal) {
                           throw std::logic_error("Deadzone not next to an inner or outer edge." );
                        }
                     }
                  }
               }
            }

            if (sub_matrix_idx == std::numeric_limits<unsigned int >::max()) {
               // register the split in the diode tree
               // the sub_matrix_idx is needed to set sub-matrix or diode indices for the 4 new sub-matrices
               sub_matrix_idx = diode_tree.split( std::array<PixelDiodeTree::CellIndexType,2>{ static_cast< PixelDiodeTree::CellIndexType>(split_idx[0]),
                                                                                               static_cast< PixelDiodeTree::CellIndexType>(split_idx[1]) },
                                                  matrix_data.m_pos+width[0],
                                                  matrix_data.m_attribute,
                                                  matrix_data.m_subMatrixIdx,
                                                  matrix_data.m_splitIdx);
            }
            unsigned int split_i= axis_split_i[0]+axis_split_i[1]*2;
            if (debug_out) {
               (*debug_out) << "new sub matrix : "
                            << sub_matrix_idx << " . " << split_i << " axis sides "
                            << axis_split_i[0] << " ,  " << axis_split_i[1] << " : "
                            << std::setw(4) << idx[0] << ", " << std::setw(4) << idx[1]
                            << " dim " << std::setw(4) << new_dim[axis_split_i[0]][0] << " , " << std::setw(4) <<  new_dim[axis_split_i[1]][1]
                            << "  " << std::setw(6) << pos[0] << ", " << std::setw(6) << pos[1]
                            << " w " << std::setw(6) << width[axis_split_i[0]][0] << " , "  << std::setw(6) << width[axis_split_i[1]][1]
                            << " chips " << split_chip_dim[axis_split_i[0]][0] << " , " << split_chip_dim[axis_split_i[1]][1]
                            << " edges " << (*edgeName)[edge_type[0]] << "|" << (*edgeName)[edge_type[1]] << " , "
                            << (*edgeName)[edge_type[2]] << "|" << (*edgeName)[edge_type[3]]
                            << std::endl;
            }

            // @TODO turn into assert?
            if (new_dim[axis_split_i[0]][0] > matrix_data.m_dim[0] && new_dim[axis_split_i[1]][1] > matrix_data.m_dim[1]) {
               throw std::logic_error("splitter increased size.");
            }
            // store data for all non empty sub-matrices for further splitting or to create or assign diodes
            sub_matrix_list.emplace_back(pos, PixelDiodeTree::Vector2D{width[axis_split_i[0]][0],
                                                                       width[axis_split_i[1]][1]},
                                         idx, std::array<unsigned int,2>{new_dim[axis_split_i[0]][0],new_dim[axis_split_i[1]][1]},
                                         std::array<unsigned int, 2>{split_chip_dim[axis_split_i[0]][0],split_chip_dim[axis_split_i[1]][1]},
                                         edge_type,
                                         attribute[axis_split_i[0]][axis_split_i[1]],
                                         sub_matrix_idx,
                                         split_i);
         }
      }
   }

}

namespace {
   // helper class to store diode index and for debug build also the attribute
   // the attribute is used to verify that computed diode attributes agree
   // for all "diodes" which are mapped to the same diode parameter set.
   // The functional which is used to compute the diode attributes has to
   // ensure this.
   struct DiodeInfo {
      DiodeInfo(unsigned int idx, [[maybe_unused]] const PixelDiodeTree::AttributeType &attribute)
         : m_idx(idx)
#ifndef NDEBUG
          ,m_attribute(attribute)
#endif
      {}
      void setIndex(unsigned int idx) { m_idx=idx; }
      unsigned int diodeIndex() const { return m_idx; }
#ifndef NDEBUG
      bool attributeAgrees(const PixelDiodeTree::AttributeType &attribute) {
         return attribute == m_attribute;
      }
#endif
   private:
      unsigned int m_idx;
#ifndef NDEBUG
      PixelDiodeTree::AttributeType m_attribute;
#endif
   };
}

PixelDiodeTree createPixelDiodeTree(const std::array<unsigned int,2> &chip_dim,
                                    const std::array<unsigned int,2> &chip_matrix_dim,
                                    const PixelDiodeTree::Vector2D &pitch,
                                    const std::array<std::array<unsigned int,2>,2> &edge_dim,
                                    const std::array<PixelDiodeTree::Vector2D,2> &edge_pitch,
                                    const std::array<std::array<unsigned int,2>,2> &dead_zone,
                                    const AttributeRefiner &func_compute_attribute,
                                    std::ostream *debug_out)   {
   // strategy :
   //  split the entire pixel matrix first by circuit, then by special pixel areas
   //  add new diodes for sub matrices only composed of a single diode type, where the
   //  diode type is defined by the area i.e. it is located on one of the outer edges of the full matrix,
   //  one of the inner edges i.e. areas "between" circuits, in the "dead" zone between circuits
   //  or on the regular matrix.
   //
   //  1) first split circuit arrays in roughly by half until all sub-matrices span only a single circuit
   //  2) split sub-matrices until the sub matrix only contains pixel of an outer edge, inner edge
   //     or the "normal" pixels. Inner-edge sub-matrices are split further if there is a dead zone.
   //  @TODO split by circuit and edge type independently per axis, to reduce number of unused splits
   //        original pixel matrix (Run 1-Run 3)

   std::array<unsigned int,2> dim{};
   PixelDiodeTree::Vector2D width(PixelDiodeTree::Vector2D::Zero());
   // compute total width of diode matrix
   for (unsigned int axis_i=0; axis_i<2; ++axis_i) {
      dim[axis_i] = chip_dim[axis_i] * chip_matrix_dim[axis_i];
      unsigned int n_inner_edges = chip_dim[axis_i]*2-2 ;
      constexpr unsigned int n_outer_edges = 2;
      width[axis_i] = (  (  chip_dim[axis_i]*chip_matrix_dim[axis_i]
                          - edge_dim[kInner][axis_i]*n_inner_edges
                          - edge_dim[kOuter][axis_i]*n_outer_edges   ) * pitch[axis_i]
                       + edge_dim[kInner][axis_i]*edge_pitch[kInner][axis_i]*n_inner_edges
                       + edge_dim[kOuter][axis_i]*edge_pitch[kOuter][axis_i]*n_outer_edges);
   }
   PixelDiodeTree diode_tree(width);
   std::unordered_map<unsigned int, DiodeInfo > diode_idx;

   std::array<std::string,SubMatrixData::kDeadZoneInner+1> edgeName {
      std::string("Outer"),
      std::string("Inner"),
      std::string("Internal"),
      std::string("DeadZoneOuter"),
      std::string("DeadZoneInner")
   };


   std::vector< SubMatrixData > stack;
   stack.emplace_back(width,dim, chip_dim);
   while (!stack.empty()) {
      SubMatrixData current_submatrix = std::move(stack.back());
      const std::array<unsigned int,2 > &split_chip_dim = current_submatrix.m_chipDim;
      stack.pop_back();

      std::array<PixelDiodeTree::Vector2D,2> split_width{current_submatrix.m_width,
                                                         PixelDiodeTree::Vector2D::Zero()};         // default for no splitting
      std::array<unsigned int,2> split_idx{current_submatrix.m_idx[0]+current_submatrix.m_dim[0],   //set split point to outside outermost cell
                                           current_submatrix.m_idx[1]+current_submatrix.m_dim[1]};
      std::array<std::array<unsigned int,2>,2 > new_chip_dim{ split_chip_dim, split_chip_dim};
      unsigned int n_sub_matrices=0;

      if (debug_out) {
         *debug_out << "Split : " << std::setw(4) << current_submatrix.m_idx[0] << ", " << std::setw(4) << current_submatrix.m_idx[1]
                    << " " << std::setw(4) << current_submatrix.m_dim[0] << " x " << std::setw(4) << current_submatrix.m_dim[1]
                    << " pos " << std::setw(6) << current_submatrix.m_pos[0] << " , " <<  std::setw(6) << current_submatrix.m_pos[1]
                    << "  w " << std::setw(6) << current_submatrix.m_width[0] << " , " <<  std::setw(6) << current_submatrix.m_width[1]
                    << " chips " << current_submatrix.m_chipDim[0] << " x " << current_submatrix.m_chipDim[1]
                    << " edges: " << edgeName[current_submatrix.m_edgeType[0] ] <<  " | " << edgeName[current_submatrix.m_edgeType[1] ]
                    << " , " << edgeName[current_submatrix.m_edgeType[2] ] <<  " | " << edgeName[current_submatrix.m_edgeType[3] ]
                    << std::endl;
      }

      // 1) split circuits until sub-matrix composed of a single circuit
      for (unsigned int axis_i=0; axis_i<2; ++axis_i) {
         if (current_submatrix.m_chipDim[axis_i]>1) {
            n_sub_matrices+=2;
            unsigned int odd = current_submatrix.m_chipDim[axis_i] & 1;
            unsigned int axis_idx= current_submatrix.m_idx[axis_i];

            for (unsigned int part_i=0; part_i<2; ++part_i) {
               unsigned n_chips = (current_submatrix.m_chipDim[axis_i] + odd)/2;
               new_chip_dim[part_i][axis_i]=n_chips;
               unsigned int n_outer_edges=(current_submatrix.m_edgeType[axis_i*2+part_i]==SubMatrixData::kOuter);
               unsigned int n_inner_edges=n_chips*2-n_outer_edges;
               split_idx[axis_i]=axis_idx;
               split_width[part_i][axis_i] = (  n_chips*chip_matrix_dim[axis_i]
                                              - edge_dim[kInner][axis_i]*n_inner_edges
                                              - edge_dim[kOuter][axis_i]*n_outer_edges   ) * pitch[axis_i]
                                             + n_inner_edges*edge_dim[kInner][axis_i]*edge_pitch[kInner][axis_i]
                                             + n_outer_edges*edge_dim[kOuter][axis_i]*edge_pitch[kOuter][axis_i];
               axis_idx += n_chips * chip_matrix_dim[axis_i];
               odd=0u;
            }
         }
      }
      std::array<bool,2> dead_zone_split{};
      // composed of a single circuit split off special pixel edges, or split off dead zone
      if (n_sub_matrices<=1) {
         if (debug_out){
            (*debug_out) << "Split circuit arrays into " << n_sub_matrices << " sub arrays." << std::endl;
         }
         n_sub_matrices=0;
         for (unsigned int axis_i=0; axis_i<2; ++axis_i) {
            // compute the dimension of the edges on both sides
            std::array<unsigned int, 2> axis_edge_dim{0,0};
            for (unsigned int side_i=0; side_i<2; ++side_i) {
               new_chip_dim[side_i][axis_i]=0u;
               if (current_submatrix.m_edgeType[axis_i*2+side_i]<SubMatrixData::kInternal) {
                  axis_edge_dim[side_i]=edge_dim[ kOuter + (current_submatrix.m_edgeType[axis_i*2+side_i]==SubMatrixData::kInner) ][axis_i];
               }
            }

            if (debug_out) {
               (*debug_out) << "Split submatrix " << (axis_i == 0 ? "x: " : "y: ")
                            << edgeName[current_submatrix.m_edgeType[axis_i*2]]
                            << " | "
                            << edgeName[current_submatrix.m_edgeType[axis_i*2+1]]
                            << " -> " << axis_edge_dim[0] << " , "  << axis_edge_dim[1]
                            << std::endl;
            }
            unsigned int axis_dim = std::min(current_submatrix.m_dim[axis_i],chip_matrix_dim[axis_i]);
            split_idx[axis_i]=current_submatrix.m_idx[axis_i]+axis_dim;

            // loop over the two sides until one side has an edge or dead zone that can be split off
            if (axis_edge_dim[0]+axis_edge_dim[1]>0 ) { // if either side has a margin tht needs to be split off
               for (unsigned int side_i=0; side_i<2; ++side_i) {
                  split_width[side_i][axis_i]=0.;
                  if (axis_edge_dim[side_i] > 0 ) {
                     unsigned int inner_outer = kOuter + (current_submatrix.m_edgeType[axis_i*2+side_i]==SubMatrixData::kInner);
                     ++n_sub_matrices;
                     split_idx[axis_i]=current_submatrix.m_idx[axis_i];
                     if (side_i==0) {
                        split_idx[axis_i] += std::min(axis_edge_dim[side_i], current_submatrix.m_dim[axis_i]);
                        // if there is no split check whether the edge has a dead zone
                        if (split_idx[axis_i] == current_submatrix.m_idx[axis_i] + axis_dim
                            && axis_dim == edge_dim[inner_outer][axis_i]
                            && dead_zone[inner_outer][axis_i]>0u ) {
                           split_idx[axis_i] = current_submatrix.m_idx[axis_i] + dead_zone[inner_outer][axis_i];
                           split_width[side_i][axis_i]=dead_zone[inner_outer][axis_i]*edge_pitch[inner_outer][axis_i];
                           split_width[side_i^1][axis_i]=(edge_dim[inner_outer][axis_i] - dead_zone[inner_outer][axis_i])*edge_pitch[inner_outer][axis_i];
                           dead_zone_split[axis_i]=true;
                           break;
                        }
                     }
                     else {
                        split_idx[axis_i] += axis_dim - std::min(axis_edge_dim[side_i],current_submatrix.m_dim[axis_i]) ;
                        // if there is no split check whether the edge has a dead zone
                        if (split_idx[axis_i] == current_submatrix.m_idx[axis_i]
                            && axis_dim == axis_edge_dim[side_i]
                            && dead_zone[inner_outer][axis_i]>0u ) {
                           split_idx[axis_i] = current_submatrix.m_idx[axis_i] + current_submatrix.m_dim[axis_i] - dead_zone[inner_outer][axis_i];
                           split_width[side_i][axis_i]=dead_zone[inner_outer][axis_i]*edge_pitch[inner_outer][axis_i];
                           split_width[side_i^1][axis_i]=(edge_dim[inner_outer][axis_i] - dead_zone[inner_outer][axis_i])*edge_pitch[inner_outer][axis_i];
                           dead_zone_split[axis_i]=true;
                           break;
                        }
                        // lower side only
                     }
                     if (split_idx[axis_i]<current_submatrix.m_idx[axis_i]
                         || split_idx[axis_i]>current_submatrix.m_idx[axis_i]+current_submatrix.m_dim[axis_i]
                         || (current_submatrix.m_dim[axis_i] < edge_dim[kOuter][axis_i] && current_submatrix.m_dim[axis_i] < edge_dim[kInner][axis_i])
                         ) {
                        throw std::logic_error("invalid split index.");
                     }
                     split_width[side_i][axis_i]=axis_edge_dim[side_i] * edge_pitch[inner_outer][axis_i];
                     // only edges on side 0 here
                     unsigned int n_inner_edges = current_submatrix.m_edgeType[axis_i*2+(side_i^1)]==SubMatrixData::kInner;
                     unsigned int n_outer_edges = current_submatrix.m_edgeType[axis_i*2+(side_i^1)]==SubMatrixData::kOuter;
                     split_width[side_i^1][axis_i] = (  (  axis_dim
                                                           // edges on both sides here
                                                           - edge_dim[kInner][axis_i]*(n_inner_edges+current_submatrix.m_edgeType[axis_i*2+side_i]==SubMatrixData::kInner)
                                                           - edge_dim[kOuter][axis_i]*(n_outer_edges+current_submatrix.m_edgeType[axis_i*2+side_i]==SubMatrixData::kOuter))
                                                        * pitch[axis_i]
                                                        + edge_dim[kInner][axis_i]*edge_pitch[kInner][axis_i]*n_inner_edges
                                                        + edge_dim[kOuter][axis_i]*edge_pitch[kOuter][axis_i]*n_outer_edges);
                     break;
                  }
               }
            }
         }
         if (n_sub_matrices>0) {
            if(    (split_idx[0]>=current_submatrix.m_idx[0]+current_submatrix.m_dim[0] || split_idx[0]==current_submatrix.m_idx[0])
                && (split_idx[1]>=current_submatrix.m_idx[1]+current_submatrix.m_dim[1] || split_idx[1]==current_submatrix.m_idx[1])) {
               // nothing to split
               n_sub_matrices=0;
            }
         }
      }

      if (n_sub_matrices>0)
      {
         // split current matrix further into sub-matrices
         if (debug_out) {
            (*debug_out)  << "Split : " << current_submatrix.m_idx[0] << " , " << current_submatrix.m_idx[1] << " " << current_submatrix.m_dim[0] << "x" << current_submatrix.m_dim[1]
                          << " split at  " << split_idx[0] << ", " << split_idx[1]
                          << "  " << (split_idx[0] - current_submatrix.m_idx[0]) << "|" << (current_submatrix.m_idx[0]+current_submatrix.m_dim[0]-split_idx[0])
                          << ", " << (split_idx[1] - current_submatrix.m_idx[1]) << "|" << (current_submatrix.m_idx[1]+current_submatrix.m_dim[1]-split_idx[1])
                          << " width " << split_width[0][0] << ", " << split_width[0][1] << "; " << split_width[1][0] << ", " << split_width[1][1]
                          << std::endl;
         }
         std::size_t stack_size=stack.size();
         splitMatrix(current_submatrix,
                     split_idx,
                     split_width,
                     new_chip_dim,
                     dead_zone_split,
                     std::array<std::array<PixelDiodeTree::AttributeType, 2>,2>  {std::array<PixelDiodeTree::AttributeType, 2>{SubMatrixData::s_defaultMatrixAttribute,
                                                                                                                               SubMatrixData::s_defaultMatrixAttribute},
                                                                                  std::array<PixelDiodeTree::AttributeType, 2>{SubMatrixData::s_defaultMatrixAttribute,
                                                                                                                               SubMatrixData::s_defaultMatrixAttribute}},
                     stack,
                     diode_tree,
                     debug_out,
                     &edgeName);
         if (stack_size == stack.size()) {
            // split surprisingly resulted in zero new sub-matrices. @TODO is this possible ? Should this be a logic error ?
            n_sub_matrices=0;
         }
      }

      // if the current matrix was not split into further sub-matrices
      // register and assign a diode for it.
      if (n_sub_matrices==0) {

         // if the diode tree is still empty
         // create a dummy split of which only the first area is used.
         if (diode_tree.empty()) {
            assert( stack.empty() ); // this can only happen if no matrix has been split yet, and then the stack must also be empty
            assert( chip_dim[0]==1 && chip_dim[1]==1); // should only happen for single chip modules
            assert( current_submatrix.m_subMatrixIdx==std::numeric_limits<unsigned int>::max());
            current_submatrix.m_subMatrixIdx
               = diode_tree.split( std::array<PixelDiodeTree::CellIndexType,2>{ static_cast< PixelDiodeTree::CellIndexType>(current_submatrix.m_dim[0]),
                                                                              static_cast< PixelDiodeTree::CellIndexType>(current_submatrix.m_dim[1]) },
                                   current_submatrix.m_pos+current_submatrix.m_width,
                                   current_submatrix.m_attribute);
         }

         // determine diode type by evaluating whether it is on an inner, or outer edge, a dead zone or just a normal pixel diode
         PixelDiodeTree::Vector2D diode_width(PixelDiodeTree::Vector2D::Zero());
         PixelDiodeTree::AttributeType full_diode_type{};
         for (unsigned int axis_i=0; axis_i<2; ++axis_i) {
            // count inner and outer edges of the current sub matrix
            unsigned int axis_inner_edges = (  (current_submatrix.m_edgeType[axis_i*2] == SubMatrixData::kInner)
                                             + (current_submatrix.m_edgeType[axis_i*2+1] == SubMatrixData::kInner));
            unsigned int axis_outer_edges = (  (current_submatrix.m_edgeType[axis_i*2] == SubMatrixData::kOuter)
                                             + (current_submatrix.m_edgeType[axis_i*2+1] == SubMatrixData::kOuter));

            // set corresponding bit in diode type if sub-matrix has a non empty inner or outer edge
            // in principle only one of the two should be true per axis
            unsigned int diode_type = ((axis_inner_edges * edge_dim[kInner][axis_i]) > 0) << (kInner+1);
            diode_type |= ((axis_outer_edges * edge_dim[kOuter][axis_i]) > 0) << (kOuter+1);
            bool ganged=false;
            unsigned int n = 0;
            // special handling if this is an edge with a dead zone
            // @TODO should distinguish inner or outer edges adjacent to dead zone from actual dead zones.
            if (   current_submatrix.m_edgeType[axis_i*2]>SubMatrixData::kInternal
                || current_submatrix.m_edgeType[axis_i*2+1]>SubMatrixData::kInternal) {
               diode_type =   ((current_submatrix.m_edgeType[axis_i*2]==SubMatrixData::kDeadZoneInner) << (kInner+1))
                            | ((current_submatrix.m_edgeType[axis_i*2+1]==SubMatrixData::kDeadZoneInner) << (kInner+1))
                            | ((current_submatrix.m_edgeType[axis_i*2]==SubMatrixData::kDeadZoneOuter) << (kOuter+1))
                            | ((current_submatrix.m_edgeType[axis_i*2+1]==SubMatrixData::kDeadZoneOuter) << (kOuter+1));
               ganged=true;
            }
            else {
               n = current_submatrix.m_dim[axis_i] - edge_dim[kOuter][axis_i] *axis_outer_edges - edge_dim[kInner][axis_i] * axis_inner_edges;
            }
            diode_type |= (n>0) << 0u;
            if (debug_out) {
               *debug_out << (axis_i==0 ? "x: " : "y:" )
                          << " outer " <<  axis_outer_edges << " * " << edge_dim[kOuter][axis_i] << " * " << edge_pitch[kOuter][axis_i]
                          << " inner "  << axis_inner_edges << " * " << edge_dim[kInner][axis_i] << " * " << edge_pitch[kInner][axis_i]
                          << " normal " << n
                          << " -> " << diode_type
                          << std::endl;
            }
            // determine diode width depending on diode type
            switch (diode_type) {
            case (1u<<0):
               diode_width[axis_i]=pitch[axis_i];
               break;
            case (1u<<(kOuter+1)):
               diode_width[axis_i]=edge_pitch[kOuter][axis_i];
               break;
            case (1u<<(kInner+1)):
               diode_width[axis_i]=edge_pitch[kInner][axis_i];
               break;
            default:
               throw std::logic_error("Invalid diode type. Matrix not fully split.");
            }

            // one byte per axis
            full_diode_type<<=8;
            diode_type|=ganged<<3u;
            full_diode_type|=(diode_type&0xff);
         }
         // first two elements whether the diode is in a ganged area in one of the two directions
         // second two elements whether
         // if the diode is adjacent to an inner or outer edge it is in the dead zone if the diode is marked ganged.
         std::array<bool,4> ganged_flags{ (static_cast<std::size_t>(full_diode_type) & (1u<<3u)) != 0u,
                                          (static_cast<std::size_t>(full_diode_type) & (1u<<(3u+8u))) != 0u,
                                          current_submatrix.m_edgeType[0]==SubMatrixData::kInner
                                          || current_submatrix.m_edgeType[0+1]==SubMatrixData::kInner
                                          || current_submatrix.m_edgeType[0]==SubMatrixData::kOuter
                                          || current_submatrix.m_edgeType[0+1]==SubMatrixData::kOuter,
                                          current_submatrix.m_edgeType[2]==SubMatrixData::kInner
                                          || current_submatrix.m_edgeType[2+1]==SubMatrixData::kInner
                                          || current_submatrix.m_edgeType[2]==SubMatrixData::kOuter
                                          || current_submatrix.m_edgeType[2+1]==SubMatrixData::kOuter};
         // cannot be in a dead-zone if the pixel is not marked ganged.
         ganged_flags[2]=ganged_flags[2]&ganged_flags[0];
         ganged_flags[3]=ganged_flags[3]&ganged_flags[1];

         PixelDiodeTree::AttributeType current_sub_matrix_attribute=diode_tree.attribute(current_submatrix.m_subMatrixIdx);
         auto [new_sub_matrix_attribute, new_diode_attribute]
            = func_compute_attribute(std::array<PixelDiodeTree::CellIndexType,2>{ static_cast<PixelDiodeTree::CellIndexType>(current_submatrix.m_idx[0]),
                                                                                  static_cast<PixelDiodeTree::CellIndexType>(current_submatrix.m_idx[1])},
                                     diode_width,
                                     ganged_flags,
                                     current_submatrix.m_splitIdx,
                                     current_sub_matrix_attribute,
                                     full_diode_type);

         std::pair< std::unordered_map<unsigned int, DiodeInfo >::iterator, bool>
            ret = diode_idx.insert( std::make_pair(full_diode_type, DiodeInfo(std::numeric_limits<unsigned int>::max(),new_diode_attribute)));
         if (ret.second) {
            unsigned int diode_idx = diode_tree.addDiode(diode_width,
                                                         new_diode_attribute);
            assert( diode_idx < std::numeric_limits<PixelDiodeTree::IndexType>::max());
            ret.first->second.setIndex( diode_idx );
         }
         assert( ret.second || ret.first->second.attributeAgrees(new_diode_attribute) );
         diode_tree.setAttribute(current_submatrix.m_subMatrixIdx, new_sub_matrix_attribute);
         assert(   current_sub_matrix_attribute==SubMatrixData::s_defaultMatrixAttribute
                || current_sub_matrix_attribute==new_sub_matrix_attribute);

         diode_tree.setDiodeForSubMatrix(current_submatrix.m_subMatrixIdx, current_submatrix.m_splitIdx,ret.first->second.diodeIndex());
         if (debug_out) {
            (*debug_out) << "Created Diode "
                         << current_submatrix.m_idx[0] << ", " << current_submatrix.m_idx[1] << " "
                         << current_submatrix.m_dim[0] << " x " << current_submatrix.m_dim[1]
                         << " attr: " << new_sub_matrix_attribute
                         << " : diode_pitch " << diode_width[0] << ", " << diode_width[1]
                         << " sub-matrix index " << current_submatrix.m_subMatrixIdx
                         << " split index " << current_submatrix.m_splitIdx
                         << " -> " << -static_cast<PixelDiodeTree::IndexType>(ret.first->second.diodeIndex())
                         << " attribute " << full_diode_type << " -> " << new_diode_attribute
                         << std::endl;
         }
      }
   }
   if (diode_tree.cloneSingleSplitsToUnusedHalf()>0) {
      std::cout << diode_tree.debugStringRepr() << std::endl;
      throw std::logic_error("Some splits have invalid indices. That should not happen.");
   }
   // set the positions of the upper and lower corner of the matrix
   diode_tree.computeMatrixCorner(PixelDiodeTree::makeCellIndex(dim[0],dim[1]));
   return diode_tree;
}
}
