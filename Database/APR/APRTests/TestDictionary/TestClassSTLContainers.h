/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TESTCLASSSTLCONTAINERS_H
#define TESTCLASSSTLCONTAINERS_H

#include <string>
#include <vector>
#include <map>
#include <set>
#include <ostream>

class TestClassSTLContainers {

public:
  TestClassSTLContainers() = default;
    
  void setNonZero();
  bool operator==( const TestClassSTLContainers& rhs ) const;
  bool operator!=( const TestClassSTLContainers& rhs ) const;
  std::ostream& streamOut( std::ostream& os ) const;
  
  std::vector< int > m_vector_of_int;
  std::map< std::string, long > m_map_of_string_to_long;
  std::set< unsigned short > m_set_of_ushort;
};


inline void
TestClassSTLContainers::setNonZero()
{
  m_vector_of_int.clear();
  m_vector_of_int.push_back( 1 );
  m_map_of_string_to_long.clear();
  m_map_of_string_to_long.insert( std::make_pair( std::string( "key1" ), 4L ) );
  m_map_of_string_to_long.insert( std::make_pair( std::string( "key2" ), 5L ) );
  m_set_of_ushort.clear();
  m_set_of_ushort.insert( 6 );
  m_set_of_ushort.insert( 7 );
  m_set_of_ushort.insert( 8 );
}

inline std::ostream&
TestClassSTLContainers::streamOut( std::ostream& os ) const
{
  os << "m_vector_of_int( " << m_vector_of_int.size() << " ) :";
  for ( std::vector< int >::const_iterator i = m_vector_of_int.begin();
        i != m_vector_of_int.end(); ++i ) {
    os << " " << *i;
  }
  os << std::endl;
  os << "m_map_of_string_to_long( " << m_map_of_string_to_long.size() << " ) :";
  for ( std::map< std::string, long >::const_iterator i = m_map_of_string_to_long.begin();
        i != m_map_of_string_to_long.end(); ++i ) {
    os << " (" << i->first << "," << i->second << ")";
  }
  os << std::endl;
  os << "m_set_of_ushort( " << m_set_of_ushort.size() << " ) :";
  for ( std::set< unsigned short >::const_iterator i = m_set_of_ushort.begin();
        i != m_set_of_ushort.end(); ++i ) {
    os << " " << *i;
  }
  return os;
}

inline bool
TestClassSTLContainers::operator==( const TestClassSTLContainers& rhs ) const
{
  // Check first the sizes of the containers
  if ( m_vector_of_int.size() != rhs.m_vector_of_int.size() ) return false;
  if ( m_map_of_string_to_long.size() != rhs.m_map_of_string_to_long.size() ) return false;
  if ( m_set_of_ushort.size() != rhs.m_set_of_ushort.size() ) return false;

  // Check element by element.
  std::vector< int >::const_iterator this_i_vector_of_int = m_vector_of_int.begin();
  std::vector< int >::const_iterator rhs_i_vector_of_int = rhs.m_vector_of_int.begin();
  while ( this_i_vector_of_int != m_vector_of_int.end() ) {
    if ( *this_i_vector_of_int != *rhs_i_vector_of_int ) return false;
    ++this_i_vector_of_int;
    ++rhs_i_vector_of_int;
  }

  std::map< std::string, long >::const_iterator this_i_map_of_string_to_long = m_map_of_string_to_long.begin();
  std::map< std::string, long >::const_iterator rhs_i_map_of_string_to_long = rhs.m_map_of_string_to_long.begin();
  while ( this_i_map_of_string_to_long != m_map_of_string_to_long.end() ) {
    if ( this_i_map_of_string_to_long->first != rhs_i_map_of_string_to_long->first ||
         this_i_map_of_string_to_long->second != rhs_i_map_of_string_to_long->second ) return false;
    ++this_i_map_of_string_to_long;
    ++rhs_i_map_of_string_to_long;
  }

  std::set< unsigned short >::const_iterator this_i_set_of_ushort = m_set_of_ushort.begin();
  std::set< unsigned short >::const_iterator rhs_i_set_of_ushort = rhs.m_set_of_ushort.begin();
  while ( this_i_set_of_ushort != m_set_of_ushort.end() ) {
    if ( *this_i_set_of_ushort != *rhs_i_set_of_ushort ) return false;
    ++this_i_set_of_ushort;
    ++rhs_i_set_of_ushort;
  }

  return true;
}

inline bool
TestClassSTLContainers::operator!=( const TestClassSTLContainers& rhs ) const
{
  return ( ( (*this) == rhs ) ? false : true );
}


#endif
