// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/bit_int.h
 * @author Andrii Verbytskyi <andrii.verbytskyi@mpp.mpg.de>
 * @date Jun, 2026
 * @brief Ispired by std::bit_int is proposed for a future C++ standard (P3666R2)
 */

#ifndef CXXUTILS_BIT_INT_H
#define CXXUTILS_BIT_INT_H
#include <cstdint>
#include <cstddef>

namespace CxxUtils {

template <size_t Bits> struct bit_int;
template <> struct bit_int<8>  { using type = std::int8_t;  };
template <> struct bit_int<16> { using type = std::int16_t; };
template <> struct bit_int<32> { using type = std::int32_t; };
template <> struct bit_int<64> { using type = std::int64_t; };

template <size_t Bits> using bit_int_t = typename bit_int<Bits>::type;

} // namespace CxxUtils
#endif