/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IDENTIFIER_RANGE_H
#define IDENTIFIER_RANGE_H
 
#include <Identifier/ExpandedIdentifier.h>
#include "Identifier/IdentifierField.h"
#include <vector>
#include <cassert>
#include <stdexcept>
#include <bit>
#include <iosfwd>
 
/** 
 *    A Range describes the possible ranges for the field values of an ExpandedIdentifier 
 * 
 *    Specifications can be : 
 *       No bound      * 
 *       Low bound     n: 
 *       High bound    :m 
 *       Both bounds   n:m 
 *       Enumeration   v1, v2, v3, ... , vn 
 * 
 *    Trailing * are implicit for all trailing fields 
 * 
 */ 
class Range { 
public: 
 
  using element_type = ExpandedIdentifier::element_type ; 
  using size_type = ExpandedIdentifier::size_type ; 
  using field = IdentifierField ;
  using field_vector = std::vector<field> ; 
 
  /** 
   *    This factory is able to generate all possible identifiers, from a  
   *  fully bounded Range. 
   *    The precondition is that the Range used to parameterize the factory  
   *  must have all its fields completely bounded. 
   */ 
  class identifier_factory 
  { 
  public: 
    identifier_factory () = default; 
    identifier_factory (const Range& range); 
   
    void operator ++ (); 
 
    const ExpandedIdentifier& operator * () const; 
    bool operator == (const identifier_factory& other) const; 
 
  private: 
    std::vector<size_type> m_indices; 
    ExpandedIdentifier m_id; 
    ExpandedIdentifier m_min; 
    ExpandedIdentifier m_max; 
    const Range* m_range{}; 
  }; 
 
  class const_identifier_factory { 
  public: 
    const_identifier_factory () = default; 
    const_identifier_factory (const Range& range); 
 
    void operator ++ (); 
 
    const ExpandedIdentifier& operator * () const; 
    bool operator == (const const_identifier_factory& other) const; 
 
  private: 
    std::vector<size_type> m_indices; 
    ExpandedIdentifier m_id; 
    ExpandedIdentifier m_min; 
    ExpandedIdentifier m_max; 
    const Range* m_range{}; 
  }; 
 
  Range () = default; 


  /** 
   *   This is a sub-range copy constructor.  
   * It copies the portion of the other Range, starting from the  
   * specified starting index up to its last field. 
   */ 
  Range (const Range& other, size_type start); 
 
  
 
  /** 
   *   Construct from a simple ExpandedIdentifier. This implies that all fields 
   *   will have their min=max=id[i] 
   */ 
  Range (const ExpandedIdentifier& root); 

 /** 
   * Build Range from a textual description. 
   * 
   *  The syntax is : 
   * 
   * range : 
   *      \<value-range\> [ "/" \<value-range\> ... ] 
   * 
   * value-range : 
   *      "*" 
   *    | \<value\> 
   *    | ":" \<max\> 
   *    | \<min\> ":" 
   *    | \<min\> ":" \<max\> 
   *    | \<value\> "," \<value\> "," ... "," \<value\> 
   * 
   */ 
  void build (const std::string& text);
 
  /** 
   *   Build a range from a single ExpandedIdentifier 
   *   (see similar constructor for comment) 
   */ 
  void build (const ExpandedIdentifier& root); 
 
  /// Modifications 
 
  void clear (); 
 
  /// Add a wild card field. 
  void add (); 
 
  /// Add a required value. (ie. low = high = value) 
  void add (element_type value); 
 
  /// Add a bounded value. 
  void add (element_type minimum, element_type maximum); 
 
  /// Add a range bounded by a minimum. 
  void add_minimum (element_type minimum); 
 
  /// Add a range bounded by a maximum. 
  void add_maximum (element_type maximum); 
 
  /// Add a range specified using a field  
  void add (const field& f); 
 
  /// Add a range specified using a field, with move semantics.
  void add (field&& f);
 
  /// Append a subrange 
  void add (const Range& subrange); 

  /// Append a subrange, with move semantics.
  void add (Range&& subrange); 
 
  /// Match an identifier 
  int match (const ExpandedIdentifier& id) const; 
  
  /// Access the field elements 
  const field& operator [] (size_type index) const; 
  size_type fields () const; 
  bool is_empty () const; 
 
  /** 
   *   min and max ExpandedIdentifiers  
   *  (if they exist, ie. for fully bounded Ranges) 
   *  Question : what if the Range has wild cards ?? 
   */ 
  ExpandedIdentifier minimum () const; 
  ExpandedIdentifier maximum () const; 
 
  /** 
   *  Computes a possible cardinality : 
   *   - all bounded fields are counted as they are 
   *   - unbounded fields are counted for one value. 
   */ 
  size_type cardinality () const;
  //  Up to a given id
  size_type cardinalityUpTo (const ExpandedIdentifier& id) const;
 
  /// Identifier_factory management 
  identifier_factory factory_begin (); 
  const_identifier_factory factory_begin () const; 
  identifier_factory factory_end (); 
  const_identifier_factory factory_end () const; 
 
  /// Check if two Ranges overlap. 
  bool overlaps_with (const Range& other) const; 
 
  void show () const; 
  void show (std::ostream& s) const; 
 
  /// Produce a textual representation of the range using the input format 
  operator std::string () const; 
 
  bool operator == (const Range& other) const; 

private: 
  field_vector m_fields; 
}; 
 

 


//----------------------------------------------- 
inline Range::size_type Range::fields () const { 
  return (m_fields.size ()); 
} 
 

 
//----------------------------------------------- 
inline bool Range::is_empty () const { 
  if (m_fields.size () == 0) return (true); 
  return (false); 
} 


std::ostream & 
operator << (std::ostream &out, const Range &r);

std::istream & 
operator >> (std::istream &in, Range &r);


#endif 
