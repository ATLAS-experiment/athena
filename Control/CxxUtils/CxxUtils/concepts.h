// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/concepts.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Apr, 2020
 * @brief A couple standard-library related concepts.
 */


#ifndef CXXUTILS_CONCEPTS_H
#define CXXUTILS_CONCEPTS_H


#include <type_traits>
#include <iterator>
#include <concepts>


// Some library concepts.

namespace CxxUtils {
namespace detail {


// Standard library Hash requirement.
template <class HASHER, class T>
concept IsHash =
  std::destructible<HASHER> &&
  std::copy_constructible<HASHER> &&
  requires (const HASHER& h, T x)
{
  { h(x) } -> std::same_as<std::size_t>;
};


// Standard library BinaryPredicate requirement.
template <class PRED, class ARG1, class ARG2=ARG1>
concept IsBinaryPredicate =
  std::copy_constructible<PRED> &&
  std::predicate<PRED, ARG1, ARG2>;


template <class CONTAINER>
concept IsContiguousContainer =
  requires (CONTAINER& c)
  {
    requires std::contiguous_iterator<decltype(c.begin())>;
  };


template <class ITERATOR, class VAL>
concept InputValIterator =
  std::input_iterator<ITERATOR> &&
  std::convertible_to<std::iter_value_t<ITERATOR>, VAL>;


} // namespace detail
} // namespace CxxUtils



#endif // not CXXUTILS_CONCEPTS_H
