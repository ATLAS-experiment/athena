/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


// gcc12 gives false positive warnings from copying boost::small_vector,
// as the sort operation in show_all_ids.
#if __GNUC__ >= 12
# pragma GCC diagnostic ignored "-Wstringop-overread"
#endif


#include "Identifier/Range.h" 
#include "src/RangeParser.h"
#include <algorithm> 
#include <cstdio> 
#include <string> 
#include <vector> 
 
#include <limits>
#include <iostream> 
#include <iomanip> 
#include <set>
#include <stdexcept>
#include <cassert> 

using Identifier::RangeParser;
    // Constructors 
//----------------------------------------------- 
Range::Range () 
{ 
} 
 
//----------------------------------------------- 
Range::Range (const Range& other)
  : m_fields (other.m_fields)
{ 
} 
 
//----------------------------------------------- 
Range::Range (Range&& other)
  : m_fields (std::move (other.m_fields))
{ 
} 
 
//----------------------------------------------- 
Range& Range::operator= (const Range& other) 
{ 
  if (this != &other)
    m_fields = other.m_fields; 
  return *this;
} 
 
//----------------------------------------------- 
Range& Range::operator= (Range&& other) 
{ 
  if (this != &other)
    m_fields = std::move (other.m_fields);
  return *this;
} 

//--------------------------------------------------------------------------
const Range::field& Range::operator [] (Range::size_type index) const 
//--------------------------------------------------------------------------
{ 
  if (index >= m_fields.size ()) 
    { 
      static const field f; 
 
      return (f); 
    } 
 
  return (m_fields[index]); 
} 
 
//----------------------------------------------- 
Range::Range (const Range& other, size_type start) 
{ 
  if (start < other.fields ()) 
    { 
      field_vector::const_iterator it = other.m_fields.begin (); 
      it += start; 
 
      m_fields.insert (m_fields.end (), it, other.m_fields.end ()); 
    } 
} 
 
//----------------------------------------------- 
//  Range::Range (const std::string& text) 
//  { 
//    build (text); 
//  } 
 
//----------------------------------------------- 
Range::Range (const ExpandedIdentifier& root) 
{ 
  // Construct from a root (i.e. add wild card for below) 
  build (root); 
} 
 
//----------------------------------------------- 
void Range::build (const std::string& text) 
{ 
  RangeParser parser; 
 
  parser.run (*this, text); 
} 
 
//----------------------------------------------- 
void Range::build (const ExpandedIdentifier& root) 
{ 
    // Construct from a root  
  m_fields.clear (); 
 
  for (size_type i = 0; i < root.fields (); ++i) 
    { 
      m_fields.push_back (field (root[i])); 
    } 
} 
 
    // Modifications 
//----------------------------------------------- 
void Range::add () 
{ 
  m_fields.push_back (field ()); 
} 
 
//----------------------------------------------- 
void Range::add (element_type value) 
{ 
  m_fields.push_back (field (value)); 
} 
 
//----------------------------------------------- 
void Range::add (element_type minimum, element_type maximum) 
{ 
  m_fields.push_back (field (minimum, maximum)); 
} 
 
//----------------------------------------------- 
void Range::add_minimum (element_type minimum) 
{ 
  field f; 
   
  f.set_minimum (minimum); 
  m_fields.push_back (f); 
} 
 
//----------------------------------------------- 
void Range::add_maximum (element_type maximum) 
{ 
  field f; 
 
  f.set_maximum (maximum); 
  m_fields.push_back (f); 
} 
 
/// Add a range specified using a field  
void Range::add (const field& f) 
{
  m_fields.emplace_back (f); 
} 
 
/// Add a range specified using a field, using move semantics.
void Range::add (field&& f) 
{
  m_fields.emplace_back (std::move(f));
} 
 
/// Append a subrange 
void Range::add (const Range& subrange) 
{ 
  for (size_t i = 0; i < subrange.fields (); ++i) 
    { 
      const field& f = subrange[i]; 
      m_fields.push_back (f); 
    } 
} 

void Range::add (Range&& subrange) 
{
  if (m_fields.empty())
    m_fields.swap (subrange.m_fields);
  else {
    size_t sz = subrange.m_fields.size();
    m_fields.reserve (m_fields.size() + sz);
    for (size_t i = 0; i < sz; ++i) 
    { 
      m_fields.emplace_back (std::move(subrange.m_fields[i]));
    }
  }
} 




//----------------------------------------------- 
void Range::clear () 
{ 
  m_fields.clear (); 
} 
 
//----------------------------------------------- 
int Range::match (const ExpandedIdentifier& id) const 
{ 
  size_type my_fields = m_fields.size (); 
  const size_type id_fields = id.fields (); 
 
    // Remove trailing wild cards since they are meaningless. 
  while ((my_fields > 1) && 
         (!m_fields[my_fields-1].is_valued ())) 
    { 
      my_fields--; 
    } 
 
    // Ranges with only wild cards always match. 
  if (my_fields == 0) return (1); 
 
    // More fields in the range than in the identifier will never match. 
  //if (my_fields > id_fields) return (0); 

  // Allow match for id shorter than range - assume "wildcards" for
  // missing id fields
  size_type nfields =  (my_fields > id_fields) ? id_fields : my_fields;
 
    // Test fields one by one. 
  //for (size_type field_number = 0; field_number < my_fields; field_number++) 
  for (size_type field_number = 0; field_number < nfields; field_number++) 
    { 
      const field& f = m_fields[field_number]; 
 
      if (!f.match (id[field_number])) return (0); 
    } 
 
    // All conditions match. 
  return (1); 
} 
 
// Accessors 
 
//----------------------------------------------- 
ExpandedIdentifier Range::minimum () const 
{ 
  size_type my_fields = m_fields.size (); 
  ExpandedIdentifier result; 
   
    // Remove trailing wild cards since they are meaningless. 
  while ((my_fields > 1) && 
         (!m_fields[my_fields-1].has_minimum ()))  
    { 
      my_fields--; 
    } 
   
    // Ranges with only wild cards: set first field of min to 0 
  if (my_fields == 0) {
    result << 0;
    return result; // Don't combine these two lines --- it inhibits RVO.
  }
   
    // Copy fields to result - look for wild cards 
  for (size_type field_number = 0; field_number < my_fields; field_number++)  
    { 
      const field& f = m_fields[field_number]; 
       
      if (!f.has_minimum ())  
        { 
            // Wilds card -> set field to 0 
          result << 0; 
        }        
      else  
        { 
            // Valued field 
          result << f.get_minimum (); 
        } 
    } 
   
  return (result); 
} 
 
//----------------------------------------------- 
ExpandedIdentifier Range::maximum () const 
{ 
  size_type my_fields = m_fields.size (); 
  ExpandedIdentifier result; 
   
    // Remove all by the last trailing wild card, extra ones are 
    // meaningless. 
  while ((my_fields > 1) && 
         (!m_fields[my_fields-1].has_maximum ())) 
    { 
      my_fields--; 
    } 
 
    // Ranges with only wild cards: set first field of min to ExpandedIdentifier::max_value 
  if (my_fields == 0) {
    result << ExpandedIdentifier::max_value;
    return result; // Don't combine these two lines --- it inhibits RVO.
  }
 
    // Copy fields to result - look for wild cards 
  for (size_type field_number = 0; field_number < my_fields; field_number++)  
    { 
      const field& f = m_fields[field_number]; 
 
      if (!f.has_maximum ())  
        { 
            // Wilds card  
          if (field_number == 0) 
            { 
                // For 1st field set it to ExpandedIdentifier::max_value 
              result << ExpandedIdentifier::max_value; 
            } 
          else 
            { 
                // 
                // For subsequent fields, set do ++ for field-1 
                // This requires rebuilding the result 
                // 
              ExpandedIdentifier new_result; 
 
              for (size_type new_field_number = 0;  
                   new_field_number < (field_number - 1); 
                   ++new_field_number) 
                { 
                  new_result << result[new_field_number]; 
                } 
 
              element_type last = result[field_number - 1]; 
 
              new_result << ((ExpandedIdentifier::max_value == last) ? last : last + 1); 
              new_result << 0; 
 
              assert ( result.fields () == new_result.fields () ); 
 
              result = new_result; 
            } 
        } 
      else 
        { 
            // Normal field 
          result << f.get_maximum (); 
        } 
    } 
   
    return (result); 
} 
 
Range::size_type Range::cardinality () const 
{ 
  size_type result = 1; 
 
  const Range& me = *this; 
 
  for (size_type i = 0; i < fields (); ++i) 
    { 
      const field& f = me[i]; 
 
      result *= f.get_indices (); 
    } 
 
  return (result); 
} 
 

/**
 *   Get the cardinality from the beginning up to the given ExpandedIdentifier
 */
Range::size_type Range::cardinalityUpTo (const ExpandedIdentifier& id) const 
{ 
  size_type result = 0; 

//    std::cout << " Range::cardinality: id, fields " << (std::string) id 
//  	    << " " << id.fields() << " " << fields() << " "
//  	    << (std::string)(*this)  << std::endl;

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
      if (!(f.get_mode() == Range::field::both_bounded ||
	    f.get_mode() == Range::field::enumerated)) return 0;
      
      //int index = 0;  // Contains number of nodes below match

      if (f.get_mode() == Range::field::enumerated) {
	  // Continue testing for a match
	  size_type max = f.get_values().size() - 1;
	  if (f.get_values()[max] < id[level]) {
	      // above max
	      is_match = false;
	      indices[level] = max + 1;
	  }
	  else {
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
      }
      else {
	  if (f.get_maximum() < id[level]) {
	      // above max
	      is_match = false;
	      indices[level] = f.get_maximum() - f.get_minimum() + 1;
	  }
	  else if (id[level] < f.get_minimum()) {
	      // below min
	      is_match = false;
	      indices[level] = 0;
	  }
	  else {
	      indices[level] = id[level] - f.get_minimum();
	  }
      }

//        std::cout << " level, id, field, indices[[level], is enum, is_match " << level << " " 
//  		<< id[level] << " " << (std::string)f << " " 
//  		<< indices[level] << " " << (f.get_mode() == Range::field::enumerated) << " " 
//  		<< is_match << std::endl;


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

//        std::cout << " j, indices, card " << j << " " 
//  		<< indices[j] << " " << card << std::endl;
  }
  
      
  return result;
  
} 
 
 
/** 
 *   Check overlap between two Ranges : 
 * 
 *   As soon as one pair of corresponding fields do not match, 
 *   the global overlap is empty. 
 */ 
bool Range::overlaps_with (const Range& other) const 
{ 
  const Range& me = *this; 
 
  if ((fields () == 0) || (other.fields () == 0)) return (false); 
 
  for (size_type i = 0; i < std::min (fields (), other.fields ()); ++i) 
    { 
      const field& f1 = me[i]; 
      const field& f2 = other[i]; 
 
      if (!f1.overlaps_with (f2)) return (false); 
    } 
 
  return (true); 
} 
 
//----------------------------------------------- 
void Range::show () const 
{
  show (std::cout);
}

void Range::show (std::ostream& s) const 
{ 
  const Range& me = *this; 
 
  s << (std::string) me << " (";
 
  int allbits = 0;

  for (size_type i = 0; i < fields (); ++i) 
    { 
      const field& f = me[i]; 

      if (i > 0) s << "+";

      size_type indices = f.get_indices ();

      int bits = 1;
      indices--;
      if (indices > 0)
        {
          bits = 0;
          while (indices > 0)
            {
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
Range::operator std::string () const 
{ 
  std::string result; 
 
  size_type my_fields = m_fields.size (); 

    // Remove trailing wild cards since they are meaningless. 
  while ((my_fields > 1) && 
         (!m_fields[my_fields-1].is_valued ())) 
    { 
      my_fields--; 
    } 
 
  if (my_fields == 0) return (result); 
 
    // print fields one by one. 
  for (size_type field_number = 0; field_number < my_fields; field_number++) 
    { 
      const field& f = m_fields[field_number]; 

      if (field_number > 0) result += "/"; 
      result += (std::string) f; 
    } 
 
  return (result); 
} 
 
//----------------------------------------------- 
bool 
Range::operator == (const Range& other) const
{
    if (m_fields.size() != other.m_fields.size()) return false;
    field_vector::const_iterator it1  = m_fields.begin();
    field_vector::const_iterator it2  = other.m_fields.begin();
    field_vector::const_iterator last = m_fields.end();
    for (; it1 != last; ++it1, ++it2) {
	if ((*it1) != (*it2)) return false;
    }
    return (true);
}


//----------------------------------------------- 
bool 
Range::operator != (const Range& other) const
{
    return (!((*this) == other));
}



//----------------------------------------------- 
Range::identifier_factory Range::factory_begin () 
{ 
  const Range& me = *this; 
 
  return (identifier_factory (me)); 
} 
 
//----------------------------------------------- 
Range::const_identifier_factory Range::factory_begin () const 
{ 
  const Range& me = *this; 
 
  return (const_identifier_factory (me)); 
} 
 
//----------------------------------------------- 
Range::identifier_factory Range::factory_end () 
{ 
  static const Range r; 
  static const identifier_factory factory (r); 
 
  return (factory); 
} 
 
//----------------------------------------------- 
Range::const_identifier_factory Range::factory_end () const 
{ 
  static const const_identifier_factory factory; 
 
  return (factory); 
} 
 
//----------------------------------------------- 
Range::identifier_factory::identifier_factory () : m_range (nullptr) 
{ 
} 
 
//----------------------------------------------- 
Range::identifier_factory::identifier_factory (const Range& range) : m_range (&range) 
{ 
  /** 
   *    Fill all running identifiers 
   *    m_id : the current id 
   *    m_min : the set of low bounds 
   *    m_max : the set of high bounds 
   */ 
  for (Range::size_type i = 0; i < range.fields (); ++i) 
    { 
      element_type minimum; 
      element_type maximum; 
 
      m_indices.push_back (0); 
 
      const field& f = range[i]; 
 
      switch (f.get_mode ()) 
        { 
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
         throw std::runtime_error("Mode not recognised in Range::identifier_factory::identifier_factory.");
         break;
        } 
        
    } 
} 
 
//----------------------------------------------- 
Range::identifier_factory& Range::identifier_factory::operator = (const identifier_factory& other) 
{ 
  if (&other != this) {
    m_indices = other.m_indices; 
    m_id      = other.m_id; 
    m_min     = other.m_min; 
    m_max     = other.m_max; 
    m_range   = other.m_range; 
  }
 
  return (*this); 
} 
 
//----------------------------------------------- 
Range::identifier_factory& Range::identifier_factory::operator = (identifier_factory&& other) 
{ 
  if (&other != this) {
    m_indices = std::move(other.m_indices);
    m_id      = std::move(other.m_id);
    m_min     = std::move(other.m_min);
    m_max     = std::move(other.m_max);
    m_range   = other.m_range; 
  }
 
  return (*this); 
} 
 
//----------------------------------------------- 
void Range::identifier_factory::operator ++ () 
{ 
  if (m_id.fields () == 0) return; 
 
  size_type fields = m_id.fields (); 
  size_type i = fields - 1; 
 
    // 
    // Starting from the end, we try to increment the m_id fields 
    // If at a given position it's not possible (max reached) 
    // then we move back one pos and try again. 
    // 
    //  As soon as increment is possible, then the rest of the m_id 
    // is reset to min values. 
    // 
 
  for (;;) 
    { 
      const field& f = (*m_range)[i]; 
      bool done = false; 
      element_type value = 0; 
 
      if (f.get_mode () == Range::field::enumerated) 
        { 
          Range::size_type index = m_indices[i]; 
          index++; 
          if (index < f.get_indices ()) 
            { 
              m_indices[i] = index; 
              value = f.get_value_at (index); 
              done = true; 
            } 
        } 
      else 
        { 
          value = m_id[i]; 
 
          if (value < m_max[i]) 
            { 
              /** 
               *   The local range is not exceeded. 
               *   increase the value then reset the remaining fields. 
               */ 
              ++value; 
              done = true; 
            } 
        } 
 
      if (done) 
        { 
          m_id[i] = value; 
           
          for (++i; i < fields; ++i) 
            { 
              m_indices[i] = 0; 
              m_id[i] = m_min[i]; 
            } 
           
          break; 
        } 
 
      /** 
       *  The current range field was exhausted 
       *  check the previous one. 
       */ 
 
      if (i == 0)  
        { 
          m_id.clear (); 
          break; 
        } 
      
        
          --i; 
        
    } 
} 
 
//----------------------------------------------- 
const ExpandedIdentifier& Range::identifier_factory::operator * () const 
{ 
  return (m_id); 
} 
 
//----------------------------------------------- 
bool Range::identifier_factory::operator == (const identifier_factory& other) const 
{ 
  if (m_id == other.m_id) return (true); 
  return (false); 
} 
 
//----------------------------------------------- 
bool Range::identifier_factory::operator != (const identifier_factory& other) const 
{ 
  if (m_id != other.m_id) return (true); 
  return (false); 
} 
 
//----------------------------------------------- 
Range::const_identifier_factory::const_identifier_factory () : m_range (nullptr) 
{ 
} 
 
//----------------------------------------------- 
Range::const_identifier_factory::const_identifier_factory (const Range& range) :  
  m_range (&range) 
{ 
  /** 
   *    Fill all running identifiers 
   *    m_id : the current id 
   *    m_min : the set of low bounds 
   *    m_max : the set of high bounds 
   */ 
  for (Range::size_type i = 0; i < range.fields (); ++i) 
    { 
      element_type minimum; 
      element_type maximum; 
 
      m_indices.push_back (0); 
 
      const field& f = range[i]; 
 
      switch (f.get_mode ()) 
        { 
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
          throw std::runtime_error("Mode not recognised in Range::const_identifier_factory::const_identifier_factory");
          break;
        } 
    } 
} 
 
//----------------------------------------------- 
Range::const_identifier_factory& Range::const_identifier_factory::operator = (const const_identifier_factory& other) 
{ 
  if (&other != this) {
    m_indices = other.m_indices; 
    m_id      = other.m_id; 
    m_min     = other.m_min; 
    m_max     = other.m_max; 
    m_range   = other.m_range; 
  }
 
  return (*this); 
} 
 
//----------------------------------------------- 
Range::const_identifier_factory& Range::const_identifier_factory::operator = (const_identifier_factory&& other) 
{ 
  if (&other != this) {
    m_indices = std::move(other.m_indices);
    m_id      = std::move(other.m_id);
    m_min     = std::move(other.m_min);
    m_max     = std::move(other.m_max);
    m_range   = other.m_range;
  }
 
  return (*this); 
} 
 
//----------------------------------------------- 
void Range::const_identifier_factory::operator ++ () 
{ 
  if (m_id.fields () == 0) return; 
 
  size_type fields = m_id.fields (); 
  size_type i = fields - 1; 
 
    // 
    // Starting from the end, we try to increment the m_id fields 
    // If at a given position it's not possible (max reached) 
    // then we move back one pos and try again. 
    // 
    //  As soon as increment is possible, then the rest of the m_id 
    // is reset to min values. 
    // 
 
  for (;;) 
    { 
      const field& f = (*m_range)[i]; 
      bool done = false; 
      element_type value = 0; 
 
      if (f.get_mode () == Range::field::enumerated) 
        { 
          Range::size_type index = m_indices[i]; 
          index++; 
          if (index < f.get_indices ()) 
            { 
              m_indices[i] = index; 
              value = f.get_value_at (index); 
              done = true; 
            } 
        } 
      else 
        { 
          value = m_id[i]; 
 
          if (value < m_max[i]) 
            { 
              /** 
               *   The local range is not exceeded. 
               *   increase the value then reset the remaining fields. 
               */ 
              ++value; 
              done = true; 
            } 
        } 
 
      if (done) 
        { 
          m_id[i] = value; 
           
          for (++i; i < fields; ++i) 
            { 
              m_indices[i] = 0; 
              m_id[i] = m_min[i]; 
            } 
           
          break; 
        } 
 
      /** 
       *  The current range field was exhausted 
       *  check the previous one. 
       */ 
 
      if (i == 0)  
        { 
          m_id.clear (); 
          break; 
        } 
      
        
          --i; 
        
    } 
} 
 
//----------------------------------------------- 
const ExpandedIdentifier& Range::const_identifier_factory::operator * () const 
{ 
  return (m_id); 
} 
 
//----------------------------------------------- 
bool Range::const_identifier_factory::operator == (const const_identifier_factory& other) const 
{ 
  if (m_id == other.m_id) return (true); 
  return (false); 
} 
 
//----------------------------------------------- 
bool Range::const_identifier_factory::operator != (const const_identifier_factory& other) const 
{ 
  if (m_id != other.m_id) return (true); 
  return (false); 
} 
 
 
 
class MultiRangeParser 
{ 
public: 
  typedef MultiRange::size_type size_type; 
 
  MultiRangeParser () 
      { 
        m_multirange = nullptr; 
      } 
 
  bool run (const std::string& text, MultiRange& multirange) 
      { 
        m_multirange = &multirange; 
        multirange.clear (); 
        size_type pos = 0; 
        return (parse (text, pos)); 
      } 
 
private: 
 
  bool parse_number (const std::string& text,  
                     size_type& pos,  
                     int& value) 
      { 
        if (pos == std::string::npos) return (false); 
         
        const char& cha = (text.at (pos)); 
        const char* ptr = &cha; 
        int items; 
 
        value = 0; 
         
        items = sscanf (ptr, "%80d", &value); 
        if (items == 0)  
          { 
            return (false); 
          } 
         
        pos = text.find_first_not_of ("0123456789+- \t", pos); 
         
        return (true); 
      } 
 
  bool parse_field (const std::string& text, size_type& pos) 
      { 
        bool result = true; 
        int minimum; 
        int maximum; 
 
        if (!skip_spaces (text, pos)) return (false); 
         
        char c = text[pos]; 
        switch (c) 
          { 
            case '0': 
            case '1': 
            case '2': 
            case '3': 
            case '4': 
            case '5': 
            case '6': 
            case '7': 
            case '8': 
            case '9': 
            case '+': 
            case '-': 
              if (true) 
                { 
                  // 
                  // The current range is considered 
                  // 
                  Range& r = m_multirange->back (); 
                   
                  if (!parse_number (text, pos, minimum))  
                    { 
                      result = false; 
                      break; 
                    } 
 
                  if (test_token (text, pos, ':')) 
                    { 
                      if (!parse_number (text, pos, maximum))  
                        { 
                          r.add_minimum ((MultiRange::element_type) minimum); 
                        } 
                      else 
                        { 
                          r.add ((MultiRange::element_type) minimum,  
                                 (MultiRange::element_type) maximum); 
                        } 
                    } 
                  else 
                    { 
                      r.add ((MultiRange::element_type) minimum); 
                    } 
                } 
 
              break; 
            case ':': 
              pos++; 
              { 

                  Range& r = m_multirange->back (); 

 

                  if (!parse_number (text, pos, maximum))  

                    { 

                      result = false; 

                    } 

                  else 

                    { 

                      r.add_maximum ((MultiRange::element_type) maximum); 

                    } 

                } 
 
              break; 
            case '*': 
              pos++; 
              { 

                  Range& r = m_multirange->back (); 

                  r.add (); 

                } 
 
              break; 
            default: 
              result = false; 
          } 
         
        return (result); 
      } 
   
  bool parse_fields (const std::string& text, size_type& pos) 
      { 
        bool finished = false; 
        bool result = true; 
         
        while (!finished) 
          { 
            if (!skip_spaces (text, pos)) 
              { 
                result = false; 
                break; 
              } 
             
            char c = text[pos]; 
             
            switch (c) 
              { 
                case '0': 
                case '1': 
                case '2': 
                case '3': 
                case '4': 
                case '5': 
                case '6': 
                case '7': 
                case '8': 
                case '9': 
                case '+': 
                case '-': 
                case ':': 
                case '*': 
                  if (!parse_field (text, pos))  
                    { 
                      result = false; 
                      finished = true; 
                    } 
                  break; 
                case '/': 
                  pos++; 
                  break; 
                default: 
                  finished = true; 
                  break; 
              } 
          } 
         
        return (result); 
      } 
   
  bool parse (const std::string& text, size_type& pos) 
      { 
        bool result = true; 
        bool finished = false; 
         
        if (!skip_spaces (text, pos)) return (true); 
 
        while (!finished) 
          { 
            char c = text[pos]; 
         
            switch (c) 
              { 
                case '0': 
                case '1': 
                case '2': 
                case '3': 
                case '4': 
                case '5': 
                case '6': 
                case '7': 
                case '8': 
                case '9': 
                case '+': 
                case '-': 
                case ':': 
                case '*': 
                  m_multirange->add_range (); 
                  if (!parse_fields (text, pos))  
                    { 
                      result = false; 
                      finished = true; 
                    } 
                  break; 
                case '|': 
                  pos++; 
                  break; 
                default: 
                  finished = true; 
                  break; 
              } 
          } 
         
        return (result); 
      } 
   
  bool skip_spaces (const std::string& text, size_type& pos) 
      { 
        pos = text.find_first_not_of (" \t", pos); 
        if (pos == std::string::npos) return (false); 
        return (true); 
      } 
 
  bool test_token (const std::string& text, size_type& pos, char token) 
      { 
        if (!skip_spaces (text, pos))return (false); 
 
        char c = text[pos]; 
        if (c != token) return (false); 
 
        pos++; 
        return (true); 
      } 
 
  MultiRange* m_multirange; 
}; 
 
 
//----------------------------------------------- 
MultiRange::MultiRange () 
{ 
} 
 
//----------------------------------------------- 
MultiRange::MultiRange (const MultiRange& other) 
    : 
    m_ranges (other.m_ranges)
{ 
} 

//----------------------------------------------- 
MultiRange& MultiRange::operator= (const MultiRange& other) 
{
  if (this != &other) {
    m_ranges = other.m_ranges;
  }
  return *this;
}

/** 
 *    The reduction algorithm is no longer provided, since it does not 
 *   work in the most general case. 
 *    See previous and temptative implementation in the RangeReduction.cxx  
 *   source file. 
 */ 
MultiRange::MultiRange (const Range& r, const Range& s) 
{ 
  m_ranges.push_back (r); 
  m_ranges.push_back (s); 
} 

//----------------------------------------------- 
void MultiRange::clear () 
{ 
  m_ranges.clear (); 
} 
 
//----------------------------------------------- 
void MultiRange::add (const Range& range) 
{ 
    // Add new range ONLY if an equivalent does NOT exist
    for (size_type i = 0; i < m_ranges.size(); ++i) {
	const Range& test_range = m_ranges[i];
	if (test_range == range) return;
    }
    m_ranges.push_back (range); 
} 
 
//----------------------------------------------- 
void MultiRange::add (Range&& range) 
{ 
    // Add new range ONLY if an equivalent does NOT exist
    for (size_type i = 0; i < m_ranges.size(); ++i) {
	const Range& test_range = m_ranges[i];
	if (test_range == range) return;
    }
    m_ranges.emplace_back (std::move(range));
} 
 
//----------------------------------------------- 
void MultiRange::add (const ExpandedIdentifier& id) 
{ 
  m_ranges.push_back (Range (id)); 
} 
 
//----------------------------------------------- 
void MultiRange::remove_range (const ExpandedIdentifier& id) 
{
  // Remove all ranges for which id matches
  range_vector::iterator end =
    std::remove_if (m_ranges.begin(), m_ranges.end(),
                    [&] (const Range& r) { return r.match(id); });
  m_ranges.erase (end, m_ranges.end());
}

 
//----------------------------------------------- 
Range& MultiRange::add_range () 
{ 
  size_type size = m_ranges.size (); 
  m_ranges.resize (size + 1); 
  return (m_ranges.back ()); 
} 
 
//----------------------------------------------- 
Range& MultiRange::back () 
{ 
  return (m_ranges.back ()); 
} 
 
//----------------------------------------------- 
int MultiRange::match (const ExpandedIdentifier& id) const 
{ 
  range_vector::size_type i; 
 
  for (i = 0; i < m_ranges.size (); ++i) 
    { 
      const Range& r = m_ranges[i]; 
 
      if (r.match (id)) return (1); 
    } 
 
  return (0); 
} 
 
//----------------------------------------------- 
const Range& MultiRange::operator [] (MultiRange::size_type index) const 
{ 
  static const Range null_range; 
 
  if (index >= m_ranges.size ()) return (null_range); 
 
  return (m_ranges[index]); 
} 
 
//----------------------------------------------- 
MultiRange::size_type MultiRange::size () const 
{ 
  return (m_ranges.size ()); 
} 
 
MultiRange::const_iterator MultiRange::begin () const 
{ 
  return (m_ranges.begin ()); 
} 
 
MultiRange::const_iterator MultiRange::end () const 
{ 
  return (m_ranges.end ()); 
} 
 
MultiRange::size_type MultiRange::cardinality () const 
{ 
  size_type result = 0; 
 
  for (size_type i = 0; i < m_ranges.size (); ++i) 
    { 
      const Range& r = m_ranges[i]; 
 
      result += r.cardinality (); 
    } 
 
  return (result); 
} 
 
MultiRange::size_type MultiRange::cardinalityUpTo (const ExpandedIdentifier& id) const
{
    // Loop over ranges in MultiRange and calculate hash for each
    // range

    size_type result = 0;
    for (unsigned int i = 0; i < m_ranges.size(); ++i) {
	const Range& range = m_ranges[i];
	result += range.cardinalityUpTo(id);
    }
    return (result);
}


//----------------------------------------------- 
bool MultiRange::has_overlap () const 
{ 
  range_vector::size_type i; 
  range_vector::size_type j; 
 
  for (i = 0; i < m_ranges.size (); ++i) 
    { 
      const Range& r = m_ranges[i]; 
      for (j = i + 1; j < m_ranges.size (); ++j) 
        { 
          const Range& s = m_ranges[j]; 
          if (r.overlaps_with (s)) return (true); 
        } 
    } 
 
  return (false); 
} 
 
//----------------------------------------------- 
MultiRange::identifier_factory MultiRange::factory_begin () 
{ 
  const MultiRange& me = *this; 
  return (identifier_factory (me)); 
} 
 
//----------------------------------------------- 
MultiRange::const_identifier_factory MultiRange::factory_begin () const 
{ 
  const MultiRange& me = *this; 
  return (const_identifier_factory (me)); 
} 
 
//----------------------------------------------- 
MultiRange::identifier_factory MultiRange::factory_end () 
{ 
  static const identifier_factory factory;
 
  return (factory); 
} 
 
//----------------------------------------------- 
MultiRange::const_identifier_factory MultiRange::factory_end () const 
{ 
  static const const_identifier_factory factory; 
 
  return (factory); 
} 
 
//----------------------------------------------- 
MultiRange::identifier_factory::identifier_factory () 
    : 
    m_multirange(nullptr)
{ 
}

//----------------------------------------------- 
MultiRange::identifier_factory::identifier_factory (const identifier_factory& other) 
    :
    m_id 		(other.m_id),
    m_id_fac_it  	(other.m_id_fac_it),
    m_id_fac_end 	(other.m_id_fac_end),
    m_range_it 		(other.m_range_it),
    m_range_end		(other.m_range_end),
    m_id_vec_it 	(other.m_id_vec_it),
    m_id_vec_end 	(other.m_id_vec_end),
    m_multirange	(other.m_multirange)
{
} 
 
//----------------------------------------------- 
MultiRange::identifier_factory::identifier_factory (const MultiRange& multirange) 
    :
    m_range_it(multirange.m_ranges.begin()),
    m_range_end(multirange.m_ranges.end()),
    m_multirange(&multirange)
{ 
    if (m_range_it == m_range_end)return;  // no ranges
    /** 
     *  Set up iterators over ranges and ids.
     */
    if (m_range_it != m_range_end) {
      m_id_fac_it  = (*m_range_it).factory_begin();
      m_id_fac_end = (*m_range_it).factory_end();
      if(m_id_fac_it != m_id_fac_end) {
        // Set id
        m_id = *m_id_fac_it;
      }
    }
}
 
MultiRange::identifier_factory::~identifier_factory ()
{
}


//----------------------------------------------- 
MultiRange::identifier_factory& MultiRange::identifier_factory::operator = (const identifier_factory& other) 
{ 
  if (this != &other) {
    m_id 		= other.m_id;
    m_id_fac_it  	= other.m_id_fac_it;
    m_id_fac_end 	= other.m_id_fac_end;
    m_range_it 		= other.m_range_it;
    m_range_end		= other.m_range_end;
    m_id_vec_it 	= other.m_id_vec_it;
    m_id_vec_end 	= other.m_id_vec_end;
    m_multirange	= other.m_multirange;
  }
  return (*this); 
} 
 
//----------------------------------------------- 
MultiRange::identifier_factory& MultiRange::identifier_factory::operator = (identifier_factory&& other) 
{ 
  if (this != &other) {
    m_id 		= std::move(other.m_id);
    m_id_fac_it  	= std::move(other.m_id_fac_it);
    m_id_fac_end 	= std::move(other.m_id_fac_end);
    m_range_it 		= other.m_range_it;
    m_range_end		= other.m_range_end;
    m_id_vec_it 	= other.m_id_vec_it;
    m_id_vec_end 	= other.m_id_vec_end;
    m_multirange	= other.m_multirange;
    other.m_multirange = nullptr;
  }
  return (*this); 
} 
 
//----------------------------------------------- 
void MultiRange::identifier_factory::operator ++ () 
{ 
 
    if (m_id.fields () == 0) return; 

    m_id.clear();
    if (m_range_it != m_range_end) {
      if (m_id_fac_it != m_id_fac_end) {
        ++m_id_fac_it;
      }
      if (m_id_fac_it == m_id_fac_end) {
        ++m_range_it;
        if (m_range_it != m_range_end) {
          m_id_fac_it  = (*m_range_it).factory_begin();
          m_id_fac_end = (*m_range_it).factory_end();
        }
      }
      if (m_id_fac_it != m_id_fac_end) {
        m_id = *m_id_fac_it;
      }
    }
} 

 
//----------------------------------------------- 
const ExpandedIdentifier& MultiRange::identifier_factory::operator * () const 
{ 
  return (m_id); 
} 
 
//----------------------------------------------- 
bool MultiRange::identifier_factory::operator == (const identifier_factory& other) const 
{ 
  if (m_id == other.m_id) return (true); 
  return (false); 
} 
 
//----------------------------------------------- 
bool MultiRange::identifier_factory::operator != (const identifier_factory& other) const 
{ 
  if (m_id != other.m_id) return (true); 
  return (false); 
} 

//----------------------------------------------- 
MultiRange::const_identifier_factory::const_identifier_factory () 
    : 
    m_multirange(nullptr)
{ 
} 

//----------------------------------------------- 
MultiRange::const_identifier_factory::const_identifier_factory (const const_identifier_factory& other) 
    :
    m_id 		(other.m_id),
    m_id_fac_it  	(other.m_id_fac_it),
    m_id_fac_end 	(other.m_id_fac_end),
    m_range_it 		(other.m_range_it),
    m_range_end		(other.m_range_end),
    m_id_vec_it 	(other.m_id_vec_it),
    m_id_vec_end 	(other.m_id_vec_end),
    m_multirange	(other.m_multirange)
{
} 
 
 
//----------------------------------------------- 
MultiRange::const_identifier_factory::const_identifier_factory (const MultiRange& multirange) 
    :
    m_range_it(multirange.m_ranges.begin()),
    m_range_end(multirange.m_ranges.end()),
    m_multirange(&multirange)
{ 

    if (m_range_it == m_range_end)return;  // no ranges
    /** 
     *  Set up iterators over ranges and ids.
     */
    if (m_range_it != m_range_end) {
      m_id_fac_it  = (*m_range_it).factory_begin();
      m_id_fac_end = (*m_range_it).factory_end();
      if(m_id_fac_it != m_id_fac_end) {
        // Set id
        m_id = *m_id_fac_it;
      }
    }
} 

 
MultiRange::const_identifier_factory::~const_identifier_factory ()
{
}

 
//----------------------------------------------- 
MultiRange::const_identifier_factory& MultiRange::const_identifier_factory::operator = (const const_identifier_factory& other) 
{ 
  if (this != &other) {
    m_id 		= other.m_id;
    m_id_fac_it  	= other.m_id_fac_it;
    m_id_fac_end 	= other.m_id_fac_end;
    m_range_it 		= other.m_range_it;
    m_range_end		= other.m_range_end;
    m_id_vec_it 	= other.m_id_vec_it;
    m_id_vec_end 	= other.m_id_vec_end;
    m_multirange	= other.m_multirange;
  } 
  return (*this); 
} 
 
//----------------------------------------------- 
MultiRange::const_identifier_factory& MultiRange::const_identifier_factory::operator = (const_identifier_factory&& other) 
{ 
  if (this != &other) {
    m_id 		= std::move(other.m_id);
    m_id_fac_it  	= std::move(other.m_id_fac_it);
    m_id_fac_end 	= std::move(other.m_id_fac_end);
    m_range_it 		= other.m_range_it;
    m_range_end		= other.m_range_end;
    m_id_vec_it 	= other.m_id_vec_it;
    m_id_vec_end 	= other.m_id_vec_end;
    m_multirange	= other.m_multirange;
    other.m_multirange = nullptr;
  } 
  return (*this); 
} 
 
//----------------------------------------------- 
void MultiRange::const_identifier_factory::operator ++ () 
{ 
 
    if (m_id.fields () == 0) return; 

    m_id.clear();
    if (m_range_it != m_range_end) {
      if (m_id_fac_it != m_id_fac_end) {
        ++m_id_fac_it;
      }
      if (m_id_fac_it == m_id_fac_end) {
        ++m_range_it;
        if (m_range_it != m_range_end) {
          m_id_fac_it  = (*m_range_it).factory_begin();
          m_id_fac_end = (*m_range_it).factory_end();
        }
      }
      if (m_id_fac_it != m_id_fac_end) {
        m_id = *m_id_fac_it;
      }
    }
} 
 
//----------------------------------------------- 
const ExpandedIdentifier& MultiRange::const_identifier_factory::operator * () const 
{ 
  return (m_id); 
} 
 
//----------------------------------------------- 
bool MultiRange::const_identifier_factory::operator == (const const_identifier_factory& other) const 
{ 
  if (m_id == other.m_id) return (true); 
  return (false); 
} 
 
//----------------------------------------------- 
bool MultiRange::const_identifier_factory::operator != (const const_identifier_factory& other) const 
{ 
  if (m_id != other.m_id) return (true); 
  return (false); 
} 
 

/** 
 *   This is a debugging facility: 
 *   two vectors of ExpandedIdentifiers are filled in : 
 *     one with sorted and cleaned from duplicates 
 *     the other contains only duplicates (empty when there is no overlap) 
 * 
 *   This function may be used for validity checks 
 */ 
void MultiRange::show_all_ids (std::vector <ExpandedIdentifier>& unique_ids, 
                               std::vector <ExpandedIdentifier>& duplicate_ids) const 
{ 
  range_vector::const_iterator i; 
 
  std::vector <ExpandedIdentifier> ids; 
 
  for (i = m_ranges.begin (); i != m_ranges.end (); ++i) 
    { 
      const Range& r = *i; 
      Range::const_identifier_factory f = r.factory_begin (); 
      while (f != r.factory_end ()) 
        { 
          const ExpandedIdentifier id = *f; 
 
          ids.push_back (id); 
 
          ++f; 
        } 
    } 

  std::sort (ids.begin (), ids.end ()); 
 
  std::vector<ExpandedIdentifier>::const_iterator it; 
 
  bool first = true; 
  ExpandedIdentifier previous; 
 
  for (it = ids.begin (); it < ids.end (); ++it) 
    { 
      const ExpandedIdentifier id = *it; 
 
      if (first) 
        { 
          first = false; 
          unique_ids.push_back (id); 
          previous = id; 
        } 
      else 
        { 
          if (id.match (previous)) 
            { 
              duplicate_ids.push_back (id); 
            } 
          else 
            { 
              unique_ids.push_back (id); 
              previous = id; 
            } 
        } 
    } 
 
  std::cout << unique_ids.size () << " unique ids " << 
    duplicate_ids.size () << " duplicate ids" << std::endl; 
} 
 
/** 
 *    The reduction algorithm is no longer provided, since it does not 
 *   work in the most general case. 
 *    See previous and temptative implementation in the RangeReduction.cxx  
 *   source file. 
 */ 
void MultiRange::reduce () 
{ 
} 
 
//----------------------------------------------- 
void MultiRange::show () const 
{
  show (std::cout);
}

void MultiRange::show (std::ostream& s) const 
{ 
  range_vector::size_type i; 
 
  for (i = 0; i < m_ranges.size (); ++i) 
    { 
      if (i > 0) s << std::endl; 
 
      const Range& r = m_ranges[i]; 
      r.show (s); 
    } 
} 
 
//----------------------------------------------- 
MultiRange::operator std::string () const 
{ 
  std::string result; 
 
  range_vector::size_type i; 
 
  for (i = 0; i < m_ranges.size (); ++i) 
    { 
      if (i > 0) result += " | "; 
 
      const Range& r = m_ranges[i]; 
      result += (std::string) r; 
    } 
 
  return (result); 
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
