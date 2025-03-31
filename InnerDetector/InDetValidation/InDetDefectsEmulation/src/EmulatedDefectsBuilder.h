/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
  */
#ifndef EMULATEDDEFECTSBUILDER_H
#define EMULATEDDEFECTSBUILDER_H

#include "EmulatedDefects.h"

namespace EmulatedDefectsBuilder {

   template <typename T_ModuleHelper>
   bool treatAsInserted(typename std::vector< typename T_ModuleHelper::KEY_TYPE>::iterator begin_range_iter,
                        typename T_ModuleHelper::KEY_TYPE defect_type_key_component) {
      // collision_mask=(~collision_mask) & (T_ModuleHelper::CHIP_MASK|T_ModuleHelper::COL_MASK|T_ModuleHelper::ROW_MASK);
      // if (T_ModuleHelper::isRangeKey(*begin_range_iter) && collision_mask>1) {
      //    auto prev=begin_range_iter;
      //    --prev;
      //    if (*prev - T_ModuleHelper::makeBaseKey(*begin_range_iter) > collision_mask) return true;
      // }

      // treat as inserted when attempting to insert a lower tier defect but is ovlapping with a higher tier defect;
      if (defect_type_key_component < T_ModuleHelper::getDefectTypeComponent(*begin_range_iter)) {
         return true;
      }
      else {
         // update defect type
         *begin_range_iter = (*begin_range_iter & (~T_ModuleHelper::TYPE_MASK)) |  defect_type_key_component;
         return false;
      }
   }

   template <typename T_ModuleHelper>
   typename T_ModuleHelper::KEY_TYPE makeNextKey(typename T_ModuleHelper::KEY_TYPE key) {
      return T_ModuleHelper::makeBaseKey(key)+1;
   }
   template <typename T_ModuleHelper>
   typename T_ModuleHelper::KEY_TYPE makePrevKey(T_ModuleHelper &helper, typename T_ModuleHelper::KEY_TYPE key) {
      unsigned int row = helper.getRow(key);
      if (row==0u) {
         if (helper.getColumn(key)>0) {
            return helper.makeKey(false, helper.getChip(key), helper.getColumn(key)-1, helper.rowsPerCircuit()-1);
         }
         else if (helper.getChip(key)>0) {
            return helper.makeKey(false, helper.getChip(key)-1, helper.columnsPerCircuit()-1, helper.rowsPerCircuit()-1);
         }
         else {
            return key;
         }
      }
      else {
         if  (row >= helper.rowsPerCircuit()) {
            return helper.makeKey(false, helper.getChip(key), helper.getColumn(key), helper.rowsPerCircuit()-1);
         }
         else {
            return T_ModuleHelper::makeBaseKey(key)-1;
         }
      }
   }

   // insert
   // iter = lowe_bound  i.e. -> first key not larger than key
   // next = iter++      i.e -> first key larger than key
   // prev_key  = key just before key
   // prev_key_next = key just before the first key not larger than key
   // insert key :
   //    not connected to next ||  not connected to iter
   //    turn into range : connected to next but not to iter
   //    turn iter into range :  connected to keuy

   template <typename T_ModuleHelper>
   bool insertKey(const T_ModuleHelper &helper,
                  std::vector< typename T_ModuleHelper::KEY_TYPE> &module_defects,
                  typename T_ModuleHelper::KEY_TYPE key,
                  typename T_ModuleHelper::KEY_TYPE defect_type_key_component) {
      auto [iter, end_iter] =  InDet::EmulatedDefects<T_ModuleHelper>::lower_bound(module_defects, key);
      if (iter != module_defects.end() ) {
         // if lower_bound is a range or is identical to key, then
         // this key is already covered by the existing module_defects
         // just test whether it should be treated as a newly inserted key or as a collision
         if ((T_ModuleHelper::isRangeKey(*iter) || T_ModuleHelper::makeBaseKey(*iter)==key)) return treatAsInserted<T_ModuleHelper>(iter,defect_type_key_component);

         // if this key and lower bound are connected then turn lower_bound into a range
         // since iter is not a range but there may still be a defect type which is part of this key.
         typename T_ModuleHelper::KEY_TYPE prev_key = makePrevKey(helper,key);
         // the iter is <= key however prev_key may be < iter if extended to row end
         // and iter extended to just before the next column
         if (T_ModuleHelper::makeBaseKey(*iter)>=prev_key) {
            auto next =iter;
            ++next;
            if (next != module_defects.end() && T_ModuleHelper::isRangeKey(*next)) {
               // extend existing range
               iter = module_defects.erase( iter);
            }
            else {
               *iter = T_ModuleHelper::makeRangeKey(*iter) ;
            }
         }
      }
      // if there is a larger key ( iterator --)
      // then check if this key and the larger key are connected
      // if this key is connected to the lower bound then do not insert
      // otherwise insert and turn into range
      if (iter != module_defects.begin()) {
         auto prev = iter;
         --prev;
         typename T_ModuleHelper::KEY_TYPE prev_key = makePrevKey(helper,*prev);
         if (T_ModuleHelper::makeBaseKey(prev_key) == key) {
            // if the larger key is a range, remove it because the new range
            // starting with this key or the range of iter will coover it.
            if (T_ModuleHelper::isRangeKey(*prev)) {
               if (iter == module_defects.end() || !T_ModuleHelper::isRangeKey(*iter)) {
                  // if this key is to be inserted
                  // just extent the existing range
                  *prev = T_ModuleHelper::makeRangeKey(key) | T_ModuleHelper::getDefectTypeComponent(*prev);
                  return true;
               }
               else {
                  // otherwise remove the larger key, since it will become part of
                  // the extended range.
                  module_defects.erase(prev);
               }
               return true;
            }
            else if (iter != module_defects.end() && T_ModuleHelper::isRangeKey(*iter)) {
               // the key is in between iter and prev nothing new needs to be isnerted
               return true;
            }
            // otherwise turn this key into a range
            key=T_ModuleHelper::makeRangeKey(key) | T_ModuleHelper::getDefectTypeComponent(*prev);
         }
      }
      if constexpr(T_ModuleHelper::TYPE_BITS>0) {
         // if no defect type has been inherited from a connected larger key
         // set the defect type
         if (!T_ModuleHelper::getDefectTypeComponent(key)) {
            key |= defect_type_key_component;
         }
      }
      // the key is inserted if it is not connected to iter
      // either as single key or a range if connected to the key which is larger than this key.
      // or if it is not connected to the larger key as a single key
      iter = module_defects.insert(iter,key);
      return true;
   }

   template <typename T_ModuleHelper>
   bool insertKeyRange(const T_ModuleHelper &helper,
                       std::vector< typename T_ModuleHelper::KEY_TYPE> &module_defects,
                       const std::pair<typename T_ModuleHelper::KEY_TYPE, typename T_ModuleHelper::KEY_TYPE> &key_range,
                       typename T_ModuleHelper::KEY_TYPE defect_type_key_component) {
      if (key_range.first == key_range.second) {
         return insertKey<T_ModuleHelper>(helper, module_defects, key_range.first, defect_type_key_component);
      }

      auto [key_start,key_end ] = key_range;
      if (key_end < key_start) {
         std::swap(key_start,key_end);
      }
      if (helper.getRow(key_end)+1u==helper.rowsPerCircuit()) {
         // if the and is the last row extend to the maximum value, to allow for merging
         // adjacent columns
         key_end = helper.makeKey(false, helper.getChip(key_end), helper.getColumn(key_end), helper.getLimitRowMax());
      }
      if (helper.getRow(key_start)==0u && helper.getColumn(key_start)>0) {
         key_start = helper.makeKey(false, helper.getChip(key_start), helper.getColumn(key_start)-1, helper.rowsPerCircuit());
      }

      // cases
      // no lower bound  for end and start
      //    add range at end
      // no lower bounds for end but lower bounds for start
      //    add range start at end
      //
      //       is lower bound range
      //
      // lower bound for end and start

      auto [iter, end_iter] =  InDet::EmulatedDefects<T_ModuleHelper>::lower_bound(module_defects, key_start);
      bool is_range = (iter != end_iter && T_ModuleHelper::isRangeKey(*iter));
      bool inserted_something=false;
      if (!is_range) {
         // the prev key could be before *iter if *iter marks the end of a row which is extended just before the first row of the next column
         if (iter != end_iter && (   T_ModuleHelper::makeBaseKey(*iter) == key_start
                                  || T_ModuleHelper::makeBaseKey(*iter) >= makePrevKey(helper,key_start))) {
            auto next=iter;
            ++next;
            if (next != end_iter && T_ModuleHelper::isRangeKey(*next)) {
               // the smaller key is a range
               // so remove the range and since it will be replaced by a new range end
               iter = module_defects.erase(iter);
            }
            else {
               if (T_ModuleHelper::makeBaseKey(*iter) == key_start) {
                  if ( defect_type_key_component > helper.getDefectTypeComponent(*iter)) {
                     *iter = (*iter & (~T_ModuleHelper::TYPE_MASK)) |  defect_type_key_component;
                  }
               }
               // the small key is connected, so this will become the range start
               if (helper.getRow(*iter)==0u && helper.getColumn(*iter)>0) {
                  // if at the bottom row extend range to above the last row of the previous column
                  *iter = helper.makeKey(true, helper.getChip(*iter), helper.getColumn(*iter)-1, helper.rowsPerCircuit()) | helper.getDefectTypeComponent(*iter);
               }
               else {
                  *iter = T_ModuleHelper::makeRangeKey(*iter);
               }
            }
         }
         else {
            iter = module_defects.insert(iter,T_ModuleHelper::makeRangeKey(key_start) | defect_type_key_component);
         }
         inserted_something=true;
      }

      // remove all keys which are included in the range up to end_key
      // if end_key is included in a range, then do not insert
      while (iter != module_defects.begin()) {
         --iter; // go to the key which is larger than key_start

         // if that key is larger or equal to key_end then stop.
         if (T_ModuleHelper::makeBaseKey(*iter) >= key_end) {

            // if key_end is included in a range or connected or equal to iter
            // then iter marks already an extended range, otherwise key_end needs to be inserted.
            // (prev key can be smaller than key_end if key_end is extended towards the beginning of the next column
            // and/or prev key extend to above the last r
            if (is_range || T_ModuleHelper::makeBaseKey(*iter) == key_end ||  makePrevKey(helper,*iter)  <= key_end) {
               // if the end key and iter are connected but not the same, and the end key not included in a range with
               // a higher defect type, then treat as inserted.
               inserted_something |= ( T_ModuleHelper::makeBaseKey(*iter) > key_end ) && (!is_range || defect_type_key_component > helper.getDefectTypeComponent(*iter)) ;
               // if iter is the start of a range than it is replaced by a new range start, so needs to be erased
               if (T_ModuleHelper::isRangeKey(*iter)) {
                  module_defects.erase(iter);
               }
               else {
                  if (T_ModuleHelper::makeBaseKey(*iter) == key_end) {
                     if ( defect_type_key_component > helper.getDefectTypeComponent(*iter)) {
                        *iter = (*iter & (~T_ModuleHelper::TYPE_MASK)) |  defect_type_key_component;
                     }
                  }
               }
               return inserted_something;
            }
            ++iter;
            break;
         }
         // remember whether iter is a range start (--iter will be the range end)
         is_range=T_ModuleHelper::isRangeKey(*iter);
         // iter  is includes in the new range key_start - key_end, so remove it
         iter=module_defects.erase(iter);
      }
      module_defects.insert(iter,key_end | defect_type_key_component );
      return true;
   }
}
#endif
