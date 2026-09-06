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
 * @brief Temporarily change the current event store.
 */
CurrentEventStore::Push::Push (IProxyDict* store)
  : m_oldStore (setStore (store))
{
  // Only need up update the context if we're actually changing the store.
  if (m_oldStore != store) {
    EventContext ctx = Gaudi::Hive::currentContext();
    m_oldCtx = ctx;
    Atlas::ExtendedEventContext* ectx = Atlas::tryGetExtendedEventContext(ctx);
    if (ectx) {
      ectx->setProxy (store);
      Gaudi::Hive::setCurrentContext (ctx);
    }
  }
}


/**
 * @brief Restore the current event store.
 */
CurrentEventStore::Push::~Push()
{
  // Only need up update the context if we're actually changing the store.
  if (m_oldStore != m_curStore) {
    Gaudi::Hive::setCurrentContext (m_oldCtx);
  }
  setStore (m_oldStore);
}


} // namespace SG
