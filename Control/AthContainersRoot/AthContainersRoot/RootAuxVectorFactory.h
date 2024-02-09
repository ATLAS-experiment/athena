// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainersRoot/RootAuxVectorFactory.h
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2014
 * @brief Dynamic implementation of @c IAuxVectorFactory,
 *        relying on root's vector proxy.
 */


#ifndef ATHCONTAINERSROOT_ROOTAUXVECTORFACTORY_H
#define ATHCONTAINERSROOT_ROOTAUXVECTORFACTORY_H


#include "AthContainersInterfaces/IAuxTypeVectorFactory.h"
#include "AthContainersInterfaces/IAuxTypeVector.h"
#include "RootUtils/Type.h"
#include "CxxUtils/checker_macros.h"


class TClass;
class TVirtualCollectionProxy;


namespace SG {


class RootAuxVectorFactory;


/**
 * @brief Dynamic implementation of @c IAuxTypeVector,
 *        relying on root vector proxy.
 *
 * This is used for the case when we need to manipulate an aux data vector
 * present in an input data file but we have neither a proper template
 * instantiation for the factory (because the variable was never explicitly
 * referenced), nor can we find a dictionary entry for the factory.j
 *
 * This implementation works by relying entirely on the root
 * dictionary information.
 */
class RootAuxVector
  : public SG::IAuxTypeVector
{
public:
  /**
   * @brief Constructor.  Makes a new vector.
   * @param factory The factory object for this type.
   * @param auxid The auxid of the variable this vector represents.
   * @param size Initial size of the new vector.
   * @param capacity Initial capacity of the new vector.
   */
  RootAuxVector (const RootAuxVectorFactory* factory,
                 SG::auxid_t auxid,
                 size_t size,
                 size_t capacity);


  /**
   * @brief Constructor, from a pointer to a vector object.
   * @param factory The factory object for this type.
   * @param auxid The auxid of the variable this vector represents.
   * @param data The vector object.
   * @param isPacked If true, @c data is a @c PackedContainer.
   * @param ownFlag If true, then take ownership of @c data.
   *
   * If the element type is T, then @c data should be a pointer
   * to a std::vector<T> object, which was obtained with @c new.
   *
   * This version does not support packed containers, so @c isPacked
   * must be false.
   */
  RootAuxVector (const RootAuxVectorFactory* factory,
                 SG::auxid_t auxid,
                 void* data,
                 bool isPacked,
                 bool ownFlag);


  
  /**
   * @brief Copy constructor.
   * @param other The vector to copy.
   */
  RootAuxVector (const RootAuxVector& other);


  // Disallow assignment.
  RootAuxVector& operator= (const RootAuxVector&) = delete;


  /**
   * @brief Destructor.
   *
   * This will free the vector data.
   */
  virtual ~RootAuxVector() override;


  /**
   * @brief Make a copy of this vector.
   */
  virtual std::unique_ptr<SG::IAuxTypeVector> clone() const override;


  /**
   * @brief Return the auxid of the variable this vector represents.
   */
  SG::auxid_t auxid() const override;


  /**
   * @brief Return a pointer to the start of the vector's data.
   */
  virtual void* toPtr() override;


  /**
   * @brief Return a pointer to the start of the vector's data.
   */
  virtual const void* toPtr() const override;


  /**
   * @brief Return a pointer to the overall object.
   */
  virtual void* toVector() override;



  /**
   * @brief Return the size of the vector.
   */
  virtual size_t size() const override;


  /**
   * @brief Change the size of the vector.
   * @param sz The new vector size.
   * Returns true if it is known that iterators have not been invalidated;
   * false otherwise.
   */
  virtual bool resize (size_t sz) override;


  /**
   * @brief Change the capacity of the vector.
   * @param sz The new vector capacity.
   */
  virtual void reserve (size_t sz) override;


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
   */
  virtual void shift (size_t pos, ptrdiff_t offs) override;


  /**
   * @brief Insert elements into the vector via move semantics.
   * @param pos The starting index of the insertion.
   * @param beg Start of the range of elements to insert.
   * @param end End of the range of elements to insert.
   *
   * @c beg and @c end define a range of container elements, with length
   * @c len defined by the difference of the pointers divided by the
   * element size.
   *
   * The size of the container will be increased by @c len, with the elements
   * starting at @c pos copied to @c pos+len.
   *
   * The contents of the @c beg:end range will then be moved to our vector
   * starting at @c pos.  This will be done via move semantics if possible;
   * otherwise, it will be done with a copy.
   *
   * Returns true if it is known that the vector's memory did not move,
   * false otherwise.
   */
  virtual bool insertMove (size_t pos, void* beg, void* end) override;


  /**
   * @brief Return the type of the complete object to be saved.
   *
   * For example, if the object is a @c std::vector, then we return
   * the @c type_info of the vector.  But if we're holding
   * a @c PackedContainer, then we return the @c type_info of the
   * @c PackedContainer.
   *
   * Can return null if the operation is not supported.  In that case,
   * I/O will use the type found from the variable registry.
   */
  virtual const std::type_info* objType() const override;


private:
  /// Pointer back to the factory class for this type.
  const RootAuxVectorFactory* m_factory;

  /// The collection proxy for the vector.
  /// Cloned from the proxy held by the TClass and permanently bound
  /// to m_vec.  That makes things a bit more efficient, and prevents
  /// potential thread-safety problems.
  std::unique_ptr<TVirtualCollectionProxy> m_proxy;

  /// Pointer to the overall object itself.
  void* m_obj;

  /// Pointer to the vector object itself.
  void* m_vec;

  /// Should be delete the vector object?
  bool m_ownFlag;

  /// The auxid of the variable this vector represents.
  auxid_t m_auxid;
};


/**
 * @brief Dynamic implementation of @c IAuxVectorFactory,
 *        relying on root's vector proxy.
 *
 * This is used for the case when we need to manipulate an aux data vector
 * present in an input data file but we have neither a proper template
 * instantiation for the factory (because the variable was never explicitly
 * referenced), nor can we find a dictionary entry for the factory.j
 *
 * This implementation works by relying entirely on the root
 * dictionary information.
 *
 * We may either be dealing directly with an STL vector class, or with
 * embedded in another class (as for PackedContainer).  Here, @a vecClass
 * is the class of the STL vector and @a objClass is the overall object
 * class.  In the case of a direct STL vector, these are identical.
 */
class RootAuxVectorFactory
  : public SG::IAuxTypeVectorFactory
{
public:
  /**
   * @brief Constructor.
   * @param vecClass The @c TClass for the vector object.
   */
  RootAuxVectorFactory (TClass* objClass);


  /**
   * @brief Destructor.
   */
  virtual ~RootAuxVectorFactory() override;


  /**
   * @brief Return the ROOT type wrapper.
   */
  const RootUtils::Type& rootType() const { return m_type; }


  /**
   * @brief Return the @c TClass for the overall object.
   *
   * (Returning non-const TClass* ok here; TClass is internally thread-safe.)
   */
  TClass* objClass ATLAS_NOT_CONST_THREAD_SAFE () const { return m_objClass; }


  /**
   * @brief Return the @c TClass for the @c std::vector.
   *
   * (Returning non-const TClass* ok here; TClass is internally thread-safe.)
   */
  TClass* vecClass ATLAS_NOT_CONST_THREAD_SAFE () const { return m_vecClass; }


  /**
   * @brief Return the offset of the vector within the object.
   */
  size_t offset() const { return m_offset; }


  /**
   * @brief Create a vector object of this type.
   * @param auxid ID for the variable being created.
   * @param size Initial size of the new vector.
   * @param capacity Initial capacity of the new vector.
   *
   * Returns a newly-allocated object.
   */
  virtual
  std::unique_ptr<SG::IAuxTypeVector> create (SG::auxid_t auxid,
                                              size_t size,
                                              size_t capacity) const
    override;


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
   *
   * This version does not support packed containers, so @c isPacked
   * must be false.
   *
   * Returns a newly-allocated object.
   */
  virtual std::unique_ptr<SG::IAuxTypeVector> createFromData (SG::auxid_t auxid,
                                                              void* data,
                                                              bool isPacked,
                                                              bool ownFlag) const
    override;


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


  /**
   * @brief Return the size of an element of this vector type.
   */
  virtual size_t getEltSize() const override;


  /**
   * @brief Return the @c type_info of the overall object.
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
   *
   * May be nullptr for a dynamic vector.
   */
  virtual const std::type_info* tiAlloc() const override;


  /**
   * @brief Return the (demangled) name of the vector allocator.
   */
  virtual std::string tiAllocName() const override;


private:
  /// Helper for copy; returns a pointer to the first destination object,
  /// or nullptr if the destination was cleared rather than copied.
  char* copyImpl (SG::auxid_t auxid,
                  AuxVectorData& dst,        size_t dst_index,
                  const AuxVectorData& src,  size_t src_index,
                  size_t n) const;


  /// The @c TClass for the overall object.
  TClass* m_objClass;

  /// The @c TClass for the std::vector.
  TClass* m_vecClass;

  /// Offset of the STL vector within the overall object.
  size_t m_offset;

  /// Wrapper for the ROOT type of the element.
  RootUtils::Type m_type;

  /// Flag to tell whether we need to do thinning.
  enum { NONE, ELEMENT_LINK, ELEMENT_LINK_VECTOR, ELEMENT_LINK_NONPOINTER} m_isEL;
};


} // namespace SG


#endif // not ATHCONTAINERSROOT_ROOTAUXVECTORFACTORY_H
