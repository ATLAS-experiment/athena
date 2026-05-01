// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthContainers/tools/JaggedVecVector.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2024
 * @brief Implementation of @c IAuxTypeVector for @c JaggedVecElt types.
 */


#ifndef ATHCONTAINERS_JAGGEDVECVECTOR_H
#define ATHCONTAINERS_JAGGEDVECVECTOR_H


#include "AthContainers/tools/AuxTypeVector.h"
#include "AthContainers/JaggedVecImpl.h"
#include "AthContainers/AuxVectorData.h"


namespace SG {


//**************************************************************************


/**
 * @brief Implementation of @c IAuxTypeVector for @c JaggedVec types.
 *
 * This class is initialized with a pointer to the actual vector.
 * For a version that holds the vector internally, see the derived
 * class @c JaggedVecVector.
 *
 * Here, @c T is type of the vector payload, and @c ALLOC
 * is the allocator to use for the vector of @c JaggedVecElt.
 */
template <class T, class ALLOC = AuxAllocator_t<JaggedVecElt<T> > >
class JaggedVecVectorHolder
  : public AuxTypeVectorHolder<JaggedVecElt<T>,
                               typename AuxDataTraits<JaggedVecElt<T>, ALLOC>::vector_type>
{
  using Elt = JaggedVecElt<T>;
  using Shift = typename Elt::Shift;
  using index_type = typename Elt::index_type;
  using Base = AuxTypeVectorHolder<Elt,
                                   typename AuxDataTraits<Elt, ALLOC>::vector_type>;


public:
  /// We use as much as we can of the generic implementation.
  using Base::Base;
  using vector_type = typename Base::vector_type;
  using element_type = typename Base::element_type;


  /**
   * @brief Constructor.
   * @param auxid The auxid of the variable this vector represents.
   * @param vecPtr Pointer to the object.
   * @param linkedVec Interface for the linked payload vector.
   * @param ownFlag If true, take ownership of the object.
   */
  JaggedVecVectorHolder (auxid_t auxid,
                         vector_type* vecPtr,
                         IAuxTypeVector* linkedVec,
                         bool ownFlag);


  /**
   * @brief Change the size of the vector.
   * @param sz The new vector size.
   * Returns true if it is known that iterators have not been invalidated;
   * false otherwise.  (Will always return false when increasing the size
   * of an empty container.)
   */
  virtual bool resize (size_t sz) override;


  /**
   * @brief Shift the elements of the vector.
   * @param pos The starting index for the shift.
   * @param offs The (signed) amount of the shift.
   *
   * This operation shifts the elements in the vectors for all
   * aux data items, to implement an insertion or deletion.
   * @c offs may be either positive or negative.
   *
   * If @c offs is positive, then the container is growing.
   * The container size should be increased by @c offs,
   * the element at @c pos moved to @c pos + @c offs,
   * and similarly for following elements.
   * The elements between @c pos and @c pos + @c offs should
   * be default-initialized.
   *
   * If @c offs is negative, then the container is shrinking.
   * The element at @c pos should be moved to @c pos + @c offs,
   * and similarly for following elements.
   * The container should then be shrunk by @c -offs elements
   * (running destructors as appropriate).
   *
   * Returns true if it is known that iterators have not been invalidated;
   * false otherwise.  (Will always return false when increasing the size
   * of an empty container.)
   */
  virtual bool shift (size_t pos, ptrdiff_t offs) override;


  /**
   * @brief Insert elements into the vector via move semantics.
   * @param pos The starting index of the insertion.
   * @param src Start of the vector containing the range of elements to insert.
   * @param src_pos Position of the first element to insert.
   * @param src_n Number of elements to insert.
   * @param srcStore The source store.
   *
   * The size of the container will be increased by @c src_n, with the elements
   * starting at @c pos copied to @c pos+src_n.
   *
   * The contents of the source range will then be moved to our vector
   * starting at @c pos.  This will be done via move semantics if possible;
   * otherwise, it will be done with a copy.
   *
   * Returns true if it is known that the vector's memory did not move,
   * false otherwise.
   */
  virtual bool insertMove (size_t pos,
                           void* src, size_t src_pos, size_t src_n,
                           IAuxStore& srcStore) override;


protected:
  /**
   * @brief Fill in any trailing zeros at the end of this vector.
   */
  void fillTrailingZeros();


  /// Interface for the linked vector of DataLinks.
  IAuxTypeVector* m_linkedVec{};
};


//**************************************************************************


/**
 * @brief Implementation of @c IAuxTypeVector holding a vector of @c JaggedVec.
 *
 * This is a derived class of @c JaggedVecVectorHolder that holds the vector
 * instance as a member variable (and thus manages memory internally).
 * It is templated on the base class, so that we can also use this
 * for classes which derive from AuxTypeVectorHolder.
 *
 * When an instance of this is created, it gets ownership of the linked
 * vector via the constructor.  When this interface gets added to an
 * auxiliary store, the ownership of the linked vector will be transferred
 * away using the @c linkedVector() method, allowing it to be added
 * to the store as well.
 */
template <class HOLDER>
class JaggedVecVectorT
  : public HOLDER
{
public:
  using Base = HOLDER;
  using vector_type = typename Base::vector_type;
  using element_type = typename Base::element_type;
  using vector_value_type = typename Base::vector_value_type;


  /**
   * @brief Constructor.  Makes a new vector.
   * @param auxid The auxid of the variable this vector represents.
   * @param size Initial size of the new vector.
   * @param capacity Initial capacity of the new vector.
   * @param linkedVec Ownership of the linked vector.
   */
  JaggedVecVectorT (auxid_t auxid,
                     size_t size, size_t capacity,
                     std::unique_ptr<IAuxTypeVector> linkedVec);


  /**
   * @brief Copy constructor.
   */
  JaggedVecVectorT (const JaggedVecVectorT& other);


  /**
   * @brief Move constructor.
   */
  JaggedVecVectorT (JaggedVecVectorT&& other);


  /// No assignment.
  JaggedVecVectorT& operator= (const JaggedVecVectorT& other) = delete;
  JaggedVecVectorT& operator= (JaggedVecVectorT&& other) = delete;


  /**
   * @brief Make a copy of this vector.
   */
  virtual std::unique_ptr<IAuxTypeVector> clone() const override;


  /**
   * @brief Return ownership of the linked vector.
   */
  virtual std::unique_ptr<IAuxTypeVector> linkedVector() override;

  
private:
  /// The contained vector.
  vector_type m_vec;

  /// Holding ownership of the linked vector.
  std::unique_ptr<IAuxTypeVector> m_linkedVecHolder;
};


//**************************************************************************


template <class T, class ALLOC = AuxAllocator_t<JaggedVecElt<T> > >
using JaggedVecVector = JaggedVecVectorT<JaggedVecVectorHolder<T, ALLOC> >;


} // namespace SG


#include "AthContainers/tools/JaggedVecVector.icc"


#endif // not ATHCONTAINERS_JAGGEDVECVECTOR_H
