/* emacs: this is -*- c++ -*- */
/**
 **   @file    ConfiguredValueUtils.h        
 **                   
 **   @author  sutt
 **   @date    Tue  8 Sep 2026 12:34:40 BST
 **
 **   $Id: ConfiguredValueUtils.h, v0.0   Tue  8 Sep 2026 12:34:40 BST sutt $
 **
 **   Copyright (C) 2026 sutt (sutt@cern.ch)    
 **   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **/

#ifndef IDTPM2_CONFIGUREDVALUEUTILS_H
#define IDTPM2_CONFIGUREDVALUEUTILS_H

#include <stdexcept>

#include "Gaudi/Property.h"

#include "ConfiguredValue.h"


#define     GAUDI_OPERATOR_GENERATOR(OP)        template<typename T, typename U> bool operator OP ( const Gaudi::Property<ConfiguredValue<T>>& p, const U& a ) { return p.value() OP a; }
#define REV_GAUDI_OPERATOR_GENERATOR(OP, REVOP) template<typename T, typename U> bool operator OP ( const U& a, const Gaudi::Property<ConfiguredValue<T>>& p ) { return p.value() REVOP a; }


GAUDI_OPERATOR_GENERATOR(==)
GAUDI_OPERATOR_GENERATOR(!=)
GAUDI_OPERATOR_GENERATOR(<=)
GAUDI_OPERATOR_GENERATOR(>=)
GAUDI_OPERATOR_GENERATOR(<)
GAUDI_OPERATOR_GENERATOR(>)


REV_GAUDI_OPERATOR_GENERATOR(==,==)
REV_GAUDI_OPERATOR_GENERATOR(!=,!=)
REV_GAUDI_OPERATOR_GENERATOR(<=,>=)
REV_GAUDI_OPERATOR_GENERATOR(>=,<=)
REV_GAUDI_OPERATOR_GENERATOR(<,>)
REV_GAUDI_OPERATOR_GENERATOR(>,<)


template<typename T, bool UnsetResult=false>
bool isset( const Gaudi::Property<ConfiguredValue<T,UnsetResult>>& p ) { return p.value().isset(); }


template<typename T, bool UnsetResult=false>
StatusCode parse(ConfiguredValue<T>& p, const Gaudi::Parsers::InputData& input) {
    T value;
    StatusCode sc = parse(value, input);
    if (sc.isSuccess()) p = value;
    return sc;
}

template<typename T, bool UnsetResult = false>
class ConfiguredValueProperty : public Gaudi::Property<ConfiguredValue<T, UnsetResult>> {

  using Base = Gaudi::Property<ConfiguredValue<T, UnsetResult>>;
  
public:
  
  using Base::Base;

  operator const T&() const { return this->value(); }

  bool isset() const { return this->value().isset(); }
  
};

template<typename Exception=std::runtime_error, typename T>
const T& require_set( const T& t ) {
  if ( !t.isset() ) throw Exception( "no matching parameter " + t.name() );
  return t;
}


template <typename... T>
void require_set(const T&... args) {  (require_set<std::runtime_error>(args), ...); }

#endif  // IDTPM2_CONFIGUREDVALUEUTILS_H


