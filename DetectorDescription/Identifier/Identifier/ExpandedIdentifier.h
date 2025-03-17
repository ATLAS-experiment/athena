/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IDENTIFIER_EXPANDEDIDENTIFIER_H
#define IDENTIFIER_EXPANDEDIDENTIFIER_H

#include <string>
#include <algorithm>//for lexicographical_compare in the .icc file
#include <boost/container/small_vector.hpp>
#include <iosfwd>

//-----------------------------------------------
//
//
//  
//  ExpandedIdentifier :
//  
//  Stores a set of numbers.
//  
//  ---------------------------------------------------
//  
//  ------------------------
//  Possible operations :
//  ------------------------
//
//  ------------------------
//  Constructors:
//  ------------------------
//
//  ExpandedIdentifier ()                         : default constructor
//
//  ExpandedIdentifier (const std::string& text)  : build an identifier from a textual
//                                                  description following the syntax 
//        <value> [ "/" <value> ... ]
//
//  ExpandedIdentifier (const ExpandedIdentifier& other)  : copy constructor
//
//  ExpandedIdentifier (const ExpandedIdentifier& other, 
//                      size_type start)          : subset constructor
//
//  ------------------------
//  Initialisations:
//  ------------------------
//
//  void clear ()                         : clears up the identifier
//
//  void set (const std::string& text)    : set from a textual description
//
//  ------------------------
//  Modifications:
//  ------------------------
//
//  void add (element_type value)         : appends a numeric value to
//                                          an identifier (adds a field).
//
//  ExpandedIdentifier& operator << (element_type value)   : appends a numeric value to
//                                          an identifier (adds a field).
//
//  ------------------------
//  Accessors:
//  ------------------------
//
//  size_type fields ()                   : returns the number of fields 
//                                          currently stored into the 
//                                          identifier.
//
//  element_type operator [] (size_type field) : gets the value stored at the
//                                          specified field number.
//
//  ------------------------
//  Comparison operators:
//  ------------------------
//
//  operator < (id_type& other)          : absolute comparison, 
//
//                                         e.g. :
//
//                                       /1/2 < /1/3
//                                       /1   < /1/3
//                                       /1 is implicitly /1/0...
//  

//
//
//  ----------------------------------------------------
//
//  Example of how to use an identifier :
//
//  #include <ExpandedIdentifier.h>
//
//  ExpandedIdentifier id;
//
//  id << 125 << 236 << 306 << 2222;
//
//  for (size_type i = 0; i < id.fields (); ++i)
//    {
//      cout << "id[" << i << "] = " << id[i] << endl;
//    }
//
//-----------------------------------------------
class ExpandedIdentifier{
public:

  using id_type = ExpandedIdentifier;
  using element_type = int;
  using element_vector = boost::container::small_vector<element_type,12>;
#ifdef __CPPCHECK__
  // Otherwise cppcheck warns about passing this type by value.
  using size_type = size_t ;
#else
  using size_type = boost::container::small_vector<element_type,12>::size_type ;
#endif

  static constexpr element_type max_value = 0x3FFFFFFF;

  ExpandedIdentifier() = default;

  /// Constructor from a subset of another ExpandedIdentifier
  ExpandedIdentifier (const ExpandedIdentifier& other, size_type start);

  /// Constructor from a textual description
  ExpandedIdentifier (const std::string& text);

  /// Append a value into a new field.
  void add (element_type value);
  ExpandedIdentifier& operator << (element_type value);
  element_type& operator [] (size_type index);

  /// build from a textual description
  void set (const std::string& text);

  /// Erase all fields.
  void clear ();

  /// Get the value stored into the specified field.
  element_type operator [] (size_type index) const;

  // Size of the fields vector.
  size_type fields () const;

  /// Comparison operators
  auto operator <=>( const ExpandedIdentifier& other) const;

  bool operator == (const ExpandedIdentifier& other) const;
  
  
  /**
   *    Test if the shorter of two ids is identical
   *    to the equivalent sub-id extracted from the longer
   */
  bool match (const ExpandedIdentifier& other) const;

  /// String representation of the identifier using the input format
  operator std::string () const;
  /// Returns whether the expanded Identifier contains any information
  bool isValid() const;
  /// Send to std::cout
  void show () const;

private:

  element_vector m_fields;
};

std::ostream & operator << (std::ostream &out, const ExpandedIdentifier & x);



#include "Identifier/ExpandedIdentifier.icc"

#endif
