// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthContainers/tools/PackedLinkVector.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2023
 * @brief Implementation of @c IAuxTypeVector for @c PackedLink types.
 */


#ifndef ATHCONTAINERS_PACKEDLINKVECTOR_H
#define ATHCONTAINERS_PACKEDLINKVECTOR_H


#include "AthContainers/tools/AuxTypeVector.h"
#include "AthContainers/tools/PackedLinkVectorHelper.h"
#include "AthContainers/PackedLinkImpl.h"
#include "AthContainers/AuxVectorData.h"
#include "AthLinks/ElementLink.h"
#include "AthLinks/DataLink.h"
#include <ranges>


namespace SG {


/**
 * @brief Implementation of @c IAuxTypeVector for @c PackedLink types.
 *
 * This class is initialized with a pointer to the actual vector.
 * For a version that holds the vector internally, see the derived
 * class @c PackedLinkVector.
 *
 * Here, @c CONT is the target container of the links, and @c ALLOC
 * is the allocator to use for the vector of @c PackedLink.
 */
template <class CONT, class ALLOC = AuxAllocator_t<PackedLink<CONT> > >
class PackedLinkVectorHolder
  : public AuxTypeVectorHolder<PackedLink<CONT>,
                               typename AuxDataTraits<PackedLink<CONT>, ALLOC>::vector_type>
{
  using Base = AuxTypeVectorHolder<PackedLink<CONT>,
                                   typename AuxDataTraits<PackedLink<CONT>, ALLOC>::vector_type>;


public:
  /// We use as much as we can of the generic implementation.
  using Base::Base;
  using vector_type = typename Base::vector_type;
  using element_type = typename Base::element_type;


  /// Helper for manipulating @c PackedLink instances.
  using Helper = detail::PackedLinkVectorHelper<CONT>;


  /**
   * @brief Constructor.
   * @param auxid The auxid of the variable this vector represents.
   * @param vecPtr Pointer to the object.
   * @param linkedVec Interface for the linked vector of DataLinks.
   * @param ownFlag If true, take ownership of the object.
   */
  PackedLinkVectorHolder (auxid_t auxid,
                          vector_type* vecPtr,
                          IAuxTypeVector* linkedVec,
                          bool ownFlag);


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
  /// Interface for the linked vector of DataLinks.
  IAuxTypeVector* m_linkedVec{};
};


//**************************************************************************


/**
 * @brief Implementation of @c IAuxTypeVector for a vector of @c PackedLink.
 *
 * This class is initialized with a pointer to the actual vector.
 * For a version that holds the vector internally, see the derived
 * class @c PackedLinkVector.
 *
 * Here, @c CONT is the target container of the links, and @c VALLOC
 * is the allocator to use for the vector of @c PackedLink elements,
 * VELT is the element type (usually vector<PackedLink<CONT> >),
 * and ALLOC is the allocator to use for the outer vector.
 */
template <class CONT,
          class VALLOC = AuxAllocator_t<PackedLink<CONT> >,
          class VELT = typename AuxDataTraits<PackedLink<CONT>, VALLOC >::vector_type,
          class ALLOC = AuxAllocator_t<VELT> >
class PackedLinkVVectorHolder
  : public AuxTypeVectorHolder<VELT,
                                typename AuxDataTraits<VELT, ALLOC>::vector_type>
{
  using Base = AuxTypeVectorHolder<VELT,
                                   typename AuxDataTraits<VELT, ALLOC>::vector_type>;


public:
  /// We use as much as we can of the generic implementation.
  using Base::Base;
  using vector_type = typename Base::vector_type;
  using element_type = typename Base::element_type;


  /// Helper for manipulating @c PackedLink instances.
  using Helper = detail::PackedLinkVectorHelper<CONT>;


  /**
   * @brief Constructor.
   * @param auxid The auxid of the variable this vector represents.
   * @param vecPtr Pointer to the object (of type @c CONT).
   * @param linkedVec Interface for the linked vector of DataLinks.
   * @param ownFlag If true, take ownership of the object.
   */
  PackedLinkVVectorHolder (auxid_t auxid,
                           vector_type* vecPtr,
                           IAuxTypeVector* linkedVec,
                           bool ownFlag);


  /**
   * @brief Insert elements into the vector via move semantics.
   * @param pos The starting index of the insertion.
   * @param src Start of the vector containing the range of elements to insert.
   * @param src_pos Position of the first element to insert.
   * @param src_n Number of elements to insert.
   * @param srcStore The source store.
   *
   * @c beg and @c end define a range of container elements, with length
   * @c len defined by the difference of the pointers divided by the
   * element size.
   *
   * The size of the container will be increased by @c len, with the elements
   * starting at @c pos copied to @c pos+len.
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
  /// Interface for the linked vector of DataLinks.
  IAuxTypeVector* m_linkedVec{};
};


//**************************************************************************


/**
 * @brief Implementation of @c IAuxTypeVector holding a vector of @c PackedLink.
 *
 * This is a derived class of @c PackedLinkVectorHolder that holds the vector
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
class PackedLinkVectorT
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
  PackedLinkVectorT (auxid_t auxid,
                     size_t size, size_t capacity,
                     std::unique_ptr<IAuxTypeVector> linkedVec);


  /**
   * @brief Copy constructor.
   */
  PackedLinkVectorT (const PackedLinkVectorT& other);


  /**
   * @brief Move constructor.
   */
  PackedLinkVectorT (PackedLinkVectorT&& other);


  /// No assignment.
  PackedLinkVectorT& operator= (const PackedLinkVectorT& other) = delete;
  PackedLinkVectorT& operator= (PackedLinkVectorT&& other) = delete;


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


template <class CONT, class ALLOC = AuxAllocator_t<PackedLink<CONT> > >
using PackedLinkVector = PackedLinkVectorT<PackedLinkVectorHolder<CONT, ALLOC> >;

template <class CONT,
          class VALLOC = AuxAllocator_t<PackedLink<CONT> >,
          class VELT = typename AuxDataTraits<PackedLink<CONT>, VALLOC >::vector_type,
          class ALLOC = AuxAllocator_t<VELT> >
using PackedLinkVVector = PackedLinkVectorT<PackedLinkVVectorHolder<CONT, VALLOC, VELT, ALLOC> >;


} // namespace SG


#include "AthContainers/tools/PackedLinkVector.icc"


#endif // not ATHCONTAINERS_PACKEDLINKVECTOR_H
