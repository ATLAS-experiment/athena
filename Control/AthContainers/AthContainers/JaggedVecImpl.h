// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthContainers/JaggedVecImpl.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2024
 * @brief Definition of JaggedVecElt.
 *
 * This file defines the @c JaggedVecElt type used to store the indices
 * for one jagged vector element.  It is broken out from JaggedVec.h
 * due to dependency issues.
 */


#ifndef ATHCONTAINERS_JAGGEDVECIMPL_H
#define ATHCONTAINERS_JAGGEDVECIMPL_H


#include <cstddef>
#include <cstdint>


namespace SG {


/**
 * @brief Describe one element of a jagged vector (base class).
 *
 * Each jagged vector element holds its end index into the linked payload
 * vector.  The begin index is given by the end index of the previous element.
 * Thus, these elements must be allocated as a contiguous vector.  Further,
 * in order to get the begin index, we need to know the index of this
 * element object within its vector --- because we have to special-case
 * the first element.
 *
 * For example, if we have a jagged vector with three elements of sizes 3, 1, 2,
 * then what we would store is {3, 4, 6}.
 *
 * This implies that one cannot interpret a @c JaggedVecElt in isolation
 * (or a range of them); we need to know where they are stored in the
 * xAOD variable vector.  Fortunately, that's not something that is typically
 * done, since jagged vectors are accessed via the specialized @c Accessor
 * classes.
 *
 * This non-templated base class holds the actual data.  However,
 * users should use the @c JaggedVecElt<PAYLOAD> derived types, to allow
 * specifying the payload type.
 */
class JaggedVecEltBase
{
public:
  /// Type for the indices.  16 bits is probably too small, 64 bits
  /// is pretty definitely too large.  Use 32.
  using index_type = uint32_t;


  /**
   * @brief Default constructor.  Makes a null range.
   */
  JaggedVecEltBase () = default;


  /**
   * @brief Constructor.
   * @param beg Index of the start of the range.
   * @param end Index of the end of the range.
   */
  JaggedVecEltBase (index_type end);


  /**
   * @brief Return the index of the beginning of the range.
   * @param elt_ndx The index of this element in its container.
   */
  index_type begin (size_t elt_ndx) const;


  /**
   * @brief Return the index of the end of the range.
   */
  index_type end() const;


  /**
   * @brief Return the number of items in this range.
   * @param elt_ndx The index of this element in its container.
   */
  size_t size (size_t elt_ndx) const;


  /**
   * @brief Equality test.
   * @param other Other element with which to compare.
   */
  bool operator== (const JaggedVecEltBase& other) const;


  /**
   * @brief Helper to shift indices.
   */
  struct Shift
  {
    /// Constructor.
    Shift (int offs);

    /// Shift indices in @c e by the offset given to the constructor.
    void operator() (JaggedVecEltBase& e) const;


  private:
    /// Offset by which to shift.
    int m_offs;
  };


private:
  friend struct Shift;

  /// End index.
  index_type m_end = 0;
};


/**
 * @brief Describe one element of a jagged vector.
 */
template <class T>
class JaggedVecElt
  : public JaggedVecEltBase
{
public:
  using JaggedVecEltBase::JaggedVecEltBase;
};


} // namespace SG


#include "AthContainers/JaggedVecImpl.icc"


#endif // not ATHCONTAINERS_JAGGEDVECIMPL_H
