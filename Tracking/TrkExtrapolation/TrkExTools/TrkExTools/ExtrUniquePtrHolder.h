/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRKEXTOOLS_EXTRUNIQUEPTRHOLDER_H
#define TRKEXTOOLS_EXTRUNIQUEPTRHOLDER_H

/** @brief Special internal ptr caching
 *
 * During an extrapolation chain that we want to keep the parameters alive.
 * We use a vector of unique_ptr.
 * We can access the stored values as ptr during the extrapolation chains.
 * By the end of the extrapolation chain they will go out of scope
 * and get deleted.
 * If we want to retain a ptr we need to take it before this happens
 *
 * @author Christos Anastopoulos
 *
 * */

#include <algorithm>
#include <memory>
#include <vector>

namespace Trk{

template <typename T>
using CacheOwnedPtr = T*;

template <typename T>
struct ExtrUniquePtrHolder {

  ExtrUniquePtrHolder() = default;
  // ctor with capacity
  ExtrUniquePtrHolder(size_t capacity) { m_elements.reserve(capacity); }
  ~ExtrUniquePtrHolder() = default;
  // delete all the rest
  ExtrUniquePtrHolder(const ExtrUniquePtrHolder&) = delete;
  ExtrUniquePtrHolder(ExtrUniquePtrHolder&&) = delete;
  ExtrUniquePtrHolder& operator=(const ExtrUniquePtrHolder&) = delete;
  ExtrUniquePtrHolder& operator=(ExtrUniquePtrHolder&&) = delete;

  /** @brief push a new element to the vector and return a ptr to it*/
  CacheOwnedPtr<T> push(std::unique_ptr<T> input) {
    if (input == nullptr) {
      return nullptr;
    }
    m_elements.push_back(std::move(input));
    return m_elements.back().get();
  }

  /** @brief Release a cached ptr.
  The entry in the cache will be moved from.
  The lifetime of the object is now managed by
  the returned unique_ptr*/
  std::unique_ptr<T> move(CacheOwnedPtr<T> input) {
    if (input == nullptr) {
      return nullptr;
    }
    // start from the last as this is more likely
    auto itr = std::find_if(
        m_elements.rbegin(), m_elements.rend(),
        [&input](const std::unique_ptr<T>& x) { return x.get() == input; });
    if (itr == m_elements.crend()) {
      return nullptr;
    }
    return std::unique_ptr<T>(std::move(*itr));
  }
  // The vector of unique_ptr
  std::vector<std::unique_ptr<T>> m_elements;
};

}  // namespace Trk

#endif
