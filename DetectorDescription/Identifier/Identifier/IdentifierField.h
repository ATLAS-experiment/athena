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
  static constexpr auto minimum_possible = std::numeric_limits<element_type>::min();
  static constexpr auto maximum_possible = std::numeric_limits<element_type>::max();
  /** 
   *   Characterizes the three possible modes of any IdentifierField specification 
   */ 
  enum mode{ 
    unbounded, 
    both_bounded, 
    enumerated,
    nModes
  } ; 

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

  /// Some combined query functions on the specification mode 
  inline bool 
  is_valued () const {return (m_mode != unbounded);} 
  //
  inline bool 
  has_minimum () const { 
    return m_mode != unbounded; 
  }
  //
  inline bool 
  has_maximum () const{ return (
    m_mode != unbounded);  
  }
  //
  inline bool 
  wrap_around () const{ return (has_wrap_around == m_continuation_mode);}  
  /// Query the values 
  inline mode 
  get_mode () const {return m_mode;}
  //
  inline element_type 
  get_minimum () const {return m_minimum;}
  
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
  get_values () const {return m_values;} 
  ///  Returns false if previous/next is at end of range, or not possible
  bool get_previous (element_type current, element_type& previous) const; 
  bool get_next     (element_type current, element_type& next) const; 
  size_type get_indices () const {return m_indices;}
  const index_vector& get_indexes () const {return m_indexes;}
  size_type get_bits () const; 
  element_type get_value_at (size_type index) const; 
  size_type get_value_index (element_type value) const; 

  /// Check if this is a pure wild card IdentifierField 
  bool match_any () const {return (m_mode == unbounded);}

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

private : 
  static constexpr int m_maxNumberOfIndices = 100;
  void set_minimum (element_type value); 
  void set_maximum (element_type value); 

  /// Create index table from value table
  void create_index_table();

  /// Set m_indices
  void set_indices();

  element_type m_minimum{}; 
  element_type m_maximum{}; 
  element_vector m_values{}; 
  index_vector m_indexes{}; 
  size_type    m_indices{};
  element_type m_previous{}; 
  element_type m_next{}; 
  mode m_mode{unbounded}; 
  continuation_mode m_continuation_mode{none}; 
}; 

inline IdentifierField::element_type 
IdentifierField::get_value_at(size_type index) const { 
  // Only both_bounded and enumerated are valid to calculate the
  // value.
  // both_bounded is the more frequent case and so comes first.
  if (both_bounded == m_mode) {
    if (index >= (size_type) (m_maximum - m_minimum + 1)) {
      throw std::out_of_range("IdentifierField::get_value_at");
    }
    return (m_minimum + index); 
  } else if (enumerated == m_mode) {
    return (m_values.at(index)); 
  }
  return (0); 
} 

std::ostream & 
operator << (std::ostream &out, const IdentifierField &c);
std::istream & 
operator >> (std::istream &in, IdentifierField &c);
#endif