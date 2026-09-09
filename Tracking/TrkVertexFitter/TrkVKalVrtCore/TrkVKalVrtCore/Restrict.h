#ifndef TRKVKALVRTCORE_RESTRICT_H
#define TRKVKALVRTCORE_RESTRICT_H

/*
 * Portable non-aliasing qualifier for pointer parameters.
 *
 * C99:          restrict
 * GCC/Clang:    __restrict__
 * MSVC:         __restrict
 * Intel classic: __restrict
 * Fallback:     empty
 */

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) && !defined(__cplusplus)
#  define VKAL_RESTRICT restrict

#elif defined(__clang__)
#  define VKAL_RESTRICT __restrict__

#elif defined(__INTEL_COMPILER) || defined(__INTEL_LLVM_COMPILER)
#  define VKAL_RESTRICT __restrict

#elif defined(__GNUC__)
#  define VKAL_RESTRICT __restrict__

#elif defined(_MSC_VER)
#  define VKAL_RESTRICT __restrict

#else
#  define VKAL_RESTRICT
#endif

#endif /* TRKVKALVRTCORE_RESTRICT_H */