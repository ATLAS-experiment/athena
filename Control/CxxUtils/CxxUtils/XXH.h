// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/XXH.h
 * @author Beojan Stanislaus <bstanislaus@lbl.gov>
 * @date August 2023
 * @brief C++ native wrapper for the C xxhash API
 */
#ifndef CXXUTILS_XXH_H
#define CXXUTILS_XXH_H

#include <cstdint>
#include <type_traits>

/**
 * Wrapper namespace for our xxhash (xxh3) wrapper function
 */
namespace xxh3 {

/**
 * Passthrough to XXH3_64bits
 * @param data Pointer to data to be hashed
 * @param size Size of data to be hashed (in bytes)
 * @return 64-bit xxh3 hash of data
 */
std::uint64_t hash64(const void* data, std::size_t size);

/**
 * Hash a container with `data()` and `size()` members
 *
 * Examples include `std::string`, `std::vector`, `std::string_view`
 *
 * @param cont Container to be hashed
 * @return 64-bit xxh3 hash of container
 */
template <typename Cont>
std::uint64_t hash64(const Cont& cont) {
  return hash64(cont.data(), cont.size() * sizeof(typename Cont::value_type));
}

/**
 * Hash a POD value / struct
 *
 * @param pod Value to be hashed
 * @return 64-bit xxh3 hash of value
 */
template <typename POD,
          typename = std::enable_if_t<std::is_standard_layout_v<POD> &&
                                      std::is_trivial_v<POD>>>
std::uint64_t hash64(const POD& pod) {
  return hash64(&pod, sizeof(POD));
}
}  // namespace xxh3

#endif  // CXXUTILS_XXH_H
