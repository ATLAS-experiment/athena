/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IDENTIFIER_MULTIRANGE_H
#define IDENTIFIER_MULTIRANGE_H


#include "Identifier/ExpandedIdentifier.h" //for typedefs
#include "Identifier/Range.h"
#include <vector>
#include <iosfwd>
#include <string>



/// @brief A MultiRange combines several Ranges 
class MultiRange { 
public: 
  typedef std::vector<Range> range_vector; 
  typedef ExpandedIdentifier::element_type element_type; 
  typedef ExpandedIdentifier::size_type size_type;
  typedef range_vector::const_iterator const_iterator;
 
  /** 
   *    This factory is able to generate all possible identifiers, from a  
   *  fully bounded Range. 
   *    The precondition is that the Range used to parameterize the factory  
   *  must have all its fields completely bounded. 
   */ 
  class identifier_factory  { 
  public: 
    identifier_factory () = default; 
    identifier_factory (const MultiRange& multirange); 
    void operator ++ (); 
    const ExpandedIdentifier& operator * () const; 
    bool operator == (const identifier_factory& other) const; 
 
  private: 
    using id_vec = std::vector<ExpandedIdentifier>;
    using id_iterator = id_vec::iterator;
    using id_const_iterator = id_vec::const_iterator;

    ExpandedIdentifier		m_id;
    Range::const_identifier_factory	m_id_fac_it; 
    Range::const_identifier_factory	m_id_fac_end; 
    range_vector::const_iterator   	m_range_it; 
    range_vector::const_iterator   	m_range_end; 
    id_iterator			m_id_vec_it;
    id_iterator			m_id_vec_end;
  }; 
 
  class const_identifier_factory {
  public: 
    const_identifier_factory () = default;
    const_identifier_factory (const MultiRange& multirange); 
    void operator ++ (); 
    const ExpandedIdentifier& operator * () const; 
    bool operator == (const const_identifier_factory& other) const; 
 
  private: 
    typedef std::vector<ExpandedIdentifier> id_vec;
    typedef id_vec::iterator 		id_iterator;
    typedef id_vec::const_iterator 		id_const_iterator;

    ExpandedIdentifier		m_id;
    Range::const_identifier_factory	m_id_fac_it; 
    Range::const_identifier_factory	m_id_fac_end; 
    range_vector::const_iterator   	m_range_it; 
    range_vector::const_iterator   	m_range_end; 
    id_iterator			m_id_vec_it;
    id_iterator			m_id_vec_end;
  }; 
 
  /// Constructors 
  MultiRange (); 
  MultiRange (const MultiRange& other); 
  /// Assignment.
  MultiRange& operator= (const MultiRange& other);
  /** 
   *   Construct a non-overlapping MultiRange from 
   *   two overlapping ones 
   */ 
  MultiRange (const Range& r, const Range& s); 
  
  void clear (); 
 
  void add (const Range& range); 

  /// Add with move semantics.
  void add (Range&& range); 

  /// Add a Range made from a single ExpandedIdentifier 
  void add (const ExpandedIdentifier& id); 

  /// Remove a Range made from a single ExpandedIdentifier
  void remove_range (const ExpandedIdentifier& id); 
 
  /// Create a new empty Range that can be adapted afterwards 
  Range& add_range (); 
 
  /// Get the last entered Range 
  Range& back (); 
 
  /// Match an identifier 
  int match (const ExpandedIdentifier& id) const; 
 
  /// Accessors 
  const Range& operator [] (size_type index) const; 
  size_type size () const;
  const_iterator begin () const;
  const_iterator end () const;
 
  /** 
   *  Computes a possible cardinality from all ranges. 
   */ 
  size_type cardinality () const; 
  //  Up to a given id
  size_type cardinalityUpTo (const ExpandedIdentifier& id) const;
 
  /// Check if there are overlaps between any couple of Ranges 
  bool has_overlap () const; 
  void reduce (); 
 
  // identifier_factory management 
  identifier_factory 		factory_begin (); 
  const_identifier_factory 	factory_begin () const; 
  identifier_factory 		factory_end (); 
  const_identifier_factory 	factory_end () const; 
  void show () const; 
  void show (std::ostream& s) const; 
  /// Generate a textual representation of the multirange using the input format 
  operator std::string () const; 
  void show_all_ids (std::vector <ExpandedIdentifier>& unique_ids, 
                     std::vector <ExpandedIdentifier>& duplicate_ids) const; 
 
  private: 
    friend class identifier_factory;
    friend class const_identifier_factory;
    typedef std::vector<ExpandedIdentifier>	id_vec;
    range_vector 		m_ranges; 
}; 

#endif
