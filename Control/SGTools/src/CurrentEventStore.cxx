/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file SGTools/src/CurrentEventStore.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Apr, 2015
 * @brief Hold a pointer to the current event store.
 */


#include "SGTools/CurrentEventStore.h"
#include "AthenaKernel/ExtendedEventContext.h"
#include "GaudiKernel/ThreadLocalContext.h"


namespace SG {


thread_local IProxyDict* CurrentEventStore::m_curStore = nullptr;


/**
 * @brief Set the current store.
 * Returns the previous store.
 */
IProxyDict* CurrentEventStore::setStore (IProxyDict* store)
{
  IProxyDict* oldstore = m_curStore;
  m_curStore = store;
  return oldstore;
}


/**
 * @brief Update the current @c EventContext to reference @c store.
 * @param store The new store to set in the current context.
 */
void CurrentEventStore::storeToCtx (IProxyDict* store)
{
  EventContext ctx = Gaudi::Hive::currentContext();
  Atlas::ExtendedEventContext* ectx = Atlas::tryGetExtendedEventContext(ctx);
  if (ectx) {
    ectx->setProxy (store);
    Gaudi::Hive::setCurrentContext (ctx);
  }
}


} // namespace SG
