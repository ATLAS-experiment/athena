/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
  */
#include "ReadoutGeometryBase/PixelDiodeMatrix.h"
#include <ReadoutGeometryBase/PixelDiodeTree.h>
#include <ReadoutGeometryBase/PixelDiodeTreeBuilder.h>
#include "PixelGeoUtils.h"
#include "PixelGeometryManager.h"

namespace InDetDD {
namespace detail {
PixelDiodeTree PixelDiodeTreeMakerBase::make(InDetDD::PixelReadoutTechnology readoutTechnology,
                                             const std::array<int,kNDirections> &circuits,
                                             const std::array<int,kNDirections> &dimPerCircuit,
                                             const std::array<std::array<double,kNDirections>,kNPixelLocations> &pitch) {

  // helper function to associate correct diode type and front-end number to sub-matrices and diodes
  // in the diode tree as attributes.
  auto computeAttribute = [readoutTechnology,
                           &pitch=pitch[InDetDD::detail::kCentral],
                           &circuits,
                           &dimPerCircuit
                           ](const std::array<PixelDiodeTree::IndexType,2> &split_idx,
                             const PixelDiodeTree::Vector2D &diode_width,
                             [[maybe_unused]] const std::array<bool,4> &ganged,
                             [[maybe_unused]] unsigned int split_i,
                             PixelDiodeTree::AttributeType current_matrix_attribute,
                             PixelDiodeTree::AttributeType current_diode_attribute)
     -> std::tuple<PixelDiodeTree::AttributeType,PixelDiodeTree::AttributeType>
  {
     // split_i   defines which of the 4 areas the diode belongs to :  2 | 3        ^
     //                                                                -----        |  local-y (chip-columns)
     //                                                                0 | 1        |
     //                                                                ---> local-x (chip-rows)
     //
     // split_idx the absolute index at which this sub-matrix is split into 4 sub-sub-matrices
     // diode_width the diode pitch in both directions
     // ganged ganged[0],ganged[1] whether the pixel diode is ganged in the corresponding direction
     //        ganged[2],ganged[3] whether the diode is inside (true) or outside the dead zone
     //        where ganged[2] denotes the flag in local-x and ganged[3] in local-y direction
     // current_matrix_attribute the default attribute for the unsplit sub-matrix assigned by the builder
     // current_diode_attribute the default attribute assigned to the current diode associated to the split
     //                         area specified by split_i
     // return new matrix attribute, new diode attribute

     // if the pixel is significantly wider in one direction consider the pixel to be long
     // or if wider in both directions large
     assert(split_idx[0]>=0 && split_idx[1]>=0);
     std::array<int,2> chip_idx{split_idx[0]/dimPerCircuit[InDetDD::detail::kPhi], split_idx[1]/dimPerCircuit[InDetDD::detail::kEta]};

     unsigned int n_large_dimensions = (  (std::abs(diode_width[0]-pitch[InDetDD::detail::kPhi])>pitch[InDetDD::detail::kPhi]*.25)
                                         +(std::abs(diode_width[1]-pitch[InDetDD::detail::kEta])>pitch[InDetDD::detail::kEta]*.25));
     std::cout << "DEBUG compute diode-type for " << split_idx[0] << " " << split_idx[1] << " | " << split_i
               << " width " << diode_width[0] << " " << diode_width[1]  << " normal pitch " << pitch[InDetDD::detail::kPhi] << " " << pitch[InDetDD::detail::kEta]
               << " large dim " << n_large_dimensions
               << std::endl;

     switch (n_large_dimensions) {
     case 1:
        current_diode_attribute=InDetDD::detail::makeAttributeType(InDetDD::PixelDiodeType::LONG);
        break;
     case 2:
        current_diode_attribute=InDetDD::detail::makeAttributeType(InDetDD::PixelDiodeType::LARGE);
        break;
     default:
        current_diode_attribute=InDetDD::detail::makeAttributeType(InDetDD::PixelDiodeType::NORMAL);
     }

     // FEI3 front-end numbering scheme
     // if there is a single row just  the chip-column (local-y, eta)
     // if there are two rows: top row chip-column starting from the opposite end; bottom row: chip column + chips per top row
     //            ^    0   |..  |n/2-1
     // local-x    |    ---------------         [swapped axis direction to fit into fewer lines]
     // /phi/row   |    n-1 |... |n/2
     //            --> local-y (chip-column, eta)
     current_matrix_attribute
       = InDetDD::detail::makeAttributeType( chip_idx[0] > 0
                                             ? circuits[InDetDD::detail::kEta] - chip_idx[1] - 1
                                             : (circuits[InDetDD::detail::kPhi]-1) * circuits[InDetDD::detail::kEta] + chip_idx[1]);
     return std::make_tuple(current_matrix_attribute, current_diode_attribute);
  };

  unsigned int nEtaLongEnd = pitch[InDetDD::detail::kOuterEdge][InDetDD::detail::kEta]>0. ? 1u : 0u;
  unsigned int nEtaLong =    pitch[InDetDD::detail::kInnerEdge][InDetDD::detail::kEta]>0. ? 1u : 0u;
  PixelDiodeTree diode_tree
     = createPixelDiodeTree(std::array<unsigned int,2>{static_cast<unsigned int>(circuits[InDetDD::detail::kPhi]),
                                                       static_cast<unsigned int>(circuits[InDetDD::detail::kEta])},
                            std::array<unsigned int,2>{static_cast<unsigned int>(dimPerCircuit[InDetDD::detail::kPhi]),
                                                       static_cast<unsigned int>(dimPerCircuit[InDetDD::detail::kEta])},
                            PixelDiodeTree::Vector2D{pitch[InDetDD::detail::kCentral][InDetDD::detail::kPhi],
                                                     pitch[InDetDD::detail::kCentral][InDetDD::detail::kEta]},     // regular ptich
                            std::array<std::array<unsigned int,2>, 2>{ std::array<unsigned int,2>{0u,nEtaLongEnd}, // outer edge in pixels (correct?)
                                                                       std::array<unsigned int,2>{0u,nEtaLong}},   // inner edge in pixels (correct?)
                            std::array<PixelDiodeTree::Vector2D,2>{PixelDiodeTree::Vector2D{0.,
                                                                                            pitch[InDetDD::detail::kOuterEdge][InDetDD::detail::kEta]},
                                                                   PixelDiodeTree::Vector2D{0.,
                                                                                            pitch[InDetDD::detail::kInnerEdge][InDetDD::detail::kEta]}
                            },
                            std::array<std::array<unsigned int,2>, 2>{ std::array<unsigned int,2>{0u,0u},   // @TODO add dead zone for run1-3 pixels
                                                                       std::array<unsigned int,2>{0u,0u}    // @TODO add dead zone for run1-3 pixels
                            },
                            computeAttribute,
                            nullptr);
  GEO_MSG_DEBUG( "module " << " " << circuits[InDetDD::detail::kPhi] << "x" << circuits[InDetDD::detail::kEta] << " "
                << dimPerCircuit[InDetDD::detail::kPhi] << " " << dimPerCircuit[InDetDD::detail::kEta] << ":\n"
                << " pitch: regular " << pitch[InDetDD::detail::kCentral][InDetDD::detail::kPhi]
                << " "  << pitch[InDetDD::detail::kCentral][InDetDD::detail::kEta]
                << " long: " << pitch[InDetDD::detail::kInnerEdge][InDetDD::detail::kEta] << " end " << pitch[InDetDD::detail::kOuterEdge][InDetDD::detail::kEta] << ":\n"
                << diode_tree.debugStringRepr());
  return diode_tree;
}
}
}
