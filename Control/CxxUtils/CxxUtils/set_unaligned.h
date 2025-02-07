// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/set_unaligned.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2015
 * @brief Write little-endian values through possibly unaligned pointers.
 *
 * Doing, eg,
 *@code
 *   char* p = ...;
 *   int* ip = reinterpret_cast<int*>(p);
 *   *ip++ = i;
 *@endcode
 *
 * is undefined if @c p isn't aligned to an int boundary.  It further
 * will read the value using the host endianness, which is an issue
 * for the common use case of reading persistent data.
 *
 * The functions here allow writing little-endian data via possibly
 * unaligned pointers regardless of the host byte ordering and
 * without relying on undefined behavior.
 */


#ifndef CXXUTILS_SET_UNALIGNED_H
#define CXXUTILS_SET_UNALIGNED_H


#include "CxxUtils/restrict.h"
#include <cstring>
#include <bit>
#include <cstdint>


#ifdef __linux__
# include <endian.h>
#else
namespace CxxUtils {
  inline uint16_t htole16 (uint16_t x) { return x; }
  inline uint32_t htole32 (uint32_t x) { return x; }
  inline uint64_t htole64 (uint64_t x) { return x; }
}
#endif


namespace CxxUtils {


/**
 * @brief Write a 2-byte little-endian value to a possibly unaligned pointer.
 * @param p Pointer to which to write.  Advanced to the next value.
 * @param val Value to write.
 *
 * Writes a little-endian value, regardless of the host byte ordering,
 * and advances the pointer.
 * Should not rely on undefined behavior, regardless of the alignment of @c p.
 *
 * If used in a loop, you'll get better code with a restricted pointer.
 */
inline
void set_unaligned16 (uint8_t* ATH_RESTRICT &  p, uint16_t val)
{
  uint16_t tmp = htole16 (val);
  memcpy (p, &tmp, sizeof(tmp));
  p += sizeof(tmp);
}


/**
 * @brief Write a 4-byte little-endian value to a possibly unaligned pointer.
 * @param p Pointer to which to write.  Advanced to the next value.
 * @param val Value to write.
 *
 * Writes a little-endian value, regardless of the host byte ordering,
 * and advances the pointer.
 * Should not rely on undefined behavior, regardless of the alignment of @c p.
 *
 * If used in a loop, you'll get better code with a restricted pointer.
 */
inline
void set_unaligned32 (uint8_t* ATH_RESTRICT &  p, uint32_t val)
{
  uint32_t tmp = htole32 (val);
  memcpy (p, &tmp, sizeof(tmp));
  p += sizeof(tmp);
}


/**
 * @brief Write an 8-byte little-endian value to a possibly unaligned pointer.
 * @param p Pointer to which to write.  Advanced to the next value.
 * @param val Value to write.
 *
 * Writes a little-endian value, regardless of the host byte ordering,
 * and advances the pointer.
 * Should not rely on undefined behavior, regardless of the alignment of @c p.
 *
 * If used in a loop, you'll get better code with a restricted pointer.
 */
inline
void set_unaligned64 (uint8_t* ATH_RESTRICT &  p, uint64_t val)
{
  uint64_t tmp = htole64 (val);
  memcpy (p, &tmp, sizeof(tmp));
  p += sizeof(tmp);
}


/**
 * @brief Write a little-endian float value to a possibly unaligned pointer.
 * @param p Pointer to which to write.  Advanced to the next value.
 * @param val Value to write.
 *
 * Writes a little-endian value, regardless of the host byte ordering,
 * and advances the pointer.
 * Should not rely on undefined behavior, regardless of the alignment of @c p.
 *
 * If used in a loop, you'll get better code with a restricted pointer.
 */
inline
void set_unaligned_float (uint8_t* ATH_RESTRICT &  p, float val)
{
  set_unaligned32 (p, std::bit_cast<uint32_t> (val));
}


/**
 * @brief Write a little-endian double value to a possibly unaligned pointer.
 * @param p Pointer to which to write.  Advanced to the next value.
 * @param val Value to write.
 *
 * Writes a little-endian value, regardless of the host byte ordering,
 * and advances the pointer.
 * Should not rely on undefined behavior, regardless of the alignment of @c p.
 *
 * If used in a loop, you'll get better code with a restricted pointer.
 */
inline
void set_unaligned_double (uint8_t* ATH_RESTRICT &  p, double val)
{
  set_unaligned64 (p, std::bit_cast<uint64_t> (val));
}


/// Define templated versions of the above functions.


template <class T>
void set_unaligned (uint8_t* ATH_RESTRICT &  p, T val);


template <>
inline
void set_unaligned<uint8_t> (uint8_t* ATH_RESTRICT &  p, uint8_t val)
{
  *p++ = val;
}


template <>
inline
void set_unaligned<uint16_t> (uint8_t* ATH_RESTRICT &  p, uint16_t val)
{
  set_unaligned16 (p, val);
}


template <>
inline
void set_unaligned<uint32_t> (uint8_t* ATH_RESTRICT &  p, uint32_t val)
{
  set_unaligned32 (p, val);
}


template <>
inline
void set_unaligned<uint64_t> (uint8_t* ATH_RESTRICT &  p, uint64_t val)
{
  set_unaligned64 (p, val);
}


template <>
inline
void set_unaligned<float> (uint8_t* ATH_RESTRICT &  p, float f)
{
  set_unaligned_float (p, f);
}


template <>
inline
void set_unaligned<double> (uint8_t* ATH_RESTRICT &  p, double f)
{
  set_unaligned_double (p, f);
}


template <>
inline
void set_unaligned<int8_t> (uint8_t* ATH_RESTRICT &  p, int8_t val)
{
  set_unaligned<uint8_t> (p, std::bit_cast<uint8_t> (val));
}


template <>
inline
void set_unaligned<int16_t> (uint8_t* ATH_RESTRICT &  p, int16_t val)
{
  set_unaligned<uint16_t> (p, std::bit_cast<uint16_t> (val));
}


template <>
inline
void set_unaligned<int32_t> (uint8_t* ATH_RESTRICT &  p, int32_t val)
{
  set_unaligned<uint32_t> (p, std::bit_cast<uint32_t> (val));
}


template <>
inline
void set_unaligned<int64_t> (uint8_t* ATH_RESTRICT &  p, int64_t val)
{
  set_unaligned<uint64_t> (p, std::bit_cast<uint64_t> (val));
}


} // namespace CxxUtils


#endif // not CXXUTILS_GET_UNALIGNED_H
