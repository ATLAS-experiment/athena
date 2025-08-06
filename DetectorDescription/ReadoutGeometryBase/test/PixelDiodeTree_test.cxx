/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
  */
#include "ReadoutGeometryBase/PixelDiodeTree.h"
#include "ReadoutGeometryBase/PixelDiodeTreeBuilder.h"
#include <iostream>
#include <cstdlib>
#include <cstring>
#ifdef NDEBUG
#  undef NDEBUG
#endif

namespace Units {
   constexpr double um=1e-3;
};
using namespace InDetDD;
bool test_pixelDiodeTree(const PixelDiodeTree &diode_tree,
                         const std::array<PixelDiodeTree::CellIndexType,2> &chip_dim,
                         const std::array<unsigned int,2> &n_chips,
                         const std::array<unsigned int,2> &outer_border_width,
                         const std::array<unsigned int,2> &inner_border_width,
                         const std::array<PixelDiodeTree::Vector2D,3> &pitch,
                         std::ostream *debug_out) {
   std::vector< std::pair<  std::array<PixelDiodeTree::CellIndexType,2>,
                            PixelDiodeTree::Vector2D > > test_data;
   const std::array<PixelDiodeTree::CellIndexType,2> &dim(chip_dim);
   std::array<int, 5> offset {0,1,2,-2,-1};

   std::array<unsigned int,2> outer_edge_total;
   std::array<unsigned int,2> dim_total;
   std::array<double, 2> tolerance_scale;
   for (unsigned int axis_i=0; axis_i<2; ++axis_i) {
      outer_edge_total[axis_i]  = chip_dim[axis_i]*n_chips[axis_i] - outer_border_width[axis_i];
      dim_total[axis_i]  = chip_dim[axis_i]*n_chips[axis_i];
      unsigned int bits_i=0;
      for (; bits_i<16 && 1u<<bits_i < dim_total[axis_i]*3; ++bits_i);
      tolerance_scale[axis_i]=1u<<bits_i;
   }
   if (debug_out) {
      (*debug_out) << "Tolerance scale " << tolerance_scale[0] << " "  << tolerance_scale[1] << std::endl;
   }

   PixelDiodeTree::Vector2D matrix_center;
   for (unsigned int axis_i=0; axis_i<2; ++axis_i) {
      std::array<unsigned int,3> n;
      n[2] = (n_chips[axis_i]*2 - 2) * inner_border_width[axis_i];
      n[1] = 2 * outer_border_width[axis_i];
      n[0] = n_chips[axis_i] * chip_dim[axis_i]-n[2]-n[1];

      double axis_offset=0.;
      for (unsigned int i=0;i<n.size(); ++i) {
         assert( i<pitch.size());
         assert( axis_i < pitch[i].size());
         axis_offset += n[i]*pitch[i][axis_i];
      }
      axis_offset *= 0.5;
      matrix_center[axis_i]=static_cast<PixelDiodeTree::FloatType>(axis_offset);
   }
   if (debug_out) {
      (*debug_out) << "matrix_center " << matrix_center[0] << " " << matrix_center[1] << std::endl;
   }
   for (unsigned int mult_i=0; mult_i<n_chips[0]; ++mult_i) {
      for (unsigned int mult_j=0; mult_j<n_chips[1]; ++mult_j) {
         for (int offset_i : offset) {
            for (int offset_j : offset) {
               std::array<PixelDiodeTree::CellIndexType,2> idx{
                  static_cast<PixelDiodeTree::CellIndexType>((mult_i+(offset_i<0)) * dim[0] + offset_i),
                  static_cast<PixelDiodeTree::CellIndexType>((mult_j+(offset_j<0)) * dim[1] + offset_j)};
               if (   idx[0]<0 || static_cast<unsigned int>(idx[0])>dim_total[0]
                   || idx[1]<0 || static_cast<unsigned int>(idx[1])>dim_total[1]) {
                  continue;
               }


               PixelDiodeTree::Vector2D pos;
               for (unsigned int axis_i=0; axis_i<2; ++axis_i) {

                  // compute index per chip and number of full chips before idx.
                  unsigned int chip_idx = idx[axis_i]%chip_dim[axis_i];
                  unsigned int chip_i = idx[axis_i]/chip_dim[axis_i];

                  // count the number of inner and outer edge cells for full chips before idx
                  unsigned int n_outer_edge_cells = (static_cast<unsigned int>(idx[axis_i]) < outer_border_width[axis_i])
                     ?  (idx[axis_i]>0 ? idx[axis_i] : 0u)
                     :  outer_border_width[axis_i];
                  n_outer_edge_cells +=  ((static_cast<unsigned int>(idx[axis_i]) > outer_edge_total[axis_i])
                                          ?  idx[axis_i]-outer_edge_total[axis_i] : 0u );
                  unsigned int n_inner_edge_cells  = chip_i>0 ? (chip_i * 2-1) * inner_border_width[axis_i] : 0u;

                  // by default half pitch set to pitch of normal cells
                  unsigned int pitch_idx=0;

                  if (static_cast<unsigned int>(idx[axis_i])<outer_border_width[axis_i] || static_cast<unsigned int>(idx[axis_i])>=outer_edge_total[axis_i]) {
                     // if in a lower or upper outer edge use the outer border pitch for the half pitch
                     pitch_idx=1;
                  }
                  // if chip_idx is within the lower inner border check that it actually is an inner and not an outer border
                  // the cell at index only counts as half
                  n_inner_edge_cells += (chip_idx < inner_border_width[axis_i])
                     ? (chip_i>0 ? chip_idx : 0u )
                     : (chip_i>0 ? inner_border_width[axis_i] : 0u);
                  // if chip_idx is within the upper inner border also check that it actually is an inner and not an outer border
                  // the cell at index is not counted because it only counts as half
                  n_inner_edge_cells += ( (chip_idx > chip_dim[axis_i] - inner_border_width[axis_i] ) &&  chip_i+1 != n_chips[axis_i]
                                          ? chip_idx-(chip_dim[axis_i] - inner_border_width[axis_i])
                                          : 0u);
                  // if in a lower or upper inner edge use the inner border pitch for the half pitch
                  if ((chip_idx<inner_border_width[axis_i]  && chip_i>0)|| (chip_idx>= chip_dim[axis_i]-inner_border_width[axis_i] && chip_i+1 != n_chips[axis_i])) {
                     pitch_idx=2;
                  }

                  pos[axis_i] =(  (idx[axis_i] - n_outer_edge_cells - n_inner_edge_cells) * pitch[0][axis_i]
                                 + n_outer_edge_cells*pitch[1][axis_i]
                                 + n_inner_edge_cells*pitch[2][axis_i]
                                 + pitch[pitch_idx][axis_i]*.5)
                                - matrix_center[axis_i];
                  if (debug_out) {
                     (*debug_out) << (axis_i==0 ? "x: " : "y: ")
                                  << idx[axis_i] << " -> chips/c.idx " << chip_i << ", " << chip_idx
                                  << " -> edge cells outer/inner "  << n_outer_edge_cells << ", " << n_inner_edge_cells
                                  << " normal " <<  (idx[axis_i] - n_outer_edge_cells - n_inner_edge_cells)
                                  << " -> pitch idx " << pitch_idx
                                  << " pos: " << pos[axis_i]
                                  << std::endl;
                  }
               }

               test_data.push_back( std::make_pair ( idx,pos));
            }
         }
      }
   }
   static constexpr PixelDiodeTree::FloatType tolerance = 1e-2*Units::um;
   unsigned int errors=0;
   unsigned int comparisons=0;
   std::array<double, 2> max_diff {0.,0.};

   for (auto a_test : test_data) {
      auto a_pos = diode_tree.diodeProxyFromIdx( a_test.first).computePosition(a_test.first);
      bool above_threshold=false;
      for (unsigned int axis_i=0; axis_i<2; ++axis_i) {
         double diff = std::abs(a_test.second[0] - a_pos[0]);
         max_diff[axis_i] = std::max(max_diff[axis_i],diff);
         above_threshold |= diff > tolerance;
      }
      ++comparisons;
      if (above_threshold) {
         std::cerr << "Mismatch for " << a_test.first[0] << " , " << a_test.first[1]
                   << " got " << a_pos[0] << ", " << a_pos[1] << " expected "
                   << a_test.second[0] << " , " << a_test.second[1]
                   << " diff "  << (a_test.second[0] - a_pos[0]) << " , " << (a_test.second[1] - a_pos[1])
                   << std::endl;
         // for debugging
         auto b_pos = diode_tree.findFromIdx( a_test.first);
         (void ) b_pos;
         ++errors;
      }
      auto idx_a = diode_tree.diodeProxyFromPos( a_pos ).computeIndex(a_pos);
      auto idx_b = diode_tree.diodeProxyFromPos( a_test.second ).computeIndex(a_test.second);
      ++comparisons;
      if (idx_a != a_test.first || idx_b != a_test.first) {
         std::cerr << "Failed to recover index  " << a_test.first[0] << " , " << a_test.first[1]
                   << " -> " << a_pos[0] << ", " << a_pos[1] << "  -> " << idx_a[0] << " , " << idx_a[1]
                   << " | " << a_test.second[0] << " , " << a_test.second[1] << " -> " << idx_b[0] << " , " << idx_b[1]
                   << std::endl;
         // for debugging
         auto idx_again = diode_tree.diodeProxyFromPos( a_test.second ).computeIndex(a_test.second);
         (void) idx_again;
         ++errors;
      }
      auto proxy = diode_tree.diodeProxyFromIdx( a_test.first);
      auto cell_pos = proxy.computePosition(a_test.first);
      for (int offset_x_i=-2; offset_x_i<=2; offset_x_i +=1) {
         PixelDiodeTree::FloatType offset_x = offset_x_i *  proxy.width()[0] * 0.5/2.;
         offset_x -= std::numeric_limits<PixelDiodeTree::FloatType>::epsilon() * tolerance_scale[0] *offset_x;
         for (int offset_y_i=-2; offset_y_i<=2; offset_y_i +=1) {
            PixelDiodeTree::FloatType offset_y = offset_y_i *  proxy.width()[1] * 0.5 / 2.;
            offset_y -= std::numeric_limits<PixelDiodeTree::FloatType>::epsilon() * tolerance_scale[1] * offset_y;

            PixelDiodeTree::Vector2D test_pos{a_test.second[0] + offset_x,
                                                              a_test.second[1] + offset_y };
            auto idx_test = diode_tree.diodeProxyFromPos(test_pos).computeIndex(test_pos);
            ++comparisons;
            if (idx_test != a_test.first || idx_test != a_test.first) {
               if (debug_out) {
                  (*debug_out) << "Failed to recover index  " << a_test.first[0] << " , " << a_test.first[1]
                               << " offset " << offset_x_i << " " << offset_y_i << " : "
                               << " from " << test_pos[0] << ", " << test_pos[1] << "  -> " << idx_test[0] << " , " << idx_test[1]
                               << " cell " << (cell_pos[0] - proxy.width()[0]*.5) << " .. " <<  (cell_pos[0] + proxy.width()[0]*.5)
                               << " "
                               << (cell_pos[1] - proxy.width()[1]*.5) << " .. " <<  (cell_pos[1] + proxy.width()[1]*.5)
                               << " | pos diff " << ((test_pos[0] - cell_pos[0]) * proxy.invWidth()[0] * 2)
                               << " " << ((test_pos[1] - cell_pos[1]) * proxy.invWidth()[1] *2)
                               << std::endl;
               }
               // for debugging
               auto idx_test_b = diode_tree.diodeProxyFromPos(test_pos).computeIndex(test_pos);
               (void) idx_test_b;
               ++errors;
            }
         }
      }
   }
   if (debug_out) {
      (*debug_out) << "max diff : "  << max_diff[0] << " " << max_diff[1] << std::endl;
      if (errors>0) {
         (*debug_out) << errors << " / " << comparisons << " comparisons failed." << std::endl;
      }
   }
   return errors==0;
}

const auto attributePassThrough = []([[maybe_unused]] const std::array<PixelDiodeTree::IndexType,2> &split_idx,
                                     [[maybe_unused]] const PixelDiodeTree::Vector2D &diode_width,
                                     [[maybe_unused]] const std::array<bool,4> &ganged,
                                     [[maybe_unused]] unsigned int split_i,
                                     PixelDiodeTree::AttributeType current_matrix_attribute,
                                     PixelDiodeTree::AttributeType current_diode_attribute)
        -> std::tuple<PixelDiodeTree::AttributeType,PixelDiodeTree::AttributeType>
   { return std::make_tuple(current_matrix_attribute, current_diode_attribute); };

int test_quad(std::ostream *debug_out) {
   // PixelDiodeTree for ITk quad modules
   PixelDiodeTree diode_tree
      = createPixelDiodeTree(std::array<unsigned int,2>{2u,2u},
                             std::array<unsigned int,2>{384u,400u},
                             PixelDiodeTree::Vector2D{50.*Units::um,50.*Units::um},
                             std::array<std::array<unsigned int,2>, 2>{ std::array<unsigned int,2>{0,0}   /* no outer edge */,
                                                                        std::array<unsigned int,2>{2u,2u} /* inner edge width in number if pixels*/},
                             std::array< PixelDiodeTree::Vector2D,2>{PixelDiodeTree::Vector2D{50.*Units::um,  50.*Units::um} /* no outer edge*/,
                                                                     PixelDiodeTree::Vector2D{100.*Units::um,100.*Units::um}   /* inner edge*/
                             },
                             std::array<std::array<unsigned int,2>, 2>{ std::array<unsigned int,2>{0u,0u}   /* no dead zone in outer edge */,
                                                                        std::array<unsigned int,2>{0u,0u}  /* no dead zone in inner edge*/},
                             attributePassThrough,
                             debug_out);
   return test_pixelDiodeTree(diode_tree,
                              std::array<PixelDiodeTree::CellIndexType,2>{384u,400u} /* single chip matrix dimension*/,
                              std::array<unsigned int,2>{2u,2u} /* number of chips*/,
                              std::array<unsigned int,2>{0u,0u} /* number of pixels composing the outer edge */,
                              std::array<unsigned int,2>{2u,2u} /* number of pixels composing the inner edge*/,
                              std::array<PixelDiodeTree::Vector2D,3> {
                                 PixelDiodeTree::Vector2D{50*Units::um,50*Units::um },    /* pitch for normal pixel */
                                 PixelDiodeTree::Vector2D{50*Units::um,50*Units::um },     /* pitch in outer edge */
                                 PixelDiodeTree::Vector2D{100*Units::um, 100*Units::um } /* pitch in inner edge */
                              },
                              debug_out
                              )  ? 0 : 1  ;
}

int test_innerBarrelTriplet(std::ostream *debug_out) {
   // PixelDiodeTree for ITk inner barrel triplet
   PixelDiodeTree diode_tree
   = createPixelDiodeTree(std::array<unsigned int,2>{1u,1u},
                          std::array<unsigned int,2>{384u*2,200u},
                          PixelDiodeTree::Vector2D{25.*Units::um,100.*Units::um},
                          std::array<std::array<unsigned int,2>, 2>{ std::array<unsigned int,2>{0,0}   /* no outer edge */,
                                                                     std::array<unsigned int,2>{0u,0u} /* no inner edge */},
                          std::array< PixelDiodeTree::Vector2D,2>{ PixelDiodeTree::Vector2D{0.*Units::um,  0.*Units::um} /* no outer edge*/,
                                                                   PixelDiodeTree::Vector2D{0.*Units::um,  0.*Units::um} /* no inner edge*/
                          },
                          std::array<std::array<unsigned int,2>, 2>{ std::array<unsigned int,2>{0u,0u} /* no dead zone in outer edge */,
                                                                     std::array<unsigned int,2>{0u,0u} /* no dead zone in inner edge*/},
                          attributePassThrough,
                          debug_out
                          );
   return test_pixelDiodeTree(diode_tree,
                              std::array<PixelDiodeTree::CellIndexType,2>{768u,200u} /* single chip matrix dimension*/,
                              std::array<unsigned int,2>{1u,1u} /* number of chips*/,
                              std::array<unsigned int,2>{0u,0u} /* number of pixels composing the outer edge */,
                              std::array<unsigned int,2>{0u,0u} /* number of pixels composing the inner edge*/,
                              std::array<PixelDiodeTree::Vector2D,3> {
                                 PixelDiodeTree::Vector2D{25*Units::um,100*Units::um }, /* pitch normal pixels*/
                                 PixelDiodeTree::Vector2D{0*Units::um,0*Units::um }, /* no outer edge */
                                 PixelDiodeTree::Vector2D{0*Units::um,0*Units::um } /* no inner edge*/
                              },
                              debug_out
                              )  ? 0 : 1  ;
}

int test_Run1(std::ostream *debug_out) {
   PixelDiodeTree diode_tree
   = createPixelDiodeTree(std::array<unsigned int,2>{2u,8u},
                          std::array<unsigned int,2>{164u,18},
                          PixelDiodeTree::Vector2D{50.*Units::um,400.*Units::um},
                          std::array<std::array<unsigned int,2>, 2>{ std::array<unsigned int,2>{0,1u}   /* no outer edge */,
                                                                     std::array<unsigned int,2>{4+2*4u,1u} /* no inner edge */},
                          std::array< PixelDiodeTree::Vector2D,2>{PixelDiodeTree::Vector2D{50.*Units::um,  500.*Units::um} /* no outer edge*/,
                                                                  PixelDiodeTree::Vector2D{50.*Units::um,  450.*Units::um}   /* no inner edge*/
                          },
                          std::array<std::array<unsigned int,2>, 2>{ std::array<unsigned int,2>{0u,0u} /* no dead zone in outer edge */,
                                                                     std::array<unsigned int,2>{4u,0u} /* 4 pixel wide dead zone in phi/local-x in the inner edge*/},
                          attributePassThrough,
                          debug_out
                          );
   return test_pixelDiodeTree(diode_tree,
                              std::array<PixelDiodeTree::CellIndexType,2>{164u,18u} /* single chip matrix dimension*/,
                              std::array<unsigned int,2>{2u,8u} /* number of chips*/,
                              std::array<unsigned int,2>{0u,1u} /* number of pixels composing the outer edge */,
                              std::array<unsigned int,2>{4+2*4u,1u} /* area with ganged pixels, number of pixels composing the inner edge*/,
                              std::array<PixelDiodeTree::Vector2D,3> {
                                 PixelDiodeTree::Vector2D{50*Units::um,400*Units::um}, /* pitch normal, outer, inner edge pixels*/
                                 PixelDiodeTree::Vector2D{50*Units::um,500*Units::um }, /* pitch in outer edge*/
                                 PixelDiodeTree::Vector2D{50*Units::um,450*Units::um } /* pitch in inner edge */
                              },
                              debug_out
                              )  ? 0 : 1  ;
}

int test_Run1b(std::ostream *debug_out) {
   PixelDiodeTree diode_tree
   = createPixelDiodeTree(std::array<unsigned int,2>{1u,8u},
                          std::array<unsigned int,2>{328u,18},
                          PixelDiodeTree::Vector2D{50.*Units::um,400.*Units::um},
                          std::array<std::array<unsigned int,2>, 2>{ std::array<unsigned int,2>{0,1u}   /* no outer edge */,
                                                                     std::array<unsigned int,2>{4+2*4u,1u} /* no inner edge */},
                          std::array< PixelDiodeTree::Vector2D,2>{PixelDiodeTree::Vector2D{50.*Units::um,  500.*Units::um} /* no outer edge*/,
                                                                  PixelDiodeTree::Vector2D{50.*Units::um,  450.*Units::um}   /* no inner edge*/
                          },
                          std::array<std::array<unsigned int,2>, 2>{ std::array<unsigned int,2>{0u,0u} /* no dead zone in outer edge */,
                                                                     std::array<unsigned int,2>{4u,0u} /* 4 pixel wide dead zone in phi/local-x in the inner edge*/},
                          attributePassThrough,
                          debug_out
                          );
   return test_pixelDiodeTree(diode_tree,
                              std::array<PixelDiodeTree::CellIndexType,2>{328u,18u} /* single chip matrix dimension*/,
                              std::array<unsigned int,2>{1u,8u} /* number of chips*/,
                              std::array<unsigned int,2>{0u,1u} /* number of pixels composing the outer edge */,
                              std::array<unsigned int,2>{4+2*4u,1u} /* area with ganged pixels, number of pixels composing the inner edge*/,
                              std::array<PixelDiodeTree::Vector2D,3> {
                                 PixelDiodeTree::Vector2D{50*Units::um,400*Units::um}, /* pitch normal, outer, inner edge pixels*/
                                 PixelDiodeTree::Vector2D{50*Units::um,500*Units::um },   /* no outer edge*/
                                 PixelDiodeTree::Vector2D{50*Units::um,450*Units::um } /* pitch in inner edge */
                              },
                              debug_out
                              )  ? 0 : 1  ;
}

int main(int argc, char **argv) {
   bool verbose=false;
   unsigned int tests=0u;
   for (int arg_i=1; arg_i<argc; ++arg_i) {
      if (strcmp(argv[arg_i], "--verbose")==0 || strcmp(argv[arg_i], "-v")==0) {
         verbose=true;
      }
      else if ((strcmp(argv[arg_i], "--test")==0 || strcmp(argv[arg_i], "-t")==0) && arg_i+1<argc) {
         while (arg_i+1 < argc && argv[arg_i+1][0]!='-') {
            ++arg_i;
            tests |= 1u<<(atoi(argv[arg_i]));
         }
      }
      else {
         std::cout << "USAGE: " << argv[0] << " [--verbose/-v] [-t 0..2 [...] ]" << std::endl;
         return 1;
      }
   }
   if (tests ==0) {
      tests=0xffffffff;
   }
   std::ostream *debug_out = (verbose ? &std::cout : nullptr);
   return 0
      | ((tests & 1u<<0) ? test_quad(debug_out) : 0)
      | ((tests & 1u<<1) ? test_innerBarrelTriplet(debug_out) : 0)
      | ((tests & 1u<<2) ? test_Run1(debug_out) : 0 )
      | ((tests & 1u<<3) ? test_Run1b(debug_out) : 0 )
      ;
}
