/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthenaKernel/src/proxyDictFromEventContext.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2024
 * @brief Some out-of-line helpers for ExtendedEventContext.
 *
 * Out-of-line functions for retrieving IProxySict/SourceID
 * from an EventContext.  Keeping these out-of-line allows to reduce header
 * dependencies on ExtendedEventContext.
 */


#include "AthenaKernel/proxyDictFromEventContext.h"
#include "AthenaKernel/ExtendedEventContext.h"
#include "AthenaKernel/IProxyDict.h"
#include "GaudiKernel/ThreadLocalContext.h"


namespace Atlas {


/**
 * @brief Return the @c IProxyDict for this thread's current context.
 */
IProxyDict* proxyDictFromEventContext ()
{
  const EventContext& ctx = Gaudi::Hive::currentContext();
  return Atlas::getExtendedEventContext(ctx).proxy();
}


/**
 * @brief Return the @c IProxyDict for a context.
 * @param ctx The context.
 */
IProxyDict* proxyDictFromEventContext (const EventContext& ctx)
{
  return Atlas::getExtendedEventContext(ctx).proxy();
}


/**
 * @brief Return the @c SourceID for this thread's current context.
 */
SG::SourceID sourceIDFromEventContext ()
{
  const EventContext& ctx = Gaudi::Hive::currentContext();
  return Atlas::getExtendedEventContext(ctx).proxy()->sourceID();
}


/**
 * @brief Return the @c SourceID a context.
 * @param ctx The context.
 */
SG::SourceID sourceIDFromEventContext (const EventContext& ctx)
{
  return Atlas::getExtendedEventContext(ctx).proxy()->sourceID();
}


} // namespace Atlas
