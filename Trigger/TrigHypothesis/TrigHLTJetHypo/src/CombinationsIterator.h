/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIG_HLTJETHYPO_COMBINATIONSITERATOR_H
#define TRIG_HLTJETHYPO_COMBINATIONSITERATOR_H

#include "CombinationsGenerator.h"
#include "TrigHLTJetHypo/TrigHLTJetHypoUtils/HypoJetDefs.h"

#include <vector>
#include <iterator>
#include <iosfwd>

/*
 * CombinatonsIterator is a forward iterator. It iterates over
 * combinations (n choose k) gnerated by a CombinationsGenerator instance.
 * of a HypoJetVector.
 *
 * Dereferencing the iterator returns a HypoJetVector corresponding
 * to the current combination.
 *
 * CombinationsIterator can generates the
 * end of iteration CombinationsIterator
 */

class CombinationsIterator {

 public:
  using iterator_category = std::forward_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = HypoJetVector;
  using reference = HypoJetVector&;
  using pointer = HypoJetVector*;

 
  friend std::ostream& operator << (std::ostream&, const CombinationsIterator&);
  CombinationsIterator(std::size_t k,
		       const HypoJetVector& input_vals,
		       bool end=false);

  CombinationsIterator() {}

 
  reference operator*() {return m_vals;}


  pointer operator->() {return &m_vals;}

  // pre-increment
  CombinationsIterator& operator++() {
    m_end = m_gen.bump();
    auto indices = m_gen.get();

    // overwrite m_vals according to the combinations indicies
    std::transform(indices.cbegin(),
		   indices.cend(),
		   m_vals.begin(),
		   [iv = this->m_input_vals](const auto& ind) {
		     return iv.at(ind);
		   });
    
    return *this;
  }

  // post-increment
  CombinationsIterator operator++(int) {
    CombinationsIterator tmp = *this;
    ++(*this);
    return tmp;
  }


  // create an end of iteration marker from a CombinationsIterator
  CombinationsIterator endIter() const;

  // enable an equality test with an end iterator
  friend bool operator==(const CombinationsIterator& a,
			 const CombinationsIterator& b) {
  return a.m_end == b.m_end and
    a.m_k == b.m_k and
    a.m_input_vals == b.m_input_vals;
    
  
  }


  // enable an inequality test with an end iterator
  friend bool operator!=(const CombinationsIterator& a,
			 const CombinationsIterator& b) {
    return !(a==b);
  }


 private:
  CombinationsGenerator m_gen{0,0};
  std::size_t m_k{0};

  HypoJetVector m_input_vals;
  HypoJetVector m_vals;

  bool m_end{false};
};

std::ostream& operator << (std::ostream& os, const CombinationsIterator& iter);


#endif

