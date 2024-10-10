/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IDENTIFIER_IDENTIFIERFIELD_H
#define IDENTIFIER_IDENTIFIERFIELD_H
#include <Identifier/ExpandedIdentifier.h>
#include <vector>
#include <string>
#include <stdexcept>
#include <iosfwd>
#include <limits>
#include <utility>
#include <variant>


/** 
 *   This is the individual specification for the range of one ExpandedIdentifier IdentifierField.  
 */ 
class IdentifierField 
{ 
  public : 
  using element_type = ExpandedIdentifier::element_type ; 
  using size_type = ExpandedIdentifier::size_type; 
  using element_vector = std::vector <element_type>; 
  using index_vector = std::vector <size_type>;
  using BoundedRange = std::pair<element_type, element_type>;
  static constexpr auto minimum_possible = std::numeric_limits<element_type>::min();
  static constexpr auto maximum_possible = std::numeric_limits<element_type>::max();
  static constexpr auto invalidValues = element_vector{};
 
  enum continuation_mode{ 
    none, 
    has_next, 
    has_previous, 
    has_both,
    has_wrap_around
  } ; 

  /// Create a wild-card value. 
  IdentifierField () = default;

  /// Create a unique value (understood as : low bound = high bound = value) 
  IdentifierField (element_type value); 

  /// Create a full range specification (with explicit min and max) 
  IdentifierField (element_type minimum, element_type maximum); 
  
  /// Create with enumerated values
  IdentifierField (const element_vector &values); 

  //
  inline bool 
  wrap_around() const{ return (has_wrap_around == m_continuation_mode);}  
  /// Query the values 
 
  //
  inline element_type 
  get_minimum() const {return m_minimum;}
  
  //
  inline std::pair<element_type, element_type>
  get_minmax() const {
    return {m_minimum, m_maximum};
  }
  //
  inline element_type 
  get_maximum () const {return m_maximum;} 
  //
  inline const element_vector& 
  get_values() const { 
    if (isBounded()) return invalidValues;
    return std::get<element_vector>(m_data);
  } 
  ///  Returns false if previous/next is at end of range, or not possible
  bool get_previous (element_type current, element_type& previous) const; 
  bool get_next     (element_type current, element_type& next) const; 
  size_type get_indices () const {return m_size;}
  const index_vector& get_indexes () const {return m_indexes;}
  size_type get_bits () const; 
  element_type get_value_at (size_type index) const; 
  size_type get_value_index (element_type value) const; 

  /// The basic match operation 
  bool match (element_type value) const; 

  /// Check whether two IdentifierFields overlap 
  bool overlaps_with (const IdentifierField& other) const; 

  /// Set methods 
  void clear (); 
  void set (element_type minimum, element_type maximum); 
  void add_value (element_type value); 
  void set (const element_vector& values); 
  void set (bool wraparound); 
  void set_next (int next);
  void set_previous (int previous);
  const IdentifierField& operator [] (IdentifierField::size_type index) const;
  void operator |= (const IdentifierField& other); 

  operator std::string () const; 
  bool operator == (const IdentifierField& other) const; 

  void show() const;
  
  /// Check mode - switch from enumerated to both_bounded if possible
  bool check_for_both_bounded();
    
  /// Optimize - try to switch mode to both_bounded, set up lookup
  /// table for finding index from value
  void optimize();
  
  ///
  inline bool empty() const {return m_empty;}  
  ///
  inline bool isBounded() const {return std::holds_alternative<BoundedRange>(m_data);} 
  inline bool isEnumerated() const {return std::holds_alternative<element_vector>(m_data);}
  
private : 
  static constexpr int m_maxNumberOfIndices = 100;
  void set_minimum (element_type value); 
  void set_maximum (element_type value); 

  /// Create index table from value table
  void create_index_table();
  template <class T>
  T * dataPtr(){return std::get_if<T>(&m_data);}
  //
  element_type m_minimum{}; 
  element_type m_maximum{};
  std::variant<element_vector, BoundedRange> m_data{};
  index_vector m_indexes{}; 
  size_type    m_size{};
  element_type m_previous{}; 
  element_type m_next{}; 
  bool m_empty{true};
  continuation_mode m_continuation_mode{none}; 
}; 

inline IdentifierField::element_type 
IdentifierField::get_value_at(size_type index) const { 
  // Only both_bounded and enumerated are valid to calculate the
  // value.
  // both_bounded is the more frequent case and so comes first.
  if (m_empty) return 0;
  if (const auto * p{std::get_if<BoundedRange>(&m_data)}; p) {
    if (index >= (size_type) (p->second - p->first + 1)) {
      throw std::out_of_range("IdentifierField::get_value_at");
    }
    return (p->first + index); 
  }
  return ((std::get<element_vector>(m_data)).at(index)); 

} 

std::ostream & 
operator << (std::ostream &out, const IdentifierField &c);
std::istream & 
operator >> (std::istream &in, IdentifierField &c);
#endif