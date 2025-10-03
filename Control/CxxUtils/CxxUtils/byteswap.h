// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/byteswap.h
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2025
 * @brief C++23-compatible byteswap()
 */


#ifndef CXXUTILS_BYTESWAP_H
#define CXXUTILS_BYTESWAP_H


#include <version>
#include <concepts>
#include <bit>
#include <array>
#include <algorithm>


namespace CxxUtils {


#if __cpp_lib_byteswap

// Use library version if available.
using std::byteswap;

#else


/**
 * @brief Reverse the bytes in n.
 *
 * Copied from the example implementation given in
 * https://en.cppreference.com/w/cpp/numeric/byteswap
 */
template<std::integral T>
constexpr T byteswap(T value) noexcept
{
    static_assert(std::has_unique_object_representations_v<T>, 
                  "T may not have padding bits");
    auto value_representation = std::bit_cast<std::array<std::byte, sizeof(T)>>(value);
    std::ranges::reverse(value_representation);
    return std::bit_cast<T>(value_representation);
}


#endif // not __cpp_lib_byteswap


} // namespace CxxUtils


#endif // not CXXUTILS_BYTESWAP_H
