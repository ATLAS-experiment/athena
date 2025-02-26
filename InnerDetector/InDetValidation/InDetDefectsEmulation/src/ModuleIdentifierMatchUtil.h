/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef INDET_MODULEIDENTIFIERMATCHUTIL_H
#define INDET_MODULEIDENTIFIERMATCHUTIL_H

#include "Identifier/Identifier.h"

namespace InDet {

   namespace ModuleIdentifierMatchUtil {
      namespace detail {
         template <typename T_ID>
         concept IdentifierHelperWithSideConecpt = requires(T_ID id_helper, Identifier id) { id_helper.side(id); };

         // helper to to return the side part of an identifier if available or zero
         template <class T_ID>
         inline int getZeroOrSide(const T_ID &id_helper,const Identifier &id) {
            if constexpr(IdentifierHelperWithSideConecpt<decltype(id_helper)>) {
               return id_helper.side(id);
            }
            else {
               return 0;
            }
         }
      }

      // helper templates to detect whether a type has the method columns or cells
      template <typename T_ModuleDesign>
      concept HasColumns = requires(T_ModuleDesign design) { design.columns(); };
      template <typename T_ModuleDesign>
      concept HasCells = requires(T_ModuleDesign design) { design.cells(); };

      template <typename T_ModuleDesign>
      concept ModuleDesignConcept = HasColumns<T_ModuleDesign> || HasCells<T_ModuleDesign>;

      // Get a unique module type identifier which is either based on the number of
      // columns (for pixel) or number of cells (or strips; for strips)
      template <ModuleDesignConcept T_ModuleDesign>
      int getColumnsOrCells(const T_ModuleDesign &moduleDesign) {
         if constexpr(HasColumns<T_ModuleDesign>) {
            return moduleDesign.columns();
         }
         else {
            return moduleDesign.cells();
         }
      }


      /** The identifier parts used for identifier matching
       * kAllRows denotes the element which contains a flag (0, 1), to specify
       * whether a pattern should match all modules which are connected to the same side
       * of a physical sensor
       */
      enum EModulePatternPart {
         kBarrelEndcapSelectRange=0,
         kLayerRange=2,
         kEtaRange=4,
         kPhiRange=6,
         kColumnStripRange=8,
         kLength=10,
         kSideRange=12,
         kAllRows=14,
         kNParts=15
      };

      using ModuleData_t = std::array< int, ModuleIdentifierMatchUtil::kAllRows/2 >;

      /** Test whether the given 2D vector has the expected number of elements, and correct ordering
       * @tparam test_order if true test element order otherwise only test element count
       * @tparam T element type
       * @param arr 2D array to be tested
       * @param n_expected number of elements of in each element vector of the top-level vector
       * @param test_n number of elements (not pairs) to test when verifying the correct order of element pairs
       * @return true if arr passes the tests
       * Test whether each element vector of arr, which is a vector, contains the same number of elements n_expected.
       * if test_order is true also test whether the first test_n/2 element pairs of each element vector of arr
       * are in ascending order i.e. arr[..][2*i] < arr[..][2*i+1] for all i..n_test/2
       */
      template <bool test_order, typename T>
      inline bool verifyElementCountAndOrder(const std::vector<std::vector<T> > &arr, std::size_t n_expected, [[maybe_unused]] std::size_t test_n=0) {
         for (const auto &sub_arr : arr) {
            if (sub_arr.size() != n_expected) return false;
            if (test_n%2 != 0 || test_n>=n_expected) return false;
            if constexpr(test_order) {
               for (unsigned int i=0; i<test_n; i+=2) {
                  if (sub_arr.at(i)>sub_arr.at(i+1)) return false;
               }
            }
         }
         return true;
      }

      /** Verify whether a list of module identifier patterns is consistent.
       * The function tests whether the given 2D vector has the correct number of elements and whether the element pairs
       * which represent ranges have the expected ascending order.
       * See @ref verifyElementCountAndOrder
       */
      inline bool verifyModulePatternList(const std::vector<std::vector<int> > &arr) {
         return verifyElementCountAndOrder<true,int>(arr,kNParts, kAllRows);
      }

      /** test whether the 2D vector arr has the expected number of elements.
       * See @ref verifyElementCountAndOrder
       */
      template <typename T>
      inline bool verifyElementCount(const std::vector<std::vector<T> > &arr, std::size_t n_expected) {
         return verifyElementCountAndOrder<false,T>(arr,n_expected, 0);
      }

      /** Convenience function to set the specified element in the given array
       * @param part specifies the element to set
       * @param value the value to copy to the array
       * @param dest the array to which the value is copied
       */
      inline void setModuleData(EModulePatternPart part, int value, std::array< int, kAllRows/2 > &dest) {
         assert( part%2==0 && static_cast<unsigned int>(part/2)<dest.size());
         dest[part/2] = value;
      }

      /** Convenience function to extract various identifier parts and set the given array
       * @param id_helper the ID helper e.g. PixelID, SCT_ID to extract various parts from the given identifier
       * @param identifier the identifier from which the various parts are extracted
       * @param n_columns_or_strips either the number of columns (e.g. for pixel) or strips
       * @param module_data the array which will be filled with the identifier parts
       * Fill an array with identifier parts which can be used to match module identifiers.
       */
      template <class T_ID, class T_ModuleDesign>
      inline void setModuleData(const T_ID &id_helper,
                                const Identifier &identifier,
                                const T_ModuleDesign &module_design,
                                ModuleData_t &module_data) {
         setModuleData(kBarrelEndcapSelectRange, id_helper.barrel_ec(identifier), module_data);
         setModuleData(kLayerRange, id_helper.layer_disk(identifier), module_data);
         setModuleData(kEtaRange, id_helper.eta_module(identifier), module_data);
         setModuleData(kPhiRange, id_helper.phi_module(identifier), module_data);
         setModuleData(kColumnStripRange, getColumnsOrCells(module_design), module_data);
         setModuleData(kLength, static_cast<int>(module_design.length()*1e3), module_data);
         setModuleData(kSideRange, detail::getZeroOrSide(id_helper,identifier),module_data);
      }

      /** Test whether an identifier, which is split into various parts, matches some of the given patterns.
       * @param module_pattern an array which contains ranges for various identifier parts
       * @param module_data an array which contains various parts of an identifier
       * @param module_pattern_idx an output vector which will be filled with the indices of matching patterns
       * Will test all "patterns" in module pattern whether they match the given identifier parts.
       * The patterns contain ranges of identifier parts (start and end of the range are inclusive). For a
       * pattern to match, all identifier parts have to be within the allowed range of a pattern.
       */
      inline void moduleMatches( const std::vector<std::vector<int> > &module_pattern,
                          const ModuleData_t &module_data,
                          std::vector<unsigned int> &module_pattern_idx)  {
         module_pattern_idx.clear();
         for (unsigned int pattern_i=0u; pattern_i<module_pattern.size(); ++pattern_i) {
            const std::vector<int> &pattern = module_pattern[pattern_i];
            unsigned int range_i=0;
            bool match=true;
            for (int module_value : module_data) {
               assert( range_i+1 < pattern.size());
               if (module_value<pattern[range_i] || module_value>pattern[range_i+1]) {
                  match=false;
                  break;
               }
               range_i+=2;
            }
            if (match) {
               module_pattern_idx.push_back(pattern_i);
            }
         }
      }

      /** Test whether the given pattern matches identifiers which refer to either side of a sensor
       * @param a_module_pattern a module identifier pattern
       * @return true if the side range only contains one element
       * A sensor might be connected to multiple modules where the modules might be located on different
       * sides of a sensor. The side range may contain one side only or both sides. In the latter
       * case this function returns true.
       */
      inline bool matchesBothSides(const std::vector<int> &a_module_pattern) {
         assert( ModuleIdentifierMatchUtil::kSideRange+1 < a_module_pattern.size() );
         return    a_module_pattern[ModuleIdentifierMatchUtil::kSideRange]
                != a_module_pattern[ModuleIdentifierMatchUtil::kSideRange+1];
      }
      /** Test whether the given pattern matches the given side.
       */
      inline bool isSideMatching(const std::vector<int> &a_module_pattern, int side) {
         assert( ModuleIdentifierMatchUtil::kSideRange+1 < a_module_pattern.size() );
         return    side >= a_module_pattern[ModuleIdentifierMatchUtil::kSideRange]
                && side <= a_module_pattern[ModuleIdentifierMatchUtil::kSideRange+1];
      }

      /** Test whether a pattern matches all module rows of a sensor or just a single row.
       * @param a_module_pattern a module identifier pattern
       * @return true if the pattern matches all module rows connected to a physical sensor.
       * A sensor might be connected to multiple modules where the modules might be
       * connected to the same sensor side in multiple rows. If the given pattern
       * matches all rows, this function returns true.
      */
      inline bool matchesAllModuleRowsOfSensor(const std::vector<int> &a_module_pattern) {
         assert(ModuleIdentifierMatchUtil::kAllRows < a_module_pattern.size());
         return a_module_pattern[ModuleIdentifierMatchUtil::kAllRows] != 0;
      }
   }
}
#endif
