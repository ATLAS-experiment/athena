/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s):
#include "xAODRootAccess/tools/RObjectManager.h"

#include "xAODRootAccess/tools/Message.h"
#include "xAODRootAccess/tools/THolder.h"

// ROOT include(s).
#include <TError.h>

namespace xAOD::Experimental {

RObjectManager::RObjectManager(ROOT::RNTupleView<void> field,
                               const ::Long64_t& entry,
                               std::unique_ptr<THolder> holder)
    : IObjectManager(std::move(holder)),
      m_field(std::move(field)),
      m_entryToLoad(entry),
      m_entry(-1),
      m_isSet(kTRUE) {}

RObjectManager::~RObjectManager() = default;

ROOT::RNTupleView<void>& RObjectManager::field() {

  return m_field;
}

const ROOT::RNTupleView<void>& RObjectManager::field() const {

  return m_field;
}

/// This function is used to load the contents of a field only when it
/// needs to be done. It keeps track of which entry was already loaded for
/// a field/object, and only asks the RNTupleView for the field to load an
/// entry when it really has to be done. The next entry to load (m_entryToLoad)
/// is managed by the owning Event object, set in this object's constructor.
///
/// @return 0 if no new entry was read, the number of read bytes otherwise
///
::Int_t RObjectManager::getEntry(::Int_t) {

  // Must be valid entry value
  if (m_entryToLoad.get() < 0) {
    // Raise error as a negative entry is incorrect
    Error("xAOD::RObjectManager::getEntry",
          XAOD_MESSAGE(
              "Entry to read must be larger than or equal to 0. entry=%lld"),
          m_entryToLoad.get());
    return -1;
  }

  // Check if anything needs to be done:
  if (m_entryToLoad == m_entry) {
    return 0;
  }

  // Load the entry.
  m_field(m_entryToLoad);

  // If successful, save entry number
  m_entry = m_entryToLoad;

  // We don't know how to get the number of bytes read, so we just return 1.
  return 1;
}

/// This function gives an easy access to the object managed by this
/// object.
///
/// @return A typeless pointer to the object being managed
///
const void* RObjectManager::object() const {

  return holder()->get();
}

void* RObjectManager::object() {

  return holder()->get();
}

/// This is just a convenient way of calling THolder::Set from TEvent.
///
/// @param obj The object to replace the previously managed one
///
void RObjectManager::setObject(void* obj) {

  holder()->set(obj);
  m_isSet = kTRUE;
  return;
}

/// Dummy implementation as full objects can't be missing
///
::Bool_t RObjectManager::create() {

  return m_isSet;
}

/// @returns <code>kTRUE</code> if the object for this event was set,
///          <code>kFALSE</code> otherwise
///
::Bool_t RObjectManager::isSet() const {

  return m_isSet;
}

/// This function needs to be called after an event was filled into
/// the output TTree. It tells the manager object that it needs to wait
/// for another object to be set up for the upcoming event.
///
void RObjectManager::reset() {

  m_isSet = kFALSE;
  return;
}

}  // namespace xAOD::Experimental
