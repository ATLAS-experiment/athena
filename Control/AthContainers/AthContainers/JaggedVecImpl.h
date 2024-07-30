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


#include <cstdint>


namespace SG {


/**
 * @brief Describe one element of a jagged vector (base class).
 *
 * Each jagged vector element consists of begin and end indices
 * into the linked payload vector.
 *
 * Using two indices per element is redundant, but we need to do that in order
 * to be consistent with the basic xAOD requirements: a variable is stored
 * as a contiguous array of objects, in one-to-one correspondence
 * with the elements of the @c DataVector.
 *
 * This non-templated base class holds the actual packed value.  However,
 * users should use the @c PackedLink<STORABLE> derived types, to allow
 * specifying the target of the links.
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
  JaggedVecEltBase (index_type beg, index_type end);


  /**
   * @brief Return the index of the start of the range.
   */
  index_type begin() const;


  /**
   * @brief Return the index of the end of the range.
   */
  index_type end() const;


  /**
   * @brief Return the number of payload items in this jagged vector element.
   */
  index_type size() const;


  /**
   * @brief Equality test.
   * @param other Other element with which to compare.
   */
  bool operator== (const JaggedVecEltBase& other) const;


  /**
   * @brief Helper to shift both begin and end indices by the same amount.
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

  /// Begin index.
  index_type m_beg = 0;

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
