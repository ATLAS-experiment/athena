/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "IdDict/IdDictDictionary.h"

#include "IdDict/IdDictField.h"
#include "IdDict/IdDictLabel.h"
#include "IdDict/IdDictRegion.h"
#include "IdDict/IdDictGroup.h"
#include "IdDict/IdDictSubRegion.h"
#include "IdDict/IdDictFieldImplementation.h"
#include "IdDict/IdDictMgr.h"
#include "IdDict/IdDictRange.h"
#include "src/Debugger.h"
#include "src/get_bits.h"
#include "Identifier/MultiRange.h"
#include <iostream>

using IdDict::get_bits;

static bool isNumber(const std::string& str) {
  bool result = true;

  for (unsigned int i = 0; i < str.size(); ++i) {
    if (!isdigit(str[i])) return(false);
  }
  if (0 == str.size()) return(false);

  return(result);
}

IdDictDictionary::IdDictDictionary () = default;
IdDictDictionary::~IdDictDictionary () = default;

IdDictField* IdDictDictionary::find_field(const std::string& name) const {
  std::map <std::string, IdDictField*>::const_iterator it;

  it = m_fields.find(name);

  if (it == m_fields.end()) {
    // If parent exists, look for field there
    if (m_parent_dict) {
      it = m_parent_dict->m_fields.find(name);
      if (it == m_parent_dict->m_fields.end()) {
        return(0);
      }
    } else {
      return(0);
    }
  }

  return((*it).second);
}

IdDictLabel* IdDictDictionary::find_label(const std::string& field, const std::string& label) const {
  IdDictField* idField = find_field(field);

  if (!idField) return nullptr;

  return(idField->find_label(label));
}

int IdDictDictionary::get_label_value(const std::string& field, const std::string& label, int& value) const {
  IdDictLabel* idLabel = find_label(field, label);

  if (!idLabel || !idLabel->m_valued) return(1);

  value = idLabel->m_value;
  return(0);
}

void IdDictDictionary::add_field(IdDictField* field) {
  if (field == 0) return;

  std::string& name = field->m_name;

  m_fields[name] = field;
}

IdDictSubRegion*
IdDictDictionary::find_subregion(const std::string& name) const {
  std::map <std::string, IdDictSubRegion*>::const_iterator it;

  it = m_subregions.find(name);

  if (it == m_subregions.end()) return(0);

  return((*it).second);
}

IdDictRegion* IdDictDictionary::find_region(const std::string& region_name) const {
  return find_region(region_name, "");
}

IdDictRegion* IdDictDictionary::find_region(const std::string& region_name, const std::string& group_name) const {
  for (size_t i = 0; i < m_regions.size(); ++i) {
    IdDictRegion* region = m_regions[i];
    if ((group_name != "") && (region->group_name() != group_name)) continue;
    if ((region != 0) && (region_name == region->m_name)) return(region);
  }

  return(0);
}

IdDictGroup* IdDictDictionary::find_group(const std::string& group_name) const {
  for (size_t i = 0; i < m_groups.size(); ++i) {
    IdDictGroup* group = m_groups[i];
    if ((group != 0) && (group->name() == group_name)) return(group);
  }

  return(0);
}

void IdDictDictionary::add_subregion(IdDictSubRegion* subregion) {
  if (subregion == 0) return;

  std::string& name = subregion->m_name;

  m_subregions[name] = subregion;
}

void IdDictDictionary::add_subdictionary_name(const std::string& name) {
  m_subdictionary_names.push_back(name);
}

void IdDictDictionary::add_dictentry(IdDictDictEntry* region) {
  // Add region to corresponding group
  IdDictGroup* group = find_group(region->group_name());

  if (0 == group) {
    group = new IdDictGroup(region->group_name());
    m_groups.push_back(group);
  }
  group->add_dictentry(region);
}

void IdDictDictionary::resolve_references(const IdDictMgr& idd) {
  {
    std::map<std::string, IdDictField*>::iterator it;

    for (it = m_fields.begin(); it != m_fields.end(); ++it) {
      IdDictField* field = (*it).second;
      field->resolve_references(idd);
    }
  }
  {
    std::map<std::string, IdDictSubRegion*>::iterator it;

    for (it = m_subregions.begin(); it != m_subregions.end(); ++it) {
      IdDictSubRegion* subregion = (*it).second;
      subregion->resolve_references(idd, *this);
    }
  }
  {
    size_t index = 0;

    IdDictDictionary::groups_it it;
    for (it = m_groups.begin(); it != m_groups.end(); ++it) {
      (*it)->resolve_references(idd, *this, index);
    }
  }
}

void IdDictDictionary::generate_implementation(const IdDictMgr& idd,
                                               const std::string& tag) {
  if (Debugger::debug()) {
    std::cout << "IdDictDictionary::generate_implementation>" << std::endl;
  }

  if (!m_generated_implementation) {
    // Propagate to each region and copy their generation into the
    // dict's vector.
    IdDictDictionary::groups_it it;
    for (it = m_groups.begin(); it != m_groups.end(); ++it) {
      (*it)->generate_implementation(idd, *this, tag);
      // Get regions from group and save in m_regions
      const regions_type& regions = (*it)->regions();
      IdDictDictionary::regions_const_it it1;
      for (it1 = regions.begin(); it1 != regions.end(); ++it1) {
        m_regions.push_back(*it1);
      }
    }

    // Loop again over groups and set the bit-packing - starting at
    // level 0
    for (it = m_groups.begin(); it != m_groups.end(); ++it) {
      // Copy to temporary vector all regions in the group. And
      // look for regions in local m_regions vector for any
      // regions "dummy", which come from reference
      // dictionaries.
      // Skip special group
      if ("dummy" == (*it)->name()) continue;

      get_bits(m_regions, 0, (*it)->name());
//	  get_bits (regions, 0, (*it)->name());
    }

    // Set integral over the number of bits
    integrate_bits();

    // Set neighbours for regions
    IdDictDictionary::regions_it itr;
    for (itr = m_regions.begin(); itr != m_regions.end(); ++itr) {
      (*itr)->find_neighbours(*this);
    }

    m_generated_implementation = true;
  }
}

void IdDictDictionary::reset_implementation() {
  if (m_generated_implementation) {
    m_regions.clear();
    IdDictDictionary::groups_it it;
    for (it = m_groups.begin(); it != m_groups.end(); ++it) {
      (*it)->reset_implementation();
    }
    m_generated_implementation = false;
  }
}

int
IdDictDictionary::find_region(const ExpandedIdentifier& id, size_type& index) const {
  // Find first region that matches id

  IdDictDictionary::regions_const_it it;

  size_type i = 0;
  for (it = m_regions.begin(); it != m_regions.end(); ++it, ++i) {
    const IdDictRegion& region = *(*it);

    Range range = region.build_range();

    if (range.match(id) && range.fields() >= id.fields()) {
      index = i;
      return(0);
    }
  }

  return(1);
}

IdDictRegion* IdDictDictionary::find_region(const ExpandedIdentifier& id) const {
  return find_region(id, "");
}

IdDictRegion* IdDictDictionary::find_region(const ExpandedIdentifier& id, const std::string& group_name) const {
  // Find first region that matches id

  IdDictRegion* pRegion = 0;

  IdDictDictionary::regions_const_it it;

  for (it = m_regions.begin(); it != m_regions.end(); ++it) {
    IdDictRegion& region = *(*it);
    if ((group_name != "") && (region.group_name() != group_name)) continue;

    Range range = region.build_range();

    if (range.match(id) && range.fields() >= id.fields()) {
      pRegion = &region;
    }
  }

  return(pRegion);
}

void
IdDictDictionary::integrate_bits() {
  // For each region, loop over its levels and set the bit offset
  // for each FieldImplementation

  for (IdDictRegion* region : m_regions) {
    size_t bits_offset = 0;
    for (IdDictFieldImplementation& impl : region->m_implementation) {
      impl.optimize(); // optimize for decoding
      impl.set_bits_offset(bits_offset);
      bits_offset += impl.bits();

      // Set whether or not to decode index
      Range::field field = impl.ored_field();
      if ((not field.isBounded()) || (0 != field.get_minimum())) {
        impl.set_decode_index(true);
      }
    }
  }
}

MultiRange IdDictDictionary::build_multirange() const {
  MultiRange result;

  IdDictDictionary::groups_const_it it;

  for (it = m_groups.begin(); it != m_groups.end(); ++it) {
    const IdDictGroup& group = *(*it);

    MultiRange group_mr = group.build_multirange();

    for (unsigned int i = 0; i < group_mr.size(); ++i) {
      const Range& range = group_mr[i];
      result.add(range);
    }
  }

  return(result);
}

MultiRange IdDictDictionary::build_multirange(const ExpandedIdentifier& region_id,
                                              const Range& prefix,
                                              const std::string& last_field) const {
  MultiRange result;

  IdDictDictionary::regions_const_it it;
  if ("" == last_field) {
    // Take all fields
    for (it = m_regions.begin(); it != m_regions.end(); ++it) {
      const IdDictRegion& region = *(*it);

      // skip regions created from parents
      if ("dummy" == region.m_name) continue;

      // skip empty regions - may arise from alternate_regions
      // where a tag selects an empty region
      if (region.m_is_empty) continue;

      Range range(region.build_range());
      // Check region selection
      if (range.match(region_id)) result.add(std::move(range));
    }
  } else {
    // Not all fields required
    for (it = m_regions.begin(); it != m_regions.end(); ++it) {
      const IdDictRegion& region = *(*it);

      // skip regions created from parents
      if ("dummy" == region.m_name) continue;

      // skip empty regions - may arise from alternate_regions
      // where a tag selects an empty region
      if (region.m_is_empty) continue;

      Range range(region.build_range());
      // Check region selection
      if (range.match(region_id)) {
        // Build new range up to last_field and add it to result -
        // remove duplicate ranges with addRangeToMR
        Range new_range(prefix); // Prepend with prefix

        std::vector <IdDictFieldImplementation>::const_iterator fit;
        for (fit = region.m_implementation.begin();
             fit != region.m_implementation.end();
             ++fit) {
          const IdDictFieldImplementation& impl = *fit;

//              new_range.add(impl.m_field);
          new_range.add(impl.range()->build_range());

          if (last_field == impl.range()->m_field->m_name) {
            break;
          }
        }
        result.add(std::move(new_range));
      }
    }
  }

  return(result);
}

MultiRange IdDictDictionary::build_multirange(const ExpandedIdentifier& region_id,
                                              const std::string& group_name,
                                              const Range& prefix,
                                              const std::string& last_field) const {
  MultiRange result;

  IdDictDictionary::regions_const_it it;
  if ("" == last_field) {
    // Take all fields
    for (it = m_regions.begin(); it != m_regions.end(); ++it) {
      const IdDictRegion& region = *(*it);

      // skip regions created from parents
      if ("dummy" == region.m_name) continue;

      // skip empty regions - may arise from alternate_regions
      // where a tag selects an empty region
      if (region.m_is_empty) continue;

      Range range(region.build_range());
      // Check region selection
      if (range.match(region_id) && region.group_name() == group_name) result.add(std::move(range));
    }
  } else {
    // Not all fields required
    for (it = m_regions.begin(); it != m_regions.end(); ++it) {
      const IdDictRegion& region = *(*it);

      // skip regions created from parents
      if ("dummy" == region.m_name) continue;

      // skip empty regions - may arise from alternate_regions
      // where a tag selects an empty region
      if (region.m_is_empty) continue;

      Range range(region.build_range());
      // Check region selection
      if (range.match(region_id) && region.group_name() == group_name) {
        // Build new range up to last_field and add it to result -
        // remove duplicate ranges with addRangeToMR
        Range new_range(prefix); // Prepend with prefix

        std::vector <IdDictFieldImplementation>::const_iterator fit;
        for (fit = region.m_implementation.begin();
             fit != region.m_implementation.end();
             ++fit) {
          const IdDictFieldImplementation& impl = *fit;

//              new_range.add(impl.m_field);
          new_range.add(impl.range()->build_range());

          if (last_field == impl.range()->m_field->m_name) {
            break;
          }
        }
        result.add(std::move(new_range));
      }
    }
  }

  return(result);
}

/**
 *   Pack to 32bits the subset of id between (inclusive) index1
 *   and index2 - this is generic, i.e. not the most efficient
 *
 *   Assumptions:
 *     - packedId is initialized to 0
 *     - the expanded id begins at index = 0
 *     - index1,2 correspond to the level/field number, this is
 *       thus the same as the index into the expanded id due to the
 *       previous assumption.
 *
 */

int
IdDictDictionary::pack32(const ExpandedIdentifier& id,
                         size_t index1,
                         size_t index2,
                         Identifier& packedId) const {
  packedId = 0;


  // Preconditions...

  if (index2 < index1) {
    // bad parameters.
    return(1);
  }

  if (index1 >= id.fields()) {
    // nothing very useful !!
    return(1);
  }

  if (index2 >= id.fields()) {
    // bad parameter...
    return(1);
  }

  /**
   *   First we need to check whether the specified identifier
   *   really matches the Dictionary specifications.
   *
   *    However, we stop the search in the set of regions at the first
   *   matching region.
   *    Should we request that only one region matches ????
   */

  for (size_t k = 0; k < m_regions.size(); ++k) {
    bool selected = true;

    const IdDictRegion& region = *m_regions[k];

    // Must skip empty regions - can arise when a tag selects an
    // empty region
    if (region.m_is_empty) continue;

    for (size_t i = 0; i < region.m_implementation.size(); ++i) {
      if (i >= id.fields()) break;

      const IdDictFieldImplementation& impl = region.m_implementation[i];

      if (!impl.field().match(id[i])) {
        selected = false;
        break;
      }
    }

    if (selected) {
      size_t position = Identifier::NBITS;
      // We have the proper region.
      for (size_t i = index1; i <= index2; ++i) {
        const IdDictFieldImplementation& impl = region.m_implementation[i];

        Identifier::value_type index = impl.ored_field().get_value_index(id[i]);

        if (0 == position && impl.bits() > 0) {
          return(1);
        }

        position -= impl.bits();
        packedId |= (index << position);
      }
      break;
    }
  }


  return(0);
}

int
IdDictDictionary::pack32(const int* fields,
                         size_t index1,
                         size_t index2,
                         size_t region_index,
                         Identifier& packedId,
                         size_t first_field_index) const {
  // Preconditions...

  if (m_do_checks) {
    if (index2 < index1) {
      // bad parameters.
      std::cout << "IdDictDictionary::pack32 - index2 < index1 - 1,2"
                << index1 << " " << index2 << std::endl;
      return(1);
    }

    if (region_index >= m_regions.size()) {
      // bad parameters.
      std::cout << "IdDictDictionary::pack32 - region index incorrect - index,size"
                << region_index << " " << m_regions.size() << std::endl;
      return(1);
    }
  }


  // Get the region
  const IdDictRegion& region = *m_regions[region_index];

  if (m_do_checks) {
    if (region.m_is_empty) {
      std::cout << "IdDictDictionary::pack32 - region id empty" << std::endl;
      // bad parameters.
      return(1);
    }
    if (index1 < first_field_index) {
      std::cout << "IdDictDictionary::pack32 - first_field_index > index1 "
                << first_field_index << " " << index1 << std::endl;
      return(1);
    }
  }

  // Set the starting position
  size_t position = Identifier::NBITS;
  if (!first_field_index) {
    const IdDictFieldImplementation& impl = region.m_implementation[index1];
    position -= impl.bits_offset();
  }

  size_t field_index = 0;
  for (size_t i = index1; i <= index2; ++i, ++field_index) {
    const IdDictFieldImplementation& impl = region.m_implementation[i];

    if (m_do_checks) {
      // Field should be within allowed range
      if (!impl.ored_field().match(fields[field_index])) {
        std::cout << "IdDictDictionary::pack32 - field does NOT match: value, allowed values "
                  << fields[field_index] << " " << (std::string) impl.ored_field()
                  << std::endl;
        // bad parameters.
        return(1);
      }

      // Check that we don't try to go below 0
      if (0 == position && impl.bits() > 0) {
        std::cout << "IdDictDictionary::pack32 - going past 0" << std::endl;
        return(1);
      }
    }

    Identifier::value_type index;
    if (impl.decode_index()) {
      index = impl.ored_field().get_value_index(fields[field_index]);
    } else {
      index = (Identifier::value_type) fields[field_index];
    }

    position -= impl.bits();
    packedId |= (index << position);
  }


  return(0);
}

/**
 *   Reset fields from index1 to index2.
 * */
int IdDictDictionary::reset(size_t index1,
                            size_t index2,
                            size_t region_index,
                            Identifier& packedId) const {
  // Preconditions...

  if (m_do_checks) {
    if (index2 < index1) {
      // bad parameters.
      std::cout << "IdDictDictionary::pack32 - index2 < index1 - 1,2"
                << index1 << " " << index2 << std::endl;
      return(1);
    }

    if (region_index >= m_regions.size()) {
      // bad parameters.
      std::cout << "IdDictDictionary::pack32 - region index incorrect - index,size"
                << region_index << " " << m_regions.size() << std::endl;
      return(1);
    }
  }


  // Get the region
  const IdDictRegion& region = *m_regions[region_index];

  if (m_do_checks) {
    if (region.m_is_empty) {
      std::cout << "IdDictDictionary::pack32 - region id empty" << std::endl;
      // bad parameters.
      return(1);
    }
  }

  size_t field_index = 0;
  for (size_t i = index1; i <= index2; ++i, ++field_index) {
    const IdDictFieldImplementation& impl = region.m_implementation[i];

    size_t position = Identifier::NBITS - impl.bits_offset() - impl.bits();

    Identifier::value_type mask = (((Identifier::value_type) 1 << impl.bits()) - 1) << position;

    mask ^= Identifier::ALL_BITS;

    packedId &= (mask);
  }
  return(0);
}

/**
 *  Unpack the bits32 id to an expanded Identifier, considering the
 *  provided prefix (result will include the prefix)
 */
#if defined(FLATTEN) && defined(__GNUC__)
// We compile this package with optimization, even in debug builds; otherwise,
// the heavy use of Eigen makes it too slow.  However, from here we may call
// to out-of-line Eigen code that is linked from other DSOs; in that case,
// it would not be optimized.  Avoid this by forcing all Eigen code
// to be inlined here if possible.
__attribute__ ((flatten))
#endif
int
IdDictDictionary::unpack(const std::string& /*group*/,
                         const Identifier& id,
                         const ExpandedIdentifier& prefix,
                         size_t index2,
                         ExpandedIdentifier& unpackedId) const {
  ExpandedIdentifier localPrefix(prefix);

  unpackedId.clear();
  if (localPrefix.isValid()) unpackedId = localPrefix;

  /**
   *   First we need to check whether the specified identifier prefix
   *   really matches the Dictionary specifications.
   */

  size_t index1 = 0;  // field index
  size_t position = Identifier::NBITS; // overall bit position - used for debugging only

  for (size_t k = 0; k < m_regions.size(); ++k) {
    bool selected = false;

    const IdDictRegion& region = *m_regions[k];

    // Must skip empty regions - can arise when a tag selects an
    // empty region
    if (region.m_is_empty) continue;

    for (size_t i = 0; i < region.m_implementation.size(); ++i) {
      if (i >= localPrefix.fields()) {
        /**
         *   ok
         *   we require that the region is at least as large as the prefix.
         */
        selected = true;
        index1 = i;
        break;
      }

      const IdDictFieldImplementation& impl = region.m_implementation[i];

      if (!impl.field().match(localPrefix[i])) {
        break;
      }
    }
    if (!selected) {
      continue;
    }

      /**
       *   We have one region that matches the prefix.
       *   Let's now try to expand the bits32 from the fields of the region
       *   that are beyond the prefix.
       */
      for (size_t i = index1; i < region.m_implementation.size(); ++i) {
        const IdDictFieldImplementation& impl = region.m_implementation[i];

        if (impl.bits() == 0) continue;
        Identifier::value_type mask = (static_cast<Identifier::value_type>(1) << impl.bits()) - 1;
        if (position < impl.bits()) break;                                                 // Nothing more to get
        size_t index = id.extract(position - impl.bits(), mask);

        if (index >= impl.ored_field().get_indices()) {
          /**
           *  this index extracted from the bits32 does not seem to
           * match this field in the region...
           * Let's try another region
           */

          selected = false;
          break;
        }

        ExpandedIdentifier::element_type value = impl.ored_field().get_value_at(index);

        /**
         *  this index extracted from the bits32 does not
         * match this field in the region...
         * Let's try another region
         */

        if (!impl.field().match(value)) {
          selected = false;
          break;
        }



        // Found value

        unpackedId.add(value);
        localPrefix.add(value);

        position -= impl.bits(); // overall bit position



        index1++;  // next field

        if (index1 > index2) break; // quit at index2
      }

      if (selected) break;
  }

  return(0);
}

/**
 *  Unpack the bits32 id to a string, considering the provided
 *  prefix (result will include the prefix) and up to index2 -
 *  (index1 is assumed to be 0, i.e. part of prefix).
 */
int
IdDictDictionary::unpack(const std::string& /*group*/,
                         const Identifier& id,
                         const ExpandedIdentifier& prefix,
                         size_t index2,
                         const std::string& sep,
                         std::string& unpackedId) const {
  ExpandedIdentifier localPrefix(prefix);


  /**
   *   First we need to check whether the specified identifier prefix
   *   really matches the Dictionary specifications.
   */

  size_t index1 = 0;  // field index
  size_t position = Identifier::NBITS; // overall bit position - used for debugging only

  for (size_t k = 0; k < m_regions.size(); ++k) {
    bool selected = false;

    const IdDictRegion& region = *m_regions[k];

    // Must skip empty regions - can arise when a tag selects an
    // empty region
    if (region.m_is_empty) continue;

//      for (size_t i = index1; i < region.m_implementation.size (); ++i)
    for (size_t i = 0; i < region.m_implementation.size(); ++i) {
      if (i >= localPrefix.fields()) {
        /**
         *   ok
         *   we require that the region is at least as large as the prefix.
         */
        selected = true;
        index1 = i;
        break;
      }

      const IdDictFieldImplementation& impl = region.m_implementation[i];

      if (!impl.field().match(localPrefix[i])) {
        break;
      }
    }

    if (selected) {
      /**
       *   We have one region that matches the prefix.
       *   Let's now try to expand the bits32 from the fields of the region
       *   that are beyond the prefix.
       */

//               bits32 temp = id;

//        std::cout << "Region #" << region.m_index << " selected" << std::endl;

      for (size_t i = index1; i < region.m_implementation.size(); ++i) {
        const IdDictFieldImplementation& impl = region.m_implementation[i];

        if (impl.bits() == 0) continue;

        Identifier::value_type mask = (static_cast<Identifier::value_type>(1) << impl.bits()) - 1;

        if (position < impl.bits()) break;                                               // Nothing more to get
        size_t index = id.extract(position - impl.bits(), mask);

        if (index >= impl.ored_field().get_indices()) {
          selected = false;
          break;
        }

        ExpandedIdentifier::element_type value = impl.ored_field().get_value_at(index);

        /**
         *  this index extracted from the bits32 does not
         * match this field in the region...
         * Let's try another region
         */

        if (!impl.field().match(value)) {
          selected = false;
          break;
        }


        // Add value to string

        std::string str_value("nil");
        char temp[20];

        // The policy below is:
        //   - if a value is a character string or name, just add this name
        //   - if a value is a number, then prefix it with the
        //     name of the field
        //
        // NOTE: min/max is a number, but for value/label we
        // distinguish between number and name by looking for an IdDictLabel
        const IdDictRange* range = impl.range();
        IdDictLabel* label = range->m_field->find_label(range->m_label);
        switch (range->m_specification) {
        case IdDictRange::by_minmax:
          // For a range of values (numbers), add in the field name
          str_value = range->m_field->m_name + ' ';
          sprintf(temp, "%d", value);
          str_value += temp;
          break;

        case IdDictRange::by_value:
        case IdDictRange::by_label:
          str_value = "";
          if (!label) {
            // Is a number, add in field name
            str_value += range->m_field->m_name + ' ';
          }
          str_value += range->m_label;
          break;

        case IdDictRange::by_values:
        case IdDictRange::by_labels:
          str_value = "";
          // Is a name
          if (label) {
            // Found label with "find_label" on the field
            if (label->m_valued) {
              str_value += range->m_label;
            }
          } else {
            // If not found with the "find" above, we must
            // get the value and name from the range
            // itself.

            unsigned int index1 = 0;
            for (; index1 < range->m_values.size(); ++index1) {
              if (value == range->m_values[index1]) {
                break;
              }
            }

            // In some cases we
            if (index1 < range->m_labels.size()) {
              if (isNumber(range->m_labels[index1])) {
                str_value += range->m_field->m_name + ' ';
              }
              str_value += range->m_labels[index1];
            } else {
              std::cout << "IdDictDictionary::unpack - Could not find value." << std::endl;
              std::cout << "value " << value << std::endl;
              std::cout << "field values " << std::endl;
              for (unsigned int i = 0; i < range->m_values.size(); ++i) {
                std::cout << range->m_values[i] << " ";
              }
              std::cout << std::endl;
            }
          }
          break;

        case IdDictRange::unknown:

          std::cout << "unknown" << std::endl;

          break;
        }


        if (index1) unpackedId += sep;
        unpackedId += str_value;
        localPrefix.add(value);

        position -= impl.bits(); // overall bit position



        index1++;  // next field

        if (index1 > index2) break; // quit at index2
      }
      if (selected) break;
    }
  }

  return(0);
}

/**
**  Unpack a single field of the bits32 id. One specifies the
**  field index desired and passes in the region index to be used
**  for decoding. The region index is obtained from
**  find_region. The first_field_index is normally 0. It is
**  non-zero when fields 0 to first_field_index -1 are missing
**  from the compact
*/
int
IdDictDictionary::unpack(const Identifier& id,
                         size_t first_field_index,
                         size_t field_index,
                         size_t region_index,
                         int& field) const {
  field = 0;

  if (m_do_checks) {
    // Check regions
    if (region_index >= m_regions.size()) {
      std::cout << "IdDictDictionary::unpack - region index too large. Index, nregions "
                << region_index << " " << m_regions.size() << std::endl;
      return(1);
    }
  }

  const IdDictRegion& region = *m_regions.at(region_index);

  if (m_do_checks) {
    // check number of fields
    if (field_index >= region.m_implementation.size()) {
      std::cout << "IdDictDictionary::unpack - field index too large. Index, nfields "
                << field_index << " " << region.m_implementation.size()
                << std::endl;
      return(1);
    }
  }

  const IdDictFieldImplementation& impl = region.m_implementation.at(field_index);
  size_t prefix_offset = 0;

  size_t position = Identifier::NBITS; // overall bit position

  // One or more fields missing from prefix, get the offset
  if (first_field_index) {
    if (m_do_checks) {
      if (first_field_index >= region.m_implementation.size()) {
        std::cout << "IdDictDictionary::unpack - first_field_index too large. Index, nfields "
                  << first_field_index << " " << region.m_implementation.size()
                  << std::endl;
        return(1);
      }
    }

    // One or more fields missing from prefix, get the offset
    prefix_offset = region.m_implementation[first_field_index].bits_offset();

    if (m_do_checks) {
      // Should have a non-zero number of bits
      if (impl.bits() == 0) {
        std::cout << "IdDictDictionary::unpack - no bits for this field. Region, field indexes "
                  << region_index << " " << field_index << std::endl;
        return(1);
      }

      // Check the shift value
      if (impl.bits() + impl.bits_offset() - prefix_offset > position) {
        std::cout << "IdDictDictionary::unpack - bits + offset too large. Region, field indexes, bits, offset "
                  << region_index << " " << field_index << " "
                  << impl.bits() << " " << impl.bits_offset() << " " << prefix_offset
                  << std::endl;
        return(1);
      }
    }
  }

  Identifier::value_type mask = (static_cast<Identifier::value_type>(1) << impl.bits()) - 1;

  size_t index = id.extract(position -= impl.bits() + impl.bits_offset() - prefix_offset, mask);

  field = index;
  if (impl.decode_index()) field = impl.ored_field().get_value_at(index);


  return(0);
}

/**
**  Copy a number of fields of the bits32 id into another bits32
**  id. One specifies the begin and end (inclusive) field indexes
**  desired and passes in the region index to be used for
**  decoding. The region index is obtained from find_region.  The
**  first_field_index is normally 0. It is non-zero when fields 0 to
**  first_field_index -1 are missing from the compact. In this case,
**  idout is shift by the number of missing bits, if possible.
*/
int
IdDictDictionary::copy(const Identifier& idin,
                       size_t first_field_index,
                       size_t begin_field_index,
                       size_t end_field_index,
                       size_t region_index,
                       Identifier& idout) const {
  idout = Identifier();
  if (region_index >= m_regions.size()) {
    std::cout << "IdDictDictionary::copy - region index too large. Index, nregions "
              << region_index << " " << m_regions.size() << std::endl;
    return(1);
  }

  const IdDictRegion& region = *m_regions[region_index];

  if (first_field_index >= region.m_implementation.size() ||
      begin_field_index >= region.m_implementation.size() ||
      end_field_index >= region.m_implementation.size()) {
    std::cout << "IdDictDictionary::copy - field index too large. Indexes first, begin, end, nfields "
              << first_field_index << " "
              << begin_field_index << " "
              << end_field_index << " "
              << region.m_implementation.size()
              << std::endl;
    return(1);
  }

  size_t missing_offset = 0;
  if (first_field_index) {
    if (first_field_index > begin_field_index) {
      std::cout << "IdDictDictionary::copy - first_field_index > begin_field_index. Indexes "
                << first_field_index << " " << begin_field_index << std::endl;
      return(1);
    }
    // One or more fields missing from prefix, get the offset
    missing_offset = region.m_implementation[first_field_index].bits_offset();
  }

  size_t prefix_offset = 0;
  if (begin_field_index) {
    if (begin_field_index > end_field_index) {
      std::cout << "IdDictDictionary::copy - begin_field_index > end_field_index. Indexes "
                << begin_field_index << " " << end_field_index << std::endl;
      return(1);
    }
    // One or more fields missing from prefix, get the offset
    prefix_offset = region.m_implementation[begin_field_index].bits_offset();
  }

  const IdDictFieldImplementation& impl = region.m_implementation[end_field_index];
  size_t suffix_offset = impl.bits() + impl.bits_offset();

  size_t position = Identifier::NBITS; // overall bit position

  if (position < prefix_offset - missing_offset) {
    std::cout << "IdDictDictionary::copy - position < prefix + missing. "
              << prefix_offset << " " << missing_offset << std::endl;
    return(1);
  }


  if (position < suffix_offset + missing_offset) {
    std::cout << "IdDictDictionary::copy - position < suffix + missing. "
              << suffix_offset << " " << missing_offset << std::endl;
    return(1);
  }


  // prepare the mask for copying

  Identifier::value_type mask = Identifier::ALL_BITS;

  Identifier::value_type prefix_mask =
    prefix_offset ? (static_cast<Identifier::value_type>(1) << (position - prefix_offset + missing_offset)) - 1 : 0;

  Identifier::value_type suffix_mask =
    (static_cast<Identifier::value_type>(1) << (position - suffix_offset + missing_offset)) - 1;

  mask -= prefix_mask + suffix_mask;

  idout = idin.mask_shift(mask, missing_offset);


  return(0);
}

bool
IdDictDictionary::do_checks(void) const {
  return(m_do_checks);
}

void
IdDictDictionary::set_do_checks(bool do_checks) {
  m_do_checks = do_checks;
}

bool
IdDictDictionary::do_neighbours(void) const {
  return(m_do_neighbours);
}

void
IdDictDictionary::set_do_neighbours(bool do_neighbours) {
  m_do_neighbours = do_neighbours;
}

/**
 *
 *   Here, we verify global constraints :
 *   (this must only be applied after the resolve_references and
 *    generate_implementation operations)
 *
 *   a) There should be no overlap between any region duet.
 *
 *   b) In a dictionary, a given field (ie. with a given name) must always be
 *     referenced at the same depth across all regions and subregions.
 *
 *   c) When a region specifies a reference to a sub-dictionary :
 *      - the reference must be the last entry in the region
 *      - the first region entries of all regions should not overlap (regardless
 *        of the definition of the sub-dictionary). IE the first fields in
 *        these regions must be sufficient to unambiguously select the
 *        sub-dictionary.
 *
 *   d) Within a dictionary, a given field must always be described either through
 *     an enumeration, or through its bounds. However, the various usages of a
 *     given field may specify different tag set (for enumerations) or bounds (for
 *     a bounded field).
 *
 */
bool IdDictDictionary::verify() const {
  // check #1

  MultiRange mr = build_multirange();

  if (mr.has_overlap()) return(false);


  return(true);
}

/**
 *
 *   Sort:
 *
 *      Prerequisite: first verify the dictionary - no overlaps
 *
 *      Then loop over regions and sort according to their first
 *      identifier
 *
 */
void IdDictDictionary::sort() {
  // verify
  if (verify()) {
    std::map< ExpandedIdentifier, IdDictDictEntry* > regions;

    IdDictDictionary::groups_it it;

    for (it = m_groups.begin(); it != m_groups.end(); ++it) {
      (*it)->sort();
    }
  } else {
    std::cout << "IdDictDictionary::sort - WARNING verify is FALSE - cannot sort "
              << std::endl;
  }
}

void IdDictDictionary::clear() {
  {
    std::map<std::string, IdDictSubRegion*>::iterator it;

    for (it = m_subregions.begin(); it != m_subregions.end(); ++it) {
      IdDictSubRegion* subregion = (*it).second;
      subregion->clear();
      delete subregion;
    }

    m_subregions.clear();
  }

  {
    std::map<std::string, IdDictField*>::iterator it;

    for (it = m_fields.begin(); it != m_fields.end(); ++it) {
      IdDictField* field = (*it).second;
      field->clear();
      delete field;
    }

    m_fields.clear();
  }

  {
    IdDictDictionary::groups_it it;

    for (it = m_groups.begin(); it != m_groups.end(); ++it) {
      IdDictGroup* group = *it;
      group->clear();
      delete group;
    }

    m_groups.clear();
  }
}
