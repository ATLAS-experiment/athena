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

template <typename SET>
concept SimpleAssociativeContainer = requires(SET s, typename SET::key_type k) {
    typename SET::value_type;
    typename SET::key_type;
    typename SET::iterator;
    typename SET::const_iterator;
    typename SET::reference;
    typename SET::const_reference;
    typename SET::const_pointer;

    { s.find(k) } -> std::convertible_to<typename SET::const_iterator>;
    { s.insert(k) } -> std::convertible_to<std::pair<typename SET::iterator, bool>>;
};

template <typename MAP>
concept PairAssociativeContainer = requires(MAP m, typename MAP::key_type k, typename MAP::mapped_type v) {
    typename MAP::value_type;
    typename MAP::key_type;
    typename MAP::mapped_type;

    { m[k] } -> std::convertible_to<typename MAP::mapped_type>;
    { m.find(k) } -> std::convertible_to<typename MAP::iterator>;
    { m.insert(std::make_pair(k, v)) } -> std::same_as<std::pair<typename MAP::iterator, bool>>;
};

// An allocation function.  Can be used like <code>T* p = F()</code> to allocate
// a new object.
template <typename F, typename T>
concept AllocationFunction =
  std::invocable<F> && std::convertible_to<std::invoke_result_t<F>, T*>;


/// Has addRef() and release()
template <class T>
concept RefCounted =
  requires (T& x)
{
  { x.addRef() };
  { x.release() };
};


} // namespace detail
} // namespace CxxUtils



#endif // not CXXUTILS_CONCEPTS_H
