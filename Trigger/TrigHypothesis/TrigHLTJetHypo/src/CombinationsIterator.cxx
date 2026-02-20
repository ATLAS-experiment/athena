/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "CombinationsIterator.h"
#include <iostream>

std::ostream& operator << (std::ostream& os, const CombinationsIterator& iter) {
  os << std::boolalpha
     << " generator: " << iter.m_gen
     << " end: "  << iter.m_end
     << " input vals: ";
  for(const auto& hj : iter.m_input_vals) {
    os  << static_cast<const void*>(hj.get()) << '\n';
  }

  os << '\n' <<" vals: ";

  for(const auto& hj : iter.m_vals) {
    os  << static_cast<const void*>(hj.get()) << '\n';
  }
  os<<std::noboolalpha; //restore stream state
  return os;
}

CombinationsIterator::CombinationsIterator(std::size_t k,
					   const HypoJetVector& iv,
					   bool end):
  m_gen{CombinationsGenerator(iv.size(), k)},
  m_k{k},
  m_input_vals{iv},
  m_end{end}
{
  auto indices = m_gen.get();
  std::transform(indices.cbegin(),
		 indices.cend(),
		 std::back_inserter(m_vals),
		 [iv = this->m_input_vals](const auto& ind) {
		   return iv.at(ind);
		 });
}


CombinationsIterator CombinationsIterator::endIter() const {
  return CombinationsIterator(m_k, m_input_vals, true);
}
