/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "Identifier/MultiRange.h"
#include <iostream>
#include <algorithm> //remove_if

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
MultiRange::identifier_factory::identifier_factory (const MultiRange& multirange) 
    :
    m_range_it(multirange.m_ranges.begin()),
    m_range_end(multirange.m_ranges.end())
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
MultiRange::const_identifier_factory::const_identifier_factory (const MultiRange& multirange) 
    :
    m_range_it(multirange.m_ranges.begin()),
    m_range_end(multirange.m_ranges.end())
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
