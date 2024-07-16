// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/*
 */
/**
 * @file CxxUtils/bitmask.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2018
 * @brief Helpers for treating a class enum as a bitmask.
 *
 * C++11 class enums are very nice from a type-safety viewpoint.
 * However, they are a bit awkward if you want to use them to represent
 * a bitmask.  For example:
 *@code
   enum class Mask { Bit0 = 1, Bit1 = 2, Bit2 = 4 };
   Mask m = Mask::Bit1 | Mask::Bit2;
 @endcode
 * doesn't compile because the enumerators are not convertible to integers.
 * One can cast back and forth between the enum and integers, but that's
 * awkward to do everywhere.
 *
 * This header defines overloads for the bitwise operators that work with
 * a class enum.  To enable these overloads, you need to add the @c ATH_BITMASK
 * macro at the end of your enum declaration:
 *
 *@code
   enum class Mask { Bit0 = 1, Bit1 = 2, Bit2 = 4,
                     ATH_BITMASK };
 @endcode
 *
 * After that, the usual bitwise operations (&, |, ^, &=, |=, ^=, ~) should
 * work as expected.  There are also a few convenience functions defined
 * in the @c CxxUtils namespace: @c set, @c reset, and @c test.
 *
 * In case of two different enum types (with common underlying type) the (non-assignment)
 * boolean operators (&, |, ^) are also defined:
 *
 *@code
   int result = Mask::Bit1 & OtherMask::Bit2
 @endcode
 *
 * This approach was suggested by these postings:
 *
 * <http://blog.bitwigglers.org/using-enum-classes-as-type-safe-bitmasks>
 * <https://www.justsoftwaresolutions.co.uk/cplusplus/using-enum-classes-as-bitfields.html>
 *
 * except that we rely on injecting a known enumerator into the type rather
 * than using a separate traits class.  This way works better if the enumeration
 * is defined in a nested scope.
 */


#include <type_traits>


#ifndef CXXUTILS_BITMASK_H
#define CXXUTILS_BITMASK_H


/**
 * @brief Mark that a class enum should be treated as a bitmask.
 *        Put this at the end of the enumeration, after a comma, like this:
 *@code
   enum class Mask { Bit0 = 1, Bit1 = 2, Bit2 = 4,
                     ATH_BITMASK };
 @endcode
 */
#define ATH_BITMASK IS_ATH_BITMASK=1


/// Internal helpers
namespace {

  /// SFINAE friendly underlying_type that also works on other types
  template <typename T, bool = std::is_enum_v<T>>
  struct relaxed_underlying_type {
    using type = std::underlying_type_t<T>;
  };
  template <typename T>
  struct relaxed_underlying_type<T, false> {
    using type = T;
  };
  template <class T>
  using relaxed_underlying_type_t = typename relaxed_underlying_type<T>::type;

  /// Check if E and F have same underlying type
  template <class E, class F>
  constexpr bool has_same_underlying_v = std::is_same_v<relaxed_underlying_type_t<E>,
                                                        relaxed_underlying_type_t<F>>;

  /// Common (underlying) type for enum classes E and F
  template <class E, class F>
  struct enum_or_underlying {
    using type = std::conditional_t<std::is_same_v<E,F> && has_same_underlying_v<E,F>,
                                    E, relaxed_underlying_type_t<E>>;
  };
  template <class E, class F>
  using enum_or_underlying_t = typename enum_or_underlying<E,F>::type;

  /// Check if enum is an ATH_BITMASK
  template <class E, typename Enable = void>
  constexpr bool is_bitmask_v = false;

#ifdef __CUDACC__
# pragma nv_diagnostic push
// expression has no effect
# pragma nv_diag_suppress 174
#endif
  template <class E>
  constexpr bool is_bitmask_v<E, std::enable_if_t<(E::IS_ATH_BITMASK,1)>> = true;
#ifdef __CUDACC__
# pragma nv_diagnostic pop
#endif
}


/*
 * Define bitwise operators for class enums.
 * These all cast the operands to the underlying type, and then cast
 * the result back again to the enumeration.
 *
 * These functions are enabled only for enumerators that have used the
 * @c ATH_BITMASK macro.
 */

/// operator~
template <class E>
constexpr
std::enable_if_t<is_bitmask_v<E>, E>
operator~ (E lhs)
{
  typedef std::underlying_type_t<E> underlying;
  return static_cast<E> (~static_cast<underlying>(lhs));
}


/// operator&
///
/// One operand needs to be a bitmask and the other share at least the same
/// underlying type. This allows bit operations with the underlying type (e.g. int)
/// and chained operations involving more than two bitmasks.
template <class E, class F,
          typename = std::enable_if_t<(is_bitmask_v<E> || is_bitmask_v<F>) &&
                                      has_same_underlying_v<E,F>>>
constexpr auto operator& (E lhs, F rhs)
{
  typedef relaxed_underlying_type_t<E> underlying;
  return static_cast<enum_or_underlying_t<E,F>>(static_cast<underlying>(lhs) &
                                                static_cast<underlying>(rhs));
}


/// operator|
/// @copydetails operator&
template <class E, class F,
          typename = std::enable_if_t<(is_bitmask_v<E> || is_bitmask_v<F>) &&
                                      has_same_underlying_v<E,F>>>
constexpr auto operator| (E lhs, F rhs)
{
  typedef relaxed_underlying_type_t<E> underlying;
  return static_cast<enum_or_underlying_t<E,F>>(static_cast<underlying>(lhs) |
                                                static_cast<underlying>(rhs));
}


/// operator^
/// @copydetails operator&
template <class E, class F,
          typename = std::enable_if_t<(is_bitmask_v<E> || is_bitmask_v<F>) &&
                                      has_same_underlying_v<E,F>>>
constexpr auto operator^ (E lhs, F rhs)
{
  typedef relaxed_underlying_type_t<E> underlying;
  return static_cast<enum_or_underlying_t<E,F>>(static_cast<underlying>(lhs) ^
                                                static_cast<underlying>(rhs));
}


/// operator&=
template <class E>
constexpr
std::enable_if_t<is_bitmask_v<E>, E&>
operator&= (E& lhs, E rhs)
{
  typedef std::underlying_type_t<E> underlying;
  lhs = static_cast<E> (static_cast<underlying>(lhs) & static_cast<underlying>(rhs));
  return lhs;
}


/// operator|=
template <class E>
constexpr
std::enable_if_t<is_bitmask_v<E>, E&>
operator|= (E& lhs, E rhs)
{
  typedef std::underlying_type_t<E> underlying;
  lhs = static_cast<E> (static_cast<underlying>(lhs) | static_cast<underlying>(rhs));
  return lhs;
}


/// operator^=
template <class E>
constexpr
std::enable_if_t<is_bitmask_v<E>, E&>
operator^= (E& lhs, E rhs)
{
  typedef std::underlying_type_t<E> underlying;
  lhs = static_cast<E> (static_cast<underlying>(lhs) ^ static_cast<underlying>(rhs));
  return lhs;
}


namespace CxxUtils {


/**
 * @brief Convenience function to set bits in a class enum bitmask.
 * @param lhs Target bitmask.
 * @param rhs Bits to set.
 *
 * Same as lhs |= rhs.  
 * This function is enabled only for enumerators that have used the
 * @c ATH_BITMASK macro.
 */
template <class E>
constexpr
std::enable_if_t<is_bitmask_v<E>, E&>
set (E& lhs, E rhs)
{
  lhs |= rhs;
  return lhs;
}


/**
 * @brief Convenience function to clear bits in a class enum bitmask.
 * @param lhs Target bitmask.
 * @param rhs Bits to clear.
 *
 * Same as lhs &= ~rhs.  
 * This function is enabled only for enumerators that have used the
 * @c ATH_BITMASK macro.
 */
template <class E>
constexpr
std::enable_if_t<is_bitmask_v<E>, E&>
reset (E& lhs, E rhs)
{
  lhs &= ~rhs;
  return lhs;
}


/**
 * @brief Convenience function to test bits in a class enum bitmask.
 * @param lhs Target bitmask.
 * @param rhs Bits to test.
 *
 * Same as (lhs & rhs) != 0.
 * This function is enabled only for enumerators that have used the
 * @c ATH_BITMASK macro.
 */
template <class E>
constexpr
std::enable_if_t<is_bitmask_v<E>, bool>
test (E lhs, E rhs)
{
  typedef std::underlying_type_t<E>underlying;
  return static_cast<underlying> (lhs & rhs) != 0;
}


} // namespace CxxUtils


#endif // not CXXUTILS_BITMASK_H
