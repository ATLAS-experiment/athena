/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


// gcc12 gives false positive warnings from copying boost::small_vector,
// as the sort operation in show_all_ids.
#if __GNUC__ >= 12
# pragma GCC diagnostic ignored "-Wstringop-overread"
#endif


#include "Identifier/Range.h" 
#include <algorithm> 
#include <cstdio> 
#include <string> 
#include <vector> 
 
#include <limits>
#include <iostream> 
#include <iomanip> 
#include <stdexcept>
#include <cassert> 

 

//--------------------------------------------------------------------------
const Range::field& Range::operator [] (Range::size_type index) const { 
  if (index >= m_fields.size ()){ 
      static const field f; 
      return (f); 
  } 
  return (m_fields[index]); 
} 
 
//----------------------------------------------- 
Range::Range (const Range& other, size_type start) { 
  if (start < other.fields ()) { 
    field_vector::const_iterator it = other.m_fields.begin (); 
    it += start; 
    m_fields.insert (m_fields.end (), it, other.m_fields.end ()); 
  } 
} 
 
Range::Range(const std::string & text):Range(){
   if (text.empty()) return;
   std::istringstream in(text);
   in>>*this;
}
 
//----------------------------------------------- 
Range::Range (const ExpandedIdentifier& root) { 
  // Construct from a root (i.e. add wild card for below) 
  build (root); 
} 
 
//----------------------------------------------- 
void Range::build (const std::string& text) { 
  if (text.empty()) return;
  std::istringstream in(text);
  in>>*this;
} 
 
//----------------------------------------------- 
void Range::build (const ExpandedIdentifier& root) { 
    // Construct from a root  
  m_fields.clear (); 
  for (size_type i = 0; i < root.fields (); ++i){ 
    m_fields.emplace_back(root[i]); 
  } 
} 
 

 
//----------------------------------------------- 
void Range::add (element_type value) { 
  m_fields.emplace_back (value); 
} 
 
//----------------------------------------------- 
void Range::add (element_type minimum, element_type maximum) { 
  m_fields.emplace_back (minimum, maximum); 
} 
 

 
/// Add a range specified using a field  
void Range::add (const field& f) {
  m_fields.emplace_back(f); 
} 
 
/// Add a range specified using a field, using move semantics.
void Range::add (field&& f) {
  m_fields.emplace_back(std::move(f));
} 
 
/// Append a subrange 
void Range::add (const Range& subrange) { 
  for (size_t i = 0; i < subrange.fields (); ++i) { 
    const field& f = subrange[i]; 
    m_fields.push_back (f); 
  } 
} 

void Range::add (Range&& subrange) {
  if (m_fields.empty())
    m_fields.swap (subrange.m_fields);
  else {
    size_t sz = subrange.m_fields.size();
    m_fields.reserve (m_fields.size() + sz);
    for (size_t i = 0; i < sz; ++i) { 
      m_fields.emplace_back (std::move(subrange.m_fields[i]));
    }
  }
} 




//----------------------------------------------- 
void Range::clear () { 
  m_fields.clear (); 
} 
 
//----------------------------------------------- 
int Range::match (const ExpandedIdentifier& id) const { 

  size_type my_fields = m_fields.size (); 
  const size_type id_fields = id.fields (); 
  
  // More fields in the range than in the identifier will never match. 
  //if (my_fields > id_fields) return (0); 

  // Allow match for id shorter than range - assume "wildcards" for
  // missing id fields
  size_type nfields =  (my_fields > id_fields) ? id_fields : my_fields;
  // Test fields one by one. 
  for (size_type field_number = 0; field_number < nfields; field_number++) { 
      const field& f = m_fields[field_number]; 
      if (!f.match (id[field_number])) return (0); 
  } 
  // All conditions match. 
  return (1); 
} 
 
// Accessors 
 
//----------------------------------------------- 
ExpandedIdentifier Range::minimum () const { 
  size_type my_fields = m_fields.size (); 
  ExpandedIdentifier result; 
  // Copy fields to result 
  for (size_type field_number = 0; field_number < my_fields; field_number++) { 
      const field& f = m_fields[field_number]; 
      // Valued field 
      result << f.get_minimum (); 
  } 
  return (result); 
} 
 
//----------------------------------------------- 
ExpandedIdentifier Range::maximum () const { 
  size_type my_fields = m_fields.size (); 
  ExpandedIdentifier result;
  // Copy fields to result  
  for (size_type field_number = 0; field_number < my_fields; field_number++) { 
      const field& f = m_fields[field_number]; 
       // Normal field 
      result << f.get_maximum (); 
    } 
    return (result); 
} 
 
Range::size_type Range::cardinality () const { 
  size_type result = 1; 
  const Range& me = *this; 
  for (size_type i = 0; i < fields (); ++i) { 
    const field& f = me[i]; 
    result *= f.get_indices (); 
  } 
  return (result); 
} 
 

/**
 *   Get the cardinality from the beginning up to the given ExpandedIdentifier
 */
Range::size_type Range::cardinalityUpTo (const ExpandedIdentifier& id) const { 
  size_type result = 0; 
  if (id.fields() != fields()) return (result);
  const Range& me = *this; 
  // Check if we are above or below this range
  if (id < minimum()) return 0;
  if (maximum() < id) return cardinality();
  // Collect the indices of a match
  std::vector<size_type> indices(id.fields (), 0);
  bool is_match = true;
  size_type level = 0;
  for (; level < id.fields (); ++level) {
    const field& f = me[level]; 
    // Require all fields to be bounded or enumerated
    if (f.empty()) return 0;
    if (f.isEnumerated()) {
      // Continue testing for a match
      size_type max = f.get_values().size() - 1;
      if (f.get_values()[max] < id[level]) {
        // above max
        is_match = false;
        indices[level] = max + 1;
      } else {
	      for (size_type j = 0; j < f.get_values().size(); ++j) {
		      if (id[level] <= f.get_values()[j]) {
		        if (id[level] != f.get_values()[j]) {
              // between two values or below first one
              is_match = false;
		        }
		        indices[level] = j;
		        break;
		      }
	      }
	    }
    } else {
      if (f.get_maximum() < id[level]) {
          // above max
          is_match = false;
          indices[level] = f.get_maximum() - f.get_minimum() + 1;
      } else if (id[level] < f.get_minimum()) {
          // below min
          is_match = false;
          indices[level] = 0;
      } else {
          indices[level] = id[level] - f.get_minimum();
      }
    }
    if (!is_match) break;
  }
  
  // Calculate the cardinality
  if (level < id.fields ()) ++level;
  for (size_type j = 0; j < level; ++j) {
    size_type card = indices[j];
    for (size_type k = j + 1; k < id.fields(); ++k) {
	    const field& f = me[k]; 
	    card *= f.get_indices();
    }
    result += card;
  }
  return result;
} 
 
 
/** 
 *   Check overlap between two Ranges : 
 * 
 *   As soon as one pair of corresponding fields do not match, 
 *   the global overlap is empty. 
 */ 
bool Range::overlaps_with (const Range& other) const { 
  const Range& me = *this; 
  if ((fields () == 0) || (other.fields () == 0)) return (false); 
  for (size_type i = 0; i < std::min (fields (), other.fields ()); ++i){ 
    const field& f1 = me[i]; 
    const field& f2 = other[i]; 
    if (!f1.overlaps_with (f2)) return (false); 
  } 
  return (true); 
} 
 
//----------------------------------------------- 
void Range::show () const {
  show (std::cout);
}

void Range::show (std::ostream& s) const { 
  const Range& me = *this; 
  s << (std::string) me << " (";
  int allbits = 0;
  for (size_type i = 0; i < fields (); ++i) { 
    const field& f = me[i]; 
    if (i > 0) s << "+";
    size_type indices = f.get_indices ();
    int bits = 1;
    indices--;
    if (indices > 0) {
      bits = 0;
      while (indices > 0){
        indices /= 2;
        bits++;
      }
    }
    allbits += bits;
    s << bits; 
  } 
  s << "=" << allbits << ") ";
} 
 
//----------------------------------------------- 
Range::operator std::string () const { 
  std::string result; 
  size_type my_fields = m_fields.size (); 
  if (my_fields == 0) return (result); 
    // print fields one by one. 
  for (size_type field_number = 0; field_number < my_fields; field_number++) { 
    const field& f = m_fields[field_number]; 
    if (field_number > 0) result += "/"; 
    result += (std::string) f; 
  } 
  return (result); 
} 
 
//----------------------------------------------- 
bool 
Range::operator == (const Range& other) const{
  if (m_fields.size() != other.m_fields.size()) return false;
  field_vector::const_iterator it1  = m_fields.begin();
  field_vector::const_iterator it2  = other.m_fields.begin();
  field_vector::const_iterator last = m_fields.end();
  for (; it1 != last; ++it1, ++it2) {
	  if ((*it1) != (*it2)) return false;
  }
  return (true);
}






std::ostream & 
operator << (std::ostream &out, const Range &r){
  out<<std::string(r);
  return out;
}

std::istream & 
operator >> (std::istream &in, Range &r){
  r.clear ();
  for (int c{}; c!=EOF;c=in.peek()){
    Range::field field; 
    in>>field;
    r.add(field);
    if (int c = in.peek();(c == '/') or (c ==' ')){  
      in.ignore();
    }
  }
  return in;
}
