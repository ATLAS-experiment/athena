/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// Test of EDM type list

#include "TrigStorageDefinitions/TypeInformation.h"

struct testX{};
struct testY{};


// Just a "compilation test"
int main(){
  using namespace HLT::TypeInformation;

  // with newlist
  using mylist = newlist::
    add<testX>::go::
    add<testY>::go::
    done;

  static_assert( std::is_same_v< at<mylist,0>::type, testX > );
  static_assert( std::is_same_v< at<mylist,1>::type, testY > );

  static_assert( std::is_same_v< mylist::get<0>::type, testX > );
  static_assert( std::is_same_v< mylist::get<1>::type, testY > );

  // manual list
  using manual_list = list<testY,
                           list<testX,nil>>;

  static_assert( std::is_same_v< at<manual_list,0>::type, testX > );
  static_assert( std::is_same_v< at<manual_list,1>::type, testY > );

  static_assert( std::is_same_v< manual_list::get<0>::type, testX > );
  static_assert( std::is_same_v< manual_list::get<1>::type, testY > );

  // single entry
  using single_entry_list = list<testY,nil>;

  static_assert( std::is_same_v< single_entry_list::get<0>::type, testY > );

  return 0;
}
