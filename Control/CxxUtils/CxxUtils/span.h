// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/span.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2022
 * @brief Simplified version of the C++20 std::span.
 */


#ifndef CXXUTILS_SPAN_H
#define CXXUTILS_SPAN_H


#include "CxxUtils/concepts.h"
#include <cstdlib>
#include <type_traits>
#include <iterator>
#include <cassert>
#include <ranges>


namespace CxxUtils {


/// Used to specify a subrange of indefinite size in subspan().
inline constexpr size_t dynamic_extent = static_cast<size_t>(-1);


/// Is U* a valid type to use to initialize a span<T>?
/// No more than const-conversion.
/// Same logic as used in libstdc++.
template <class T, class U>
inline constexpr bool valid_span_type_v = std::is_convertible_v<U(*)[], T(*)[]>;


/**
 * @brief Simplified version of the C++20 std::span.
 *
 * This class is meant to be very similar to the C++20 std::span class.
 * There are however a few differences.
 *  - The option for a fixed-length span is not implemented; hence, there
 *    is no second template argument.
 *  - Constructors from arrays / std::array are not implemented.
 *  - The external member functions as_bytes / as_writable_bytes and
 *    not all deduction guides are implemented.
 *
 * Contact the core sw group if any of these are a problem.
 *
 * In addition:
 *  - Accessor methods are split into const / non-const versions.
 *    This is required to avoid warnings from the thread-safety checker,
 *    but be careful of potential incompatibilies with std::span.
 *    Again, contact the core sw group if this is an issue.
 */
template <class T>
class span
  : public std::ranges::view_base
{
public:
  /// Required typedefs.
  using element_type           = T;
  using value_type             = std::remove_cv_t<T>;
  using size_type              = std::size_t;
  using difference_type        = std::ptrdiff_t;
  using pointer                = T*;
  using const_pointer          = const T*;
  using reference              = element_type&;
  using const_reference        = const element_type&;
  using iterator               = pointer;
  using const_iterator         = const_pointer;
  using reverse_iterator       = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;


  /*
   * @brief Default constructor.
   * Makes an empty span.
   */
  span();


  /**
   * @brief Constructor from start and length.
   * @param ptr Start of the span.
   * @param sz Length of the span.
   */
  template <class U>
  requires (valid_span_type_v<T, U>)
  span (U* ptr, size_type sz);


  /**
   * @brief Constructor from start and end.
   * @param beg Start of the span.
   * @param end One past the end of the span.
   */
  template <class U>
  requires (valid_span_type_v<T, U>)
  span (U* beg, U* end);


  // Default copy / assignment.
  span (const span&) = default;
  span& operator= (const span&) = default;


  /**
   * @brief Constructor from another span.
   * @param other Span to copy from.
   */
  template <class U>
  requires (valid_span_type_v<T, U>)
  span (const span<U>& other);


  /**
   * @brief Return the size of the span.
   */
  constexpr size_type size() const noexcept;

  
  /**
   * @brief Return the size of contents of the span, in bytes.
   */
  constexpr size_type size_bytes() const noexcept;

  
  /**
   * @brief Test if the span is empty.
   */
  [[nodiscard]] constexpr bool empty() const noexcept;

  
  /**
   * @brief Return a reference to the first element in the span.
   */
  constexpr reference front() noexcept;

   
  /**
   * @brief Return a reference to the first element in the span.
   */
  constexpr const_reference front() const noexcept;

  
  /**
   * @brief Return a reference to the last element in the span.
   */
  constexpr reference back() noexcept;

  
  /**
   * @brief Return a reference to the last element in the span.
   */
  constexpr const_reference back() const noexcept;

  
  /**
   * @brief Return a reference to the i-th element in the span.
   * @param i Index of the element to return.
   */
  constexpr reference operator[] (size_type i) noexcept;

  
  /**
   * @brief Return a reference to the i-th element in the span.
   * @param i Index of the element to return.
   */
  constexpr const_reference operator[] (size_type i) const noexcept;

  
  /**
   * @brief Return a reference to the i-th element in the span (bounds-checked)./
   * @param i Index of the element to return.
   */
  constexpr reference at (size_type i);


  /**
   * @brief Return a reference to the i-th element in the span (bounds-checked)./
   * @param i Index of the element to return.
   */
  constexpr const_reference at (size_type i) const;


  /**
   * @brief Return a pointer to the start of the span.
   */
  constexpr pointer data() noexcept;

  
  /**
   * @brief Return a pointer to the start of the span.
   */
  constexpr const_pointer data() const noexcept;

  
  /**
   * @brief Return a begin iterator.
   */
  constexpr iterator begin() noexcept;

  
  /**
   * @brief Return a begin iterator.
   */
  constexpr const_iterator begin() const noexcept;

  
  /**
   * @brief Return an end iterator.
   */ 
  constexpr iterator end() noexcept;

  
  /**
   * @brief Return an end iterator.
   */ 
  constexpr const_iterator end() const noexcept;

  
  /**
   * @brief Return a begin reverse iterator.
   */
  constexpr reverse_iterator rbegin() noexcept;

  
  /**
   * @brief Return a begin reverse iterator.
   */
  constexpr const_reverse_iterator rbegin() const noexcept;

  
  /**
   * @brief Return an end reverse iterator.
   */
  constexpr reverse_iterator rend() noexcept;


  /**
   * @brief Return an end reverse iterator.
   */
  constexpr const_reverse_iterator rend() const noexcept;


  /**
   * @brief Return a subspan from the start.
   * @param n Number of elements in the subspan.
   */
  constexpr span first (size_type n) noexcept;

  
  /**
   * @brief Return a subspan from the start.
   * @param n Number of elements in the subspan.
   */
  constexpr span<const T> first (size_type n) const noexcept;

  
  /**
   * @brief Return a subspan from the end.
   * @param n Number of elements in the subspan.
   */
  constexpr span last (size_type n) noexcept;

  
  /**
   * @brief Return a subspan from the end.
   * @param n Number of elements in the subspan.
   */
  constexpr span<const T> last (size_type n) const noexcept;

  
  /**
   * @brief Return a subspan.
   * @param offs Starting element of the subspan.
   * @param n Number of elements in the subspan.
   *          If defaulted, take all remaining elements.
   */
  constexpr span
  subspan (size_type offs, size_type n = dynamic_extent) noexcept;


  /**
   * @brief Return a subspan.
   * @param offs Starting element of the subspan.
   * @param n Number of elements in the subspan.
   *          If defaulted, take all remaining elements.
   */
  constexpr span<const T>
  subspan (size_type offs, size_type n = dynamic_extent) const noexcept;


private:
  /// Pointer to the start of the span.
  T* m_ptr;

  /// Number of elements in the span.
  size_t m_size;
};


/// A couple needed deduction guides.
template <class T>
span (T* ptr, std::size_t sz) -> span<T>;
template <class T>
span (T* beg, T* end) -> span<T>;


/**
 * @brief Helper to make a span from a container.
 * @param c The container for which to make a span.
 *          It must have contiguous iterators.
 */
template <detail::IsContiguousContainer CONTAINER>
auto make_span (CONTAINER& c)
{
  return CxxUtils::span (c.data(), c.size());
}


// Stand-alone begin/end functions, findable by ADL.
// Needed to work around issues seen when root 6.30.04 is used with gcc14.
template <class T>
auto begin (span<T>& s) { return s.begin(); }
template <class T>
auto begin (const span<T>& s) { return s.begin(); }
template <class T>
auto end (span<T>& s) { return s.end(); }
template <class T>
auto end (const span<T>& s) { return s.end(); }


} // namespace CxxUtils


#include "CxxUtils/span.icc"


#endif // not CXXUTILS_SPAN_H
