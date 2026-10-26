/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TESTCLASSSTLCONTAINERSEXT_H
#define TESTCLASSSTLCONTAINERSEXT_H

#include <string>
#include <vector>
#include <list>
#include <deque>
#include <ostream>

class TestClassSTLContainersExt {

public:
  TestClassSTLContainersExt() = default;
    
  void setNonZero();
  bool operator==( const TestClassSTLContainersExt& rhs ) const;
  bool operator!=( const TestClassSTLContainersExt& rhs ) const;
  std::ostream& streamOut( std::ostream& os ) const;
  
  std::list< double > m_list_of_double;
  std::deque< float > m_deque_of_float;
};


inline void
TestClassSTLContainersExt::setNonZero()
{
  m_list_of_double.clear();
  m_list_of_double.push_back( 2 );
  m_list_of_double.push_back( 3 );
  m_deque_of_float.clear();
  m_deque_of_float.push_back( 9 );
  m_deque_of_float.push_front( 10 );
}

inline std::ostream&
TestClassSTLContainersExt::streamOut( std::ostream& os ) const
{
  os << "m_list_of_double( " << m_list_of_double.size() << " ) :";
  for ( std::list< double >::const_iterator i = m_list_of_double.begin();
        i != m_list_of_double.end(); ++i ) {
    os << " " << *i;
  }
  os << std::endl;
  os << "m_deque_of_float( " << m_deque_of_float.size() << " ) :";
  for ( std::deque< float >::const_iterator i = m_deque_of_float.begin();
        i != m_deque_of_float.end(); ++i ) {
    os << " " << *i;
  }
  return os;
}

inline bool
TestClassSTLContainersExt::operator==( const TestClassSTLContainersExt& rhs ) const
{
  // Check first the sizes of the containers
  if ( m_list_of_double.size() != rhs.m_list_of_double.size() ) return false;
  if ( m_deque_of_float.size() != rhs.m_deque_of_float.size() ) return false;

  // Check element by element.
  std::list< double >::const_iterator this_i_list_of_double = m_list_of_double.begin();
  std::list< double >::const_iterator rhs_i_list_of_double = rhs.m_list_of_double.begin();
  while ( this_i_list_of_double != m_list_of_double.end() ) {
    if ( *this_i_list_of_double != *rhs_i_list_of_double ) return false;
    ++this_i_list_of_double;
    ++rhs_i_list_of_double;
  }

  std::deque< float >::const_iterator this_i_deque_of_float = m_deque_of_float.begin();
  std::deque< float >::const_iterator rhs_i_deque_of_float = rhs.m_deque_of_float.begin();
  while ( this_i_deque_of_float != m_deque_of_float.end() ) {
    if ( *this_i_deque_of_float != *rhs_i_deque_of_float ) return false;
    ++this_i_deque_of_float;
    ++rhs_i_deque_of_float;
  }

  return true;
}

inline bool
TestClassSTLContainersExt::operator!=( const TestClassSTLContainersExt& rhs ) const
{
  return ( ( (*this) == rhs ) ? false : true );
}

#endif
