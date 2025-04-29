// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

// Local include(s):
#include "xAODRootAccess/tools/RAuxManager.h"

#include "xAODRootAccess/RAuxStore.h"


namespace xAOD {

RAuxManager::RAuxManager(RAuxStore* store, ::Long64_t& entry,
                         ::Bool_t sharedOwner)
    : m_store(), m_storePtr(store), m_entry(entry) {

  if (sharedOwner) {
    m_store.reset(store);
  }
}

::Int_t RAuxManager::getEntry(::Int_t getall) {

  if (m_storePtr->getEntry(m_entry.get(), getall).isFailure()) {
    return -1;
  } else {
    return 0;
  }
}

const void* RAuxManager::object() const {

  return m_storePtr;
}

void* RAuxManager::object() {

  return m_storePtr;
}

void RAuxManager::setObject(void* ptr) {

  if (m_store.get()) {
    m_store.reset(reinterpret_cast<RAuxStore*>(ptr));
  }
  m_storePtr = reinterpret_cast<RAuxStore*>(ptr);

  return;
}

/// There is no need for a default object.
///
::Bool_t RAuxManager::create() {

  return kTRUE;
}

/// The state of a RAuxStore object is always "set". So this
/// interface unfortunately doesn't make much sense for this
/// manager class...
///
::Bool_t RAuxManager::isSet() const {

  return kTRUE;
}

/// Resetting an auxiliary store needs to be done in a smart way.
/// Container stores need to be emptied, while object stores don't need
/// to be touched.
///
void RAuxManager::reset() {

  // Clear the store if it's a container store:
  if (m_storePtr->structMode() == RAuxStore::EStructMode::kContainerStore) {
    m_storePtr->resize(0);
    m_storePtr->reset();
  }

  return;
}

RAuxStore* RAuxManager::getStore() {

  return m_storePtr;
}

const SG::IConstAuxStore* RAuxManager::getConstStore() const {

  return m_storePtr;
}

}  // namespace xAOD
