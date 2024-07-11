// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainersInterfaces/IAuxTypeVector.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Apr, 2013
 * @brief Abstract interface for manipulating vectors of arbitrary types.
 */


#ifndef ATHCONTAINERSINTERFACES_IAUXTYPEVECTOR_H
#define ATHCONTAINERSINTERFACES_IAUXTYPEVECTOR_H


#include "AthContainersInterfaces/AuxTypes.h"
#include "AthContainersInterfaces/IAuxStore.h"
#include "AthContainersInterfaces/AuxDataSpan.h"
#include "CxxUtils/CachedValue.h"
#include <cstddef>
#include <memory>
#include <typeinfo>


namespace SG {


class AuxDataOption;


/**
 * @brief Abstract interface for manipulating vectors of arbitrary types.
 *
 * The auxiliary data for a container are stored in a set of STL vectors,
 * one for each data item.  However, we want to allow storing arbitrary
 * types in these vectors.  Thus, we define this abstract interface
 * to operate on the vectors.  The concrete version of this will
 * own one vector.
 */
class IAuxTypeVector
{
public:
  /**
   * @brief Constructor.
   * @param auxid The ID of the variable that this vector represents.
   * @param isLinked True if this variable is linked from another one.
   */
  IAuxTypeVector (auxid_t auxid, bool isLinked)
    : m_auxid (auxid),
      m_isLinked (isLinked)
  {
  }


  /// Destructor.
  virtual ~IAuxTypeVector() = default;


  /**
   * @brief Make a copy of this vector.
   */
  virtual std::unique_ptr<IAuxTypeVector> clone() const = 0;

  
  /**
   * @brief Return a pointer to the start of the vector's data.
   */
  virtual void* toPtr() = 0;


  /**
   * @brief Return a pointer to the start of the vector's data.
   */
  virtual const void* toPtr() const = 0;


  /**
   * @brief Return a pointer to the STL vector itself.
   */
  virtual void* toVector() = 0;


  /**
   * @brief Return the size of the vector.
   */
  virtual size_t size() const = 0;


  /**
   * @brief Change the size of the vector.
   * @param sz The new vector size.
   * Returns true if it is known that iterators have not been invalidated;
   * false otherwise.  (Will always return false when increasing the size
   * of an empty container.)
   */
  virtual bool resize (size_t sz) = 0;


  /**
   * @brief Change the capacity of the vector.
   * @param sz The new vector capacity.
   */
  virtual void reserve (size_t sz) = 0;


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
  virtual bool shift (size_t pos, ptrdiff_t offs) = 0;


  /**
   * @brief Insert elements into the vector via move semantics.
   * @param pos The starting index of the insertion.
   * @param beg Start of the range of elements to insert.
   * @param end End of the range of elements to insert.
   * @param srcStore The source store.
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
  virtual bool insertMove (size_t pos, void* beg, void* end,
                           IAuxStore& srcStore) = 0;


  /**
   * @brief Set an option for this variable.
   * @param option The option to set.
   *
   * The interpretation of the option depends on the particular representation
   * of the variable provided by the concrete class.
   *
   * Returns true if the option setting was successful; false otherwise.
   */
  virtual bool setOption (const AuxDataOption& /*option*/)
  { return false; }


  /**
   * @brief Make a packed version of the variable.
   *
   * If possible, return a new vector object that stores the data
   * in a @c PackedContainer.  The data itself should be _moved_
   * to the new container (so that this vector becomes empty).
   * This ensures that pointers to the data are preserved.
   *
   * If successful, a newly-allocated object is returned.
   * A null pointer is returned on failure (operation not supported,
   * type can't be packed, type is already packed).
   */
  virtual std::unique_ptr<IAuxTypeVector> toPacked() { return 0; }


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
  virtual const std::type_info* objType() const { return 0; }


  /**
   * @brief Return @c IAuxTypeVector of a linked variable, if there is one.
   *
   * If the variable represented by this vector has a linked variable,
   * then return its @c IAuxTypeVector.  Otherwise, return nullptr.
   * Beware of potential threading issues: the returned object is not locked,
   * so it should not be modified in contexts where the parent container
   * cannot be modified.
   *
   * This returns a @c unique_ptr, so it can generally be called only once
   * on a given instance.  After that, it will return nullptr.
   */
  virtual std::unique_ptr<IAuxTypeVector> linkedVector() { return nullptr; }


  /**
   * @brief Return true if this variable is linked from another one.
   *
   * This is inlined here rather than being a virtual function because
   * this is frequently called from loops over auxids.
   */
  bool isLinked() const { return m_isLinked; }


  /**
   * @brief Return the auxid of the variable this vector represents.
   */
  auxid_t auxid() const
  {
    return m_auxid;
  }


  /**
   * @brief Return a reference to a description of this vector's start+size.
   *
   * This returns a reference to an @c AuxDataSpanBase, which gives the
   * start and size of the vector.  This object will be updated if the
   * vector changes.
   *
   * For low overhead, we want this to be a non-virtual function call.
   * However, for variables being read, the usage pattern is that we first
   * create the @IAuxTypeVector object, give the underlying @c std::vector
   * object to ROOT, and then ROOT fills the vector without the involvement
   * of the @c IAuxTypeVector.  To be able to have this work correctly,
   * we need to defer initializing the span object until the first time
   * that @c getDataSpan gets called.  We do this with a @c CachedValue.
   * If the span has already been initialized, we just return it; otherwise,
   * we make a virtual call to fetch the vector from the derived class.
   *
   * Be aware: this is in principle a const-correctness violation,
   * since @c AuxDataSpanBase has a non-const pointer to the start
   * of the vector.  But doing it properly is kind of painful, and as
   * this interface is only meant to be used internally, it's likely
   * not a real problem.
   */
  const AuxDataSpanBase& getDataSpan() const
  {
    if (!m_span.isValid()) {
      m_span.set (this->getDataSpanImpl());
    }
    return *m_span.ptr();
  }


protected:
  /**
   * @brief Return a span object describing the current vector.
   *        Used to initialize @c m_span the first time that @c getDataSpan
   *        is called.
   */
  virtual AuxDataSpanBase getDataSpanImpl() const = 0;


  /**
   * @brief Update the stored span.
   * @param beg The start of the vector.
   * @param size The length of the vector.
   */
  void storeDataSpan (void* beg, size_t size)
  {
    // Only do this if the span is already valid, so that it doesn't
    // get marked valid before ROOT I/O.
    if (m_span.isValid()) {
      m_span.store (AuxDataSpanBase (beg, size));
    }
  }


  /**
   * @brief Invalidate the stored span.
   */
  void resetDataSpan()
  {
    m_span.reset();
  }
    

private:
  /// The auxid of the variable this vector represents.
  auxid_t m_auxid;

  /// True if this variable is linked from another one.
  bool m_isLinked;

  /// Description of the vector start+size.
  CxxUtils::CachedValue<AuxDataSpanBase> m_span;
};


} // namespace SG


#endif // not ATHCONTAINERSINTERFACES_IAUXTYPEVECTOR_H
