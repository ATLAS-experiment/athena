/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//====================================================================
//  Implementation file of a Database options
//--------------------------------------------------------------------
//
//  Package    : StorageSvc (The POOL project)
//
//  @author      M.Frank
//====================================================================

// Framework include files
#include "StorageSvc/pool.h"
#include "StorageSvc/DbOption.h"
#include "GaudiKernel/StatusCode.h"

// C++ include files
#include <iostream>

using namespace pool;

namespace {
  template <class T, class Q> struct Marshal {
    static StatusCode get(const void* from, void* to)   {
      *(Q*)to = *(T*)from;  
      return StatusCode::SUCCESS;
    }
  };
  template <>
    inline StatusCode Marshal<double, float>::get(const void* from, void* to)  {
    *(float*)to = float(*(double*)from);  
    return StatusCode::SUCCESS;
  }
}


/// Set the option value
StatusCode DbOption::i_setValue(const std::type_info& typ, const void* value)  {
  if( typ == typeid(bool) ) {
    m_type = DbColumn::BOOL;
    m_value.val_int = *(bool*)value ? 1 : 0;
    return StatusCode::SUCCESS;
  }
  else if( typ == typeid(char) ) {
    m_type = DbColumn::CHAR;
    return Marshal<char,int>::get(value, &m_value.val_int);
  }
  else if( typ == typeid(signed char) ) {
    m_type = DbColumn::CHAR;
    return Marshal<signed char,int>::get(value, &m_value.val_int);
  }
  else if( typ == typeid(unsigned char) ) {
    m_type = DbColumn::UCHAR;
    return Marshal<unsigned char,int>::get(value, &m_value.val_int);
  }
  else if( typ == typeid(short) ) {
    m_type = DbColumn::SHORT;
    return Marshal<short,int>::get(value, &m_value.val_int);
  }
  else if( typ == typeid(unsigned short) ) {
    m_type = DbColumn::USHORT;
    return Marshal<unsigned short,int>::get(value, &m_value.val_int);
  }
  else if( typ == typeid(int) ) {
    m_type = DbColumn::INT;
    return Marshal<int,int>::get(value, &m_value.val_int);
  }
  else if( typ == typeid(unsigned int) ) {
    m_type = DbColumn::UINT;
    return Marshal<unsigned int,int>::get(value, &m_value.val_int);
  }
  else if( typ == typeid(long) ) {
    m_type = DbColumn::LONG;
    return Marshal<long,int>::get(value, &m_value.val_int);
  }
  else if( typ == typeid(unsigned long) ) {
    m_type = DbColumn::ULONG;
    return Marshal<unsigned long,int>::get(value, &m_value.val_int);
  }
  else if( typ == typeid(long long) ) {
    m_type = DbColumn::LONGLONG;
    return Marshal<long long,long long>::get(value, &m_value.val_long);
  }
  else if( typ == typeid(unsigned long long) ) {
    m_type = DbColumn::ULONGLONG;
    return Marshal<unsigned long long,long long>::get(value, &m_value.val_long);
  }
  else if( typ == typeid(double) ) {
    m_type = DbColumn::DOUBLE;
    return Marshal<double,double>::get(value, &m_value.val_double);
  }
  else if( typ == typeid(float) ) {
    m_type = DbColumn::FLOAT;
    return Marshal<float,double>::get(value, &m_value.val_double);
  }
  else if( typ == typeid(void*) or typ == typeid(const void*) ) {
    m_type = DbColumn::ANY;
    m_value.val_pvoid = *(void**)value;
    return StatusCode::SUCCESS;
  }
  else if( typ == typeid(char*) or typ == typeid(const char*) ) {
    m_type = DbColumn::NTCHAR;
    m_value.val_pchar = *(char**)value;
    return StatusCode::SUCCESS;
  }
  else
    std::cout << "DbOption::setValue> unsupported data type: " << typ.name() << std::endl;
  return StatusCode::FAILURE;
}


/// Read the option value
StatusCode DbOption::i_getValue(const std::type_info& typ, void* value) const {
  if( typ == typeid(bool) ) {
    *(bool*)value = (bool)m_value.val_int;
    return StatusCode::SUCCESS;
  }
  else if( typ == typeid(char) ) {
    if( m_type == DbColumn::CHAR )
      return Marshal<int, char>::get(&m_value.val_int, value);
  }
  else if( typ == typeid(signed char) ) {
    if( m_type == DbColumn::CHAR )
      return Marshal<int, signed char>::get(&m_value.val_int, value);
  }
  else if( typ == typeid(unsigned char) ) {
    if( m_type == DbColumn::UCHAR )
      return Marshal<int, unsigned char>::get(&m_value.val_int, value);
  }
  else if( typ == typeid(short) ) {
    if( m_type == DbColumn::SHORT )
      return Marshal<int, short>::get(&m_value.val_int, value);
  }
  else if( typ == typeid(unsigned short) ) {
    if( m_type == DbColumn::USHORT )
      return Marshal<int, unsigned short>::get(&m_value.val_int, value);
  }
  else if( typ == typeid(int) ) {
    if( m_type == DbColumn::INT or m_type == DbColumn::SHORT )
      return Marshal<int, int>::get(&m_value.val_int, value);
  }
  else if( typ == typeid(unsigned int) ) {
    if( m_type == DbColumn::UINT or m_type == DbColumn::USHORT )
      return Marshal<int, unsigned int>::get(&m_value.val_int, value);
  }
  else if( typ == typeid(long) ) {
    if( m_type == DbColumn::LONG or m_type == DbColumn::INT or m_type == DbColumn::SHORT )
      return Marshal<int, long>::get(&m_value.val_int, value);
  }
  else if( typ == typeid(unsigned long) ) {
    if( m_type == DbColumn::ULONG or m_type == DbColumn::UINT or m_type == DbColumn::USHORT )
      return Marshal<int, unsigned long>::get(&m_value.val_int, value);
  }
  else if( typ == typeid(long long int) ) {
    if( m_type == DbColumn::LONGLONG )
      return Marshal<long long, long long>::get(&m_value.val_long, value);
    if( m_type == DbColumn::LONG or m_type == DbColumn::INT or m_type == DbColumn::SHORT )
      return Marshal<int, long long>::get(&m_value.val_int, value);
  }
  else if( typ == typeid(unsigned long long int) ) {
    if( m_type == DbColumn::ULONGLONG )
      return Marshal<long long, unsigned long long>::get(&m_value.val_long, value);
    if( m_type == DbColumn::LONG or m_type == DbColumn::INT or m_type == DbColumn::SHORT )
      return Marshal<int, unsigned long long>::get(&m_value.val_int, value);
  }
  else if( typ == typeid(double) ) {
    if( m_type == DbColumn::DOUBLE or m_type == DbColumn::FLOAT )
      return Marshal<double, double>::get(&m_value.val_double, value);
  }
  else if( typ == typeid(float) ) {
    if( m_type == DbColumn::FLOAT )
      return Marshal<double, float>::get(&m_value.val_double, value);
  }
  else if( typ == typeid(void*) or typ == typeid(const void*) ) {
    if( m_type == DbColumn::ANY ) {
      *(void**)value = m_value.val_pvoid;
      return StatusCode::SUCCESS;
    }
  }
  else if( typ == typeid(char*) or typ == typeid(const char*) ) {
    if( m_type == DbColumn::NTCHAR ) {
      *(void**)value = m_value.val_pchar;
      return StatusCode::SUCCESS;
    }
  }
  else
    std::cout << "DbOption::getValue(): datatype mismatch - option type is: " 
              << DbColumn::typeName(m_type) << ", requested type is: " << typ.name() << std::endl;
  return StatusCode::FAILURE;
}
