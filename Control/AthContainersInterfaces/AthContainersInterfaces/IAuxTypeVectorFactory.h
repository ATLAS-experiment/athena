// This file's extension implies that it's C, but it's really -*- C++ -*-.

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainersInterfaces/IAuxTypeVectorFactory.h
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2014
 * @brief Interface for factory objects that create vectors.
 */


#ifndef ATHCONTAINERSINTERFACES_IAUXTYPEVECTORFACTORY_H
#define ATHCONTAINERSINTERFACES_IAUXTYPEVECTORFACTORY_H


#include "AthContainersInterfaces/AuxTypes.h"
#include <cstddef>
#include <typeinfo>
#include <memory>
#include <cstdlib>


namespace SG {


class IAuxTypeVector;
class AuxVectorData;


/**
 * @brief Interface for factory objects that create vectors.
 *
 * The auxiliary data for a container are stored in a set of STL vectors,
 * one for each data item.  However, we want to allow storing arbitrary
 * types in these vectors.  Thus, we define an abstract interface
 * to operate on the vectors, @c IAuxTypeVector.
 *
 * Now, we need to registry of how to create an appropriate
 * @c IAuxTypeVector for a given @c std::type_info.  To make that
 * easier, we encapsulate the creation of those vector objects
 * using this factory interface.  There will be one instance of this
 * for each vector type that we deal with.  Usually, this interface
 * will be implemented by @c AuxTypeVector; however, other implementations
 * are used, for example, for reading auxiliary data from root when
 * we don't know its type at compile-time.
 */
class IAuxTypeVectorFactory
{
public:
  /**
   * @brief Destructor.
   */
  virtual ~IAuxTypeVectorFactory() {}


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
                                          size_t capacity) const = 0;


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
                                                  bool ownFlag) const = 0;


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
                     size_t n) const = 0;


  /**
   * @brief Copy elements between vectors.
   * @param auxid The aux data item being operated on.
   * @param dst Container for the destination vector.
   *            Declared as a rvalue reference to allow passing a temporary
   *            here (such as from AuvVectorInterface).
   * @param dst_index Index of the first destination element in the vector.
   * @param src Container for the source vector.
   * @param src_index Index of the first source element in the vector.
   * @param n Number of elements to copy.
   *
   * @c dst and @ src can be either the same or different.
   */
  void copy (SG::auxid_t auxid,
             AuxVectorData&& dst,       size_t dst_index,
             const AuxVectorData& src,  size_t src_index,
             size_t n) const
  {
    copy (auxid, dst, dst_index, src, src_index, n);
  }


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
                              size_t n) const = 0;


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
                     size_t n) const = 0;


  /**
   * @brief Clear a range of elements within a vector.
   * @param auxid The aux data item being operated on.
   * @param dst Container holding the element
   * @param dst_index Index of the first element in the vector.
   * @param n Number of elements to clear.
   */
  virtual void clear (SG::auxid_t auxid,
                      AuxVectorData& dst, size_t dst_index,
                      size_t n) const = 0;


  /**
   * @brief Return the size of an element of this vector type.
   */
  virtual size_t getEltSize() const = 0;


  /**
   * @brief Return the @c type_info of the vector.
   */
  virtual const std::type_info* tiVec() const = 0;


  /**
   * @brief True if the vectors created by this factory work by dynamic
   *        emulation (via @c TVirtualCollectionProxy or similar); false
   *        if the std::vector code is used directly.
   */
  virtual bool isDynamic() const = 0;


  /**
   * @brief Return the @c type_info of the vector allocator.
   *
   * May be nullptr for a dynamic vector.
   */
  virtual const std::type_info* tiAlloc() const = 0;


  /**
   * @brief Return the (demangled) name of the vector allocator.
   */
  virtual std::string tiAllocName() const = 0;
};


} // namespace SG


#endif // not ATHCONTAINERSINTERFACES_IAUXTYPEVECTORFACTORY_H
