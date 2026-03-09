/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef __Identifier32_h__
#define __Identifier32_h__


#include <vector>
#include <string>
#include <iosfwd>

class MsgStream;

/**
 **-----------------------------------------------
 **
 **  Identifier32 is a simple type-safe 32 bit unsigned integer. An
 **  Identifier32 relies on other classes - IdHelpers - to encode and
 **  decode its information.
 **  
 **  The default constructor creates an Identifier32 in an invalid state
 **  which can be used to check with the "is_valid" method to allow some error
 **  checking.
 **  
 **-----------------------------------------------
 */
class Identifier32{
public:
    using id_type = Identifier32;
    using value_type = unsigned int;
    using size_type = unsigned int;

    Identifier32 () = default;

    /// Constructor from value_type (unsigned int)
    inline explicit Identifier32 (value_type value):m_id(value){}

    /// Bitwise operations 
    Identifier32& operator |= (value_type value);
    Identifier32& operator &= (value_type value);

    /// Reset to invalid state
    inline void clear () {m_id = m_maxValue;}

    /// Get the compact id
    inline value_type  get_compact  () const{ return m_id;}
    
    //All comparisons, since C++20
    auto operator<=>(const Identifier32&) const = default;
   
    /// Check if id is in a valid state
    inline bool is_valid () const {return m_id != m_maxValue;}

    /// Provide a string form of the identifier - hexadecimal
    std::string  getString() const;

    /// Print out in hex form
    void show (std::ostream & out) const;
    void show (MsgStream & out) const;
    
    //convert to string representation
    explicit operator std::string() const {return getString();}

private:
    static constexpr value_type m_maxValue{0xFFFFFFFF};
    value_type m_id{m_maxValue};
};


inline Identifier32&                                   
Identifier32::operator |= (unsigned int value){
    m_id |= value;
    return (*this);
}

inline Identifier32& 
Identifier32::operator &= (unsigned int value){
    m_id &= value;
    return (*this);
}

//stream insertion
std::ostream & operator << (std::ostream &out, const Identifier32 &c);



#endif
