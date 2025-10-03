/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
  */
// test for the EmulatedDefectsBuilder
// will test that connected or overlapping defects and defect ranges are correctly merged
// into larger ranges.
// also test that defects which are marked as defect in the defect-range container are really identified
// as defects and cells outside this range are not identified as defects.
#undef NDEBUG
#include <utility>
#include <vector>
#include <algorithm>
#include "CxxUtils/checker_macros.h"

// not executed in MT and only changed in main by arg
bool gVerbose ATLAS_THREAD_SAFE=false;

// for compatibility with SiDetectorElement
#include "ReadoutGeometryBase/InDetDD_Defs.h"
#include "ReadoutGeometryBase/DetectorDesign.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"

#include <cstdint>
#include <limits>
#include <tuple>
#include <iostream>
#include <cstring>

#include "../src/PixelModuleHelper.h"
#include "../src/ModuleKeyHelper.h"
#include "../src/EmulatedDefects.h"
#include "../src/EmulatedDefectsBuilder.h"


template <class T_KeyHelper, class T_Coordinates>
auto makeKey(const T_KeyHelper &helper, const T_Coordinates &coordinates) {
   if constexpr(std::is_same_v<T_Coordinates, std::array<int16_t,2> >) {
      assert(coordinates[0]>=0 && coordinates[1]>=0);
      return helper.hardwareCoordinates(static_cast<unsigned int>(coordinates[0]),static_cast<unsigned int>(coordinates[1]));
   }
   if constexpr(std::is_same_v<T_Coordinates, std::array<uint16_t,2> >) {
      return helper.hardwareCoordinates(coordinates[0],coordinates[1]);
   }
   else if constexpr(std::is_same_v<T_Coordinates, std::array<int16_t,1> >) {
      assert(coordinates[0]>=0);
      return helper.hardwareCoordinates(static_cast<unsigned int>(coordinates[0]),0u);
   }
   else if constexpr(std::is_same_v<T_Coordinates, std::array<uint16_t,1> >) {
      return helper.hardwareCoordinates(coordinates[0],0u);
   }
   else if constexpr(std::is_same_v<T_Coordinates, int16_t>) {
      assert( coordinates>=0);
      return helper.hardwareCoordinates(static_cast<unsigned int>(coordinates), 0u);
   }
   else if constexpr(std::is_same_v<T_Coordinates, uint16_t>) {
      return helper.hardwareCoordinates(coordinates, 0u);
   }
}

template <class T_KeyHelper, typename T_Coordinates, typename T_DefectKey>
void addSingleDefect(const T_KeyHelper &key_helper, const T_Coordinates &coordinates, std::vector<T_DefectKey> &defects) {
   auto key = makeKey(key_helper, coordinates);
   EmulatedDefectsBuilder::insertKey<T_KeyHelper>(key_helper, defects, key, T_KeyHelper::makeDefectTypeKey(0u) );
}

template <class T_KeyHelper, typename T_Coordinates, typename T_DefectKey>
void addRangeDefect(const T_KeyHelper &key_helper,
                    const T_Coordinates &coordinates_start,
                    const T_Coordinates &coordinates_end,
                    unsigned int defect_type,
                    std::vector<T_DefectKey> &defects) {
   std::pair<typename T_KeyHelper::KEY_TYPE,typename  T_KeyHelper::KEY_TYPE> keys {
      makeKey(key_helper, coordinates_start),
      makeKey(key_helper, coordinates_end)
   };
   if (keys.first > keys.second) {
      std::swap(keys.first, keys.second);
   }
   EmulatedDefectsBuilder::insertKeyRange<T_KeyHelper>(key_helper, defects, keys, T_KeyHelper::makeDefectTypeKey(defect_type) );
}


template <class T_KeyHelper, typename T_Coordinates, typename T_DefectKey>
void addCoreColumnDefect(const T_KeyHelper &key_helper, const T_Coordinates &coordinates, std::vector<T_DefectKey> &defects) {
   auto key = T_KeyHelper::makeRangeForMask(makeKey(key_helper, coordinates),key_helper.masks()[1] );
   EmulatedDefectsBuilder::insertKeyRange<T_KeyHelper>(key_helper, defects, key, T_KeyHelper::makeDefectTypeKey(2u) );
}

template <class T_KeyHelper, typename T_DefectKey>
void dumpDefects(const T_KeyHelper &key_helper, const std::vector<T_DefectKey> &defects) {
   for (auto riter = defects.rbegin(); riter != defects.rend(); ++riter) {
      auto key = *riter;
      std::cout << (riter - defects.rbegin()) << " : " << key_helper.getChip(key) << " " << key_helper.getColumn(key) << " " << key_helper.getRow(key)
                << " (" << key_helper.getDefectType(key) << ")";
      if (key_helper.isRangeKey(key)) {
         std::cout << " .. ";
         ++riter;
         if (riter == defects.rend()) {
            std::cout << "no end key (ERROR)" <<  std::endl;
            break;
         }
         key = *riter;
         std::cout << key_helper.getChip(key) << " " << key_helper.getColumn(key) << " " << key_helper.getRow(key)
                   << " (" << key_helper.getDefectType(key) << ")";
      }
      std::cout << std::endl;
   }
}

namespace Dbg {
   void dump( const InDet::PixelModuleHelper &helper, const std::vector<unsigned int> &defects) {
      dumpDefects(helper, defects);
   }

   void dumpCoords( const std::vector<std::array< int16_t,2> > &coords) {
      for (const auto &elm : coords) {
         std::cout << elm[0] << " " << elm[1] << std::endl;
      }
   }
}

std::array<int16_t,2> makeCoordinates(unsigned int row, unsigned int col) {
   assert( row < std::numeric_limits<int16_t>::max() && col < std::numeric_limits<int16_t>::max());
   return std::array<int16_t,2>{static_cast<int16_t>(row),static_cast<int16_t>(col)};
}

template <class T_ModuleHelper>
bool isDefect(const T_ModuleHelper &helper, const std::vector<typename T_ModuleHelper::KEY_TYPE> &defects, typename T_ModuleHelper::KEY_TYPE key)  {
   auto [defect_iter, end_iter] =  InDet::EmulatedDefects<T_ModuleHelper>::lower_bound(defects, key);
   return (defect_iter != end_iter) &&  helper.isMatchingDefect(*defect_iter,key);
}


void singleInsertTest(const InDet::PixelModuleHelper &helper,
          const std::vector< std::array<int16_t,2> > &insert,
          const std::vector< InDet::PixelModuleHelper::KEY_TYPE> &result) {
   std::vector<InDet::PixelModuleHelper::KEY_TYPE> defects;
   for (const auto &coord : insert) {
      if (gVerbose)  {
         std::cout << "insert " << coord[0] << " " << coord[1] << std::endl;
      }
      addSingleDefect( helper, coord, defects );
   }
   if (gVerbose) {
      dumpDefects(helper, defects);
   }
   assert( defects == result);
   for (const auto &coord : insert) {
      InDet::PixelModuleHelper::KEY_TYPE key = makeKey(helper, coord);
      assert(isDefect(helper, defects, key));
   }
}

void rangeInsertTest(const InDet::PixelModuleHelper &helper,
                     const std::vector< std::tuple< std::array<int16_t,2>, std::array<int16_t,2>, unsigned int > > &insert,
                     const std::vector< InDet::PixelModuleHelper::KEY_TYPE> &result) {
   std::vector<InDet::PixelModuleHelper::KEY_TYPE> defects;
   for (const auto &coord : insert) {
      if (gVerbose)  {
         if (std::get<0>(coord) == std::get<1>(coord) ) {
            std::cout << "insert " << std::get<0>(coord)[0] << " " << std::get<0>(coord)[1] << " type " << std::get<2>(coord) << std::endl;
         }
         else {
            std::cout << "insert " << std::get<0>(coord)[0] << " " << std::get<0>(coord)[1]
                      << " .. " << std::get<1>(coord)[0] << " " << std::get<1>(coord)[1]
                      << " type " << std::get<2>(coord) << std::endl;
         }
      }
      addRangeDefect( helper, std::get<0>(coord), std::get<1>(coord), std::get<2>(coord), defects );
   }
   if (gVerbose) {
      dumpDefects(helper, defects);
   }
   assert( defects == result);

   // test that coords inside an inserted range are marked defect and that coords just outside those ranges are not defect
   // (provided they are not enclosed by some other range)
   for (const auto &coord : insert) {
      {
         InDet::PixelModuleHelper::KEY_TYPE key = makeKey(helper, std::get<0>(coord));
         assert(isDefect(helper, defects, key));
      }
      {
         InDet::PixelModuleHelper::KEY_TYPE key = makeKey(helper, std::get<1>(coord));
         assert(isDefect(helper, defects, key));
      }

      // compute coordinates just outside this range
      std::array<std::array<int16_t,2>, 2> outside_range{
         std::get<0>(coord),
         std::get<1>(coord)
      };
      if (outside_range[0][0]==0) {
         if (outside_range[0][1]>0) {
            outside_range[0][0]=helper.rowsPerCircuit()-1;
            outside_range[0][1]-=1;
         }
      }
      else {
         outside_range[0][0]-=1;
      }
      outside_range[1][0]+=1;
      if (static_cast<unsigned int>(outside_range[1][0])==helper.rowsPerCircuit()) {
         if (static_cast<unsigned int>(outside_range[1][1])<helper.columnsPerCircuit()) {
            outside_range[1][0]=0;
            outside_range[1][1]+=1;
         }
         else {
            // otherwise ignore
            outside_range[1][0]-=1;
         }
      }
      for (auto test_coord : outside_range) {
         bool in_a_range=false;
         InDet::PixelModuleHelper::KEY_TYPE key = makeKey(helper, test_coord);
         for (const auto &other : insert) {
            InDet::PixelModuleHelper::KEY_TYPE start_key = makeKey(helper, std::get<0>(other));
            InDet::PixelModuleHelper::KEY_TYPE end_key = makeKey(helper, std::get<1>(other));
            in_a_range |= (key>=start_key && key<=end_key);
            if (in_a_range) break;
         }
         assert(isDefect(helper, defects, key) == in_a_range);
      }
   }

}



template <typename T>
std::vector<T> extent(bool at_start, const T &start_elm, bool at_end, const T &end_elm, unsigned int option, std::vector<T> &&in) {
   if (option & 1) {
      in.insert( (at_start ? in.begin() : in.end()), start_elm);
   }
   if (option & 2) {
      in.insert( (at_end ? in.end() : in.begin()), end_elm);
   }
   return in;
}

std::array<int16_t,2> shiftRow( const InDet::PixelModuleHelper &helper,
                                int row_offset,
                                std::array<int16_t,2> coordinates
                                ) {
   std::array<int16_t,2> tmp { static_cast<int16_t>(coordinates[0] + static_cast<int16_t>(row_offset)) ,coordinates[1] };
   if (tmp[0]<0) {
      tmp[0]+=helper.rowsPerCircuit();
      tmp[1]-=1;
   }
   else if (static_cast<unsigned int>(tmp[0])>=helper.rowsPerCircuit()) {
      tmp[0]-=helper.rowsPerCircuit();
      tmp[1]+=1;
   }
   return tmp;
}

int main(int argc, char **argv) {
   for (unsigned int arg_i=1; arg_i<static_cast<unsigned int>(argc); ++arg_i) {
      if (strcmp(argv[arg_i],"-v")==0) {
         gVerbose=true;
      }
      else {
         std::cerr << "ERROR unhandled argument " << argv[arg_i] << "\n"
                   << "USAGE " << argv[0] << " [-v]" << std::endl;
         return 0;
      }
   }
   // create something does not have to be correct but the number of circuits and cells per column/row matters
   InDetDD::PixelModuleDesign pixel_design(.2,
                                           2 /* circuitsPerColumn*/,
                                           2 /* circuitsPerRow*/,
                                           400 /*cellColumnsPerCircuit*/, 
                                           384 /*cellRowsPerCircuit*/,
                                           400 /*diodeColumnsPerCircuit*/,
                                           384 /*diodeRowsPerCircuit*/,
                                           InDetDD::PixelDiodeTree(InDetDD::PixelDiodeTree::Vector2D(400,384)),
                                           InDetDD::electrons,
                                           -1 /*readoutSide*/,
                                           false /*is3D*/,
                                           InDetDD::Undefined /* DetectorType */,
                                           InDetDD::PixelReadoutTechnology{});

   
   InDet::PixelModuleHelper pixel_key_helper(pixel_design);

   using PixelKeyType = InDet::PixelModuleHelper::KEY_TYPE;
   std::vector<PixelKeyType> pixel_defects;

   // simple single insertion tests
   for (unsigned int option=0; option<4; ++option) {
      singleInsertTest(pixel_key_helper,
                       extent(true, makeCoordinates(56u,0u),
                              true, makeCoordinates(63u,0u),
                              option,
                              std::vector< std::array<int16_t,2> >{
                                 makeCoordinates(59u,0u),
                                 makeCoordinates(60u,0u),
                                 makeCoordinates(58u,0u)
                              } ),
                       extent(false, pixel_key_helper.makeKey(false,0,0,56),
                              false, pixel_key_helper.makeKey(false,0,0,63),
                              option,
                              std::vector< InDet::PixelModuleHelper::KEY_TYPE > {
                                 pixel_key_helper.makeKey(false,0,0,60),
                                 pixel_key_helper.makeKey(true,0,0,58)
                              }
                              ));
      singleInsertTest(pixel_key_helper,
                       extent(true, makeCoordinates(56u,0u),
                              true, makeCoordinates(63u,0u),
                              option,
                              std::vector< std::array<int16_t,2> >{
                                 makeCoordinates(59u,0u),
                                 makeCoordinates(60u,0u),
                                 makeCoordinates(61u,0u)
                              } ),
                       extent(false, pixel_key_helper.makeKey(false,0,0,56),
                              false, pixel_key_helper.makeKey(false,0,0,63),
                              option,
                              std::vector< InDet::PixelModuleHelper::KEY_TYPE > {
                                 pixel_key_helper.makeKey(false,0,0,61),
                                 pixel_key_helper.makeKey(true,0,0,59)
                              }
                              ));
   }

   {
      // test single defect insertion
      // start from different base coordinates : inside a cricuit
      // and at the edge of a circuit at the top row
   std::array< std::array<int16_t,2>,2 > bases {
      makeCoordinates(pixel_key_helper.rowsPerCircuit()-2,1u),
      makeCoordinates(pixel_key_helper.rowsPerCircuit()-2,pixel_key_helper.columnsPerCircuit()-1),
   };
   for (auto elm : bases) {
      // shift the base by some rows up 
      for (int offset_i=0; offset_i<3; ++offset_i) {
         auto base = shiftRow(pixel_key_helper, offset_i, elm);

         // row offset for the inserted defects
         std::vector<std::array<int,3> > shifts{
            std::array<int,3>{0,1,2},
            std::array<int,3>{0,-1,1},
            std::array<int,3>{0,1,-1}
         };
         for (auto shift : shifts ) {
            for (unsigned int option=0; option<4; ++option) {

               std::array< std::array<int16_t,2> ,5 > pos {
                  shiftRow( pixel_key_helper, -3, base),
                  shiftRow( pixel_key_helper, shift[0], base),
                  shiftRow( pixel_key_helper, shift[1], base),
                  shiftRow( pixel_key_helper, shift[2], base),
                  shiftRow( pixel_key_helper, +5, base)
               };

               std::array<InDet::PixelModuleHelper::KEY_TYPE,5> key;
               unsigned int idx=0;
               for (const auto &a_pos : pos) {
                  key[idx++] = pixel_key_helper.hardwareCoordinates(a_pos[0],a_pos[1]);
               }
               std::sort( key.begin(), key.end(),[](const InDet::PixelModuleHelper::KEY_TYPE &a,
                                                    const InDet::PixelModuleHelper::KEY_TYPE &b) {
                  return b<a;
               });

               singleInsertTest(pixel_key_helper,
                                extent(true, pos[0], true, pos[4], option,
                                       std::vector< std::array<int16_t,2> >{pos[1],pos[2],pos[3]}),
                                extent(false, key[4], false, key[0],
                                       option,
                                       std::vector< InDet::PixelModuleHelper::KEY_TYPE > { key[1], InDet::PixelModuleHelper::makeRangeKey(key[3])}
                                       ));
            }
         }
      }
   }
   }

   {
   // add range after/before ramge
   // add range after/before single
   // add iso range, iso range, single dist.
   // add iso range, iso range, range dist.

   // base defect coordinate at the bottom or top row
   std::vector< std::array<int16_t,2> > bases {
      makeCoordinates(0,5u),
      makeCoordinates(pixel_key_helper.rowsPerCircuit()-3,1u),
      //      makeCoordinates(pixel_key_helper.rowsPerCircuit()-2,pixel_key_helper.columnsPerCircuit()-1),
   };
   for (auto elm : bases) {
      // sfhit base coordinates by some rows up
      for (int offset_i=0; offset_i<3; ++offset_i) {
         auto base = shiftRow(pixel_key_helper, offset_i, elm);

         // defintiions of range insertions
         std::vector< std::vector< std::pair<unsigned int, int> > > tests {
            std::vector<std::pair<unsigned int, int> >{ std::make_pair(3u,0), std::make_pair(3u,3)},     // range after range
            std::vector<std::pair<unsigned int, int> >{ std::make_pair(3u,0), std::make_pair(3u,-3)},    // range before range
            std::vector<std::pair<unsigned int, int> >{ std::make_pair(1u,0), std::make_pair(3u,1)},     // range after single
            std::vector<std::pair<unsigned int, int> >{ std::make_pair(1u,0), std::make_pair(3u,-3)},    // range before single
            std::vector<std::pair<unsigned int, int> >{ std::make_pair(3u,0),
                                                        std::make_pair(3u,6),
                                                        std::make_pair(3u,3)},     // iso range after iso range, fill with range
            std::vector<std::pair<unsigned int, int> >{ std::make_pair(6u,0),
                                                        std::make_pair(3u,1)},
            std::vector<std::pair<unsigned int, int> >{ std::make_pair(3u,6),
                                                        std::make_pair(3u,0),
                                                        std::make_pair(3u,3)},     // iso range before iso range, fill with range
            std::vector<std::pair<unsigned int, int> >{ std::make_pair(3u,0),
                                                        std::make_pair(3u,4),
                                                        std::make_pair(1u,3)}     // iso range after iso range, fill with single

         };

         // create some addition range insertion tests:
         // test range merging
         // iso single + iso ranges, envelop range starting before, at, after single or range
         // iso single + iso ranges, envelop range ending before, at, after single or range
         // full envelop
         std::vector<std::pair<unsigned int, int> > enclosed{
               std::make_pair(3u,2),
               std::make_pair(1u,6),
               std::make_pair(3u,8),
               std::make_pair(1u,12),
               std::make_pair(3u,14),
               std::make_pair(1u,18)};
         std::array<std::pair<unsigned int, unsigned int>,4 > subrange {
            std::make_pair(0,0),   // range single
            std::make_pair(1,0),   // single single
            std::make_pair(0,1),   // range range
            std::make_pair(1,1)    // single range
         };

         std::array<int,4> offset {-2,-1,0,1};
         for (auto an_offset_start : offset) {
            for (auto an_offset_end : offset) {
               for (auto a_sub_range : subrange) {
                  auto tmp = enclosed;
                  for (unsigned int i=0; i<a_sub_range.first; ++i) {
                     tmp.erase(tmp.begin());
                  }
                  for (unsigned int i=0; i<a_sub_range.second; ++i) {
                     if (!tmp.empty()) {
                        tmp.erase(tmp.end()-1);
                     }
                  }
                  unsigned int first_pos = tmp.front().second +an_offset_start;
                  unsigned int last_pos = tmp.back().second + tmp.back().first -1 - an_offset_end;
                  tmp.push_back( std::make_pair( last_pos - first_pos +1, first_pos ) );
                  if (gVerbose) {
                     std::cout << " Test_range offset : " << an_offset_start << " " << (-an_offset_end)
                               << " range[" << a_sub_range.first << ":-" << a_sub_range.second << "] :";
                     for (auto elm : tmp) {
                        std::cout << " [" << elm.second << "," << (elm.second + elm.first-1) << "]";
                     }
                     std::cout << std::endl;
                  }
                  tests.emplace_back(std::move(tmp));
               }
            }
         }

         auto extendToRowEnd=[&pixel_key_helper](InDet::PixelModuleHelper::KEY_TYPE a_key) {
            if (pixel_key_helper.getRow(a_key)==0u && pixel_key_helper.getColumn(a_key)>0) {
               return pixel_key_helper.makeKey(false, pixel_key_helper.getChip(a_key),
                                               pixel_key_helper.getColumn(a_key)-1,
                                               pixel_key_helper.rowsPerCircuit()) | InDet::PixelModuleHelper::getDefectTypeComponent(a_key);
            }
            else {
               return a_key;
            }
         };
         auto extendToNextColumn=[&pixel_key_helper](InDet::PixelModuleHelper::KEY_TYPE a_key) {
            if (pixel_key_helper.getRow(a_key)==pixel_key_helper.rowsPerCircuit()-1 && pixel_key_helper.getColumn(a_key)+1<pixel_key_helper.columns()) {
               return pixel_key_helper.makeKey(false, pixel_key_helper.getChip(a_key),
                                               pixel_key_helper.getColumn(a_key),
                                               InDet::PixelModuleHelper::ROW_MASK) | InDet::PixelModuleHelper::getDefectTypeComponent(a_key);
            }
            else {
               return a_key;
            }
         };

         // run the range insertion tests
         // optionally add defects before and/or after the ranges that are inserted separated by a no-defect area.
         for (const auto &a_test : tests ) {
            for (unsigned int option=0; option<4; ++option) {
               int min_shift=std::numeric_limits<int>::max();
               int max_shift=std::numeric_limits<int>::min();
               for (const auto &elm : a_test ) {
                  min_shift = std::min(min_shift, elm.second);
                  max_shift = std::max(max_shift, static_cast<int>(elm.second + elm.first-1));
               }
               std::vector< std::tuple<std::array<int16_t,2>, std::array<int16_t,2>, unsigned int>  > pos;
               pos.reserve( a_test.size()+2);
               pos.push_back( std::make_tuple(shiftRow( pixel_key_helper, min_shift-2, base),
                                              shiftRow( pixel_key_helper, min_shift-2, base),
                                              1)
                              );
               if (gVerbose) {
                  std::cout << "DEBUG add position " << std::get<0>(pos.back())[0] << " "  << std::get<0>(pos.back())[1]
                            << " .. " << std::get<1>(pos.back())[0] << " "  << std::get<1>(pos.back())[1]
                            << " (" << std::get<2>(pos.back())
                            << std::endl;
               }
               unsigned int max_defect_type =0;
               unsigned int min_defect_type =std::numeric_limits<unsigned int>::max();
               for (const auto &elm : a_test ) {
                  //                  assert( elm.first < 4u); // other wise it cannot be used as a defefect type in this toy data
                  unsigned int defect_type = std::min(3u,elm.first);
                  pos.push_back( std::make_tuple(shiftRow( pixel_key_helper, elm.second, base),
                                                 shiftRow( pixel_key_helper, elm.second + elm.first-1, base),
                                                 defect_type )
                                       );
                  if (gVerbose) {
                     std::cout << "DEBUG add position " << std::get<0>(pos.back())[0] << " "  << std::get<0>(pos.back())[1]
                               << " .. " << std::get<1>(pos.back())[0] << " "  << std::get<1>(pos.back())[1]
                               << " (" << std::get<2>(pos.back()) << ")"
                               << std::endl;
                  }
                  max_defect_type = std::max(max_defect_type, defect_type);
                  min_defect_type = std::min(min_defect_type, defect_type);
               }
               pos.push_back( std::make_tuple(shiftRow( pixel_key_helper, max_shift+2, base),
                                              shiftRow( pixel_key_helper, max_shift+2, base),
                                              1)
                              );
               if (gVerbose) {
                  std::cout << "DEBUG add position " << std::get<0>(pos.back())[0] << " "  << std::get<0>(pos.back())[1]
                            << " .. " << std::get<1>(pos.back())[0] << " "  << std::get<1>(pos.back())[1]
                            << " (" << std::get<2>(pos.back())
                            << std::endl;
               }

               std::vector<InDet::PixelModuleHelper::KEY_TYPE> key;
               key.reserve(pos.size()*2);
               for (const auto &elm : pos) {
                  auto [pos_start_ref, pos_end_ref, defect_type ] = elm;
                  std::array<int16_t,2> pos_start = pos_start_ref;
                  std::array<int16_t,2> pos_end = pos_end_ref;
                  key.push_back(pixel_key_helper.hardwareCoordinates(pos_start[0],pos_start[1]) | InDet::PixelModuleHelper::makeDefectTypeKey(defect_type));
                  if (gVerbose) {
                     std::cout << "DEBUG add key " << pos_start[0] << " " << pos_start[1] << " ->  " << std::hex << key.back() << std::dec << std::endl;
                  }
                  key.push_back(pixel_key_helper.hardwareCoordinates(pos_end[0],pos_end[1]) | InDet::PixelModuleHelper::makeDefectTypeKey(defect_type));
                  // if (pixel_key_helper.getRow(key.back())==0u && pixel_key_helper.getColumn(key.back())>0) {
                  //    key.back() = pixel_key_helper.makeKey(false, pixel_key_helper.getChip(key.back()),
                  //                                          pixel_key_helper.getColumn(key.back())-1,
                  //                                          pixel_key_helper.rowsPerCircuit());
                  // }
                  if (gVerbose) {
                     std::cout << "DEBUG add key " << pos_end[0] << " " << pos_end[1] << " ->  " << std::hex << key.back() << std::dec << std::endl;
                  }
               }
               std::sort( key.begin(), key.end(),[](const InDet::PixelModuleHelper::KEY_TYPE &a,
                                                    const InDet::PixelModuleHelper::KEY_TYPE &b) {
                  return InDet::PixelModuleHelper::makeBaseKey(b)<InDet::PixelModuleHelper::makeBaseKey(a)
                     || (   InDet::PixelModuleHelper::makeBaseKey(b)==InDet::PixelModuleHelper::makeBaseKey(a)
                         && InDet::PixelModuleHelper::getDefectTypeComponent(b) < InDet::PixelModuleHelper::getDefectTypeComponent(a));
               });

               std::array<  std::tuple<std::array<int16_t,2>, std::array<int16_t,2>, unsigned int>,2 > outer_pos;
               outer_pos[0] = pos[0];
               outer_pos[1] = pos[pos.size()-1];
               pos.erase(pos.begin());
               pos.erase(pos.end()-1);

               //               bool insert_single_start = key[key.size()-3] == key[key.size()-4];
               bool insert_single_end = key[2] == key[3];
               InDet::PixelModuleHelper::KEY_TYPE end_key   = key[key.size()-3];
               InDet::PixelModuleHelper::KEY_TYPE start_key = key[2];
               for (unsigned int key_i=2;
                       key_i<key.size()
                    && InDet::PixelModuleHelper::makeBaseKey(key[key_i]) == InDet::PixelModuleHelper::makeBaseKey(start_key);
                    ++key_i) {
                  if (InDet::PixelModuleHelper::getDefectTypeComponent(start_key) < InDet::PixelModuleHelper::getDefectTypeComponent(key[key_i])) {
                     start_key = key[key_i];
                  }
               }
               for (unsigned int key_i=key.size()-3;
                       key_i-->0u
                    && InDet::PixelModuleHelper::makeBaseKey(key[key_i]) == InDet::PixelModuleHelper::makeBaseKey(end_key);) {
                  if (InDet::PixelModuleHelper::getDefectTypeComponent(end_key) < InDet::PixelModuleHelper::getDefectTypeComponent(key[key_i])) {
                     end_key = key[key_i];
                  }
               }
               rangeInsertTest(pixel_key_helper,
                               extent(true, outer_pos[0],
                                      true, outer_pos[1],
                                      option,
                                      std::move(pos)),
                               extent(false, key[key.size()-1],
                                      false, key[0],
                                      option,
                                      std::vector< InDet::PixelModuleHelper::KEY_TYPE > {
                                         ( insert_single_end ? start_key : extendToNextColumn(start_key))
                                         ,
                                         InDet::PixelModuleHelper::makeRangeKey(extendToRowEnd(end_key)) }
                                      ));
            }
         }
      }
   }

   }

   return 0;
}
