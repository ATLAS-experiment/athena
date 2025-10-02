/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef INDET_EMULATEDDEFECTS_H
#define INDET_EMULATEDDEFECTS_H

#include <vector>
#include <algorithm>
#include <utility>

#include "InDetReadoutGeometry/SiDetectorElementCollection.h"

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
   class EmulatedDefects : public std::vector<std::vector< typename T_ModuleHelper::KEY_TYPE> >
   {
   public:
      static constexpr bool s_needMasking = T_ModuleHelper::RANGE_FLAG_MASK != 0;
      static constexpr unsigned int MASK_FOR_COMPARISON = T_ModuleHelper::CHIP_MASK | T_ModuleHelper::ROW_MASK | T_ModuleHelper::COL_MASK;
      using KEY_TYPE = typename T_ModuleHelper::KEY_TYPE;

      EmulatedDefects(const InDetDD::SiDetectorElementCollection &detector_elements)
         : m_detectorElements(&detector_elements)
      {
         resize( m_detectorElements->size() );
      }

      /** Special greater operator which ignores the column group flag in the comparison
       */
      class greater {
      public:
         /*static (c++23) */ bool operator()(KEY_TYPE key_a, KEY_TYPE key_b) {
            if constexpr(s_needMasking) {
               return (key_a & MASK_FOR_COMPARISON)
                    > (key_b & MASK_FOR_COMPARISON);
            }
            else {
               return key_a
                    > key_b;
            }
         }
      };
      /** Convenience method to find the preceding defect.
       * @param module_defects the defect list of a particular module
       * @param key packed hardware coordinates addressing a single pixel or column group defect.
       * @return pair of the iterator of the preceding element and the end iterator
       * If there is no preceding defect then both returned iterators will be the end iterator
       */
      static std::pair< typename std::vector<KEY_TYPE>::iterator,
                        typename std::vector<KEY_TYPE>::iterator> lower_bound(std::vector<KEY_TYPE> &module_defects,
                                                                              KEY_TYPE key) {
         return std::make_pair(  std::lower_bound( module_defects.begin(),module_defects.end(), key, greater()),
                                 module_defects.end());
      }

      /** Convenience method to find the preceding defect (read only).
       * @param module_defects the defect list of a particular module
       * @param key packed hardware coordinates addressing a single pixel or column group defect.
       * @return pair of the iterator of the preceding element and the end iterator
       * If there is no preceding defect then both returned iterators will be the end iterator
       */
      static std::pair< typename std::vector<KEY_TYPE>::const_iterator,
                        typename std::vector<KEY_TYPE>::const_iterator> lower_bound(const std::vector<KEY_TYPE> &module_defects,
                                                                                    KEY_TYPE key) {
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
      std::pair< typename std::vector<KEY_TYPE>::const_iterator,
                 typename std::vector<KEY_TYPE>::const_iterator> lower_bound(unsigned int id_hash, KEY_TYPE key) const {
         const std::vector<KEY_TYPE> &module_defects = this->at(id_hash);
         return std::make_pair(  std::lower_bound( module_defects.begin(),module_defects.end(), key, greater()),
                                 module_defects.end());
      }

      /** Convenience method to get a range of keys.
       * @param key_iter a valid iterator to defects
       * @return a range of defect keys or a pair of identical keys
       * If the iterator is a range iterator return the start key and the key of the last element of this range
       * otherwise the given key twice.
       */
      static std::pair<KEY_TYPE,KEY_TYPE> getRange(typename std::vector<KEY_TYPE>::const_iterator key_iter) {
         if (T_ModuleHelper::isRangeKey(*key_iter)) {
            auto prev = key_iter;
            --prev;
            return std::make_pair(*key_iter,*prev);
         }
         else {
            return std::make_pair(*key_iter,*key_iter);
         }
      }

      /** Test whether a pixel or strip on a certain module is marked as defect.
       * @param helper utility matching this defect data to check whether a defect overlaps with pixel coordinates.
       * @param id_hash a valid ID hash
       * @param key packed hardware coordinates of the pixel to be tested.
       * @return true if this data structure contains a defect for this module which overlaps with the given pixel coordinates.
       * Will throw a range_error if the ID hash is invalid.
       */
      bool isDefect(const T_ModuleHelper &helper, unsigned int id_hash, KEY_TYPE key) const {
         auto [defect_iter, end_iter] =lower_bound(id_hash, key);
         return (defect_iter != end_iter) &&  helper.isMatchingDefect(*defect_iter,key);
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
         return isDefect(helper, id_hash, helper.hardwareCoordinates(row_idx_aka_phi, col_idx_aka_eta) );
      }

      /** Return true if the module defined by the given ID hash is defect.
       * Will throw a range_error if ID hash is invalid.
       */
      bool isModuleDefect(unsigned int id_hash) const {
         return m_moduleIsDefect.at(id_hash);
      }

      /** Mark the specified module as defect.
       */
      void setModuleDefect(unsigned int id_hash) {
         m_moduleIsDefect.at(id_hash)=true;
      }

      /** Return the detector element for the given ID hash.
       * will throw a range_error if the ID hash is invalid.
       */
      const InDetDD::SiDetectorElement &getDetectorElement(unsigned int id_hash) const {
         return *(m_detectorElements->at(id_hash));
      }
      /** Return the detector element collection.
       */
      const InDetDD::SiDetectorElementCollection &getDetectorElementCollection() const {
         return *m_detectorElements;
      }
   protected:
      /** Resize data structures for this number of modules.
       */
      void resize( std::size_t n_modules) {
         std::vector<std::vector< typename T_ModuleHelper::KEY_TYPE> >::resize(n_modules);
         m_moduleIsDefect.resize(n_modules,false);
      }
      std::vector<bool> m_moduleIsDefect;
      const InDetDD::SiDetectorElementCollection *m_detectorElements; // pointer to the detector element collection these defects are for
   };
}

#endif
