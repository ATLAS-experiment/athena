//
// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_FPHELPERS_H
#define CALORECGPU_FPHELPERS_H

#ifndef CALORECGPU_INCLUDE_CUDA_SUPPORT

  #define CALORECGPU_INCLUDE_CUDA_SUPPORT 1

  //If CUDA is available, we will support its native floating point operations.
  //We can disable this by defining CALORECGPU_INCLUDE_CUDA_SUPPORT as 0...

#endif

#include <cstdint>
#include <climits>
#include <cstring>

#if defined (__CUDA_ARCH__) && CALORECGPU_INCLUDE_CUDA_SUPPORT

  #include "cuda_fp16.h"

  #include "cuda_bf16.h"

#endif

#if __cpp_lib_bitops || __cpp_lib_bit_cast

  #include <bit>

#endif


/** @file FPHelpers.h
 *  Contains functions to deal with arbitrary IEEE754-like floating point formats.
 */

//In its current form, it is really only used
//to provide to_total_ordering for floats
//used in the GPU. For a while (before we
//came up with the current tag assignment
//for the splitter), it provided us with
//several utilities to emulate less precise
//floating point numbers so we could
//squash the energy of the tags...

namespace FloatingPointHelpers
{

  /*! @enum RoundingModes
   *  Specifies the rounding mode to use for the operations.
  */
  enum class RoundingModes
  {
    ToPlusInfinity, ToMinusInfinity, ToZero, ToNearestEven, ToNearestAwayFromZero, Default = ToNearestEven
  };

  namespace LeadingZerosPortability
  {

#if __cpp_lib_bitops

    template <class T>
    inline static constexpr unsigned int count_leading_zeros(const T num)
    {
      return std::countl_zero(num);
    }

#else

    template <class T>
    inline static constexpr unsigned int count_leading_zeros(const T num)
    //I know this could be greatly optimized.
    //The point is, either pray for the compiler's smartness
    //or replace this with a non-portable built-in
    //whenever / wherever necessary...
    {
      T probe = T(1) << (sizeof(T) * CHAR_BIT - 1);
      unsigned int ret = 0;
      while ((num & probe) == 0 && probe)
        {
          ++ret;
          probe >>= 1;
        }
      return ret;
    }

#endif

#define CALORECGPU_MULTIPLE_PORTABILITY_CLZ_FUNC_HELPER(ATTRIB, TYPE, BUILTIN) \
  template<>                                                                   \
  ATTRIB inline unsigned int count_leading_zeros(const TYPE num)               \
  {                                                                            \
    if (!num)                                                                  \
      {                                                                        \
        return sizeof(TYPE) * CHAR_BIT;                                        \
      }                                                                        \
    return BUILTIN(num);                                                       \
  }                                                                            \



#if defined (__CUDA_ARCH__) && CALORECGPU_INCLUDE_CUDA_SUPPORT


    CALORECGPU_MULTIPLE_PORTABILITY_CLZ_FUNC_HELPER(__device__, int, __clz)

    CALORECGPU_MULTIPLE_PORTABILITY_CLZ_FUNC_HELPER(__device__, unsigned int, __clz)

    CALORECGPU_MULTIPLE_PORTABILITY_CLZ_FUNC_HELPER(__device__, long long, __clzll)

    CALORECGPU_MULTIPLE_PORTABILITY_CLZ_FUNC_HELPER(__device__, unsigned long long, __clzll)


#elif defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)


    CALORECGPU_MULTIPLE_PORTABILITY_CLZ_FUNC_HELPER(constexpr, int, __builtin_clz)

    CALORECGPU_MULTIPLE_PORTABILITY_CLZ_FUNC_HELPER(constexpr, unsigned int, __builtin_clz)

    CALORECGPU_MULTIPLE_PORTABILITY_CLZ_FUNC_HELPER(constexpr, long, __builtin_clzl)

    CALORECGPU_MULTIPLE_PORTABILITY_CLZ_FUNC_HELPER(constexpr, unsigned long, __builtin_clzl)

    CALORECGPU_MULTIPLE_PORTABILITY_CLZ_FUNC_HELPER(constexpr, long long, __builtin_clzll)

    CALORECGPU_MULTIPLE_PORTABILITY_CLZ_FUNC_HELPER(constexpr, unsigned long long, __builtin_clzll)


#if  defined(__clang__)


  CALORECGPU_MULTIPLE_PORTABILITY_CLZ_FUNC_HELPER(constexpr, short, __builtin_clzs)

  CALORECGPU_MULTIPLE_PORTABILITY_CLZ_FUNC_HELPER(constexpr, unsigned short, __builtin_clzs)

#endif


#elif defined(_MSC_VER)


  }
}

#include <intrin.h>

namespace FloatingPointHelpers
{
  namespace LeadingZerosPortability
  {

    CALORECGPU_MULTIPLE_PORTABILITY_CLZ_FUNC_HELPER(, unsigned short, __lzcnt16)

    CALORECGPU_MULTIPLE_PORTABILITY_CLZ_FUNC_HELPER(, unsigned int, __lzcnt)

    CALORECGPU_MULTIPLE_PORTABILITY_CLZ_FUNC_HELPER(, short, __lzcnt16)

    CALORECGPU_MULTIPLE_PORTABILITY_CLZ_FUNC_HELPER(, int, __lzcnt)

#if defined(_WIN64)

    //__lzcnt64 is only available in 64 bit, I think?
    CALORECGPU_MULTIPLE_PORTABILITY_CLZ_FUNC_HELPER(, unsigned long long int, __lzcnt64)

    CALORECGPU_MULTIPLE_PORTABILITY_CLZ_FUNC_HELPER(, long long int, __lzcnt64)

#else
    template <>
    inline unsigned long long int count_leading_zeros<unsigned long long int>(unsigned long long int T num)
    {
      const auto res_1 = count_leading_zeros<unsigned int>(num >> 32);
      return res_1 + (res_1 == 32) * count_leading_zeros<unsigned int>(num);
    }

    template <>
    inline long long int count_leading_zeros<long long int>(long long int T num)
    {
      return count_leading_zeros<unsigned long long int>(num);
    }

#endif


#endif

    //We could add more compilers here if needed,
    //but the "big three" and CUDA should already be covered.

#undef CALORECGPU_MULTIPLE_PORTABILITY_CLZ_FUNC_HELPER

  }

  namespace OperatorsHelper
  {
    //Left and right shifts larger than the variable size are UB.

    template <class T>
    inline static constexpr T safe_lshift(const T x, const T amount)
    {
      const bool valid = amount < sizeof(T) * CHAR_BIT;
      return (x << (amount * valid)) * valid;
    }

    template <class T>
    inline static constexpr T safe_rshift(const T x, const T amount)
    {
      const bool valid = amount < sizeof(T) * CHAR_BIT;
      return (x >> (amount * valid)) * valid;
    }

    //To prevent underflow for unsigned variables
    template <class T>
    inline static constexpr T clamped_sub(const T x1, const T x2)
    {
      return (x1 - x2) * (x1 >= x2);
    }

    template <class T>
    inline static constexpr T min(const T x1, const T x2)
    {
      return (x1 > x2) * x2 + (x1 <= x2) * x1;
    }

    template <class T>
    inline static constexpr T max(const T x1, const T x2)
    {
      return (x1 > x2) * x1 + (x1 <= x2) * x2;
    }

    template <class T>
    inline static constexpr T clamp(const T x, const T low, const T high)
    {
      return low * (x < low) + high * (x > high) + x * (x >= low && x <= high);
    }

    //Just for occasional clarity with the arguments.

    template <class T>
    inline static constexpr T bit_and(const T x1, const T x2)
    {
      return x1 & x2;
    }

    template <class T>
    inline static constexpr T bit_or(const T x1, const T x2)
    {
      return x1 | x2;
    }

  }

  namespace BitCastHelper
  {

#if __cpp_lib_bit_cast


    template <class To, class From>
    constexpr inline static To bitcast(const From & x)
    {
      return std::bit_cast<To, From>(x);
    }

#else

    //The disadvantage here is that this won't be actually constexpr due to memcpy...

    template <class To, class From>
    constexpr inline static To bitcast(const From & x)
    {
      To ret = 0;
      std::memcpy(&ret, &x, sizeof(To));
      return ret;
    }

#endif

#if defined (__CUDA_ARCH__) && CALORECGPU_INCLUDE_CUDA_SUPPORT

#define CALORECGPU_CUDACAST_HELPER(TYPE_TO, TYPE_FROM, CONVFUNC) \
  template <> __device__ constexpr inline                        \
  TYPE_TO bitcast< TYPE_TO, TYPE_FROM >(const TYPE_FROM &x)      \
  {                                                              \
    return CONVFUNC (x);                                         \
  }                                                              \


    CALORECGPU_CUDACAST_HELPER(  int64_t,   double, __double_as_longlong );
    CALORECGPU_CUDACAST_HELPER( uint64_t,   double, __double_as_longlong );
    CALORECGPU_CUDACAST_HELPER(   double,  int64_t, __longlong_as_double );
    CALORECGPU_CUDACAST_HELPER(   double, uint64_t, __longlong_as_double );

    CALORECGPU_CUDACAST_HELPER(  int32_t,    float,  __float_as_int );
    CALORECGPU_CUDACAST_HELPER( uint32_t,    float, __float_as_uint );
    CALORECGPU_CUDACAST_HELPER(  int64_t,    float, __float_as_uint );
    CALORECGPU_CUDACAST_HELPER( uint64_t,    float, __float_as_uint );
    CALORECGPU_CUDACAST_HELPER(    float,  int32_t,  __int_as_float );
    CALORECGPU_CUDACAST_HELPER(    float, uint32_t, __uint_as_float );
    CALORECGPU_CUDACAST_HELPER(    float,  int64_t, __uint_as_float );
    CALORECGPU_CUDACAST_HELPER(    float, uint64_t, __uint_as_float );

    CALORECGPU_CUDACAST_HELPER(  int16_t,   __half,  __half_as_short );
    CALORECGPU_CUDACAST_HELPER( uint16_t,   __half, __half_as_ushort );
    CALORECGPU_CUDACAST_HELPER(  int32_t,   __half, __half_as_ushort );
    CALORECGPU_CUDACAST_HELPER( uint32_t,   __half, __half_as_ushort );
    CALORECGPU_CUDACAST_HELPER(  int64_t,   __half, __half_as_ushort );
    CALORECGPU_CUDACAST_HELPER( uint64_t,   __half, __half_as_ushort );
    CALORECGPU_CUDACAST_HELPER(   __half,  int16_t,  __short_as_half );
    CALORECGPU_CUDACAST_HELPER(   __half, uint16_t, __ushort_as_half );
    CALORECGPU_CUDACAST_HELPER(   __half,  int32_t, __ushort_as_half );
    CALORECGPU_CUDACAST_HELPER(   __half, uint32_t, __ushort_as_half );
    CALORECGPU_CUDACAST_HELPER(   __half,  int64_t, __ushort_as_half );
    CALORECGPU_CUDACAST_HELPER(   __half, uint64_t, __ushort_as_half );
    /*
        CALORECGPU_CUDACAST_HELPER(       int16_t, __nv_bfloat16,  __bfloat16_as_short );
        CALORECGPU_CUDACAST_HELPER(      uint16_t, __nv_bfloat16, __bfloat16_as_ushort );
        CALORECGPU_CUDACAST_HELPER(       int32_t, __nv_bfloat16, __bfloat16_as_ushort );
        CALORECGPU_CUDACAST_HELPER(      uint32_t, __nv_bfloat16, __bfloat16_as_ushort );
        CALORECGPU_CUDACAST_HELPER(       int64_t, __nv_bfloat16, __bfloat16_as_ushort );
        CALORECGPU_CUDACAST_HELPER(      uint64_t, __nv_bfloat16, __bfloat16_as_ushort );
        CALORECGPU_CUDACAST_HELPER( __nv_bfloat16,       int16_t,  __short_as_bfloat16 );
        CALORECGPU_CUDACAST_HELPER( __nv_bfloat16,      uint16_t, __ushort_as_bfloat16 );
        CALORECGPU_CUDACAST_HELPER( __nv_bfloat16,       int32_t, __ushort_as_bfloat16 );
        CALORECGPU_CUDACAST_HELPER( __nv_bfloat16,      uint32_t, __ushort_as_bfloat16 );
        CALORECGPU_CUDACAST_HELPER( __nv_bfloat16,       int64_t, __ushort_as_bfloat16 );
        CALORECGPU_CUDACAST_HELPER( __nv_bfloat16,      uint64_t, __ushort_as_bfloat16 );

        This is apparently not working?! Why?!
    */
#endif

  }


  /*! @class IEEE754_like
      Specifies a floating point format like those described in IEEE-754,
      with an adjustable number of bits in the exponent and mantissa.

      @p tag just allows differing floating point definitions with the same size,
         in case there are e. g. multiple implementations of native/faster
         operations available. Tag 0 is reserved for the default (i. e. non-native)
         implementation.
  */
  template <unsigned int mantiss, unsigned int exp, unsigned int tag = 1> struct IEEE754_like
  {

    static_assert(mantiss > 0 && exp > 0, "The exponent and mantissa must contain a positive number of bits!");

    constexpr inline static unsigned int total_size_bits()
    {
      return mantiss + exp + 1;
    }

    constexpr inline static unsigned int mantissa_size_bits()
    {
      return mantiss;
    }

    constexpr inline static unsigned int exponent_size_bits()
    {
      return exp;
    }

    template <class T>
    constexpr inline static T mantissa_mask()
    {
      static_assert(sizeof(T) * CHAR_BIT >= (mantiss + exp + 1),
                    "The type must be large enough to hold the bit representation of the floating point." );
      T ret = (T(1) << mantiss) - 1;
      return ret;
    }

    template <class T>
    constexpr inline static T exponent_mask()
    {
      static_assert(sizeof(T) * CHAR_BIT >= (mantiss + exp + 1),
                    "The type must be large enough to hold the bit representation of the floating point." );

      T ret = (T(1) << exp) - 1;
      return ret << mantiss;
    }

    template <class T>
    constexpr inline static T sign_mask()
    {
      static_assert(sizeof(T) * CHAR_BIT >= (mantiss + exp + 1),
                    "The type must be large enough to hold the bit representation of the floating point." );
      T ret = T(1) << (exp + mantiss);
      return ret;
    }

    template <class T>
    constexpr inline static T full_mask()
    {
      return mantissa_mask<T>() | exponent_mask<T>() | sign_mask<T>();
    }

    template <class T>
    constexpr inline static T exponent_bias()
    {
      static_assert(sizeof(T) * CHAR_BIT >= (mantiss + exp + 1),
                    "The type must be large enough to hold the bit representation of the floating point." );
      return (T(1) << (exp - 1)) - 1;
    }

    template <class T>
    constexpr inline static T max_exponent_with_bias()
    {
      static_assert(sizeof(T) * CHAR_BIT >= (mantiss + exp + 1),
                    "The type must be large enough to hold the bit representation of the floating point." );
      return exponent_bias<T>() * 2;
    }

    template <class T>
    constexpr inline static bool is_infinite(const T pattern)
    {
      return (pattern & (~sign_mask<T>())) == exponent_mask<T>();
    }

    template <class T>
    constexpr inline static bool is_NaN(const T pattern)
    {
      return (pattern & (~sign_mask<T>())) > exponent_mask<T>();
      //If it also has bits in the mantissa, it's greater than the mask.
      //Last bit is sign, so signedness of T is of no concern.
    }

    template <class T>
    constexpr inline static T absolute_value(const T pattern)
    {
      return pattern & (~sign_mask<T>());
    }

    /*! @warning Even though +0 and -0 should technically compare equal,
        we will convert between them too so the operation becomes fully reversible.
    */
    template <class T>
    constexpr inline static T to_total_ordering(const T pattern)
    {
      const T xor_mask = (!!(pattern & sign_mask<T>()) * full_mask<T>()) | sign_mask<T>();
      return pattern ^ xor_mask;
    }

    /*! @warning Even though +0 and -0 should technically compare equal,
        we will convert between them too so the operation becomes fully reversible.
    */
    template <class T>
    constexpr inline static T from_total_ordering(const T pattern)
    {
      const T xor_mask = (!(pattern & sign_mask<T>()) * full_mask<T>()) | sign_mask<T>();
      return pattern ^ xor_mask;
    }

    template <class T>
    constexpr inline static T positive_zero()
    {
      return T(0);
    }

    template <class T>
    constexpr inline static T negative_zero()
    {
      return sign_mask<T>();
    }

    template <class T>
    constexpr inline static T positive_infinity()
    {
      return exponent_mask<T>();
    }

    template <class T>
    constexpr inline static T negative_infinity()
    {
      return sign_mask<T>() | exponent_mask<T>();
    }

    template <class T>
    constexpr inline static bool round_results(const bool is_negative, const bool is_odd,
                                               const bool is_nearer_to_up, const bool is_tied,
                                               RoundingModes rt)
    {
      switch (rt)
        {
          case RoundingModes::ToPlusInfinity:
            return !is_negative;
          case RoundingModes::ToMinusInfinity:
            return is_negative;
          case RoundingModes::ToZero:
            return 0;
          //Truncate => do nothing
          case RoundingModes::ToNearestEven:
            return is_nearer_to_up || (is_odd && is_tied);
          case RoundingModes::ToNearestAwayFromZero:
            return is_nearer_to_up || is_tied;
          default:
            return 0;
        }
    }


    /*! The absolute value of @a must be greater than or equal than that of @b.
        We also don't handle zero, NaN or infinities here.
    */
    template <class T>
    constexpr inline static T add_patterns (const T a, const T b, const RoundingModes rt = RoundingModes::Default)
    {
      using namespace OperatorsHelper;

      constexpr unsigned int extra_bits = 2;
      //One sign and at least one exponent bit, we're safe!

      constexpr T first_not_mantissa_bit = T(1) << mantissa_size_bits();

      const T exp_a = (a & exponent_mask<T>()) >> mantissa_size_bits();
      const T exp_b = (b & exponent_mask<T>()) >> mantissa_size_bits();

      const bool a_denormal = (exp_a != 0);
      const bool b_denormal = (exp_b != 0);

      const bool use_second = (exp_a - exp_b) <= mantissa_size_bits() + 1 + extra_bits;
      const bool is_negative = a & sign_mask<T>();

      const T mantiss_a = ((a & mantissa_mask<T>()) | (first_not_mantissa_bit * a_denormal)) << extra_bits;
      const T mantiss_b = ((b & mantissa_mask<T>()) | (first_not_mantissa_bit * b_denormal)) << extra_bits;
      //To account for the overflow and rounding.

      T mantiss_ret = mantiss_a;

      mantiss_ret += safe_rshift(mantiss_b, exp_a - exp_b);

      mantiss_ret |= !!(safe_lshift(mantiss_b, exp_a - exp_b) & mantissa_mask<T>()) * use_second;

      const unsigned int leading_zeros = LeadingZerosPortability::count_leading_zeros<T>(mantiss_ret);
      constexpr unsigned int desired_number_of_zeros = sizeof(T) * CHAR_BIT - mantissa_size_bits() - 1 - extra_bits;
      const unsigned int shift_amount = clamped_sub(desired_number_of_zeros, leading_zeros);

      const T last_bit_mask = T(1) << (shift_amount + extra_bits);
      const T last_discarded_bit_mask = last_bit_mask >> 1;
      const T round_mask = (last_bit_mask - 1) * !!(last_bit_mask);
      const bool round_up = (mantiss_ret & round_mask) > last_discarded_bit_mask;
      const bool tied = last_discarded_bit_mask && ((mantiss_ret & round_mask) == last_discarded_bit_mask);

      bool round_bit = round_results<T>(is_negative, (mantiss_ret & last_bit_mask), round_up, tied, rt) && !!last_bit_mask;

      mantiss_ret = safe_rshift(mantiss_ret, shift_amount + extra_bits);

      mantiss_ret += round_bit * (shift_amount + extra_bits <= sizeof(T) * CHAR_BIT);

      const T exponent_ret = exp_a + shift_amount + (exp_a == 0 &&  mantiss_ret > mantissa_mask<T>());

      mantiss_ret &= mantissa_mask<T>();

      mantiss_ret &= ~( ( exponent_ret > max_exponent_with_bias<T>() ) * mantissa_mask<T>() );
      //If we somehow summed up to infinity,
      //unset the remaining bits.

      return (is_negative * sign_mask<T>()) | (exponent_ret << mantissa_size_bits()) | mantiss_ret;
    }

    /*! The absolute value of @a must be greater than or equal than that of @b.
        We also don't handle zero, NaN or infinities here.
    */
    template <class T>
    constexpr inline static T subtract_patterns (const T a, const T b, const RoundingModes rt = RoundingModes::Default)
    {
      using namespace OperatorsHelper;

      constexpr unsigned int extra_bits = 2;
      //One sign and at least one exponent bit, we're safe!

      constexpr T first_not_mantissa_bit = T(1) << mantissa_size_bits();

      const T exp_a = (a & exponent_mask<T>()) >> mantissa_size_bits();
      const T exp_b = (b & exponent_mask<T>()) >> mantissa_size_bits();

      const bool use_second = (exp_a - exp_b) <= mantissa_size_bits() + 1 + extra_bits;
      const bool is_negative = a & sign_mask<T>();

      const T mantiss_a = ((a & mantissa_mask<T>()) | (first_not_mantissa_bit * (exp_a != 0))) << extra_bits;
      const T mantiss_b = ((b & mantissa_mask<T>()) | (first_not_mantissa_bit * (exp_b != 0))) << extra_bits;
      //To account for the overflow and rounding.

      T mantiss_ret = mantiss_a;

      mantiss_ret -= safe_rshift(mantiss_b, exp_a - exp_b) * use_second;

      mantiss_ret |= !!(safe_lshift(-mantiss_b, exp_a - exp_b) & mantissa_mask<T>()) * use_second;

      const unsigned int leading_zeros = LeadingZerosPortability::count_leading_zeros<T>(mantiss_ret);
      constexpr unsigned int desired_number_of_zeros = sizeof(T) * CHAR_BIT - mantissa_size_bits() - 1 - extra_bits;
      const unsigned int shift_amount = clamped_sub(leading_zeros, desired_number_of_zeros);

      const T last_bit_mask = T(1) << extra_bits;
      const T last_discarded_bit_mask = last_bit_mask >> 1;
      const T round_mask = (last_bit_mask - 1) * !!(last_bit_mask);
      const bool round_up = (mantiss_ret & round_mask) > last_discarded_bit_mask;
      const bool tied = last_discarded_bit_mask && ((mantiss_ret & round_mask) == last_discarded_bit_mask);

      bool round_bit = round_results<T>(is_negative, (mantiss_ret & last_bit_mask), round_up, tied, rt) && !!last_bit_mask;

      mantiss_ret >>= extra_bits;

      mantiss_ret += round_bit;

      mantiss_ret = safe_lshift(mantiss_ret, shift_amount);

      const T exponent_ret = clamped_sub(exp_a, shift_amount);

      mantiss_ret = safe_rshift(mantiss_ret, clamped_sub(shift_amount, exp_a));

      mantiss_ret &= mantissa_mask<T>();

      return (is_negative * sign_mask<T>()) | (exponent_ret << mantissa_size_bits()) | mantiss_ret;
    }

    /*! @warning Branchy as everything, and thus quite inefficient.
     *  The addition/subtraction routines themselves could be branchless,
     *  but we must select between them...
     *
     *  Probably the overhead from all the operations makes it worth it?
     *
     */
    template <class T>
    constexpr inline static T add(const T a, const T b, const RoundingModes rt = RoundingModes::Default)
    {
      const T abs_a = absolute_value<T>(a);
      const T abs_b = absolute_value<T>(b);

      const bool sign_a = a & sign_mask<T>();
      const bool sign_b = b & sign_mask<T>();

      if (abs_b == 0)
        {
          return a;
        }
      if (abs_a == 0)
        {
          return b;
        }

      if (is_infinite<T>(a) && is_infinite<T>(b))
        {
          if (sign_a == sign_b)
            {
              return a;
            }
          else
            {
              return abs_a | (T(1) << (mantissa_size_bits() - 1));
              //A "quiet" NaN in most platforms.
            }
        }
      else if (is_NaN<T>(a))
        {
          return a;
        }
      else if (is_NaN<T>(b))
        {
          return b;
        }

      if (sign_a == sign_b)
        {
          if (abs_a >= abs_b)
            {
              return add_patterns<T>(a, b, rt);
            }
          else
            {
              return add_patterns<T>(b, a, rt);
            }
        }
      else
        {
          if (abs_a > abs_b)
            {
              return (sign_a * sign_mask<T>()) | subtract_patterns<T>(abs_a, abs_b, rt);
            }
          else if (abs_a == abs_b)
            {
              return 0;
            }
          else
            {
              return (sign_b * sign_mask<T>()) | subtract_patterns<T>(abs_b, abs_a, rt);
            }
        }
    }


    template <class T>
    constexpr inline static T subtract(const T a, const T b, const RoundingModes rt = RoundingModes::Default)
    {
      return add<T>(a, b ^ sign_mask<T>(), rt);
    }

  };

  template <class FLarge, class FSmall>
  struct ConversionHelper
  {
    static_assert(FSmall::mantissa_size_bits() <= FLarge::mantissa_size_bits() &&
                  FSmall::exponent_size_bits() <= FLarge::exponent_size_bits()    );


    template <class T>
    constexpr inline static T down_convert(const T pattern, const RoundingModes rt = RoundingModes::Default)
    {
      using FDest = FSmall;
      using FSource = FLarge;
      using namespace OperatorsHelper;

      const bool sign_bit = pattern & FSource::template sign_mask<T>();

      const T exponent = (pattern & FSource::template exponent_mask<T>()) >> FSource::mantissa_size_bits();
      const T mantissa = pattern & FSource::template mantissa_mask<T>();

      constexpr T delta_exponents = FSource::template exponent_bias<T>() - FDest::template exponent_bias<T>();

      const bool exponent_full = (exponent > delta_exponents + FDest::template max_exponent_with_bias<T>());
      const bool delete_mantissa = exponent_full && exponent <= FSource::template max_exponent_with_bias<T>();
      //If the number is clamped to infinity, we must delete the mantissa
      //so we don't get a NaN!
      const bool denormal = exponent <= delta_exponents;
      const bool zero = exponent + FDest::mantissa_size_bits() <= delta_exponents;

      const T final_exponent = min(clamped_sub(exponent, delta_exponents), FDest::template max_exponent_with_bias<T>() + 1);

      const T extra_mantissa_shift = clamped_sub(delta_exponents + 1, exponent) * denormal;
      const T total_mantissa_shift = (FSource::mantissa_size_bits() - FDest::mantissa_size_bits()) + extra_mantissa_shift;

      const T mantissa_keep_mask = safe_lshift(FDest::template mantissa_mask<T>(), total_mantissa_shift) & FSource::template mantissa_mask<T>();
      const T check_for_rounding_mask = FSource::template mantissa_mask<T>() & (~mantissa_keep_mask);
      const T last_mantissa_bit = safe_lshift(FDest::template mantissa_mask<T>(), total_mantissa_shift) & FSource::template mantissa_mask<T>();
      const T first_discarded_bit_mask = last_mantissa_bit >> 1;
      //In case total_mantissa_shift == 0, this is 0 too.

      const T extra_denormal_bit = safe_rshift(T(denormal) << FDest::mantissa_size_bits(), extra_mantissa_shift);

      const bool round_up = (mantissa & check_for_rounding_mask) > first_discarded_bit_mask;
      const bool tie_break = ((mantissa & check_for_rounding_mask) == first_discarded_bit_mask);

      const bool round_bit = FDest::template round_results<T>(sign_bit, mantissa & last_mantissa_bit, round_up, tie_break, rt) && !(exponent_full && !delete_mantissa && !denormal);
      //The last part is so that NaN get truncated instead of rounded.

      T final_mantissa = (safe_rshift(mantissa, total_mantissa_shift) | extra_denormal_bit) + round_bit;

      final_mantissa *= !delete_mantissa;

      return sign_bit * FDest::template sign_mask<T>() | ((final_exponent << FDest::mantissa_size_bits()) | final_mantissa) * !zero;

    }

    template <class T>
    constexpr inline static T up_convert(const T pattern, [[maybe_unused]] const RoundingModes rt = RoundingModes::Default)
    {
      using FDest = FLarge;
      using FSource = FSmall;
      using namespace OperatorsHelper;

      const bool sign_bit = pattern & FSource::template sign_mask<T>();
      T exponent = (pattern & FSource::template exponent_mask<T>()) >> FSource::mantissa_size_bits();
      T mantissa = pattern & FSource::template mantissa_mask<T>();

      constexpr T delta_exponents = (FDest::template exponent_bias<T>() - FSource::template exponent_bias<T>());

      if (exponent == 0 && FDest::exponent_size_bits() > FSource::exponent_size_bits())
        {
          const unsigned int leading_zeros = LeadingZerosPortability::count_leading_zeros<T>(mantissa);
          const unsigned int shift_amount = leading_zeros - (sizeof(T) * CHAR_BIT - FSource::mantissa_size_bits()) + 1;
          const unsigned int exponent_offset = (mantissa != 0) * (shift_amount - 1);
          mantissa = (mantissa << shift_amount) & FSource::template mantissa_mask<T>();

          exponent = delta_exponents - exponent_offset;
        }
      else if (exponent > FSource::template max_exponent_with_bias<T>())
        //Infinity or NaN
        {
          exponent = FDest::template max_exponent_with_bias<T>() + 1;
        }
      else
        {
          exponent = exponent + delta_exponents;
        }


      return sign_bit * FDest::template sign_mask<T>() | (exponent << FDest::mantissa_size_bits()) | (mantissa << (FDest::mantissa_size_bits() - FSource::mantissa_size_bits()));

    }
  };

  /*!
      Converts @p pattern from the larger floating point format @p FSource to @p FDest.

      Note: A few tests show that, under some circumstances, compilers may make this branchless.
            Clang seemed to do it, but not GCC. NVCC (for the GPU) surely did.
            There's probably still room for improvement, but...
  */
  template <class T, class FDest, class FSource>
  constexpr inline static T down_convert(const T pattern, const RoundingModes rt = RoundingModes::Default)
  {
    static_assert(FDest::mantissa_size_bits() <= FSource::mantissa_size_bits() &&
                  FDest::exponent_size_bits() <= FSource::exponent_size_bits(),
                  "The destination type must not be a larger floating point type than the source one.");

    return ConversionHelper<FSource, FDest>::template down_convert<T>(pattern, rt);
  }


  /*!
      Converts @p pattern from the smaller floating point format @p FSource to @p FDest.

      @warning: This could be re-written in a more branchless way, so performance might be sub-optimal...
  */
  template <class T, class FDest, class FSource>
  constexpr inline static T up_convert(const T pattern, const RoundingModes rt = RoundingModes::Default)
  {
    static_assert(FDest::mantissa_size_bits() >= FSource::mantissa_size_bits() &&
                  FDest::exponent_size_bits() >= FSource::exponent_size_bits(),
                  "The source type must not be a larger floating point type than the destination one.");
    return ConversionHelper<FDest, FSource>::template up_convert<T>(pattern, rt);
  }


  using StandardFloat = IEEE754_like<23, 8>;

  using StandardDouble = IEEE754_like<52, 11>;

  template <> template<class T>
  constexpr inline T StandardFloat::add (const T a, const T b, const RoundingModes)
  {
    const float float_a = BitCastHelper::bitcast<float, T>(a);
    const float float_b = BitCastHelper::bitcast<float, T>(b);

    float float_ret = float_a + float_b;

    return BitCastHelper::bitcast<uint32_t, float>(float_ret);
  }

  template <> template<class T>
  constexpr inline T StandardDouble::add (const T a, const T b, const RoundingModes)
  {
    const double double_a = BitCastHelper::bitcast<double, T>(a);
    const double double_b = BitCastHelper::bitcast<double, T>(b);

    double double_ret = double_a + double_b;

    return BitCastHelper::bitcast<uint64_t, double>(double_ret);
  }

  template <> template<class T>
  constexpr inline T StandardFloat::subtract (const T a, const T b, const RoundingModes)
  {
    const float float_a = BitCastHelper::bitcast<float, T>(a);
    const float float_b = BitCastHelper::bitcast<float, T>(b);

    float float_ret = float_a - float_b;

    return BitCastHelper::bitcast<uint32_t, float>(float_ret);
  }

  template <> template<class T>
  constexpr inline T StandardDouble::subtract (const T a, const T b, const RoundingModes)
  {
    const double double_a = BitCastHelper::bitcast<double, T>(a);
    const double double_b = BitCastHelper::bitcast<double, T>(b);

    double double_ret = double_a - double_b;

    return BitCastHelper::bitcast<uint64_t, double>(double_ret);
  }

  template<>
  struct ConversionHelper<StandardDouble, StandardFloat>
  {
    template <class T>
    constexpr inline static T up_convert(const T pattern, [[maybe_unused]] const RoundingModes rt = RoundingModes::Default)
    {
      const float f = BitCastHelper::bitcast<float, uint32_t>(pattern);
      const double d = f;
      return BitCastHelper::bitcast<T, double>(d);
    }
    template <class T>
    constexpr inline static T down_convert(const T pattern, [[maybe_unused]] const RoundingModes rt = RoundingModes::Default)
    {
      const double d = BitCastHelper::bitcast<double, uint64_t>(pattern);
      const float f = d;
      return BitCastHelper::bitcast<uint32_t, float>(f);
    }
  };


  template<class Format>
  struct ConversionHelper<Format, Format>
  {
    template <class T>
    constexpr inline static T up_convert(const T pattern, [[maybe_unused]] const RoundingModes rt = RoundingModes::Default)
    {
      return pattern;
    }
    template <class T>
    constexpr inline static T down_convert(const T pattern, [[maybe_unused]] const RoundingModes rt = RoundingModes::Default)
    {
      return pattern;
    }
  };

  using CUDAHalfFloat = IEEE754_like<10, 5>;
  using CUDABFloat16 = IEEE754_like<7, 8>;

#if defined (__CUDA_ARCH__) && CALORECGPU_INCLUDE_CUDA_SUPPORT
  
  //If not, the CUDA-related ones will just default back to the slower, emulated operations.


  template <> template<class T>
  __device__ constexpr inline T CUDAHalfFloat::add (const T a, const T b, const RoundingModes)
  {
    const __half conv_a = BitCastHelper::bitcast<__half, T>(a);
    const __half conv_b = BitCastHelper::bitcast<__half, T>(b);

    __half conv_ret = __hadd(conv_a, conv_b);

    return BitCastHelper::bitcast<uint16_t, __half>(conv_ret);
  }

  template <> template<class T>
  __device__ constexpr inline T CUDAHalfFloat::subtract (const T a, const T b, const RoundingModes)
  {
    const __nv_bfloat16 conv_a = BitCastHelper::bitcast<__half, T>(a);
    const __nv_bfloat16 conv_b = BitCastHelper::bitcast<__half, T>(b);

    __half conv_ret = __hsub(conv_a, conv_b);

    return BitCastHelper::bitcast<uint16_t, __half>(conv_ret);
  }

  template <> template<class T>
  __device__ constexpr inline T CUDABFloat16::add (const T a, const T b, const RoundingModes)
  {
    const __nv_bfloat16 conv_a = BitCastHelper::bitcast<__nv_bfloat16, T>(a);
    const __nv_bfloat16 conv_b = BitCastHelper::bitcast<__nv_bfloat16, T>(b);

    __nv_bfloat16 conv_ret = __hadd(conv_a, conv_b);

    return BitCastHelper::bitcast<uint16_t, __nv_bfloat16>(conv_ret);
  }

  template <> template<class T>
  __device__ constexpr inline T CUDABFloat16::subtract (const T a, const T b, const RoundingModes)
  {
    const __nv_bfloat16 conv_a = BitCastHelper::bitcast<__nv_bfloat16, T>(a);
    const __nv_bfloat16 conv_b = BitCastHelper::bitcast<__nv_bfloat16, T>(b);

    __nv_bfloat16 conv_ret = __hsub(conv_a, conv_b);

    return BitCastHelper::bitcast<uint16_t, __nv_bfloat16>(conv_ret);
  }

  template<>
  struct ConversionHelper<StandardFloat, CUDAHalfFloat>
  {
    template <class T>
    constexpr inline static T up_convert(const T pattern, const RoundingModes rt = RoundingModes::Default)
    {
      const __half hf = BitCastHelper::bitcast<__half, T>(pattern);
      const float fl = __half2float(hf);
      return BitCastHelper::bitcast<T, float>(fl);
    }
    template <class T>
    constexpr inline static T down_convert(const T pattern, const RoundingModes rt = RoundingModes::Default)
    {
      const float pre_conv = BitCastHelper::bitcast<float, T>(pattern);
      __half ret;
      switch (rt)
        {
          case RoundingModes::ToPlusInfinity:
            ret = __float2half_ru(pre_conv);
            break;
          case RoundingModes::ToMinusInfinity:
            ret = __float2half_rd(pre_conv);
            break;
          case RoundingModes::ToZero:
            ret = __float2half_rz(pre_conv);
            break;
          case RoundingModes::ToNearestEven:
            ret = __float2half_rn(pre_conv);
            break;
          //case RoundingModes::ToNearestAwayFromZero:
          //No support for this
          default:
            ret = __float2half(pre_conv);
            break;
        }
      return BitCastHelper::bitcast<T, __half>(ret);
    }
  };


  template<>
  struct ConversionHelper<StandardFloat, CUDABFloat16>
  {
    template <class T>
    constexpr inline static T up_convert(const T pattern, const RoundingModes rt = RoundingModes::Default)
    {
      const __nv_bfloat16  hf = BitCastHelper::bitcast<__nv_bfloat16, T>(pattern);
      const float fl = __bfloat162float(hf);
      return BitCastHelper::bitcast<T, float>(fl);
    }
    template <class T>
    constexpr inline static T down_convert(const T pattern, const RoundingModes rt = RoundingModes::Default)
    {
      const float pre_conv = BitCastHelper::bitcast<float, T>(pattern);
      __nv_bfloat16 ret;
      switch (rt)
        {
          case RoundingModes::ToPlusInfinity:
            ret = __float2bfloat16_ru(pre_conv);
            break;
          case RoundingModes::ToMinusInfinity:
            ret = __float2bfloat16_rd(pre_conv);
            break;
          case RoundingModes::ToZero:
            ret = __float2bfloat16_rz(pre_conv);
            break;
          case RoundingModes::ToNearestEven:
            ret = __float2bfloat16_rn(pre_conv);
            break;
          //case RoundingModes::ToNearestAwayFromZero:
          //No support for this
          default:
            ret = __float2bfloat16(pre_conv)
                  break;
        }
      return BitCastHelper::bitcast<T, __nv_bfloat16>(ret);
    }
  };

  template<>
  struct ConversionHelper<StandardDouble, CUDAHalfFloat>
  {
    template <class T>
    constexpr inline static T up_convert(const T pattern, const RoundingModes rt = RoundingModes::Default)
    {
      const T first_step = ConversionHelper<StandardFloat, CUDAHalfFloat>::template up_convert<T>(pattern, rt);
      return ConversionHelper<StandardDouble, StandardFloat>::template up_convert<T>(first_step, rt);
    }
    template <class T>
    constexpr inline static T down_convert(const T pattern, const RoundingModes rt = RoundingModes::Default)
    {
      if (rt == RoundingModes::Default || rt == RoundingModes::ToNearestEven)
        {
          const double d = BitCastHelper::bitcast<double, uint64_t>(pattern);
          return BitCastHelper::bitcast<T, __half>(__double2half(d));
        }
      else
        {
          const T first_step = ConversionHelper<StandardDouble, StandardFloat>::template down_convert<T>(first_step, rt);
          return ConversionHelper<StandardFloat, CUDAHalfFloat>::template down_convert<T>(first_step, rt);
        }
    }
  };

  template<>
  struct ConversionHelper<StandardDouble, CUDABFloat16>
  {
    template <class T>
    constexpr inline static T up_convert(const T pattern, const RoundingModes rt = RoundingModes::Default)
    {
      const T first_step = ConversionHelper<StandardFloat, CUDABFloat16>::template up_convert<T>(first_step, rt);
      return ConversionHelper<StandardDouble, StandardFloat>::template up_convert<T>(first_step, rt);
    }
    template <class T>
    constexpr inline static T down_convert(const T pattern, const RoundingModes rt = RoundingModes::Default)
    {
      if (rt == RoundingModes::Default || rt == RoundingModes::ToNearestEven)
        {
          const double d = BitCastHelper::bitcast<double, uint64_t>(pattern);
          return BitCastHelper::bitcast<T, __nv_bfloat16>(__double2bfloat16(d));
        }
      else
        {
          const T first_step = ConversionHelper<StandardDouble, StandardFloat>::template down_convert<T>(first_step, rt);
          return ConversionHelper<StandardFloat, CUDABFloat16>::template down_convert<T>(first_step, rt);
        }
    }
  };

#endif

}


#endif
