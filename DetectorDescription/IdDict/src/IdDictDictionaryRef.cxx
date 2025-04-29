/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */

#include "IdDict/IdDictDictionaryRef.h"
#include "IdDict/IdDictMgr.h"
#include "IdDict/IdDictDictionary.h"
#include "IdDict/IdDictRegion.h"
#include "IdDict/IdDictRegionEntry.h"
#include "Identifier/Range.h"
#include "IdDict/IdDictRangeRef.h"
#include "IdDict/IdDictRange.h"
#include "IdDict/IdDictFieldImplementation.h"

IdDictDictionaryRef::IdDictDictionaryRef ()
  :
  m_dictionary(0),
  m_resolved_references(false),
  m_generated_implementation(false),
  m_propagated_information(false) {
}

IdDictDictionaryRef::~IdDictDictionaryRef () {
}

void IdDictDictionaryRef::resolve_references(const IdDictMgr& idd,
                                             IdDictDictionary& dictionary,
                                             IdDictRegion& /*region*/) {
  if (!m_resolved_references) {
    m_dictionary = idd.find_dictionary(m_dictionary_name);
    if (m_dictionary) {
      m_dictionary->resolve_references(idd);
      m_dictionary->m_parent_dict = &dictionary;
    }
    m_resolved_references = true;
  }
}

void IdDictDictionaryRef::generate_implementation(const IdDictMgr& idd,
                                                  IdDictDictionary& dictionary,
                                                  IdDictRegion& region,
                                                  const std::string& tag) {
  if (!m_generated_implementation) {
    if (m_dictionary) {
      if (!m_propagated_information) {
        // Propagate information to referenced dictionary:
        //
        // 1) Loop over ranges in this region and add them to the
        // referenced dictionary, then propagate the generate
        // implementation

        // 2) Duplicate the regions of the current dictionary in
        // referenced dictionary. Only the top level range(s) need
        // to be propagated to correctly calculate the bits of the
        // upper levels.

        // Save a vector of entries to prepend, inverting their order
        std::vector<IdDictRegionEntry*> prepend_entries;
        std::vector<IdDictRegionEntry*>::iterator it;
        for (it = region.m_entries.begin(); it != region.m_entries.end(); ++it) {
          IdDictRegionEntry* entry = *it;
          if (this == entry) break; // end when we get to the dictionary (this)
          // If this is a range entry, add a duplicate to all
          // regions in the subdictionary
          IdDictRange* range = dynamic_cast<IdDictRange*> (entry);
          if (range) {
            prepend_entries.insert(prepend_entries.begin(), entry);
          }
        }




        // Now prepend list to each region and generate each region
        IdDictDictionary::regions_it it2;
        for (it2 = m_dictionary->m_all_regions.begin(); it2 != m_dictionary->m_all_regions.end(); ++it2) {
          IdDictRegion& region2 = *(*it2);
          for (it = prepend_entries.begin(); it != prepend_entries.end(); ++it) {
            IdDictRegionEntry* entry = *it;
            IdDictRange* range = dynamic_cast<IdDictRange*> (entry);
            if (range) {
              IdDictRangeRef* new_range = new IdDictRangeRef;
              new_range->m_range = range;
              region2.m_entries.insert(region2.m_entries.begin(), new_range);
            }
          }
        }

        // Now copy all prefixes into new regions in the
        // referenced dictionary
        if (prepend_entries.size() > 0) {
          // Save region number
          const IdDictRegion& region2 = *m_dictionary->m_all_regions.back();
          size_t region_number = region2.m_index + 1;

          IdDictDictionary::regions_it it;

          // Loop over all regions of current dict, add to ref dict (m_dictionary)
          for (it = dictionary.m_all_regions.begin(); it != dictionary.m_all_regions.end(); ++it, ++region_number) {
            IdDictRegion& region3 = *(*it);
            IdDictRegion* new_region = new IdDictRegion;
            new_region->m_name = "dummy";
            new_region->m_group = "dummy";
            new_region->m_index = region_number;

            // to all region vectors
            m_dictionary->m_all_regions.push_back(new_region);
            // to the entries of the dictionary
            m_dictionary->add_dictentry(new_region);

            // Now add in only the ranges
            std::vector<IdDictRegionEntry*>::iterator it;
            size_t i = 0;
            for (it = region3.m_entries.begin(); it != region3.m_entries.end(); ++it, ++i) {
              if (i >= prepend_entries.size()) continue;

              IdDictRegionEntry* entry = *it;
              IdDictRange* range = dynamic_cast<IdDictRange*> (entry);
              if (range) {
                IdDictRangeRef* new_range = new IdDictRangeRef;
                new_range->m_range = range;
                new_region->m_entries.push_back(new_range);
              }
            }
          }
        }
        m_propagated_information = true;
      }
      m_dictionary->generate_implementation(idd, tag);
      m_generated_implementation = true;
    } else {
      std::cout <<
      "IdDictDictionaryRef::generate_implementation: - WARNING no dictionary found, cannot generate implementation "
                << std::endl;
    }
  }
}

void IdDictDictionaryRef::reset_implementation() {
  if (m_generated_implementation) {
    m_dictionary->reset_implementation();
    m_generated_implementation = false;
  }
}

bool IdDictDictionaryRef::verify() const {
  return(true);
}

Range IdDictDictionaryRef::build_range() const {
  Range result;

  return(result);
}
