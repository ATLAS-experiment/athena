/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
  */
/* Dear emacs, this is -*-c++-*- */
#ifndef INDET_EMULATEDDEFECTS_H
#define INDET_EMULATEDDEFECTS_H

#include <vector>
#include <algorithm>
#include <utility>

namespace InDetDD {
   class SiDetectorElementCollection;
}

namespace InDet {
   /** Data structure do mark e.g. pixel  defects for a list of modules.
    * Contains a list of ordered hardware coordinates marking defect "pixels" for each module
    * identified by an ID hash (consecutive integer). Defects can be either individual
    * pixel defects or column group defects (group of 8 consecutive columns). The defects
    * are expected to be ordered in descending order (excluding the column group flag), such
    * that std::lower_bound together with the supplied greater comparator will yield
    * the defect which is either identical, has a smaller column address or smaller row address.
    * If the elements are in a different order the results will be undefined.
    */
   template <class T_ModuleHelper>
   class EmulatedDefects : public std::vector<std::vector<unsigned int> >
   {
   public:
      static constexpr unsigned int MASK_FOR_COMPARISON = T_ModuleHelper::CHIP_MASK | T_ModuleHelper::ROW_MASK | T_ModuleHelper::COL_MASK;
      /** Special greater operator which ignores the column group flag in the comparison
       */
      class greater {
      public:
         /*static (c++23) */ bool operator()(unsigned int key_a, unsigned int key_b) {
            return (key_a & MASK_FOR_COMPARISON)
                 > (key_b & MASK_FOR_COMPARISON);
         }
      };
      /** Convenience method to find the preceding defect.
       * @param module_defects the defect list of a particular module
       * @param key packed hardware coordinates addressing a single pixel or column group defect.
       * @return pair of the iterator of the preceding element and the end iterator
       * If there is no preceding defect then both returned iterators will be the end iterator
       */
      static std::pair< std::vector<unsigned int>::iterator, std::vector<unsigned int>::iterator> lower_bound(std::vector<unsigned int> &module_defects,
                                                                                                              unsigned int key) {
         return std::make_pair(  std::lower_bound( module_defects.begin(),module_defects.end(), key, greater()),
                                 module_defects.end());
      }

      /** Convenience method to find the preceding defect.
       * @param id_hash a valid ID hash of a module.
       * @param key packed hardware coordinates addressing a single pixel or column group defect.
       * @return pair of the iterator of the preceding element and the end iterator of the corresponding module
       * If there is no preceding defect then both returned iterators will be the end iterator. Will
       * throw a range_error if the ID hash is invalid.
       */
      std::pair< std::vector<unsigned int>::const_iterator, std::vector<unsigned int>::const_iterator> lower_bound(unsigned int id_hash, unsigned int key) const {
         const std::vector<unsigned int> &module_defects = this->at(id_hash);
         return std::make_pair(  std::lower_bound( module_defects.begin(),module_defects.end(), key, greater()),
                                 module_defects.end());
      }

      /** Test whether a pixel on a certain module is marked as defect.
       * @param helper utility matching this defect data to check whether a defect overlaps with pixel coordinates.
       * @param id_hash a valid ID hash
       * @param key packed hardware coordinates of the pixel to be tested.
       * @return true if this data structure contains a defect for this module which overlaps with the given pixel coordinates.
       * Will throw a range_error if the ID hash is invalid.
       */
      bool isDefect(const T_ModuleHelper &helper, unsigned int id_hash, unsigned int key) const {
         auto [defect_iter, end_iter] =lower_bound(id_hash, key);
         return (defect_iter != end_iter && helper.isSameDefectWithGroups( *defect_iter, key,helper.columnGroupRowColumnMask()) );
      }

      /** Test whether a pixel on a certain module is marked as defect.
       * @param helper utility matching this defect data to check whether a defect overlaps with pixel coordinates.
       * @param id_hash a valid ID hash
       * @param row_idx_aka_phi the offline row aka phi index of a pixel
       * @param col_idx_aka_eta the offline column aka eta index of a pixel
       * @return true if this data structure contains a defect for this module which overlaps with the given pixel coordinates
       * Will throw a range_error if the ID hash is invalid.
       */
      bool isDefect(const T_ModuleHelper &helper, unsigned int id_hash, unsigned int row_idx_aka_phi, unsigned int col_idx_aka_eta) const {
         unsigned int key = helper.hardwareCoordinates(row_idx_aka_phi, col_idx_aka_eta);
         return isDefect(helper, id_hash, key);
      }

      const InDetDD::SiDetectorElementCollection *m_detectorElements = nullptr; // pointer to the detector element collection these defects are for
   };
}

#endif
