// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

// Local include(s).
#include "xAODRootAccess/tools/AuxStoreBase.h"

#include "xAODRootAccess/tools/Message.h"
#include "xAODRootAccess/tools/Utils.h"

// Framework include(s).
#include "AthContainers/AuxStoreInternal.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/exceptions.h"
#include "AthContainersInterfaces/IAuxTypeVector.h"
#include "CxxUtils/as_const_ptr.h"
#include "CxxUtils/checker_macros.h"

// ROOT include(s):
#include <TError.h>

namespace xAOD::details {

AuxStoreBase::AuxStoreBase(bool topStore, EStructMode mode)
    : m_data{mode, topStore} {}

AuxStoreBase::~AuxStoreBase() = default;

auto AuxStoreBase::structMode() const -> EStructMode {

  return m_data.m_structMode;
}

void AuxStoreBase::setStructMode(EStructMode mode) {

  m_data.m_structMode = mode;
  reset();
}

const std::string& AuxStoreBase::prefix() const {

  return m_data.m_prefix;
}

bool AuxStoreBase::isTopStore() const {

  return m_data.m_topStore;
}

void AuxStoreBase::setTopStore(bool value) {

  m_data.m_topStore = value;
  reset();
}

const void* AuxStoreBase::getData(SG::auxid_t auxid) const {

  const SG::IAuxTypeVector* v = getVector(auxid);
  if (v) {
    return v->toPtr();
  }
  return nullptr;
}

const SG::IAuxTypeVector* AuxStoreBase::getVector(SG::auxid_t auxid) const {

  // Guard against multi-threaded execution:
  guard_t guard(m_mutex1);

  // Check if the transient store already handles this variable:
  if (m_data.m_transientStore &&
      (m_data.m_transientStore->getAuxIDs().test(auxid))) {
    return m_data.m_transientStore->getVector(auxid);
  }

  // Access the object through a non-const pointer. This is "safe" because
  // of the mutex lock above.
  auto this_nc ATLAS_THREAD_SAFE = const_cast<AuxStoreBase*>(this);

  // Connect this auxiliary variable both to the input and output
  // if needed:
  if ((auxid >= m_data.m_vecs.size()) || (!m_data.m_vecs[auxid])) {
    if ((!this_nc->setupInputData(auxid).isSuccess()) ||
        (!this_nc->setupOutputData(auxid).isSuccess())) {
      return nullptr;
    }
  }

  // Make sure the variable is up to date:
  if (this_nc->getEntryFor(auxid).isSuccess() == false) {
    ::Error("xAOD::AuxStoreBase::getVector",
            XAOD_MESSAGE("Couldn't read in variable %s"),
            SG::AuxTypeRegistry::instance().getName(auxid).c_str());
    return nullptr;
  }

  // Return the pointer to the object:
  return m_data.m_vecs[auxid].get();
}

const SG::auxid_set_t& AuxStoreBase::getAuxIDs() const {

  return m_data.m_auxIDs;
}

const SG::auxid_set_t& AuxStoreBase::getDecorIDs() const {

  return m_data.m_decorIDs;
}

void* AuxStoreBase::getDecoration(SG::auxid_t auxid, std::size_t size,
                                  std::size_t capacity) {

  // Guard against multi-threaded execution:
  guard_t guard(m_mutex1);

  // Remember the requested size:
  m_data.m_size = size;

  // If this is a locked object, deal with it correctly:
  if (m_locked) {
    // If the variable exists already and it's a decoration, then let's
    // give it back.
    if ((auxid < m_data.m_vecs.size()) && m_data.m_vecs[auxid] &&
        (auxid < m_data.m_isDecoration.size() &&
         m_data.m_isDecoration[auxid])) {
      // Things look okay...
      m_data.m_vecs[auxid]->reserve(capacity);
      m_data.m_vecs[auxid]->resize(size);
      return m_data.m_vecs[auxid]->toPtr();
    }
    // If it's in the transient store already, return it from there.
    // Since in a locked store *everything* is a decoration in the
    // transient store.
    if (m_data.m_transientStore &&
        m_data.m_transientStore->getAuxIDs().test(auxid)) {
      return m_data.m_transientStore->getDecoration(auxid, size, capacity);
    }
    // If we know this auxiliary ID, but it was not found as a decoration
    // by the previous checks, then we're in trouble.
    if (m_data.m_auxIDs.test(auxid)) {
      throw SG::ExcStoreLocked(auxid);
    }
  }

  // Check if we want to write this variable to the output:
  if (!(isAuxIDSelected(auxid) && hasOutput())) {

    // Create the store only when necessary:
    if (!m_data.m_transientStore) {
      m_data.m_transientStore = std::make_unique<SG::AuxStoreInternal>(
          m_data.m_structMode == EStructMode::kObjectStore);
      if (m_locked) {
        m_data.m_transientStore->lock();
      }
    }
    // Let the transient store create the decoration:
    const std::size_t nids = m_data.m_transientStore->getAuxIDs().size();
    void* result =
        m_data.m_transientStore->getDecoration(auxid, size, capacity);
    if (result && (nids != m_data.m_transientStore->getAuxIDs().size())) {
      m_data.m_auxIDs.insert(auxid);
      if (m_data.m_transientStore->isDecoration(auxid)) {
        m_data.m_decorIDs.insert(auxid);
      }
    }
    // Return the memory address from the transient store:
    return result;
  }

  // Doesn't exist yet. So let's make it:
  void* result = getData(auxid, size, capacity);
  if (m_locked) {
    // If the container is locked, remember that this is a decoration:
    if (m_data.m_isDecoration.size() <= auxid) {
      m_data.m_isDecoration.resize(auxid + 1);
    }
    m_data.m_isDecoration[auxid] = true;
    m_data.m_decorIDs.insert(auxid);
  }

  // Return the pointer made by getData(...):
  return result;
}

bool AuxStoreBase::isDecoration(SG::auxid_t auxid) const {
  if (m_locked) {
    if (auxid < m_data.m_isDecoration.size() && m_data.m_isDecoration[auxid]) {
      return true;
    }
    if (m_data.m_transientStore) {
      return m_data.m_transientStore->isDecoration(auxid);
    }
  }
  return false;
}

void AuxStoreBase::lock() {

  // Guard against multi-threaded execution:
  guard_t guard(m_mutex1);

  m_locked = true;
  if (m_data.m_transientStore) {
    m_data.m_transientStore->lock();
  }
}

bool AuxStoreBase::clearDecorations() {

  // Guard against multi-threaded execution:
  guard_t guard(m_mutex1);

  // Clear the transient decorations:
  bool anycleared = false;
  if (m_data.m_transientStore) {
    SG::auxid_set_t old_id_set = m_data.m_transientStore->getAuxIDs();

    // Clear the decorations from the transient store:
    anycleared = m_data.m_transientStore->clearDecorations();

    // Now remove ids that were cleared.
    if (anycleared) {
      old_id_set -= m_data.m_transientStore->getAuxIDs();
      // old_id_set is now the set of ids that were cleared.
      m_data.m_auxIDs -= old_id_set;
      m_data.m_decorIDs.clear();
    }
  }

  // The decorations which are going into the output file, are here to stay.
  // Removing their IDs from the internal set would just cause more problems
  // in my mind than just leaving them be.

  return anycleared;
}

/// Lock a decoration.
void AuxStoreBase::lockDecoration(SG::auxid_t auxid) {
  if (m_data.m_transientStore) {
    m_data.m_transientStore->lockDecoration(auxid);
  }
  m_data.m_decorIDs.erase(auxid);
}

std::size_t AuxStoreBase::size() const {

  // First, try to find a managed vector in the store:
  for (SG::auxid_t id : m_data.m_auxIDs) {
    // Make sure that we are still within the bounds of our vector:
    if (id >= m_data.m_vecs.size())
      break;
    // Skip non-existent or linked objects:
    if (!m_data.m_vecs[id] || m_data.m_vecs[id]->isLinked()) {
      continue;
    }
    // Ask the vector for its size:
    const std::size_t size = m_data.m_vecs[id]->size();
    // Only accept a non-zero size. Not sure why...
    if (size > 0) {
      return size;
    }
  }

  // Check if we have a transient store, and get the size from that:
  if (m_data.m_transientStore) {
    return m_data.m_transientStore->size();
  }

  // Apparently the store is empty:
  return 0;
}

const SG::IAuxTypeVector* AuxStoreBase::linkedVector(SG::auxid_t auxid) const {
  const SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t linked_id = r.linkedVariable(auxid);
  guard_t guard(m_mutex1);
  if (linked_id < m_data.m_vecs.size()) {
    return m_data.m_vecs[linked_id].get();
  }
  if (m_data.m_transientStore) {
    return CxxUtils::as_const_ptr(m_data.m_transientStore.get())
        ->linkedVector(auxid);
  }
  return nullptr;
}

SG::IAuxTypeVector* AuxStoreBase::linkedVector(SG::auxid_t auxid) {
  const SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t linked_id = r.linkedVariable(auxid);
  guard_t guard(m_mutex1);
  if (linked_id < m_data.m_vecs.size()) {
    return m_data.m_vecs[linked_id].get();
  }
  if (m_data.m_transientStore) {
    return m_data.m_transientStore->linkedVector(auxid);
  }
  return nullptr;
}

void* AuxStoreBase::getData(SG::auxid_t auxid, std::size_t size,
                            std::size_t capacity) {

  // Guard against multi-threaded execution:
  guard_t guard(m_mutex2);

  // Remember the size:
  m_data.m_size = size;

  // Check if we want to write this variable to the output:
  if (!(isAuxIDSelected(auxid) && hasOutput())) {
    // Create the store only when necessary:
    if (!m_data.m_transientStore) {
      m_data.m_transientStore = std::make_unique<SG::AuxStoreInternal>(
          m_data.m_structMode == EStructMode::kObjectStore);
      if (m_locked) {
        m_data.m_transientStore->lock();
      }
    }
    // Let the transient store create the variable:
    std::size_t nids = m_data.m_transientStore->getAuxIDs().size();
    void* result = m_data.m_transientStore->getData(auxid, size, capacity);
    if (result && (nids != m_data.m_transientStore->getAuxIDs().size())) {
      m_data.m_auxIDs.insert(auxid);
    }
    // Return the address in the transient memory:
    return result;
  }

  // If the variable exists already, and this is a locked store, then
  // we are in trouble.
  if (m_locked && (auxid < m_data.m_vecs.size()) && m_data.m_vecs[auxid]) {
    if (!((auxid < m_data.m_isDecoration.size()) &&
          m_data.m_isDecoration[auxid])) {
      throw SG::ExcStoreLocked(auxid);
    }
  }

  // Connect this auxiliary variable just to the output:
  if (setupOutputData(auxid).isFailure()) {
    ::Error("xAOD::AuxStoreBase::getData",
            XAOD_MESSAGE("Failed to set up variable %s"),
            SG::AuxTypeRegistry::instance().getName(auxid).c_str());
    return nullptr;
  }

  // Check whether things make sense:
  if ((m_data.m_structMode == EStructMode::kObjectStore) && (size != 1)) {
    ::Error("xAOD::AuxStoreBase::getData",
            XAOD_MESSAGE("Branch creation requested with:"));
    ::Error("xAOD::AuxStoreBase::getData", XAOD_MESSAGE("  name = %s"),
            SG::AuxTypeRegistry::instance().getName(auxid).c_str());
    ::Error("xAOD::AuxStoreBase::getData", XAOD_MESSAGE("  size = %i"),
            static_cast<int>(size));
    ::Error("xAOD::AuxStoreBase::getData",
            XAOD_MESSAGE("  m_structMode = EStructMode::kObjectStore"));
    return nullptr;
  }

  // Make sure the variable is of the right size:
  m_data.m_vecs[auxid]->reserve(capacity);
  m_data.m_vecs[auxid]->resize(size);

  // Return the object:
  return m_data.m_vecs[auxid]->toPtr();
}

const SG::auxid_set_t& AuxStoreBase::getWritableAuxIDs() const {

  return getAuxIDs();
}

bool AuxStoreBase::resize(std::size_t size) {

  // Guard against multi-threaded execution:
  guard_t guard(m_mutex1);

  // A sanity check:
  if ((m_data.m_structMode == EStructMode::kObjectStore) && (size != 1)) {
    ::Error("xAOD::AuxStoreBase::resize",
            XAOD_MESSAGE("size = %i for single-object store"),
            static_cast<int>(size));
    return false;
  }

  // Remember the new size:
  m_data.m_size = size;

  bool nomoves = true;
  for (auto& v : m_data.m_vecs) {
    if (v && !v->isLinked()) {
      if (!v->resize(size)) {
        nomoves = false;
      }
    }
  }
  if (m_data.m_transientStore) {
    if (!m_data.m_transientStore->resize(size)) {
      nomoves = false;
    }
  }

  return nomoves;
}

void AuxStoreBase::reserve(std::size_t size) {

  // Guard against multi-threaded execution:
  guard_t guard(m_mutex1);

  // A sanity check:
  if ((m_data.m_structMode == EStructMode::kObjectStore) && (size != 1)) {
    ::Error("xAOD::AuxStoreBase::reserve",
            XAOD_MESSAGE("size = %i for single-object store"),
            static_cast<int>(size));
    return;
  }

  for (auto& v : m_data.m_vecs) {
    if (v && !v->isLinked()) {
      v->reserve(size);
    }
  }

  if (m_data.m_transientStore) {
    m_data.m_transientStore->reserve(size);
  }
}

void AuxStoreBase::shift(std::size_t pos, std::ptrdiff_t offs) {

  // Guard against multi-threaded execution:
  guard_t guard(m_mutex1);

  // A sanity check:
  if (m_data.m_structMode == EStructMode::kObjectStore) {
    ::Error("xAOD::AuxStoreBase::shift",
            XAOD_MESSAGE("Should not have been called for single-object "
                         "store"));
    return;
  }

  // Adjust the size of the container:
  if ((static_cast<std::size_t>(std::abs(offs)) > m_data.m_size) &&
      (offs < 0)) {
    m_data.m_size = 0;
  } else {
    m_data.m_size += offs;
  }

  for (auto& v : m_data.m_vecs) {
    if (v && !v->isLinked()) {
      v->shift(pos, offs);
    }
  }

  if (m_data.m_transientStore) {
    m_data.m_transientStore->shift(pos, offs);
  }
}

bool AuxStoreBase::insertMove(std::size_t pos, SG::IAuxStore& other,
                              const SG::auxid_set_t& ignore_in) {
  // Guard against multi-threaded execution:
  guard_t guard(m_mutex1);

  // A sanity check:
  if (m_data.m_structMode == EStructMode::kObjectStore) {
    ::Error("xAOD::AuxStoreBase::insertMove",
            XAOD_MESSAGE("Should not have been called for single-object "
                         "store"));
    return false;
  }

  bool nomove = true;
  std::size_t other_size = other.size();

  SG::auxid_set_t ignore = ignore_in;

  for (SG::auxid_t id : m_data.m_auxIDs) {
    SG::IAuxTypeVector* v_dst = nullptr;
    if (id < m_data.m_vecs.size()) {
      v_dst = m_data.m_vecs[id].get();
    }
    if (v_dst && !v_dst->isLinked()) {
      ignore.insert(id);
      if (other.getData(id)) {
        void* src_ptr = other.getData(id, other_size, other_size);
        if (src_ptr) {
          if (!v_dst->insertMove(pos, src_ptr, 0, other_size, other)) {
            nomove = false;
          }
        }
      } else {
        const void* orig = v_dst->toPtr();
        v_dst->shift(pos, other_size);
        if (orig != v_dst->toPtr()) {
          nomove = false;
        }
      }
    }
  }

  if (m_data.m_transientStore) {
    if (!m_data.m_transientStore->insertMove(pos, other, ignore))
      nomove = false;
  }

  return nomove;
}

const void* AuxStoreBase::getIOData(SG::auxid_t auxid) const {

  // Guard against multi-threaded execution:
  guard_t guard(m_mutex1);

  auto this_nc ATLAS_THREAD_SAFE =
      const_cast<AuxStoreBase*>(this);  // locked above

  // If the variable is coming from the input, and is connected to already.
  if (hasEntryFor(auxid)) {
    if (!this_nc->getEntryFor(auxid).isSuccess()) {
      ::Error("xAOD::AuxStoreBase::getIOData",
              XAOD_MESSAGE("Couldn't read in variable %s"),
              SG::AuxTypeRegistry::instance().getName(auxid).c_str());
      return nullptr;
    }
    return getInputObject(auxid);
  }

  // Check if it's in the transient store:
  if (m_data.m_transientStore &&
      m_data.m_transientStore->getAuxIDs().test(auxid)) {
    return m_data.m_transientStore->getIOData(auxid);
  }

  // If not, try connecting to it now:
  if (!this_nc->setupInputData(auxid).isSuccess()) {
    // This is not actually an error condition anymore. We can end up here
    // when we decorate constant objects coming from the input file, but
    // on one event we can't set any decorations. For instance when the
    // input container is empty. In that case the object will still list
    // the auxiliary ID belonging to that decoration as being available,
    // but it really isn't.
    //
    // Later on it might be needed to tweak the logic of all of this, but
    // for now just silently returning 0 seems to do the right thing.
    return nullptr;
  }

  // Now we should know this variable:
  if (!hasEntryFor(auxid)) {
    ::Fatal("xAOD::AuxStoreBase::getIOData",
            XAOD_MESSAGE("Internal logic error detected"));
    return nullptr;
  }

  // Make sure that the right payload is in memory:
  if (!this_nc->getEntryFor(auxid).isSuccess()) {
    ::Error("xAOD::AuxStoreBase::getIOData",
            XAOD_MESSAGE("Couldn't read in variable %s"),
            SG::AuxTypeRegistry::instance().getName(auxid).c_str());
    return nullptr;
  }

  // Return the pointer.
  return getInputObject(auxid);
}

/// Return the type of the data to be stored for one aux data item
const std::type_info* AuxStoreBase::getIOType(SG::auxid_t auxid) const {

  // Guard against multi-threaded execution:
  guard_t guard(m_mutex1);

  // If the variable is connected to already:
  if (hasEntryFor(auxid)) {
    return getInputType(auxid);
  }

  // Check if it's in the transient store:
  if (m_data.m_transientStore &&
      m_data.m_transientStore->getAuxIDs().test(auxid)) {
    return m_data.m_transientStore->getIOType(auxid);
  }

  // If not, try connecting to it now:
  auto this_nc ATLAS_THREAD_SAFE =
      const_cast<AuxStoreBase*>(this);  // locked above
  if (!this_nc->setupInputData(auxid).isSuccess()) {
    ::Error("xAOD::AuxStoreBase::getIOType",
            XAOD_MESSAGE("Couldn't connect to auxiliary variable "
                         "%i %s"),
            static_cast<int>(auxid),
            SG::AuxTypeRegistry::instance().getName(auxid).c_str());
    return nullptr;
  }

  // Now we should know this variable:
  if (!hasEntryFor(auxid)) {
    ::Fatal("xAOD::AuxStoreBase::getIOType",
            XAOD_MESSAGE("Internal logic error detected"));
    return nullptr;
  }

  // Return the type info:
  return getInputType(auxid);
}

const SG::auxid_set_t& AuxStoreBase::getDynamicAuxIDs() const {

  // All the auxiliary decorations handled by this object are considered
  // dynamic:
  return getAuxIDs();
}

/// Select dynamic auxiliary attributes for writing
void AuxStoreBase::selectAux(const std::set<std::string>& attributes) {

  guard_t guard(m_mutex1);
  m_selection.selectAux(attributes);
}

/// Get the IDs of the selected aux variables
SG::auxid_set_t AuxStoreBase::getSelectedAuxIDs() const {

  // Guard against multi-threaded execution:
  guard_t guard(m_mutex1);
  // Leave the calculation up to the internal object:
  return m_selection.getSelectedAuxIDs(m_data.m_auxIDs);
}

/// This is a tricky one. The function can't just rely on getSelectedAuxIDs,
/// as the aux ID received here may be a new ID that the object doesn't yet
/// know about. So we have no other choice but to check this ID explicitly.
///
/// @apram auxid The auxiliary ID that should be checked
/// @returns @c true if the variable needs to be written out,
///          @c false if not
///
bool AuxStoreBase::isAuxIDSelected(SG::auxid_t auxid) const {

  // A temporary object:
  SG::auxid_set_t auxids;
  auxids.insert(auxid);

  // Check if the auxid is returned as a selected ID:
  return m_selection.getSelectedAuxIDs(auxids).size();
}

}  // namespace xAOD::details
