// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthenaKernel/DataObjectSharedPtr.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Feb, 2016
 * @brief Smart pointer to manage @c DataObject reference counts.
 */


#ifndef ATHENAKERNEL_DATAOBJECTSHAREDPTR_H
#define ATHENAKERNEL_DATAOBJECTSHAREDPTR_H


#include "AthenaKernel/StorableConversions.h"
#include "GaudiKernel/DataObject.h"
#include "CxxUtils/RefCountedPtr.h"
#include <memory>


namespace SG {


template <CxxUtils::detail::RefCounted T>
using DataObjectSharedPtr = CxxUtils::RefCountedPtr<T>;


template <typename T>
DataObject* asStorable(SG::DataObjectSharedPtr<T> pObject) {
  typedef typename DataBucketTrait<T>::type bucket_t;
  return new bucket_t (pObject.get());
}  


} // namespace SG


#endif // not ATHENAKERNEL_DATAOBJECTSHAREDPTR_H
