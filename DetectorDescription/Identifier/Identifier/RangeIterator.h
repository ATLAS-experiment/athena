 
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef Identifier_RangeIterator_h
#define Identifier_RangeIterator_h

#include "Identifier/ExpandedIdentifier.h"
#include <vector>
class Range;

/** 
 *    This iterator is able to generate all possible identifiers, from a  
 *  fully bounded Range. 
 *    The precondition is that the Range used to parameterize the iterator  
 *  must have all its fields completely bounded. 
 */ 
class RangeIterator { 
  public: 
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = ExpandedIdentifier;
    using pointer           = ExpandedIdentifier*;  // or also value_type*
    using reference         = ExpandedIdentifier&;  // or also value_type&
    
    RangeIterator() = default; 
    RangeIterator(Range& range); 
    RangeIterator & operator ++(); 
    
    RangeIterator begin() const; 
    RangeIterator end() const; 
 
    pointer operator->() { return &m_id; }
    ExpandedIdentifier& operator *(); 
    bool operator == (const RangeIterator& other) const; 
 
  private: 
    std::vector<std::size_t> m_indices; 
    ExpandedIdentifier m_id; 
    ExpandedIdentifier m_min; 
    ExpandedIdentifier m_max; 
    const Range* m_range{}; 
}; 
 
class ConstRangeIterator { 
  public: 
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = ExpandedIdentifier;
    using pointer           = ExpandedIdentifier*;  // or also value_type*
    using reference         = ExpandedIdentifier&;  // or also value_type&
    
    ConstRangeIterator () = default; 
    ConstRangeIterator (const Range& range); 
    ConstRangeIterator  begin() const; 
    ConstRangeIterator  end() const; 

    ConstRangeIterator operator ++(); 
 
    const ExpandedIdentifier& operator *() const; 
    bool operator == (const ConstRangeIterator& other) const; 
 
  private: 
    std::vector<std::size_t> m_indices; 
    ExpandedIdentifier m_id; 
    ExpandedIdentifier m_min; 
    ExpandedIdentifier m_max; 
    const Range* m_range{}; 
}; 
  
#endif