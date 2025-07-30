/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
  */
#ifndef INDETDD_PIXELDIODETREEBUILDER_H
#define INDETDD_PIXELDIODETREEBUILDER_H

#include "PixelDiodeTree.h"
#include <ostream>
#include <functional>
#include <tuple>

namespace InDetDD {

   // AttributeRefiner is a functional to "compute" the diode and sub-matrix attributes for a
   // certain split which contains at least one diode
   //
   // arguments
   // 1) split_idx the index of the split position ,
   // 2) diode_width the width of the diode in local-x and local-y direction
   // 3) ganged an array of flags to indicate whether the diode is "ganged" in local-x or local-y,
   //    then two additional flags (elements 2,3) to indicate whether the diode is in the dead-zone
   //    in local-x or local-y (true) or outside (false).
   // 4) split_i   defines which of the 4 areas the diode belongs to :  2 | 3        ^
   //                                                                   -----        |  local-y (chip-columns)
   //                                                                   0 | 1        |
   //                                                                   ---> local-x (chip-rows)
   // 5) the current_matrix_attribute which may already be set to a custom value, where all diodes of
   //    the same "split" have to set the matrix attribute to the same value otherwise the result will be undefined.
   // 6) the current_diode_attribute which may already be set if the same diode appears in multiple splits, where
   //    this functional has to yield the same attribute for each re-occurrence of the same diode.
   // The input value of current_matrix_attribute will be zero
   // The input value of current_diode_attribute will contain information about the edges the diode is in.
   // The bottom 16 bits are used for the edge-information in local-x direction, and the subsequent 16 bits
   // for the edge-information in local-y direction:
   //   1 for regular diodes,
   //   2 for diodes in the outer edge
   //   4 for diodes in the inner edge, and the value will be
   //   +8 if the pixel is ganged.
   using AttributeRefiner
   = std::function<std::tuple<PixelDiodeTree::AttributeType,PixelDiodeTree::AttributeType>
                     (const std::array<PixelDiodeTree::IndexType,2> & /* split_idx*/,
                      const PixelDiodeTree::Vector2D & /* diode_width*/,
                      const std::array<bool,4> &                      /* ganged*/,
                      unsigned int                                    /* split_i*/,
                      PixelDiodeTree::AttributeType                   /* current_matrix_attribute*/,
                      PixelDiodeTree::AttributeType                   /* current_diode_attribute*/  )>;

   /// Create a pixel diode tree.
   /// @param chip_dim the number of circuits in local-x (phi, row) and local-y (eta, column) direction.
   /// @param chip_matrix_dim the width of a matrix in number of pixels in local-x(phi, row) and local-y (eta, column) direction.
   /// @param pitch the pitch of regular pixels in local-x (phi, row) and local-y (eta, column) direction.
   /// @param edge_dim the width of the outer, and inner edge of the sub-matrix associated to one circuit.
   /// @param edge_pitch the pitch of diodes in the outer or inner edge of a sub-matrix in local-x and local-y direction.
   /// @param dead_zone the width of a "dead" zone at the the outside of an outer or inner edge in pixels.
   /// @param func_compute_attribute a functional to "compute" attributes to diodes and sub-matrices.
   /// @param debug_out nullptr or a valid output stream to get debug output.
   /// Will create a diode tree to compute diode positions or indices from indices or positions and
   PixelDiodeTree createPixelDiodeTree( const std::array<unsigned int,2> &chip_dim,
                                        const std::array<unsigned int,2> &chip_matrix_dim,
                                        const PixelDiodeTree::Vector2D &pitch,
                                        const std::array<std::array<unsigned int,2>,2> &edge_dim,
                                        const std::array<PixelDiodeTree::Vector2D,2> &edge_pitch,
                                        const std::array<std::array<unsigned int,2>,2> &dead_zone,
                                        const AttributeRefiner &func_compute_attribute,
                                        std::ostream *debug_out=nullptr);

   namespace detail {
      /// convenience method to test whether the given value can be converted into an attribute
      template<typename T>
      bool validAttributeType(T val) {
         if constexpr( std::is_signed_v<T>) {
            return val>= std::numeric_limits<InDetDD::PixelDiodeTree::AttributeType>::min()
               && val < std::numeric_limits<InDetDD::PixelDiodeTree::AttributeType>::max();
         }
         else {
            return val < std::numeric_limits<InDetDD::PixelDiodeTree::AttributeType>::max();
         }
      }

      /// convenience method to convert the given value into an attribute
      template<typename T>
      InDetDD::PixelDiodeTree::AttributeType makeAttributeType(T val) {
         assert( validAttributeType(val));
         return static_cast<InDetDD::PixelDiodeTree::AttributeType>(val);
      }
   }
}
#endif
