// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthenaKernel/TypelessDataBucket.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2026
 * @brief DataBucket holding the object as an untyped pointer.
 */


#ifndef ATHENAKERNEL_TYPELESSDATABUCKET_H
#define ATHENAKERNEL_TYPELESSDATABUCKET_H


#include "AthenaKernel/DataBucketBase.h"
#include "AthenaKernel/BaseInfo.h"


/**
 * @brief DataBucket holding the object as an untyped (void*) pointer.
 *        Type information is provided via a BaseInfoBase object.
 */
class TypelessDataBucket
  : public DataBucketBase
{
public:
  /**
   * @brief Constructor.
   * @param object The object to be held.  The DataBucket takes ownership.
   */
  TypelessDataBucket (void* object, const SG::BaseInfoBase& bib);


  /**
   * @brief Destructor.  Will destroy the held object.
   */
  ~TypelessDataBucket();


  /**
   * @brief Return the CLID.
   */
  virtual const CLID& clID() const override;


  /**
   * @brief Return the pointer as a void*.
   */
  virtual void* object() override;


  /**
   * @brief Return the @c type_info for the stored object.
   */
  virtual const std::type_info& tinfo() const override;


  /// Make visible the template version from the base class.
  using DataBucketBase::cast;


  /**
   * @brief Return the contents of the @c DataBucket,
   *        converted to type given by @a clid.  Note that only
   *        derived->base conversions are allowed here.
   * @param clid The class ID to which to convert.
   * @param irt To be called if we make a new instance.
   * @param isConst True if the object being converted is regarded as const.
   */
  virtual void* cast (CLID clid,
                      SG::IRegisterTransient* irt = 0,
                      bool isConst = true) override;


  /**
   * @brief Return the contents of the @c DataBucket,
   *        converted to type given by @a std::type_info.  Note that only
   *        derived->base conversions are allowed here.
   * @param tinfo The @a std::type_info of the type to which to convert.
   * @param irt To be called if we make a new instance.
   * @param isConst True if the object being converted is regarded as const.
   */
  virtual void* cast (const std::type_info& tinfo,
                      SG::IRegisterTransient* irt = 0,
                      bool isConst = true) override;


  /**
   * @brief Give up ownership of the  @c DataBucket contents.
   *        This leaks the contents and it is useful mainly for error handling.
   */
  virtual void relinquish() override;


  /**
   * If the held object derives from @c ILockable, call @c lock() on it.
   *
   * Not implemented for this type.
   */
  virtual void lock() override;


private:
  /// The held object.
  void* m_object;

  /// Type information for the held object.
  const SG::BaseInfoBase& m_bib;

  /// CLID of the held object.
  CLID m_clid;
};


#endif // not ATHENAKERNEL_TYPELESSDATABUCKET_H
