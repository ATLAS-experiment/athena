/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// Test of EDM type list

#include "TrigStorageDefinitions/TypeInformation.h"

#include <type_traits>

struct testX{};
struct testY{};
struct testZ{};


// Just a "compilation test"
int main(){
  using namespace HLT::TypeInformation;

  using list1 = List<testX, testY>;

  static_assert( std::is_same_v< list1::at<0>, testX > );
  static_assert( std::is_same_v< list1::at<1>, testY > );

  static_assert( list1::size == 2 );
  static_assert( list1::has<testX> );
  static_assert( list1::indexOf<testY>() == 1 );
  static_assert( list1::indexOf<struct Foo>() == list1::size );

  static_assert( std::is_same_v< list1::find<testX>, testX > );

  using list2 = list1::join<List<testZ>>;
  static_assert( list2::size == 3 );

  return 0;
}
