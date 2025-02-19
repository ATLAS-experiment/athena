/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef CONNECTEDMODULESUTIL_H
#define CONNECTEDMODULESUTIL_H

#include "InDetReadoutGeometry/SiDetectorElement.h"

#include <vector>
#include <ranges>
#include <unordered_map>
#include <functional>

#include "ModuleIdentifierMatchUtil.h"
namespace InDet {
namespace ConnectedModulesUtil {

   /** create a map between detector elements and all such elements connected the same sensor
    * @param id_helper the ID identifier helper e.g. SCT_ID
    * @param det_ele_coll collection of all detector elements e.g. all Strip detector elements
    * @return the created map.
    * Create a map between detector elements and all detector elements which are connected to
    * either side of the same physical sensor.
    */
   template <class T_ID>
   std::unordered_multimap<unsigned int, unsigned int>
   inline createModuleConnectionMap(const T_ID &id_helper,
                             const InDetDD::SiDetectorElementCollection &det_ele_coll) {
      std::unordered_multimap<unsigned int, unsigned int> connected_modules;
      std::size_t det_ele_coll_sz = det_ele_coll.size();
      for (unsigned int module_i=0u;module_i<det_ele_coll_sz;++module_i) {
         const InDetDD::SiDetectorElement *det_ele = det_ele_coll[module_i];
         const InDetDD::SiDetectorElement *side_det_ele = det_ele;
         for (unsigned int side_i=0; side_i<2; ++side_i) {
            const GeoVFullPhysVol*phys_vol =   side_det_ele->getMaterialGeom();
            if (phys_vol) {
               connected_modules.insert( std::make_pair(phys_vol->getId(), module_i));
            }
            unsigned int id_hash = id_helper.get_other_side(det_ele->identifyHash());
            if (id_hash >= det_ele_coll.size()) break;
            side_det_ele = det_ele_coll[id_hash];
         }
      }
      return connected_modules;
   }

   /** Visit all detector elements connected to the same physical sensor which match the given pattern.
    * @param id_helper the ID identifier helper e.g. SCT_ID
    * @param a_module_pattern a pattern to match module identifiers
    * @param det_ele_coll collection of all detector elements e.g. all strip detector elements
    * @param id_hash the ID hash of the module which defines the physical sensor
    * @param module_connections map which associates an ID hash to all ID hashes of modules connected to the same physical sensor
    * @param visit_func function to be called for all connected modules which match the pattern
    * Will visit all modules connected to both sides of the same physical sensor excluding the module given
    * by id_hash itself.  More over the visited modules have to match the side range defined in the given pattern,
    * and have to be in the same row unless the all-rows flag is set in the pattern.
    */
   template <class T_ID>
   inline void visitMatchingConnectedModules( const T_ID &id_helper,
                                       const std::vector<int> &a_module_pattern,
                                       const InDetDD::SiDetectorElementCollection &det_ele_coll,
                                       unsigned id_hash,
                                       const std::unordered_multimap<unsigned int, unsigned int> &connected_modules,
                                       std::function<void(unsigned int id_hash, const InDetDD::SiDetectorElement &)> visit_func) {
      assert( id_hash < det_ele_coll.size() );
      if (   ModuleIdentifierMatchUtil::matchesAllModuleRowsOfSensor(a_module_pattern)
             || ModuleIdentifierMatchUtil::matchesBothSides(a_module_pattern)) {
         const InDetDD::SiDetectorElement *det_ele = det_ele_coll[id_hash];
         // propagate the defect to all modules connected to the same physical sensor
         // i.e. also mark opposite side or other rows as defect if configured accordingly
         if (det_ele->getMaterialGeom()) {
            std::pair< std::unordered_multimap< unsigned int, unsigned int>::const_iterator,
                       std::unordered_multimap< unsigned int, unsigned int>::const_iterator >
               childs = connected_modules.equal_range(det_ele->getMaterialGeom()->getId());
            unsigned int other_side_id_hash = id_helper.get_other_side(det_ele->identifyHash());
            using ConnectedKey_t = unsigned int;
            for (const std::pair<const ConnectedKey_t, unsigned int> &child :  std::ranges::subrange(childs.first, childs.second)) {
               assert( child.second < det_ele_coll.size());
               const InDetDD::SiDetectorElement *connected_det_ele( det_ele_coll[child.second] );
               int side = id_helper.side( connected_det_ele->identify());
               if (     ModuleIdentifierMatchUtil::isSideMatching(a_module_pattern, side)
                        && (ModuleIdentifierMatchUtil::matchesAllModuleRowsOfSensor(a_module_pattern)
                            || child.second == other_side_id_hash)) {
                  assert(connected_det_ele);
                  visit_func(child.second, *connected_det_ele);
               }
            }
         }
      }
   }

}
}
#endif
