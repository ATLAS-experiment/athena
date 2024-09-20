/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "Identifier/RangeIterator.h"
#include "Identifier/Range.h"



//----------------------------------------------- 
RangeIterator
RangeIterator::begin() const{ 
  return (*this); 
} 
 
//----------------------------------------------- 
ConstRangeIterator
ConstRangeIterator::begin() const { 
  return (*this); 
} 
 
//----------------------------------------------- 
RangeIterator 
RangeIterator::end () const { 
  Range r; 
  RangeIterator factory(r); 
  return (factory); 
} 
 
//----------------------------------------------- 
ConstRangeIterator
ConstRangeIterator::end () const { 
  static const Range r; 
  static const ConstRangeIterator factory(r); 
  return (factory); 
} 
 

 
//----------------------------------------------- 
RangeIterator::RangeIterator (Range& range) : m_range (&range) { 
  /** 
   *    Fill all running identifiers 
   *    m_id : the current id 
   *    m_min : the set of low bounds 
   *    m_max : the set of high bounds 
   */ 
  for (Range::size_type i = 0; i < range.fields (); ++i) { 
      Range::element_type minimum; 
      Range::element_type maximum; 
      m_indices.push_back (0); 
      const Range::field& f = range[i]; 
      switch (f.get_mode ()) { 
        case Range::field::unbounded: 
          m_id << 0; 
          m_min << 0; 
          m_max << 0; 
          break; 
        case Range::field::low_bounded: 
          minimum = f.get_minimum (); 
          m_id << minimum; 
          m_min << minimum; 
          m_max << minimum; 
          break; 
        case Range::field::high_bounded: 
          maximum = f.get_maximum (); 
          m_id << maximum; 
          m_min << maximum; 
          m_max << maximum; 
          break; 
        case Range::field::both_bounded: 
        case Range::field::enumerated: 
          minimum = f.get_minimum (); 
          maximum = f.get_maximum (); 
          m_id << minimum; 
          m_min << minimum; 
          m_max << maximum; 
          break; 
        default:
         throw std::runtime_error("Mode not recognised in RangeIterator::RangeIterator.");
         break;
        } 
    } 
} 


 
//----------------------------------------------- 
RangeIterator&
RangeIterator::operator ++() { 
  if (m_id.fields () == 0) return *this; 
  Range::size_type fields = m_id.fields (); 
  Range::size_type i = fields - 1; 
 
    // 
    // Starting from the end, we try to increment the m_id fields 
    // If at a given position it's not possible (max reached) 
    // then we move back one pos and try again. 
    // 
    //  As soon as increment is possible, then the rest of the m_id 
    // is reset to min values. 
    // 
 
  for (;;) { 
      const Range::field& f = (*m_range)[i]; 
      bool done = false; 
      Range::element_type value = 0; 
 
      if (f.get_mode () == Range::field::enumerated) { 
          Range::size_type index = m_indices[i]; 
          index++; 
          if (index < f.get_indices ()) { 
              m_indices[i] = index; 
              value = f.get_value_at (index); 
              done = true; 
            } 
        } else { 
          value = m_id[i]; 
          if (value < m_max[i]) { 
              /** 
               *   The local range is not exceeded. 
               *   increase the value then reset the remaining fields. 
               */ 
              ++value; 
              done = true; 
          } 
      } 
      if (done) { 
          m_id[i] = value; 
          for (++i; i < fields; ++i) { 
              m_indices[i] = 0; 
              m_id[i] = m_min[i]; 
          } 
           
          break; 
        } 
 
      /** 
       *  The current range field was exhausted 
       *  check the previous one. 
       */ 
 
      if (i == 0) { 
          m_id.clear (); 
          break; 
      } 
        
      --i; 
        
    } 
    return *this;
} 
 
//----------------------------------------------- 
ExpandedIdentifier& 
RangeIterator::operator *() { 
  return (m_id); 
} 
 
//----------------------------------------------- 
bool 
RangeIterator::operator == (const RangeIterator& other) const { 
  if (m_id == other.m_id) return (true); 
  return (false); 
} 


 
//----------------------------------------------- 
ConstRangeIterator::ConstRangeIterator (const Range& range) :  
  m_range (&range) { 
  /** 
   *    Fill all running identifiers 
   *    m_id : the current id 
   *    m_min : the set of low bounds 
   *    m_max : the set of high bounds 
   */ 
  for (Range::size_type i = 0; i < range.fields (); ++i) { 
      Range::element_type minimum; 
      Range::element_type maximum; 
      m_indices.push_back (0); 
      const Range::field& f = range[i]; 
      switch (f.get_mode ()) { 
        case Range::field::unbounded: 
          m_id << 0; 
          m_min << 0; 
          m_max << 0; 
          break; 
        case Range::field::low_bounded: 
          minimum = f.get_minimum (); 
          m_id << minimum; 
          m_min << minimum; 
          m_max << minimum; 
          break; 
        case Range::field::high_bounded: 
          maximum = f.get_maximum (); 
          m_id << maximum; 
          m_min << maximum; 
          m_max << maximum; 
          break; 
        case Range::field::both_bounded: 
        case Range::field::enumerated: 
          minimum = f.get_minimum (); 
          maximum = f.get_maximum (); 
          m_id << minimum; 
          m_min << minimum; 
          m_max << maximum; 
          break;
        default:
          throw std::runtime_error("Mode not recognised in ConstRangeIterator::ConstRangeIterator");
          break;
        } 
    } 
} 

 
//----------------------------------------------- 
ConstRangeIterator 
ConstRangeIterator::operator ++() { 
  if (m_id.fields () == 0) return *this; 
  Range::size_type fields = m_id.fields (); 
  Range::size_type i = fields - 1; 
  // 
  // Starting from the end, we try to increment the m_id fields 
  // If at a given position it's not possible (max reached) 
  // then we move back one pos and try again. 
  // 
  //  As soon as increment is possible, then the rest of the m_id 
  // is reset to min values. 
  // 
  for (;;){ 
      const Range::field& f = (*m_range)[i]; 
      bool done = false; 
      Range::element_type value = 0; 
 
      if (f.get_mode () == Range::field::enumerated) { 
          Range::size_type index = m_indices[i]; 
          index++; 
          if (index < f.get_indices ()) { 
              m_indices[i] = index; 
              value = f.get_value_at (index); 
              done = true; 
          } 
        }else { 
          value = m_id[i]; 
          if (value < m_max[i]){ 
              /** 
               *   The local range is not exceeded. 
               *   increase the value then reset the remaining fields. 
               */ 
              ++value; 
              done = true; 
            } 
        } 
 
      if (done) { 
          m_id[i] = value; 
          for (++i; i < fields; ++i) { 
              m_indices[i] = 0; 
              m_id[i] = m_min[i]; 
          } 
          break; 
        } 
 
      /** 
       *  The current range field was exhausted 
       *  check the previous one. 
       */ 
 
      if (i == 0) { 
          m_id.clear (); 
          break; 
      } 
      --i; 
        
    } 
    return *this;
} 
 
//----------------------------------------------- 
const ExpandedIdentifier& 
ConstRangeIterator::operator *() const { 
  return (m_id); 
} 
 
//----------------------------------------------- 
bool 
ConstRangeIterator::operator == (const ConstRangeIterator& other) const { 
  if (m_id == other.m_id) return (true); 
  return (false); 
} 