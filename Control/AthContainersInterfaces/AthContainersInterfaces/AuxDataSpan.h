// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainersInterfaces/AuxDataSpan.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jun, 2024
 * @brief Describe a range over an auxiliary variable, for low-overhead access.
 *
 * Auxiliary variables are maintained by objects implementing @c IAuxTypeVector.
 * This provides virtual functions to obtain the start and size of the vector.
 * However, in some cases, we would like to be able to obtain this without
 * the overhead of the virtual function calls.  We do this using the
 * minimal span-like object @c AuxDataSpanBase.  A @c IAuxTypeVector
 * holds an instance of this object and provides a non-virtual function
 * to return a reference to it.  Further, it is updated when the vector
 * changes.  Thus, clients can hold a reference to it and implicitly
 * see any updates.  Besides this minimal object, a couple more
 * user-friendly wrappers are also defined.
 */


#ifndef ATHCONTAINERSINTERFACES_AUXDATASPAN_H
#define ATHCONTAINERSINTERFACES_AUXDATASPAN_H


#include "CxxUtils/throw_out_of_range.h"
#include <cstddef>
#include <stdexcept>


namespace SG {


/**
 * @brief Minimal span-like object describing the range of an auxiliary variable.
 */
struct AuxDataSpanBase
{
  /**
   * @brief Constructor.
   * @param the_beg The start of the variable's vector.
   * @param the_size The length of the variable's vector.
   */
  // cppcheck-suppress uninitMemberVar; false positive
  AuxDataSpanBase (void* the_beg = 0, size_t the_size = 0)
    : beg(the_beg), size(the_size)
  {
  }


  /// Pointer to the start of the variable's vector.
  void* beg;

  /// The length of the variable's vector.
  size_t size;
};


namespace detail {


/**
 * @brief Auxiliary variable span wrapper.
 *
 * This wraps an @c AuxDataSpanBase for more user-friendly access.
 * It holds the @c AuxDataSpanBase by reference, so it will implicity see
 * any changes.
 */
template <class T>
class AuxDataSpan
{
public:
  /**
   * @brief Constructor.
   * @param span The base object to wrap.
   */
  AuxDataSpan (const AuxDataSpanBase& span) : m_span (span) {}


  /**
   * @brief Return the size of the span.
   */
  size_t size() const { return m_span.size; }


  /**
   * @brief Test to see if the span is empty.
   */
  bool empty() const { return m_span.size == 0; }


  /**
   * @brief Return the start of the span.
   */
  T* data() { return reinterpret_cast<T*> (m_span.beg); }


  /**
   * @brief Return the start of the span (const).
   */
  const T* data() const { return reinterpret_cast<T*> (m_span.beg); }


  /**
   * @brief Element access.
   * @brief index The element index to access.
   */
  T& operator[] (size_t index)
  {
    return data()[index];
  }


  /**
   * @brief Element access (const).
   * @brief index The element index to access.
   */
  const T& operator[] (size_t index) const
  {
    return data()[index];
  }


  /**
   * @brief Bounds-checked element access.
   * @brief index The element index to access.
   */
  T& at (size_t index)
  {
    if (index >= m_span.size) {
      CxxUtils::throw_out_of_range (__PRETTY_FUNCTION__, index, m_span.size, this);
    }
    return data()[index];
  }


  /**
   * @brief Bounds-checked element access (const).
   * @brief index The element index to access.
   */
  const T& at (size_t index) const
  {
    if (index >= m_span.size) {
      CxxUtils::throw_out_of_range (__PRETTY_FUNCTION__, index, m_span.size, this);
    }
    return data()[index];
  }


  /**
   * @brief Return the first element in the range.
   */
  T& front() { return data()[0]; }


  /**
   * @brief Return the first element in the range (const).
   */
  const T& front() const { return data()[0]; }


  /**
   * @brief Return the last element in the range.
   */
  T& back() { return data()[m_span.size-1]; }


  /**
   * @brief Return the last element in the range (const).
   */
  const T& back() const { return data()[m_span.size-1]; }


private:
  /// The wrapped object.
  const AuxDataSpanBase& m_span;
};


/**
 * @brief Auxiliary variable span wrapper (const).
 *
 * This wraps an @c AuxDataSpanBase for more user-friendly access.
 * It holds the @c AuxDataSpanBase by reference, so it will implicity see
 * any changes.
 */
template <class T>
class AuxDataConstSpan
{
public:
  /**
   * @brief Constructor.
   * @param span The base object to wrap.
   */
  AuxDataConstSpan (const AuxDataSpanBase& span)
    : m_span (span) {}


  /**
   * @brief Return the size of the span.
   */
  size_t size() const { return m_span.size; }


  /**
   * @brief Test to see if the span is empty.
   */
  bool empty() const { return m_span.size == 0; }


  /**
   * @brief Return the start of the span.
   */
  const T* data() const { return reinterpret_cast<const T*> (m_span.beg); }


  /**
   * @brief Element access.
   * @brief index The element index to access.
   */
  const T& operator[] (size_t index) const
  {
    return data()[index];
  }


  /**
   * @brief Bounds-checked element access.
   * @brief index The element index to access.
   */
  const T& at (size_t index) const
  {
    if (index >= m_span.size) {
      CxxUtils::throw_out_of_range (__PRETTY_FUNCTION__, index, m_span.size, this);
    }
    return data()[index];
  }


  /**
   * @brief Return the first element in the range.
   */
  const T& front() const { return data()[0]; }


  /**
   * @brief Return the last element in the range.
   */
  const T& back() const { return data()[m_span.size-1]; }


private:
  /// The wrapped object.
  const AuxDataSpanBase& m_span;
};


} } // namespace SG::detail


#endif // not ATHCONTAINERSINTERFACES_AUXDATASPAN_H
