/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGSTORAGEDEFINITIONS_EDM_MASTERSEARCH
#define TRIGSTORAGEDEFINITIONS_EDM_MASTERSEARCH

#include "TrigStorageDefinitions/EDM_TypeInformation.h"

// Since due to the functional nature of template programming we need to pass all results/arguments
// "by reference" (what in normal C++ would be void func(arg1& a1, arg2* a2) we are creating
// overall holder structes that hold all arguments and all results that we want from the loop
// in a sense this is similar to how main() takes an argv parameter.

// Our result structure holds a list of bools (implemented as value type) that indicate in which
// package list the queried type was found and a search_result that gives back the type_info
// if the element was found in any of the packages or a special "NOT_KNOWN_TO_THE_EDM" error type.
template<typename NewList = HLT::TypeInformation::newlist,
         typename SearchResult = HLT::TypeInformation::nil>
struct my_result{
  using list = NewList;
  using search_result = SearchResult;
};


// Argument structure that hold two arguments:
// 1) a template meta-function (functor) that hold the search method with which we query a typemap
// 2) a type for which we want to seach for
template<template<class E, class L, int I> class Functor, typename T>
struct my_arg {
  template<class E, class L, int I> struct functor {
    static constexpr bool result = Functor<E,L,I>::result;
  };

  using type = T;
};

// This is the meta-function that we want to execute on each package typemap
template<typename element, typename last_result, typename arg, bool isLast>
struct my_functor{

  // Perform the search
  using result_type = HLT::TypeInformation::map_search<
    typename arg::type,
    typename element::map,
    arg::template functor >::type;

  // Did we find the type ?
  using ErrorType = HLT::TypeInformation::ERROR_THE_FOLLOWING_TYPE_IS_NOT_KNOWN_TO_THE_EDM<typename arg::type>;
  static constexpr bool is_found = !std::is_same_v<ErrorType, result_type>;

  // Add it to the list
  using added = last_result::list::template add<std::bool_constant<is_found>>::go::done;

  // If we did not find the result we just keep our current result and do not replace it
  using result = std::conditional_t<is_found, result_type, typename last_result::search_result>;

  // Return the error type if still nil otherwise result
  using type = std::conditional_t<
    std::is_same_v<result, HLT::TypeInformation::nil>,
    my_result<added, ErrorType>,
    my_result<added, result> >;
};

// Structure that actually implements the loop over all package EDMs
template<typename MASTER_EDM, template <class,class,int> class functor, typename which> struct master_search{
  using result = HLT::TypeInformation::for_each_type_c<
    MASTER_EDM,
    my_functor,
    my_result<>,
    my_arg<functor,which> >::type;
};


#endif
