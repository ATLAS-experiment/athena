/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#ifndef IDENTIFIER_IDENTIFIERHASH_H
#define IDENTIFIER_IDENTIFIERHASH_H

#include "GaudiKernel/MsgStream.h"
/**
 *  @class IdentifierHash 
 *  	
 *  @brief This is a "hash" representation of an Identifier. This encodes a
 *  32 bit index which can be used to look-up "Identifiable"s stored
 *  in a simple vector. It is intended to be a continuous hash,
 *  i.e. it runs from 0 to N-1, where there are N different possible
 *  values for an Identifier(32) within a specific context.
 *  IdentifierHashes are created by default in an invalid state which
 *  can be checked with "is_valid" method. This allows some error
 *  checking.
 *
 *  @author schaffer
 *  @date 2003-11-26
 */  
class IdentifierHash{
public:
    using value_type = unsigned int;
    /// Default methods
    IdentifierHash () = default;
    /// Initialization with value
    IdentifierHash (value_type value);
    ///@{ 
    ///Get the value 
    operator value_type() const;
    value_type value() const;
    ///@}
    /// Check if id is in a valid state
    bool is_valid () const;
    /// Assignment operators
    IdentifierHash& operator = (value_type value);
    IdentifierHash& operator += (value_type value);
    IdentifierHash& operator -= (value_type value);

private:
    ///default value, and indicator of invalid state
    static constexpr value_type m_max_value = 0xFFFFFFFF;
    //
    value_type m_value = m_max_value;
};


// Define a hash functional
namespace std {
  template<>
  struct hash<IdentifierHash>{
    size_t operator()(const IdentifierHash& id) const{
      return static_cast<size_t>(id.value());
    }
  };
}

#include "Identifier/IdentifierHash.icc"
#endif // IDENTIFIER_IDENTIFIERHASH_H
