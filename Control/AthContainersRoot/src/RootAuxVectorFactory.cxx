/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainersRoot/src/RootAuxVectorFactory.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2014
 * @brief Dynamic implementation of @c IAuxVectorFactory,
 *        relying on root vector proxy.
 */


#include "AthContainersRoot/RootAuxVectorFactory.h"
#include "AthContainers/AuxVectorData.h"
#include "AthContainers/tools/error.h"
#include "AthContainers/normalizedTypeinfoName.h"
#include "AthLinks/ElementLinkBase.h"
#include "CxxUtils/ClassName.h"
#include "CxxUtils/checker_macros.h"
#include "TClass.h"
#include "TVirtualCollectionProxy.h"
#include "TROOT.h"
#include <iostream>
#include <stdexcept>


namespace {


/**
 * @brief Find the vector type associated with @c CL by looking
 *        up the @c vector_type typedef.
 * @param cl The class for which to find the associated vector type.
 *
 * Given type CL, looks for a typedef CL::vector_type and returns the TClass
 * for that if found.  Works for, eg, SG::PackedContainer.
 */
TClass* lookupVectorType (TClass *cl)
{
  std::string tname = cl->GetName();
  tname += "::vector_type";
  TDataType* typ = gROOT->GetType (tname.c_str());
  if (typ)
    return TClass::GetClass (typ->GetFullTypeName());
  return nullptr;
}


} // anonymous namespace


namespace SG {


/**
 * @brief Constructor.  Makes a new vector.
 * @param factory The factory object for this type.
 * @param auxid The auxid of the variable this vector represents.
 * @param size Initial size of the new vector.
 * @param capacity Initial capacity of the new vector.
 */
RootAuxVector::RootAuxVector (const RootAuxVectorFactory* factory,
                              SG::auxid_t auxid,
                              size_t size, size_t /*capacity*/)
  : m_factory (factory),
    m_ownFlag (true),
    m_auxid (auxid)
{
  const TClass* vecClass = factory->vecClass();
  m_proxy.reset (vecClass->GetCollectionProxy()->Generate());
  m_obj = factory->objClass()->New ();
  m_vec = reinterpret_cast<char*> (m_obj) + factory->offset();
  m_proxy->PushProxy (m_vec);
  this->resize (size);
}


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
RootAuxVector::RootAuxVector (const RootAuxVectorFactory* factory,
                              SG::auxid_t auxid,
                              void* data,
                              bool isPacked,
                              bool ownFlag)
  : m_factory (factory),
    m_ownFlag (ownFlag),
    m_auxid (auxid)
{
  if (isPacked) std::abort();
  const TClass* vecClass = factory->vecClass();
  m_proxy.reset (vecClass->GetCollectionProxy()->Generate());
  m_obj = data;
  m_vec = reinterpret_cast<char*> (m_obj) + factory->offset();
  m_proxy->PushProxy (m_vec);
}


/**
 * @brief Copy constructor.
 * @param other The vector to copy.
 */
RootAuxVector::RootAuxVector (const RootAuxVector& other)
  : m_factory (other.m_factory),
    m_proxy (other.m_proxy->Generate()),
    m_ownFlag (true),
    m_auxid (other.m_auxid)
{
  m_obj = m_factory->objClass()->New ();
  m_vec = reinterpret_cast<char*> (m_obj) + m_factory->offset();
  m_proxy->PushProxy (m_vec);
  size_t sz = other.size();
  this->resize (sz);

  if (sz > 0) {
    const RootUtils::Type& rootType = m_factory->rootType();
    const void* otherPtr = other.toPtr();
    rootType.copyRange (this->toPtr(), otherPtr, sz);
  }
}


/**
 * @brief Destructor.
 *
 * This will free the vector data.
 */
RootAuxVector::~RootAuxVector()
{
  if (m_ownFlag)
    m_factory->objClass()->Destructor (m_obj);
}


/**
 * @brief Make a copy of this vector.
 */
std::unique_ptr<SG::IAuxTypeVector> RootAuxVector::clone() const
{
  return std::make_unique<RootAuxVector> (*this);
}


/**
 * @brief Return the auxid of the variable this vector represents.
 */
SG::auxid_t RootAuxVector::auxid() const
{
  return m_auxid;
}


/**
 * @brief Return a pointer to the start of the vector's data.
 */
void* RootAuxVector::toPtr ()
{
  if (m_proxy->Size() == 0)
    return 0;
  return m_proxy->At(0);
}


/**
 * @brief Return a pointer to the start of the vector's data.
 */
const void* RootAuxVector::toPtr () const
{
  if (m_proxy->Size() == 0)
    return 0;
  TVirtualCollectionProxy* proxy ATLAS_THREAD_SAFE = m_proxy.get();
  return proxy->At(0);
}


/**
 * @brief Return a pointer to the overall object.
 */
void* RootAuxVector::toVector ()
{
  return m_obj;
}


/**
 * @brief Return the size of the vector.
 */
size_t RootAuxVector::size() const
{
  return m_proxy->Size();
}


/**
 * @brief Change the size of the vector.
 * @param sz The new vector size.
 * Returns true if it is known that iterators have not been invalidated;
 * false otherwise.
 */
bool RootAuxVector::resize (size_t sz)
{
  const void* orig = this->toPtr();
  m_proxy->Allocate(sz, false);
  return this->toPtr() == orig;
}


/**
 * @brief Change the capacity of the vector.
 * @param sz The new vector capacity.
 */
void RootAuxVector::reserve (size_t /*sz*/)
{
}


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
void RootAuxVector::shift (size_t pos, ptrdiff_t offs)
{
  size_t eltsz = m_proxy->GetIncrement();

  const RootUtils::Type& rootType = m_factory->rootType();

  if (offs < 0) {
    if (-offs > static_cast<ptrdiff_t>(pos)) offs = -pos;
    char* beg = reinterpret_cast<char*>(m_proxy->At(0));
    rootType.copyRange (beg + eltsz*(pos+offs),
                        beg + eltsz*pos,
                        m_proxy->Size() - pos);
    m_proxy->Allocate (m_proxy->Size() + offs, false);
  }
  else if (offs > 0) {
    size_t oldsz = m_proxy->Size();
    m_proxy->Allocate (oldsz + offs, false);
    char* beg = reinterpret_cast<char*>(m_proxy->At(0));
    if (pos < oldsz)
      rootType.copyRange (beg + eltsz*(pos+offs),
                          beg + eltsz*pos,
                          oldsz - pos);
    rootType.clearRange (beg + eltsz*pos, offs);
  }
}


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
bool RootAuxVector::insertMove (size_t pos, void* beg, void* end)
{
  size_t eltsz = m_proxy->GetIncrement();
  const void* orig = this->toPtr();
  const RootUtils::Type& rootType = m_factory->rootType();

  char* begp = reinterpret_cast<char*> (beg);
  char* endp = reinterpret_cast<char*> (end);
  size_t nelt = (endp-begp) / eltsz;

  shift (pos, nelt);
  // FIXME: want move, not copy.
  // But i don't seem to be able to call move operations through cling,
  // so just use copy for now.
  rootType.copyRange (reinterpret_cast<char*>(this->toPtr()) + pos*eltsz,
                      beg, nelt);
  return this->toPtr() == orig;
}


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
const std::type_info* RootAuxVector::objType() const
{
  return m_factory->objClass()->GetTypeInfo();
}


//==================================================================


/**
 * @brief Constructor.
 * @param vecClass The @c TClass for the vector object.
 */
RootAuxVectorFactory::RootAuxVectorFactory (TClass* objClass)
  : m_objClass (objClass),
    m_vecClass (objClass),
    m_offset (0),
    m_isEL (NONE)
{
  TVirtualCollectionProxy* proxy = m_vecClass->GetCollectionProxy();

  if (!proxy) {
    TClass* vecClass = lookupVectorType (objClass);
    if (vecClass) {
      m_vecClass = vecClass;
      Int_t offs = objClass->GetBaseClassOffset (vecClass);
      if (offs >= 0) {
        m_offset = offs;
        proxy = vecClass->GetCollectionProxy();
      }
      else {
        ATHCONTAINERS_ERROR("RootAuxVectorFactory::RootAuxVectorFactory",
                            std::string("Can't find vector base class in ") +
                            objClass->GetName());
      }
    }
  }

  if (!proxy) {
    std::string err = "Can't find collection proxy for ";
    err += m_vecClass->GetName();
    throw std::runtime_error (err.c_str());
  }

  if (m_vecClass->GetTypeInfo() == 0) {
    ATHCONTAINERS_ERROR("RootAuxVectorFactory::RootAuxVectorFactory",
                        std::string("No type_info available for class ") +
                        m_vecClass->GetName() +
                        std::string(".  There is probably a missing dictionary.  We will likely crash further on."));
  }

  TClass* eltClass = proxy->GetValueClass();
  if (eltClass) {
    m_type.init (eltClass);

    const std::type_info* ti = eltClass->GetTypeInfo();
    if (ti) {

      static const CxxUtils::ClassName pat1 ("ElementLink<$T>");
      static const CxxUtils::ClassName pat2 ("std::vector<ElementLink<$T> >");

      CxxUtils::ClassName clname (SG::normalizedTypeinfoName (*ti));
      CxxUtils::ClassName::match_t matches;
      if (clname.match (pat1, matches)) {
        m_isEL = ELEMENT_LINK;
        if (eltClass->GetBaseClass ("ElementLinkBase") == nullptr) {
          m_isEL = ELEMENT_LINK_NONPOINTER;
        }
      }
      else if (clname.match (pat2, matches)) {
        m_isEL = ELEMENT_LINK_VECTOR;

        TVirtualCollectionProxy* proxy2 = eltClass->GetCollectionProxy();
        if (proxy2) {
          TClass* innerEltClass = proxy2->GetValueClass();
          if (innerEltClass) {
            if (innerEltClass->GetBaseClass ("ElementLinkBase") == nullptr) {
              m_isEL = ELEMENT_LINK_NONPOINTER;
            }
          }
        }
      }
    }
  }
  else
    m_type.init (proxy->GetType());
}


/**
 * @brief Destructor.
 */
RootAuxVectorFactory::~RootAuxVectorFactory()
{
}


/**
 * @brief Create a vector object of this type.
 * @param auxid ID for the variable being created.
 * @param size Initial size of the new vector.
 * @param capacity Initial capacity of the new vector.
 */
std::unique_ptr<SG::IAuxTypeVector>
RootAuxVectorFactory::create (SG::auxid_t auxid,
                              size_t size,
                              size_t capacity) const
{
  return std::make_unique<RootAuxVector> (this, auxid, size, capacity);
}


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
std::unique_ptr<SG::IAuxTypeVector>
RootAuxVectorFactory::createFromData (SG::auxid_t auxid,
                                      void* data,
                                      bool isPacked,
                                      bool ownFlag) const
{
  return std::make_unique<RootAuxVector> (this, auxid, data, isPacked, ownFlag);
}


/// Helper for copy; returns a pointer to the first destination object,
/// or nullptr if the destination was cleared rather than copied.
char* RootAuxVectorFactory::copyImpl (SG::auxid_t auxid,
                                      AuxVectorData& dst,
                                      size_t dst_index,
                                      const AuxVectorData& src,
                                      size_t src_index,
                                      size_t n) const
{
  size_t eltsz = m_type.getSize();
  char* dstptr = reinterpret_cast<char*> (dst.getDataArray (auxid));
  if (&src == &dst) {
    // Source and destination containers are the same,
    // so we don't need to bother with fetching the src pointer.
    // copyRange properly handles overlapping regions.
    m_type.copyRange (dstptr + eltsz*dst_index, dstptr + eltsz*src_index, n);
    return dstptr + eltsz*dst_index;
  }
  else {
    const char* srcptr = reinterpret_cast<const char*>(src.getDataArrayAllowMissing (auxid));
    if (srcptr) {
      m_type.copyRange (dstptr + eltsz*dst_index, srcptr + eltsz*src_index, n);
      return dstptr + eltsz*dst_index;
    }
    else {
      m_type.clearRange (dstptr + eltsz*dst_index, n);
      return nullptr;
    }
  }
}


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
void RootAuxVectorFactory::copy (SG::auxid_t auxid,
                                 AuxVectorData& dst,
                                 size_t dst_index,
                                 const AuxVectorData& src,
                                 size_t src_index,
                                 size_t n) const
{
  (void)copyImpl (auxid, dst, dst_index, src, src_index, n);
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
void RootAuxVectorFactory::copyForOutput (SG::auxid_t auxid,
                                          AuxVectorData& dst,        size_t dst_index,
                                          const AuxVectorData& src,  size_t src_index,
                                          size_t n) const
{
  char* dstptr = copyImpl (auxid, dst, dst_index, src, src_index, n);

  if (m_isEL == ELEMENT_LINK) {
    size_t eltsz = m_type.getSize();
    for (size_t i = 0; i < n; i++) {
      reinterpret_cast<ElementLinkBase*>(dstptr + i*eltsz)->thin();
  }
  }
  else if (m_isEL == ELEMENT_LINK_VECTOR) {
    size_t eltsz = m_type.getSize();
    for (size_t i = 0; i < n; i++) {
      std::vector<ElementLinkBase>& v = 
        *reinterpret_cast<std::vector<ElementLinkBase>* > (dstptr +i*eltsz);
      for (ElementLinkBase& el : v) {
        el.thin();
      }
    }
  }
  else if (m_isEL == ELEMENT_LINK_NONPOINTER) {
    ATHCONTAINERS_ERROR("RootAuxVectorFactory::copyForOutput",
                        std::string("Cannot apply thinning for ElementLink with non-pointer element: ") +
                        m_vecClass->GetName());
  }
}


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
void RootAuxVectorFactory::swap (SG::auxid_t auxid,
                                 AuxVectorData& a, size_t aindex,
                                 AuxVectorData& b, size_t bindex,
                                 size_t n) const
{
  void* aptr = a.getDataArray (auxid);
  void* bptr = &a == &b ? aptr : b.getDataArray (auxid);
  m_type.swapRange (aptr, aindex, bptr, bindex, n);
}


/**
 * @brief Clear a range of elements within a vector.
 * @param auxid The aux data item being operated on.
 * @param dst Container holding the element
 * @param dst_index Index of the first element in the vector.
 * @param n Number of elements to clear.
 */
void RootAuxVectorFactory::clear (SG::auxid_t auxid,
                                  AuxVectorData& dst, size_t dst_index,
                                  size_t n) const
{
  m_type.clearRange (dst.getDataArray (auxid), dst_index, n);
}


/**
 * @brief Return the size of an element of this vector type.
 */
size_t RootAuxVectorFactory::getEltSize() const
{
  return m_type.getSize();
}


/**
 * @brief Return the @c type_info of the overall object.
 */
const std::type_info* RootAuxVectorFactory::tiVec() const
{
  return m_objClass->GetTypeInfo();
}


/**
 * @brief True if the vectors created by this factory work by dynamic
 *        emulation (via @c TVirtualCollectionProxy or similar); false
 *        if the std::vector code is used directly.
 */
bool RootAuxVectorFactory::isDynamic() const
{
  return true;
}


/**
 * @brief Return the @c type_info of the vector allocator.
 *
 * May be nullptr for a dynamic vector.
 */
const std::type_info* RootAuxVectorFactory::tiAlloc() const
{
  return nullptr;
}


/**
 * @brief Return the (demangled) name of the vector allocator.
 */
std::string RootAuxVectorFactory::tiAllocName() const
{
  std::string name = SG::normalizedTypeinfoName (*m_vecClass->GetTypeInfo());
  CxxUtils::ClassName cn (name);
  std::string alloc_name;
  if (cn.ntargs() >= 2) {
    alloc_name = cn.targ(1).fullName();
  }
  else if (cn.ntargs() == 1) {
    alloc_name = "std::allocator<" + cn.targ(0).fullName();
    if (alloc_name[alloc_name.size()-1] == '>') alloc_name += " ";
    alloc_name += ">";
  }
  return alloc_name;
}


} // namespace SG
