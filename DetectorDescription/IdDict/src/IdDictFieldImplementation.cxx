/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

/**
 * @fileIdDictFieldImplementation.cxx
 * @date 2003-09-04 08:55:25
 * @author schaffer
 **/

#include "IdDict/IdDictFieldImplementation.h"
#include <iostream>
#include <format>
#include <string>
#include <string_view>



const IdDictRange*
IdDictFieldImplementation::range() const {return(m_range);}

void
IdDictFieldImplementation::set_range(const IdDictRange* range) {
  m_range = range;
}

void
IdDictFieldImplementation::show() const {
  std::cout << show_to_string() << std::endl;
}

std::string IdDictFieldImplementation::show_to_string() const {
  // Build the indexes list once
  const auto& idx = m_ored_field.get_indexes();
  std::string indexes;
  indexes.reserve(idx.size() * 3);
  for (std::size_t i = 0; i < idx.size(); ++i) {
    if (i) indexes.push_back(' ');
    indexes += std::to_string(idx[i]);
  }
  // Mode string
  std::string_view mode =
      m_ored_field.isBounded()     ? "both_bounded"
    : m_ored_field.isEnumerated()  ? "enumerated"
                                   : "unknown";
  try{
    return std::format(
        "decode {:d} vals {:<15} "
        "mask/zero mask/shift/bits/offset {:<3x} {:<3x} {:<3} {:<3} {:<3} "
        "indexes {:<20} mode  {}  ",
        m_decode_index,
        static_cast<std::string>(m_ored_field),
        m_mask,
        m_zeroing_mask,
        m_shift,
        m_bits,
        m_bits_offset,
        indexes,
        mode);
  } catch (std::format_error & ){
    return {};
  }
}
