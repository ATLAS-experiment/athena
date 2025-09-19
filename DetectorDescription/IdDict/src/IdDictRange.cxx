/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */
#include "IdDict/IdDictRange.h"
#include "IdDict/IdDictField.h"
#include "IdDict/IdDictRegion.h"
#include "IdDict/IdDictMgr.h"
#include "IdDict/IdDictDictionary.h"
#include "IdDict/IdDictFieldImplementation.h"

#include "src/Debugger.h"
#include <iostream>


void
IdDictRange::resolve_references(const IdDictMgr& /*idd*/,
                                IdDictDictionary& dictionary, IdDictRegion& /*region*/) {
  if (!m_resolved_references) {
    m_field = dictionary.find_field(m_field_name);
    if (m_field == nullptr) {
      m_field = new IdDictField;
      m_field->m_name = m_field_name;
      dictionary.add_field(m_field);
    }

    if (m_specification == unknown) {
      /**
       *  The range values were unspecified in the range element.
       *
       *  Therefore, the set of values must be obtained from the
       * field definition (if it exists!!).
       *
       *  If the field is undefined, then too bad, this range will be dummy.
       *
       */
      unsigned int labels = m_field->get_label_number();
      if (labels == 1) {
        m_specification = by_label;
        m_label = m_field->get_label(0);
      } else if (labels > 1) {
        m_specification = by_labels;
        for (size_t i = 0; i < labels; ++i) {
          m_labels.push_back(m_field->get_label(i));
        }
      }
    }

    if (m_specification == by_label) {
      m_value = m_field->get_label_value(m_label);
    } else if (m_specification == by_labels) {
      m_values.clear();
      for (size_t i = 0; i < m_labels.size(); ++i) {
        const std::string& label = m_labels[i];
        int value = m_field->get_label_value(label);
        m_values.push_back(value);
      }
    }
    m_resolved_references = true;
  }
}

void
IdDictRange::generate_implementation(const IdDictMgr& /*idd*/,
                                     IdDictDictionary& dictionary, IdDictRegion& region, const std::string& /*tag*/) {
  // Add IdDictFieldImplementation to this region

  // NOTE: we DO NOT protect this method with
  // m_generated_implementation because the same object may be
  // called more than once because there are IdDictRangeRef's which
  // point to the same IdDictRange's.

  if (Debugger::debug()) {
    std::cout << "IdDictRange::generate_implementation>" << std::endl;
  }

  region.m_implementation.resize(region.m_implementation.size() + 1);
  IdDictFieldImplementation& impl = region.m_implementation.back();
  impl.set_range(this);
  if (m_field->index() == 0) {
    m_field->m_index = region.fieldSize() - 1;
  } else if (m_field->index() != (region.fieldSize() - 1)) {
    std::cout << "Bad field index for " << m_field_name
              << " index " << m_field->index()
              << " in dictionary " << dictionary.name()
              << " region #" << region.index()
              << " group " << region.m_group
              << " tag " << region.m_tag
              << " size " << (region.m_implementation.size() - 1)
              << std::endl;
  }

  size_t index = region.m_implementation.size() - 1;
  if (region.m_implementation.size() <= index) {
    std::cout << "IdDictRange::generate_implementation: index >= impl size - "
              << index << " " << region.fieldSize()
              << std::endl;
    return;
  }

  Range::field field;
  switch (m_specification) {
  case by_value:
  case by_label: {
    impl.set_field(Range::field(m_value, m_value));
    break;
  }

  case by_values:
  case by_labels: {
    impl.set_field(Range::field(m_values));
  }
  break;

  case by_minmax:
    impl.set_field(Range::field(m_minvalue, m_maxvalue));
    break;

  case unknown:
    break;
  }
}

Range
IdDictRange::build_range() const {
  Range result;

  Range::field field;
  switch (m_specification) {
  case by_value:
  case by_label: {
    field.set(m_value, m_value);
    break;
  }

  case by_values:
  case by_labels: {
    field.set(m_values);
    break;
  }

  case by_minmax: {
    field.set(m_minvalue, m_maxvalue);
    break;
  }

  case unknown: {
    break;
  }
  }
  if (wrap_around == m_continuation_mode) {
    field.set(true);
  } else if (has_previous == m_continuation_mode) {
    field.set_previous(m_prev_value);
  } else if (has_next == m_continuation_mode) {
    field.set_next(m_next_value);
  } else if (has_both == m_continuation_mode) {
    field.set_previous(m_prev_value);
    field.set_next(m_next_value);
  }
  result.add(std::move(field));
  return(result);
}
