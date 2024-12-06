/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "src/get_bits.h"
#include "Identifier/Range.h"
#include "IdDict/IdDictRegion.h"
#include "IdDict/IdDictRange.h"
#include "IdDict/IdDictFieldImplementation.h"
#include <algorithm>

namespace IdDict{
  typedef std::vector <IdDictRegion*> RV; 
   
  void
  compute_bits (const RV& regions, size_t level, const std::string& group){ 
    Range::field ored_field; 
    unsigned int k;
    for (k = 0; k < regions.size(); ++k) { 
      IdDictRegion* region = regions[k];
      if (region->fieldSize() <= level) continue;
      const IdDictFieldImplementation& f = region->m_implementation[level]; 
      const Range::field thisField = f.field(); 
      //on first time, set the original field
      if (k == 0) ored_field = thisField;
      //on subsequent iterations, 'or' the new fields with the original
      else ored_field |= thisField; 
    } 
    //
    // Now that we have the ored width of all first fields of
    // all regions, we upgrade the bit width in the corresponding
    // field implementations.
    //
    for (k = 0; k < regions.size (); ++k) { 
      IdDictRegion* region = regions[k];
      if (region->fieldSize () <= level) continue; 
      // Don't set ored bits for regions outside of group - ok to
      // calculate with them only
      if (group != region->m_group) continue;  
      IdDictFieldImplementation& f = region->m_implementation[level]; 
      f.set_ored_field(ored_field); 
    }    
  }
  
  
  void 
  get_bits (const RV& regions, size_t level, const std::string& group){ 
    //  Invalid call - return
    if (regions.empty()) return;
    if (regions.size () == 1) { 
      /* 
         There is only one region (ie one range)
         therefore the bitsets are unambiguous
         The bit width of each field implementation is
         simply the bit width of the corresponding field.
      */ 
      IdDictRegion* region = regions[0];
      for (size_t k = level; k < region->m_implementation.size (); ++k) { 
        IdDictFieldImplementation& f = region->m_implementation[k]; 
        f.set_ored_field(f.field()); 
      } 
      return; 
    } 
 
    size_t k; 
    // Copy the regions provided in the arguments to a local clone. 
    RV mr = regions; 
    // Compute the number of bits needed at this level
    compute_bits (mr, level, group);
    /** 
     *   Split mr into two RV : 
     * 
     *    overlapping contains all Regions that overlap with their field  at <level>
     *    temp        contains all other Regions 
     */ 
    RV overlapping; 
    RV orig; 
    RV temp; 
    overlapping.reserve(mr.size()); 
    orig.reserve(mr.size());
    temp.reserve(mr.size());
    for (;;) { 
        if (mr.empty())  { 
            break; 
        } 
        overlapping.clear (); 
        temp.clear (); 
        // Find the first non empty Region within the specified group
        IdDictRegion* reference_region = 0; 
        for (k = 0; k < mr.size (); ++k) { 
          reference_region = mr[k];
          if (reference_region->m_implementation.size () > level) { 
            if (group == reference_region->m_group) {
              overlapping.push_back (reference_region); 
              break; 
            } else {
              temp.push_back(reference_region); 
            }
          } 
        } 
        // Should be redundant with the mr.size() test above, but put here
        // to try to avoid coverity warnings that reference_region may be null.
        if (reference_region == 0) break;
        if (overlapping.empty()) { 
            break; 
        } 
        // Now really split mr into the two subsets 
        ++k; 
        const IdDictFieldImplementation& f1 = reference_region->m_implementation[level]; 
        Range::field ored_field = f1.field(); 
        // copy into original test sample, some may have already been
        // added to temp above.
        for (; k < mr.size (); ++k) { 
            IdDictRegion* region = mr[k];
            temp.push_back (region); 
        }
        bool found_overlap  = false;
        // Compare the reference field at this level to those in the
        // remaining regions and check for overlap.
        //
        // The comparison must be done repeatedly because the overlap
        // field gets larger and may overlap with previously checked
        // field of a region.
        //
        // By "definition" we require regions in the same group to
        // overlap to have a uniform bit-allocation for a group.
        //
        // For regions outside the current group, the overlap is
        // allowed for the top level fields with the same names. As
        // soon as one finds a field with a different name which is in
        // a region outside the group, this and subsequent fields are
        // ignored.
        //
        do {
          // copy temp into orig, set overlap to false for this loop over regions
          orig.clear();
          orig = temp;
          temp.clear();
          found_overlap = false;
          for (size_t i = 0; i < orig.size (); ++i) { 
            IdDictRegion* region = orig[i];
            if (region->m_implementation.size () <= level) continue;  
            bool overlap = false; 
            const IdDictFieldImplementation& f = region->m_implementation[level]; 
            const Range::field& thisField = f.field(); 
            // Now expand bits by or'ing them with other regions
            // at this level, requiring the name to be the same.
            if (f1.range()->m_field_name == f.range()->m_field_name) 
              overlap = ored_field.overlaps_with (thisField); 
            // Check for either an overlap or force overlap for
            // regions in the same group
            if (overlap || (region->m_group == group)) { 
              overlapping.push_back (region); 
              ored_field |= thisField; 
              found_overlap = true;
            } else { 
              temp.push_back (region); 
            } 
          }
        } while(found_overlap);
      
        // Check overlapping collection - if there are only regions from
        // the same group, we can safely throw away "temp" the other
        // collection
        bool all_within_group = true;
        bool none_within_group = true;
        for (size_t i = 0; i < overlapping.size (); ++i) { 
          IdDictRegion* region = overlapping[i];
          if (group == region->m_group) {
            none_within_group = false;
          }else {
            all_within_group = false;
          }
        }
        // Remove temp is no longer needed
        if (all_within_group) temp.clear();
        // Recurse on the remaining fields of the overlapping regions 
        //
        // Continue only if there are regions still within the current
        // group
        if(!none_within_group) {
          get_bits (overlapping, level + 1, group); 
        }
        // Continue for the other not-yet-overlapping regions. 
        mr = temp; 
    } 
  }
  
  
   bool isNumber(const std::string& str) {
    return ( (not str.empty()) and (std::ranges::all_of(str, ::isdigit)) );
  }
}
