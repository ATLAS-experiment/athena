/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
 *
 * A field specifies a constraint on the values it may contain.
 * This may take one of three forms:
 *  - Completely unconstrained.  In this case, empty() will return true,
 *    implying that the field may hold any value representable by element_type.
 *    (Yes, it's a bad name and means the opposite of what you may think...)
 *    An empty field is made by the default constructor, and also
 *    by calling clear().
 *  - A bounded (inclusive) range.  In this case, isBounded() will return
 *    true, get_maximum/get_minimum will return the ends of the range,
 *    and get_indices() will return the number of values in the range
 *    (maximum - minimum + 1).  A bounded field may be made by calling
 *    the constructor with a minimum and maximum, or by calling set()
 *    with the same arguments.  The range is also stored in m_data
 *    as the BoundedRange option.
 *  - An enumeration.  In this case, isEnumerated() will return true,
 *    get_maximum/get_minimum will return the minimum and maximum
 *    enumeration values, and get_indices() will return the number
 *    of enumeration values.  get_values() will return a (sorted)
 *    vector of all the values.  An enumerated field may be made by passing
 *    a vector to the constructor or set(), or by calling add_value()
 *    with individual values.  The values are stored in m_data as the
 *    element_vector option.
 *
 * Each valid value is assigned an index, starting with 0.  For bounded
 * ranges, this just starts from the minimum value and counts up.
 * For enumerations, it is the index of that particular enumeration value.
 * You can convert between values and indices using get_value_at()
 * and get_value_index().  For value->index conversions, a lookup table
 * is used if optimize() has been called and the spread of values is
 * not too large.  get_indices() will return this table.
 *
 * As far as users of this class see, there is not really a distinction
 * between a dense enumeration and a bounded range.  In fact,
 * check_for_both_bounded()/optimize() will convert a dense enumeration
 * to a bounded range.
 *
 * Given a value, one can ask for the next/previous valid value using
 * get_next()/get_previous().  For an enumeration, this will return
 * the next or previous enumeration value.  If you try to call get_next()
 * for the maximum value in the range, or get_previous() from the minimum,
 * then they will by default fail (return false).  This can be changed
 * by calling set_next()/set_previous() to specify explictly values
 * to be returned in these cases, or by calling set(true) to enable
 * wraparound mode; in that case, calling get_next() for the maximum value
 * will return the minimum value, and analogously for get_previous().
 *
 * Other operations include:
 *  - get_bits(): Return the number of bits needed to store a value
 *                of this field.
 * -  match(): Given a value, test whether it satisfies the constraints
 *             for this field.
 *  - overlaps_with(): Test to see if there are any values which satisfy
 *                     the constraints of both fields.
 *  - operator|=(): Find the union of two fields.  Note: does not correctly
 *                  handle the cases of an enumeration and a bounded
 *                  range or two disjoint bounded ranges!
 *                  If either is empty (meaning no constraint), then
 *                  the result will be empty.
 *   - operator==(): Test if two fields have identical constraints.
 *                   However, a dense enumeration will not compare
 *                   identically to an equivalent bounded range.
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
  /// Return the number of bits needed to store a value of this field.
  size_type get_bits () const; 
  element_type get_value_at (size_type index) const; 
  size_type get_value_index (element_type value) const; 

  /// The basic match operation
  /// Given a value, test to see if it satisfies the constraints for this field.
  bool match (element_type value) const; 

  /// Check whether two IdentifierFields overlap
  /// (Are there any values which satisfy the constraints of both fields?)
  bool overlaps_with (const IdentifierField& other) const; 

  /// Set methods 
  void clear (); 
  void set (element_type minimum, element_type maximum); 
  void add_value (element_type value); 
  void set (const element_vector& values); 
  void set (bool wraparound); 
  void set_next (int next);
  void set_previous (int previous);
  /// Find the union of two fields.  Note: does not correctly  handle the cases
  /// of an enumeration and a bounded range or two disjoint bounded ranges!
  /// If either is empty (meaning no constraint), then the result will be empty.
  void operator |= (const IdentifierField& other); 

  operator std::string () const; 
  bool operator == (const IdentifierField& other) const; 

  void show() const;
  
  /// Check mode - switch from enumerated to both_bounded if possible
  bool check_for_both_bounded();
    
  /// Optimize - try to switch mode to both_bounded, set up lookup
  /// table for finding index from value
  void optimize();
  
  /// If true, this field does not have any constraints, and may hold
  /// any value representable by element_type.
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
