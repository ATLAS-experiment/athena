// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file SGTools/DataProxy_cast.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2024
 * @brief Helpers for retrieving the payload held by a DataProxy.
 *
 * Broken out from DataProxy.h in order to limit header dependencies.
 */


#ifndef SGTOOLS_DATAPROXY_CAST_H
#define SGTOOLS_DATAPROXY_CAST_H


#include "AthenaKernel/StorableConversions.h"
#include "SGTools/exceptions.h"
#include <typeinfo>


namespace SG {


class DataProxy;


  ///cast the proxy into the concrete data object it proxies
  //@{
  template<typename DATA>
  DATA* DataProxy_cast(DataProxy* proxy);

  ///const ref version of the cast. @throws SG::ExcBadDataProxyCast.
  template<typename DATA>
  DATA DataProxy_cast(const DataProxy& proxy);

  /**
   * @brief Try to get the pointer back from a @a DataProxy,
   *        converted to be of type @a clid.
   * @param proxy The @a DataProxy.
   * @param clid The ID of the class to which to convert.
   *
   * Only works if the held object is a @a DataBucket.
   * Returns nullptr on failure,
   */
  void* DataProxy_cast(DataProxy* proxy, CLID clid, const std::type_info* tinfo = nullptr); /// use optional>
  //@}


template <typename DATA>
DATA* DataProxy_cast(SG::DataProxy* proxy) {
  typedef typename std::remove_const<DATA>::type DATA_nc;
  DataBucketTrait<DATA_nc>::init();

  return static_cast<DATA*>(DataProxy_cast (proxy, ClassID_traits<DATA>::ID(),
                                            &typeid(DATA_nc)));
}


///const ref version of the cast. @throws SG::ExcBadDataProxyCast.
template<typename DATA>
DATA DataProxy_cast(const SG::DataProxy& proxy)
{
  const DATA* result = SG::DataProxy_cast<DATA>(&proxy);
  if (!result) SG::throwExcBadDataProxyCast(proxy, typeid(DATA));
  return *result;
}


} // namespace SG


#endif // not SGTOOLS_DATAPROXY_CAST_H
