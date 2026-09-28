/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthenaKernel/src/TypelessDataBucket.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2026
 * @brief DataBucket holding the object as an untyped pointer.
 */


#include "AthenaKernel/TypelessDataBucket.h"


/**
 * @brief Constructor.
 * @param object The object to be held.  The DataBucket takes ownership.
 */
TypelessDataBucket::TypelessDataBucket (void* object,
                                        const SG::BaseInfoBase& bib)
  : m_object (object), m_bib  (bib), m_clid (bib.clid())
{
}


/**
 * @brief Destructor.  Will destroy the held object.
 */
TypelessDataBucket::~TypelessDataBucket()
{
  if (m_object) {
    m_bib.destroy (m_object);
  }
}


/**
 * @brief Return the CLID.
 */
const CLID& TypelessDataBucket::clID() const
{
  return m_clid;
}


/**
 * @brief Return the pointer as a void*.
 */
void* TypelessDataBucket::object()
{
  return m_object;
}


/**
 * @brief Return the @c type_info for the stored object.
 */
const std::type_info& TypelessDataBucket::tinfo() const
{
  return m_bib.typeinfo();
}


/**
 * @brief Return the contents of the @c DataBucket,
 *        converted to type given by @a clid.  Note that only
 *        derived->base conversions are allowed here.
 * @param clid The class ID to which to convert.
 * @param irt To be called if we make a new instance.
 * @param isConst True if the object being converted is regarded as const.
 */
void* TypelessDataBucket::cast (CLID clid,
                                SG::IRegisterTransient* /*irt = 0*/,
                                bool isConst /*= true*/)
{
  if (!isConst) return nullptr;
  if (clid == m_clid) {
    return m_object;
  }

  return m_bib.cast (m_object, clid);
}


/**
 * @brief Return the contents of the @c DataBucket,
 *        converted to type given by @a std::type_info.  Note that only
 *        derived->base conversions are allowed here.
 * @param tinfo The @a std::type_info of the type to which to convert.
 * @param irt To be called if we make a new instance.
 * @param isConst True if the object being converted is regarded as const.
 */
void* TypelessDataBucket::cast (const std::type_info& tinfo,
                                SG::IRegisterTransient* /*irt = 0*/,
                                bool isConst /*= true*/)
{
  if (!isConst) return nullptr;
  return m_bib.cast (m_object, tinfo);
}


/**
 * @brief Give up ownership of the  @c DataBucket contents.
 *        This leaks the contents and it is useful mainly for error handling.
 */
void TypelessDataBucket::relinquish()
{
  m_object = nullptr;
}


/**
 * If the held object derives from @c ILockable, call @c lock() on it.
 *
 * Not implemented for this type.
 */
void TypelessDataBucket::lock()
{
  // Not implemented.
}
