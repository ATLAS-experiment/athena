/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef CXXUTILS_HASH_UTILS_H
#define CXXUTILS_HASH_UTILS_H

#include <cstddef>
#include <functional>
#include <iterator>
#include <type_traits>

namespace CxxUtils {

/**
 * @brief Combine @c value into @c seed using a Boost-compatible mixing step.
 */
template <class T>
inline void hash_combine(std::size_t& seed, const T& value)
{
  using ValueType = std::decay_t<T>;
  seed ^= std::hash<ValueType>{}(value) + 0x9e3779b9u + (seed << 6) + (seed >> 2);
}

/**
 * @brief Hash a half-open iterator range by repeatedly applying @c hash_combine.
 */
template <class Iterator>
inline std::size_t hash_range(Iterator first, Iterator last)
{
  std::size_t seed = 0;
  for (; first != last; ++first) {
    hash_combine(seed, *first);
  }
  return seed;
}

/**
 * @brief Convenience overload hashing an entire range/container.
 */
template <class Range>
inline std::size_t hash_range(const Range& range)
{
  using std::begin;
  using std::end;
  return hash_range(begin(range), end(range));
}

} // namespace CxxUtils

#endif
