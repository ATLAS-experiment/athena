/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IDENTIFIER_IDENTIFIER_H
#define IDENTIFIER_IDENTIFIER_H

#include "GaudiKernel/MsgStream.h"
#include "Identifier/Identifier32.h"
#include <string>

/*
 *
 *  @brief Identifier is a simple type-safe 64 bit unsigned integer. An
 *  Identifier relies on other classes - IdHelpers - to encode and
 *  decode its information.
 *  
 *  The default constructor created an Identifier an invalid state
 *  which can be check with the "is_valid" method to allow some error
 *  checking.
 *  
 */
class Identifier{
public:

    using id_type = Identifier;
    using value_type = unsigned long long;
    using diff_type = long long;
    using size_type = unsigned long long ;

    static constexpr unsigned int NBITS = sizeof(value_type) * 8; // bits per byte
    static constexpr value_type MAX_BIT = (static_cast<value_type>(1) << (NBITS - 1));
    static constexpr value_type ALL_BITS = ~(static_cast<value_type>(0));

    /// Default constructor
    Identifier() = default;

    /// @{
    /// Additional ctors
    /// Constructor from value_type
    explicit Identifier (value_type value);

    /// Constructor from Identifier32
    Identifier (const Identifier32& other);

    /// Constructor from 32-bit value_type and int
    /// (to avoid common implicit conversions)
    explicit Identifier (Identifier32::value_type value);
    explicit Identifier (int value);
    ///@}
   
    /// Assignment operators overloads
    Identifier& operator = (const Identifier32& old);
    Identifier& operator = (value_type value);
    /// Assignment to avoid common implicit conversions and shift properly
    Identifier& operator = (Identifier32::value_type value);
    Identifier& operator = (int value);

private:
    /// Bitwise operations 
    Identifier& operator |= (value_type value);
    Identifier& operator &= (value_type value);

public:

    /// build from a string form - hexadecimal
    void set (const std::string& id);

    /// Reset to invalid state
    void clear ();

    /// Set literal value
    Identifier& set_literal(value_type value);

    /// Get the 32-bit version Identifier, will be invalid if >32 bits
    /// needed
    Identifier32 get_identifier32() const;

    /// Get the compact id
    value_type   get_compact() const;

    /// A get_compact functional for use in STL algorithms
    struct get_compact_func{
      value_type operator() (const Identifier& id){
        return id.get_compact();
      }
    };
   
    bool operator == (const Identifier& other) const = default;
    inline auto operator <=> (const Identifier & other) const {return m_id <=> other.m_id;}
    /// Comparison operators with value_type.
    /// This is a hack, only because GeoAdaptors/GeoMuonHits wants to
    /// to compare explicitly with 0 as a test of whether the identifier
    /// has been constructed properly.  But is_valid() here compares
    /// with max_value, not 0, since presumably it is possible to have
    /// a zero value - just not in muons.
    bool operator ==  (value_type other) const;
    bool operator ==  (Identifier32::value_type other) const;
    bool operator ==  (int other) const;
  
    /// Check if id is in a valid state
    bool is_valid () const;

    /// Provide a string form of the identifier - hexadecimal
    std::string  getString() const;

    /// Print out in hex form
    void show () const;

private:
    /// extract field from identifier (shift first, then mask)
    value_type extract(size_type shift, size_type mask) const;

    /// extract field(s) by masking first, then shifting
    value_type mask_shift(value_type mask, size_type shift) const;

    /// extract field, no mask
    value_type extract(size_type shift) const;

    // allow IdDict access to the following private methods
    friend class IdDictDictionary;
    friend class IdDictFieldImplementation;
    friend class AtlasDetectorID;
    friend class PixelID;
    
    enum max_value_type{
      max_value = ~(static_cast<value_type>(0))
    };
    ///The only data member
    value_type m_id = max_value;
};


/// Define a hash functional
namespace std {
  template<>
  struct hash<Identifier>{
    size_t operator()(const Identifier& id) const{
      return static_cast<size_t>(id.get_compact());
    }
  };
}

#include "Identifier/Identifier.icc"

#endif
