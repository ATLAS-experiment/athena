/* emacs: this is -*- c++ -*- */
/**
 **   @file    ConfiguredValue.h        
 **                   
 **   @author  sutt
 **   @date    Tue  8 Sep 2026 12:34:40 BST
 **
 **   $Id: ConfiguredValue.h, v0.0   Tue  8 Sep 2026 12:34:40 BST sutt $
 **
 **   Copyright (C) 2026 sutt (sutt@cern.ch)    
 **   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/

#ifndef IDTPM2_CONFIGUREDVALUE_H
#define IDTPM2_CONFIGUREDVALUE_H


#include <iostream>

#define              GENERATOR(OP) { if ( !isset() ) return UnsetResult; return m_value OP a; }

#define     OPERATOR_GENERATOR(OP)        template<typename U> bool operator OP ( const U& a ) const GENERATOR(OP)
#define REV_OPERATOR_GENERATOR(OP, REVOP) template<typename T, typename U> bool operator OP ( const U& a, const ConfiguredValue<T>& p ) { return p REVOP a; }

template<typename T, bool UnsetResult=false>
struct ConfiguredValue {

  ConfiguredValue() : m_set(false), m_value(0)  {  } 
  ConfiguredValue( const T& v ) : m_set(true), m_value(v) {  }
  
  ConfiguredValue( const ConfiguredValue& c ) = default; 

  ConfiguredValue& operator=(const ConfiguredValue&) = default;
  
  operator T&()             { return m_value; }
  operator const T&() const { return m_value; }

  /// needs to be explicit in case anyone ever tries to use T=bool
  explicit operator bool() const { return isset(); }
  
  bool    isset() const { return m_set; }

  template<typename U>
  ConfiguredValue<T>& operator=(const U& a ) {
    m_value = a;
    m_set = true;
    return *this;
  }

  OPERATOR_GENERATOR(==)
  OPERATOR_GENERATOR(!=)
  OPERATOR_GENERATOR(<=)
  OPERATOR_GENERATOR(>=)
  OPERATOR_GENERATOR(<)
  OPERATOR_GENERATOR(>)
  
private:
  
  bool m_set; 

  T    m_value;

};


REV_OPERATOR_GENERATOR(==,==)
REV_OPERATOR_GENERATOR(!=,!=)
REV_OPERATOR_GENERATOR(<=,>=)
REV_OPERATOR_GENERATOR(>=,<=)
REV_OPERATOR_GENERATOR(<,>)
REV_OPERATOR_GENERATOR(>,<)


template<typename T, bool UnsetResult=false>
std::ostream& operator<<( std::ostream& s, const ConfiguredValue<T>& p ) {
  if (p.isset()) return s << (T)p;
  return s << "not set";
}

#endif  // IDTPM2_CONFIGUREDVALUE_H


