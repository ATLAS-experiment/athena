// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/tools/AuxTypeVectorFactory.h
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2014
 * @brief Factory object that creates vectors using @c AuxTypeVector.
 */


#include "AthContainersInterfaces/IAuxTypeVectorFactory.h"
#include "AthContainers/AuxVectorData.h"
#include "AthContainers/tools/AuxTypeVector.h"
#include "AthContainers/tools/AuxDataTraits.h"
#include "AthContainers/normalizedTypeinfoName.h"


#ifndef ATHCONTAINERS_AUXTYPEVECTORFACTORY_H
#define ATHCONTAINERS_AUXTYPEVECTORFACTORY_H


namespace SG {


/**
 * @brief Factory object that creates vectors using @c AuxTypeVector.
 *
 * This is an implementation of @c IAuxTypeVectorFactory that makes
 * vectors using the @c AuxTypeVector implementation.
 *
 * All of the actual code is broken out into this base class of
 * @c AuxTypeVectorFactory to make it easier to specialize that type
 * while still allowing the specializations to share this code.
 */
template <class T, class ALLOC = AuxAllocator_t<T> >
class AuxTypeVectorFactoryImpl
  : public IAuxTypeVectorFactory
{
public:
  using AuxTypeVector_t = AuxTypeVector<T, ALLOC>;

  using vector_value_type = typename AuxTypeVector_t::vector_value_type;

  /**
   * @brief Create a vector object of this type.
   * @param auxid ID for the variable being created.
   * @param size Initial size of the new vector.
   * @param capacity Initial capacity of the new vector.
   *
   * Returns a newly-allocated object.
   */
  virtual
  std::unique_ptr<IAuxTypeVector> create (SG::auxid_t auxid,
                                          size_t size,
                                          size_t capacity) const override;


  /**
   * @brief Create a vector object of this type from a data blob.
   * @param auxid ID for the variable being created.
   * @param data The vector object.
   * @param isPacked If true, @c data is a @c PackedContainer.
   * @param ownFlag If true, the newly-created IAuxTypeVector object
   *                will take ownership of @c data.
   *
   * If the element type is T, then @c data should be a pointer
   * to a std::vector<T> object, which was obtained with @c new.
   * But if @c isPacked is @c true, then @c data
   * should instead point at an object of type @c SG::PackedContainer<T>.
   *
   * Returns a newly-allocated object.
   */
  virtual
  std::unique_ptr<IAuxTypeVector> createFromData (SG::auxid_t auxid,
                                                  void* data,
                                                  bool isPacked,
                                                  bool ownFlag) const override;

  
  /**
   * @brief Copy an element between vectors.
   * @param dst Pointer to the start of the destination vector's data.
   * @param dst_index Index of destination element in the vector.
   * @param src Pointer to the start of the source vector's data.
   * @param src_index Index of source element in the vector.
   *
   * @c dst and @ src can be either the same or different.
   */
  virtual void copy (void* dst,        size_t dst_index,
                     const void* src,  size_t src_index) const override;


  /**
   * @brief Copy an element between vectors, possibly applying thinning.
   * @param dst Pointer to the start of the destination vector's data.
   * @param dst_index Index of destination element in the vector.
   * @param src Pointer to the start of the source vector's data.
   * @param src_index Index of source element in the vector.
   *
   * @c dst and @ src can be either the same or different.
   */
  virtual void copyForOutput (void* dst,        size_t dst_index,
                              const void* src,  size_t src_index) const override;


  /**
   * @brief Swap elements between vectors.
   * @param auxid The aux data item being operated on.
   * @param a Container for the first vector.
   * @param aindex Index of the first element in the first vector.
   * @param b Container for the second vector.
   * @param bindex Index of the first element in the second vector.
   * @param n Number of elements to swap.
   *
   * @c a and @ b can be either the same or different.
   * However, the ranges should not overlap.
   */
  virtual void swap (SG::auxid_t auxid,
                     AuxVectorData& a, size_t aindex,
                     AuxVectorData& b, size_t bindex,
                     size_t n) const override;


  /**
   * @brief Clear a range of elements within a vector.
   * @param auxid The aux data item being operated on.
   * @param dst Container holding the element
   * @param dst_index Index of the first element in the vector.
   * @param n Number of elements to clear.
   */
  virtual void clear (SG::auxid_t auxid,
                      AuxVectorData& dst, size_t dst_index,
                      size_t n) const override;


  /**
   * @brief Return the size of an element of this vector type.
   */
  virtual size_t getEltSize() const override;


  /**
   * @brief Return the @c type_info of the vector.
   */
  virtual const std::type_info* tiVec() const override;


  /**
   * @brief True if the vectors created by this factory work by dynamic
   *        emulation (via @c TVirtualCollectionProxy or similar); false
   *        if the std::vector code is used directly.
   */
  virtual bool isDynamic() const override;


  /**
   * @brief Return the @c type_info of the vector allocator.
   */
  virtual const std::type_info* tiAlloc() const override;


  /**
   * @brief Return the (demangled) name of the vector allocator.
   */
  virtual std::string tiAllocName() const override;


private:
  /// Helpers for creating vector from a data blob,
  std::unique_ptr<IAuxTypeVector>
  createFromData (auxid_t auxid, void* data, bool isPacked, bool ownFlag,
                  std::true_type) const;

  std::unique_ptr<IAuxTypeVector>
  createFromData (auxid_t auxid, void* data, bool isPacked, bool ownFlag,
                  std::false_type) const;
};



/**
 * @brief Factory object that creates vectors using @c AuxTypeVector.
 *
 * This is an implementation of @c IAuxTypeVectorFactory that makes
 * vectors using the @c AuxTypeVector implementation.
 */
template <class T, class ALLOC = AuxAllocator_t<T> >
class AuxTypeVectorFactory
  : public AuxTypeVectorFactoryImpl<T, ALLOC>
{
  using Base = AuxTypeVectorFactoryImpl<T, ALLOC>;
  using AuxTypeVector_t = typename Base::AuxTypeVector_t;
  using vector_value_type = typename Base::vector_value_type;
  using Base::Base;
};


} // namespace SG


#include "AthContainers/tools/AuxTypeVectorFactory.icc"


#endif // not ATHCONTAINERS_AUXTYPEVECTORFACTORY_H
