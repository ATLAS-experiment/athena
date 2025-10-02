/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "Identifier/IdentifierField.h"
#include "src/IdentifierFieldParser.h"
#include <algorithm>
#include <iostream>
#include <bit> //std::bit_width
#include <array>
#include <cctype> //std::isspace
#include <format>
#include <tuple> //for std::tie
#include <functional> //std::less

using namespace Identifier;

namespace{
template<class SetA, class SetB, typename Compare = std::less<>>
bool 
share_element(SetA&& setA, SetB&& setB, Compare comp = Compare{}){
    auto xA = setA.begin();
    auto xB = setB.begin();
    while (xA != setA.end() && xB != setB.end()){
        if (comp(*xA, *xB)) {
            ++xA;
        } else if (comp(*xB, *xA)) {
            ++xB;
        } else {
            return true;
        }
    }
    return false;
}

}


//------------------------------------------------------------------
ExpandedIdentifier::size_type IdentifierField::get_bits() const{
  size_t indices = get_indices ();
  if (--indices) return  std::bit_width(indices);
  return 1;
}

 
//----------------------------------------------- 
ExpandedIdentifier::size_type 
IdentifierField::get_value_index(element_type value) const{
  // Only both_bounded and enumerated are valid to calculate the
  // index.
  // both_bounded if the more frequent case and so comes first.
  if (isBounded()) {
    return (value - m_minimum); 
  } 
  const auto & v = get_values();
  if (v.size()==1) return 0;
  if (not m_indexes.empty()) {
    // Table has been created, do simple lookup
    assert (value >= m_minimum && value - m_minimum < (int)m_indexes.size());
    return (m_indexes.at(value - m_minimum));
  } else {
    auto it = std::ranges::find(v, value);
    if (it != v.end()) return std::distance(v.begin(), it);
  }
  return 0;
}

//----------------------------------------------- 
bool 
IdentifierField::match(element_type value) const {  
  if (isBounded()) {
    return ((value >= m_minimum) && (value <= m_maximum)); 
  }
  if (m_empty) return true;
  const auto & v = get_values();
  return (std::ranges::find(v, value)!=v.end());
} 


 
//----------------------------------------------- 
IdentifierField::IdentifierField (element_type value) 
    :
    m_minimum(value),
    m_maximum(value),
    m_data{BoundedRange{value, value}},
    m_empty{false}{
  //    
} 
 
//----------------------------------------------- 
IdentifierField::IdentifierField (element_type minimum, element_type maximum){ 
  set(minimum, maximum);    
} 


/// Create with enumerated values
IdentifierField::IdentifierField (const element_vector &values){ 
  set(values);     
}
 
 
//----------------------------------------------- 
bool 
IdentifierField::get_previous(element_type current, element_type& previous) const{
  if (m_empty){
    previous = current - 1; 
    return (current != minimum_possible);
  }
  if (isBounded()){
    if (current == m_minimum) {
      if (has_wrap_around == m_continuation_mode) {
          previous = m_maximum;
          return (true); 
      }
      if (has_previous == m_continuation_mode || has_both == m_continuation_mode) {
          previous = m_previous;
          return (true); 
      }
      previous = current;
      return (false);
    }
    previous = current - 1; 
    return (true); 
  }
  const auto & values= get_values();
  size_type index = get_value_index(current);
  if (index == 0) {
    if (has_wrap_around == m_continuation_mode) {
        previous = values.back();
        return (true); 
    }
    if (has_previous == m_continuation_mode || has_both == m_continuation_mode) {
        previous = m_previous;
        return (true); 
    }
    previous = current;
    return (false);
  }
  --index;
  previous = values[index];
  return true;
}


//----------------------------------------------- 
bool 
IdentifierField::get_next(element_type current, element_type& next) const{
  if (m_empty){
    next = current + 1; 
    return (current != maximum_possible);
  }
    
  if (isBounded()){
    if (current == m_maximum) {
      if (has_wrap_around == m_continuation_mode) {
          next = m_minimum;
          return (true); 
      }
      if (has_next == m_continuation_mode || has_both == m_continuation_mode) {
          next = m_next;
          return (true); 
      }
      next = current;
      return (false);
    }
    next = current + 1; 
    return (true); 
  }
  const auto & values= get_values();
  size_type index = get_value_index(current);
  if ((index == values.size() - 1) || (index == 0 && current != values.front())) {
    if (has_wrap_around == m_continuation_mode) {
        next = values.front();
        return (true); 
    }
    if (has_next == m_continuation_mode || has_both == m_continuation_mode) {
        next = m_next;
        return (true); 
    }
    next = current;
    return (false);
  }
  ++index;
  next = values[index];
  return true;
}

 
 
//----------------------------------------------- 
bool 
IdentifierField::overlaps_with (const IdentifierField& other) const { 
  if (m_empty or other.m_empty) return true;
  //
  if (isBounded() and other.isBounded()){
    return ((m_minimum <= other.m_maximum) && (m_maximum >= other.m_minimum));
  }
  //
  if (isBounded() and other.isEnumerated()  ){
    const element_vector& ev = other.get_values(); 
    for (const auto & v: ev) { 
      if (v >= m_minimum and v<= m_maximum) return (true); 
    } 
    return false;
  } else if (isEnumerated() and other.isBounded()){
    const element_vector& ev = get_values(); 
    for (const auto & v: ev){ 
      if ((v >= other.m_minimum) and (v <= other.m_maximum)) return (true); 
    } 
    return false;
  } 
  // Both fields are enumerated only if there is possibility of overlap 
  if ((m_minimum <= other.m_maximum) && (m_maximum >= other.m_minimum))  { 
    return share_element(get_values(), other.get_values());
  } 
  return (false); 
} 
 

//----------------------------------------------- 
void 
IdentifierField::clear () { 
  m_minimum  = 0; 
  m_maximum  = 0; 
  m_previous = 0;
  m_next     = 0;
  m_continuation_mode = none;
  m_data = element_vector{};
  m_indexes.clear();
  m_empty = true; 
} 
 
//----------------------------------------------- 
void 
IdentifierField::set(element_type minimum, element_type maximum)  { 
  if (minimum == maximum) {
      add_value(minimum);
  } else {
    std::tie(m_minimum,m_maximum)  = std::minmax(minimum,maximum); 
    m_data = BoundedRange{m_minimum,m_maximum};
    m_size = m_maximum - m_minimum + 1;
  }
  m_empty=false;
} 

//----------------------------------------------- 
void 
IdentifierField::add_value(element_type value) {
  if (auto * p = dataPtr<element_vector>(); !p)  {
    clear();
    m_data = element_vector(1,value);//now its enumerated
    m_size = 1;
  } else {
    //check whether value already exists in the enumeration vector
    if (std::ranges::binary_search(*p, value)) return;
    p->push_back(value); 
    std::ranges::sort(*p); 
    m_size = p->size();
  }
  m_minimum = get_values().front(); 
  m_maximum = get_values().back(); 
  m_empty = false;
} 
 
//----------------------------------------------- 
void 
IdentifierField::set(const std::vector <element_type>& values) {
  auto * p = dataPtr<element_vector>();
  if (not p) {
    clear();
    p = dataPtr<element_vector>();
  }
  p->insert(p->end(), values.begin(), values.end());
  std::ranges::sort (*p);
  //ensure duplicates are taken out
  p->erase( std::unique( p->begin(), p->end() ), p->end() );
  m_minimum = p->front(); 
  m_maximum = p->back(); 
  m_empty=false;
  m_size = p->size();
} 
 
//----------------------------------------------- 
void 
IdentifierField::set(bool wraparound) { 
  if (wraparound) {
    m_continuation_mode = has_wrap_around;
  }
} 
 
//----------------------------------------------- 
void 
IdentifierField::set_next (int next) {
  if (has_previous == m_continuation_mode) {
    m_continuation_mode = has_both;
  } else {
    m_continuation_mode = has_next;
  }
  m_next = next;
}


//----------------------------------------------- 
void 
IdentifierField::set_previous (int previous) {
  if (has_next == m_continuation_mode) {
    m_continuation_mode = has_both;
  } else {
    m_continuation_mode = has_previous;
  }
  m_previous = previous;
}


//----------------------------------------------- 
void 
IdentifierField::operator |= (const IdentifierField& other) {
  if (m_empty or other.m_empty){
    clear();
    m_size = 1;
    return;
  } 
  if (isEnumerated() and other.isEnumerated()){
     set(other.get_values ());
     return;
  }
  /**
  *  If there is no overlap we should build a multi-segment specification.
  *  The current algorithm is only correct if the overlap in not empty !!
  *   A multi-segment specification might also be implemented as an 
  *  expanded enumerated set (not very optimized !!)
  */
  // all other cases...
  const auto min = std::min(m_minimum, other.m_minimum);
  const auto max = std::max(m_maximum, other.m_maximum);
  set(min, max);
}


//----------------------------------------------- 
IdentifierField::operator std::string () const { 
  std::string result; 
  if (m_empty) { 
      result = "*"; 
  }  else  { 
    const auto & [minimum, maximum]  = get_minmax(); 
    if (minimum == maximum)  { 
      result =  std::to_string(minimum);
    } else  { 
      if (isEnumerated())  { 
        std::string prefix;
        for (size_type i = 0; i < get_indices (); ++i)  { 
          result += prefix+std::to_string(get_value_at(i));
          prefix = ",";
        } 
      } else {
        try {
          result = std::format("{}:{}", minimum, maximum);
        }
        catch (const std::format_error& e) {
          result = "ERROR ";
          result += e.what();
        }
      } 
    } 
  } 
  return result; 
} 

//-----------------------------------------------
bool 
IdentifierField::operator == (const IdentifierField& other) const {
    if (m_data != other.m_data)   return false;
    return (true);
}


//----------------------------------------------- 
void 
IdentifierField::show() const {
  std::cout << "min/max " << m_minimum << " " << m_maximum << " "; 
  std::cout << "values  ";
  for (const auto& v: get_values()) {
      std::cout << v << " ";
  }
  std::cout << "indexes  ";
  for (const auto & idx: m_indexes) {
      std::cout << idx << " ";
  }
  std::cout << "indices  " << m_size << " ";
  std::cout << "prev  " << m_previous << " ";
  std::cout << "next  " << m_next << " ";
  std::cout << "mode  ";
  if (m_empty){
      std::cout << "unbounded  ";
  }else if (isBounded()){
      std::cout << "both_bounded  ";
  }else if (isEnumerated()) {
      std::cout << "enumerated  ";
  }
  std::cout << "cont mode  ";
  switch (m_continuation_mode) { 
  case IdentifierField::none: 
      std::cout << "none  ";
      break; 
  case IdentifierField::has_next: 
      std::cout << "has_next  ";
      break; 
  case IdentifierField::has_previous: 
      std::cout << "has_previous  ";
      break; 
  case IdentifierField::has_both:
      std::cout << "has_both  ";
      break; 
  case IdentifierField::has_wrap_around:
      std::cout << "has_wrap_around  ";
      break; 
  }
  std::cout << std::endl;
}



//----------------------------------------------- 
void 
IdentifierField::optimize() {
  /// Check mode - switch from enumerated to both_bounded if possible
  if (not check_for_both_bounded()) create_index_table();    
}


//----------------------------------------------- 
bool 
IdentifierField::check_for_both_bounded() {
  if (m_empty) return false;
  if (isEnumerated()) {
    //the enumerated values are kept sorted
    if (m_size-1 == static_cast<size_type>(m_maximum - m_minimum)){
      m_data = BoundedRange{m_minimum, m_maximum}; 
      return true;
    }
  }
  return false;
}

//----------------------------------------------- 
void 
IdentifierField::create_index_table() {
  if (m_empty) return;
  /// Create index table from value table
  if (isEnumerated()) {
    size_type size = m_maximum - m_minimum + 1;
    // return if we are over the maximum desired vector table size  
    if (size > m_maxNumberOfIndices) {
        m_indexes.clear();
        return;
    }
    // Set up vectors for decoding
    m_indexes = std::vector<size_type>(size, 0);
    size_type index{};
    int i{};
    auto &v = std::get<element_vector>(m_data);
    for (const auto & thisValue: v) {
      if (const auto idx=(thisValue - m_minimum); idx < (int)size) {
        m_indexes[idx] = index;
        index++;
      } else {
        std::cout << "size, value, index, i " 
        << size << " " << thisValue << " "
        << index << " " << i++ << "  min, max " 
        << m_minimum << " " 
        << m_maximum 
        << std::endl;
      }
    }
  }
}

std::ostream & 
operator << (std::ostream &out, const IdentifierField &c){
  out<<std::string(c);
  return out;
}

//stream extraction allows to read from text (e.g. text file)
std::istream & 
operator >> (std::istream &is, IdentifierField &idf){
  idf.clear();
  while (std::isspace(is.peek())){is.ignore();}
  char c = is.peek();
  if (c =='*'){
    is.ignore();
    //do nothing; the 'clear' set idf to unbounded
  } else if (isDigit(c)){
    if (c =='+') is.ignore();
    int v = parseStreamDigits(is);//'is' ptr is incremented
    c = is.peek();
    //possible:  bound, list
    if (c == ','){ //found comma, so definitely list
      is.ignore();
      std::vector<int> vec(1,v);
      const auto  & restOfList = parseStreamList(is);
      vec.insert(vec.end(), restOfList.begin(), restOfList.end());
      idf.set(vec);
    } else if (c == ':'){ //bounded
      is.ignore();
      c=is.peek(); //peek char after the colon
      if (isDigit(c)){ //bounded
        int v1 = parseStreamDigits(is);
        idf.set(v,v1);
      } 
    } else { //remaining alternative: single number
      idf.add_value(v);
    }
  } else {
    std::string msg{"Stream extraction for IdentifierField: "};
    std::string remains;
    is >> remains;
    msg+=remains;
    throw std::invalid_argument(msg);
  }
  return is;
}
  
