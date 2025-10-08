/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CONSTRAINTS_KEYCONCEPT_H
#define CONSTRAINTS_KEYCONCEPT_H
#include <string>
#include <concepts>
#include <type_traits>

template <typename T> struct is_char_array : std::false_type {};

template <std::size_t N> struct is_char_array<char[N]> : std::true_type {};

//CONCEPT
template <class T, class ID=std::string > 
concept KeyConcept =
   std::is_arithmetic_v<T> || is_char_array<T>::value || std::convertible_to<T, ID> ;

#endif







