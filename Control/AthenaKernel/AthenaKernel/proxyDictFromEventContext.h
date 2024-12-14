// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthenaKernel/proxyDictFromEventContext.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Nov, 2024
 * @brief Some out-of-line helpers for ExtendedEventContext.
 *
 * Out-of-line functions for retrieving IProxySict/SourceID
 * from an EventContext.  Keeping these out-of-line allows to reduce header
 * dependencies on ExtendedEventContext.
 */


#ifndef ATHENAKERNEL_PROXYDICTFROMEVENTCONTEXT_H
#define ATHENAKERNEL_PROXYDICTFROMEVENTCONTEXT_H


#include "AthenaKernel/SourceID.h"
class IProxyDict;
class EventContext;


namespace Atlas {


/**
 * @brief Return the @c IProxyDict for this thread's current context.
 */
IProxyDict* proxyDictFromEventContext();


/**
 * @brief Return the @c IProxyDict for a context.
 * @param ctx The context.
 */
IProxyDict* proxyDictFromEventContext (const EventContext& ctx);


/**
 * @brief Return the @c SourceID for this thread's current context.
 */
SG::SourceID sourceIDFromEventContext();


/**
 * @brief Return the @c SourceID a context.
 * @param ctx The context.
 */
SG::SourceID sourceIDFromEventContext (const EventContext& ctx);


}

#endif // not ATHENAKERNEL_PROXYDICTFROMEVENTCONTEXT_H
