// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthContainers/tools/JaggedVecVectorFactory.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Apr, 2024
 * @brief Factory object that creates vectors using @c AuxTypeVector,
 *        specialized for JaggedVec.
 */


#ifndef ATHCONTAINERS_JAGGEDVECVECTORFACTORY_H
#define ATHCONTAINERS_JAGGEDVECVECTORFACTORY_H


#include "AthContainers/tools/JaggedVecVector.h"
#include "AthContainers/tools/AuxTypeVectorFactory.h"


namespace SG {


/**
 * @brief Factory objects that creates vectors for packed links
 *
 * This is an implementation of @c IAuxTypeVectorFactory that makes
 * vectors using  @c JaggedVecVector.
 */
template <class T, class ALLOC = AuxAllocator_t<JaggedVecElt<T> > >
class JaggedVecVectorFactory
  : public AuxTypeVectorFactoryImpl<JaggedVecElt<T>, ALLOC>
{
public:
  using Elt = JaggedVecElt<T>;
  using Shift = typename Elt::Shift;
  using index_type = typename Elt::index_type;

  using Base = AuxTypeVectorFactoryImpl<Elt, ALLOC>;
  using AuxTypeVector_t = JaggedVecVector<T, ALLOC>;
  using vector_value_type = typename AuxTypeVector_t::vector_value_type;


  /**
   * @brief Create a vector object of this type.
   * @param auxid ID for the variable being created.
   * @param size Initial size of the new vector.
   * @param capacity Initial capacity of the new vector.
   * @param isLinked True if this variable is linked from another one.
   *                 Must be false.
   */
  virtual
  std::unique_ptr<IAuxTypeVector> create (SG::auxid_t auxid,
                                          size_t size,
                                          size_t capacity,
                                          bool isLinked) const override;


  /**
   * @brief Create a vector object of this type from a data blob.
   * @param auxid ID for the variable being created.
   * @param data The vector object.
   * @param linkedVector The interface for another variable linked to this one.
   *                     (We do not take ownership.)
   * @param isPacked If true, @c data is a @c PackedContainer.
   * @param ownFlag If true, the newly-created IAuxTypeVector object
   *                will take ownership of @c data.
   * @param isLinked True if this variable is linked from another one.
   *
   * @c data should be a pointer to a
   * std::vector<SG::JaggedVecElt<T>, ALLOC<...> > object obtained with new.
   * For this method, isPacked and isLinked must both be false.
   */
  virtual
  std::unique_ptr<IAuxTypeVector> createFromData (SG::auxid_t auxid,
                                                  void* data,
                                                  IAuxTypeVector* linkedVector,
                                                  bool isPacked,
                                                  bool ownFlag,
                                                  bool isLinked) const override;


  /**
   * @brief Copy elements between vectors.
   * @param auxid The aux data item being operated on.
   * @param dst Container for the destination vector.
   * @param dst_index Index of the first destination element in the vector.
   * @param src Container for the source vector.
   * @param src_index Index of the first source element in the vector.
   * @param n Number of elements to copy.
   *
   * @c dst and @ src can be either the same or different.
   */
  virtual void copy (SG::auxid_t auxid,
                     AuxVectorData& dst,        size_t dst_index,
                     const AuxVectorData& src,  size_t src_index,
                     size_t n) const override;


  /**
   * @brief Copy elements between vectors, possibly applying thinning.
   * @param auxid The aux data item being operated on.
   * @param dst Container for the destination vector.
   * @param dst_index Index of the first destination element in the vector.
   * @param src Container for the source vector.
   * @param src_index Index of source element in the vector.
   * @param src_index Index of the first source element in the vector.
   * @param n Number of elements to copy.
   *
   * @c dst and @ src can be either the same or different.
   */
  virtual void copyForOutput (SG::auxid_t auxid,
                              AuxVectorData& dst,        size_t dst_index,
                              const AuxVectorData& src,  size_t src_index,
                              size_t n) const override;



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


private:
  /**
   * @brief Copy elements between vectors.
   * @param auxid The aux data item being operated on.
   * @param dst Container for the destination vector.
   * @param dst_index Index of the first destination element in the vector.
   * @param src Container for the source vector.
   * @param src_index Index of the first source element in the vector.
   * @param n Number of elements to copy.
   * @param for_output If true, apply thinning.
   *
   * @c dst and @ src can be either the same or different.
   */
  void copyImpl (SG::auxid_t auxid,
                 AuxVectorData& dst,        size_t dst_index,
                 const AuxVectorData& src,  size_t src_index,
                 size_t n,                  bool for_output) const;
};



/**
 * @brief Factory object that creates vectors using @c AuxTypeVector.
 *
 * Specialize @c AuxTypeVectorFactory for @c JaggedVec.
 */
template <class T, class ALLOC>
class AuxTypeVectorFactory<JaggedVecElt<T>, ALLOC>
  : public JaggedVecVectorFactory<T, ALLOC>
{
  using Base = JaggedVecVectorFactory<T, ALLOC>;
  using AuxTypeVector_t = typename Base::AuxTypeVector_t;
  using vector_value_type = typename Base::vector_value_type;
  using Base::Base;
};


} // namespace SG


#include "AthContainers/tools/JaggedVecVectorFactory.icc"


#endif // not ATHCONTAINERS_JAGGEDVECVECTORFACTORY_H
