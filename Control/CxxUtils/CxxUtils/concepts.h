// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/concepts.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Apr, 2020
 * @brief Compatibility helpers for using some pieces of C++20
 *        concepts with older compilers.
 *
 * Use ATH_REQUIRES for a requires clause:
 *@code
 *  template <class T>
 *    ATH_REQUIRES( std::assignable_from<T, float> )
 *  void foo (T x);
 @endcode
 *
 * The body of the ATH_REQUIRES will be hidden for compilers
 * that don't support concepts.
 *
 * ATH_MEMBER_REQUIRES can be used to selectively enable a member function.
 * Example:
 *@code
 *  template <class T>
 *  class C { ...
 *
 *    ATH_MEMBER_REQUIRES(std::is_same_v<T,int>, double) foo (double x) { ... }
 @endcode
 * declares a function @c foo returning @c double that is enabled
 * only if @c T is an @c int.   If you have a definition separate
 * from the declaration, then use ATH_MEMBER_REQUIRES_DEF in the definition
 * instead.
 *
 * Will use a @c requires clause with C++20 and @c enable_if otherwise.
 */


#ifndef CXXUTILS_CONCEPTS_H
#define CXXUTILS_CONCEPTS_H


#include <type_traits>
#include <iterator>


#ifdef __cpp_concepts

#define HAVE_CONCEPTS 1

#include <concepts>
#define ATH_REQUIRES(...) requires __VA_ARGS__

#define ATH_MEMBER_REQUIRES(CONDITION, RETTYPE) \
  template <bool = true> \
  requires (CONDITION)   \
  RETTYPE

#define ATH_MEMBER_REQUIRES_DEF(CONDITION, RETTYPE) \
  template <bool> \
  requires (CONDITION)   \
  RETTYPE


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

#else

#define HAVE_CONCEPTS 0

#define ATH_REQUIRES(...)

#define ATH_MEMBER_REQUIRES(CONDITION, RETTYPE) \
  template <bool Enable = true> \
  std::enable_if_t<Enable && (CONDITION), RETTYPE>

#define ATH_MEMBER_REQUIRES_DEF(CONDITION, RETTYPE) \
  template <bool Enable> \
  std::enable_if_t<Enable && (CONDITION), RETTYPE>

#endif


#endif // not CXXUTILS_CONCEPTS_H
