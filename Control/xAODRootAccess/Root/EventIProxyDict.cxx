// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// File holding the implementation of the xAOD::Event functions that implement
// the IProxyDict interface.
//

// Local include(s).
#include "xAODRootAccess/Event.h"
#include "xAODRootAccess/tools/IObjectManager.h"
#include "xAODRootAccess/tools/Message.h"
#include "xAODRootAccess/tools/THolder.h"

// Project include(s).
#ifndef XAOD_STANDALONE
#include "AthenaKernel/DataBucketBase.h"
#include "GaudiKernel/Converter.h"
#include "GaudiKernel/GenericAddress.h"
#include "SGTools/DataProxy.h"
#include "SGTools/TransientAddress.h"
#endif  // NOT XAOD_STANDALONE
#include "AthContainers/tools/threading.h"
#include "CxxUtils/checker_macros.h"

// ROOT include(s):
#include <TBranch.h>
#include <TClass.h>
#include <TString.h>

// System include(s):
#include <set>
#include <stdexcept>

/// Mutex type for multithread synchronization
using mutex_t = AthContainers_detail::mutex;
/// Guard type for multithreaded synchronisation
using guard_t = AthContainers_detail::lock_guard<mutex_t>;

#ifndef XAOD_STANDALONE
namespace xAODPrivate {

/// Helper object for holding something through a THolder
class HolderBucket : public DataBucketBase {

 public:
  /// Constructor with an existing holder
  HolderBucket(const std::string& key, const std::type_info& ti,
               xAOD::Event& event)
      : m_key(key), m_ti(ti), m_event(event) {}

  /// Return the object held
  void* object() override {

    // Look among the output objects first:
    static constexpr bool METADATA = false;
    const void* result = m_event.getOutputObject(m_key, m_ti, METADATA);
    // Check if it succeeded:
    static constexpr bool SILENT = false;
    if (!result) {
      // Try the input then:
      result = m_event.getInputObject(m_key, m_ti, SILENT, METADATA);
    } else {
      // Even if there is an output object with this key, it may
      // be an input object that was copied to the output. So let's
      // try to silently retrieve an input object as well. This makes
      // sure that the input/output object is updated for the current
      // event.
      static constexpr bool SILENT = false;
      m_event.getInputObject(m_key, m_ti, SILENT, METADATA);
    }

    // Return the pointer:
    void* nc_result ATLAS_THREAD_SAFE =
        const_cast<void*>(result);  // DataBucketBase interface
    return nc_result;
  }

  /// Return the type_info of the stored object
  const std::type_info& tinfo() const override { return m_ti; }

  /// Return the object, cast to a CLID's type
  void* cast(CLID, SG::IRegisterTransient*, bool) override {

    throw std::runtime_error("xAODPrivate::HolderBucket::cast not implemented");
    return 0;
  }

  /// Return the object, cast to a certain type
  void* cast(const std::type_info& tinfo, SG::IRegisterTransient*,
             bool isConst) override {

    // Do the cast:
    static constexpr bool SILENT = true;
    static constexpr bool METADATA = true;
    const void* result = 0;
    if (isConst == false) {
      // Just look among the outputs.
      result = m_event.getOutputObject(m_key, tinfo, METADATA);
    } else {
      // Look among the output objects first:
      result = m_event.getOutputObject(m_key, tinfo, METADATA);
      // Check if it succeeded:
      if (!result) {
        // Try the input then:
        result = m_event.getInputObject(m_key, tinfo, SILENT, METADATA);
      } else {
        // Even if there is an output object with this key, it may
        // be an input object that was copied to the output. So let's
        // try to silently retrieve an input object as well. This makes
        // sure that the input/output object is updated for the current
        // event.
        m_event.getInputObject(m_key, tinfo, SILENT, METADATA);
      }
    }

    // Return the pointer:
    void* nc_result ATLAS_THREAD_SAFE =
        const_cast<void*>(result);  // DataBucketBase interface
    return nc_result;
  }

  /// Return the object, cast to type.
  void* cast(CLID, const std::type_info& tinfo, SG::IRegisterTransient* irt,
             bool isConst) override {
    return HolderBucket::cast(tinfo, irt, isConst);
  }

  /// Give up ownership of the bucket's contents. A no-op.
  void relinquish() override {}
  /// Lock the held object. A no-op.
  void lock() override {}

 private:
  /// The key of the object
  const std::string m_key;
  /// The type info of the object
  const std::type_info& m_ti;
  /// The original Event object
  xAOD::Event& m_event;

};  // class HolderBucket

class Loader : public Converter {
 public:
  Loader(xAOD::Event& event, const std::string& name, const std::type_info& ti)
      : Converter(0, CLID_NULL),
        m_event(event),
        m_name(name),
        m_ti(ti),
        m_proxy(nullptr) {}

  void setProxy(SG::DataProxy& proxy) { m_proxy = &proxy; }

  virtual StatusCode createObj(IOpaqueAddress*, DataObject*& obj) override {
    static const bool SILENT = true;
    static const bool METADATA = false;

    // Try to find the object amongst the output objects first:
    if (m_event.getOutputObject(m_name, m_ti, METADATA)) {
      obj = new xAODPrivate::HolderBucket(m_name, m_ti, m_event);
    }
    // If it's not on the output, try the input:
    else if (m_event.getInputObject(m_name, m_ti, SILENT, METADATA)) {
      obj = new xAODPrivate::HolderBucket(m_name, m_ti, m_event);
      m_proxy->setConst();
    }

    return StatusCode::SUCCESS;
  }
  virtual long repSvcType() const override { return 0; }

 private:
  xAOD::Event& m_event;
  std::string m_name;
  const std::type_info& m_ti;
  SG::DataProxy* m_proxy;
};  // class Loader

}  // namespace xAODPrivate
#endif  // not XAOD_STANDALONE

namespace xAOD {

SG::DataProxy* Event::proxy(const void* const pTransient) const {

  // Look up the name of this object
  std::string name = getName(pTransient);
  if (name.empty()) {
    // Apparently the object is not known...
    return nullptr;
  }

  // Get the metadata object for it:
  const xAOD::EventFormatElement* efe = 0;
  static const bool QUIET = true;
  if (m_outputEventFormat) {
    efe = m_outputEventFormat->get(name, QUIET);
  }
  if (!efe) {
    efe = m_inputEventFormat.get(name, QUIET);
  }
  if (!efe) {
    // No metadata found...
    return nullptr;
  }

  // Return the proxy:
  const BranchInfo* bi = getBranchInfo(efe->hash());
  return bi->m_proxy.get();
}

SG::DataProxy* Event::proxy(const CLID&, const std::string& key) const {

  const SG::sgkey_t sgkey = getHash(key);
  if (!sgkey) {
    return 0;
  }
  return proxy_exact(sgkey);
}

SG::DataProxy* Event::proxy_exact(SG::sgkey_t sgkey) const {

  // Get the object describing this branch/object:
  const BranchInfo* bi = getBranchInfo(sgkey);
  if (!bi) {
    static SG::SGKeySet missingSGKeys ATLAS_THREAD_SAFE;
    static mutex_t mutex;
    guard_t lock(mutex);
    if (missingSGKeys.emplace(sgkey).second && m_printEventProxyWarnings) {
      ATH_MSG_WARNING("Can't find BranchInfo for: " << sgkey);
    }
    return 0;
  }

  // Access its data proxy:
  SG::DataProxy* proxy = bi->m_proxy.get();

  // Return the proxy:
  return proxy;
}

/// Smart pointers to objects that don't exist in the input event, can end
/// up calling this function. In this case warn the user that something
/// fishy is happening, and take posession of the received proxy.
///
/// @param clid The CLID of the type. Not taken into account.
/// @param proxy The proxy to take posession of. Not used for anything
///              useful.
///
StatusCode Event::addToStore(CLID clid, SG::DataProxy* proxy) {

  upgrading_lock_t lock(m_branchesMutex);

  // Warn the user that the function got called:
  static std::atomic_flag warningPrinted ATLAS_THREAD_SAFE = ATOMIC_FLAG_INIT;
  if (!warningPrinted.test_and_set() && m_printEventProxyWarnings) {
    ATH_MSG_WARNING(
        "Function should only be called through an invalid ElementLink");
  }

  // Hold on to the proxy with some non-existent, hopefully unique key:
  const ::TString uniqueKey =
      ::TString::Format("NonExistentKey_%lu", m_branches.size());
  BranchInfo bi;
  bi.m_proxy.reset(proxy);
  lock.upgrade();
  m_branches.insert(
      std::make_pair(stringToKey(uniqueKey.Data(), clid), std::move(bi)));

  // Return gracefully:
  return StatusCode::SUCCESS;
}

std::vector<const SG::DataProxy*> Event::proxies() const {

  upgrading_lock_t lock(m_branchesMutex);

  std::vector<const SG::DataProxy*> ret;
  for (const auto& p : m_branches) {
    const SG::DataProxy* proxy = p.second.m_proxy.get();
    if (proxy) {
      ret.push_back(proxy);
    }
  }
  return ret;
}

SG::sgkey_t Event::stringToKey(const std::string& str, CLID) {

  return getHash(str);
}

const std::string* Event::keyToString(SG::sgkey_t key) const {

  return &(getName(key));
}

const std::string* Event::keyToString(SG::sgkey_t key, CLID&) const {

  return &(getName(key));
}

void Event::registerKey(SG::sgkey_t, const std::string&, CLID) {

  return;
}

SG::DataProxy* Event::recordObject(SG::DataObjectSharedPtr<DataObject>,
                                   const std::string&, bool, bool) {

  throw std::runtime_error("xAOD::Event::recordObject is not implemented");
}

const std::string& Event::name() const {

  static const std::string result{"xAOD::Event"};
  return result;
}

const xAOD::EventFormatElement* Event::getEventFormatElement(
    SG::sgkey_t sgkey) const {

  const xAOD::EventFormatElement* efe = nullptr;
  static const bool QUIET = true;
  if (m_outputEventFormat) {
    efe = m_outputEventFormat->get(sgkey, QUIET);
  }
  if (!efe) {
    efe = m_inputEventFormat.get(sgkey, QUIET);
  }
  if (!efe) {
    static SG::SGKeySet missingSGKeys ATLAS_THREAD_SAFE;
    static mutex_t mutex;
    guard_t lock(mutex);
    if (missingSGKeys.emplace(sgkey).second) {
      if (m_printEventProxyWarnings) {
        ATH_MSG_WARNING(
            "Can't find EventFormatElement for hashed SG key: " << sgkey);
      }
      return 0;
    }
  }
  return efe;
}

const Event::BranchInfo* Event::getBranchInfo(SG::sgkey_t sgkey) const {

  {
    // We can only hold the lock (even though it's a shared lock) for
    // this limited scope because the call to getInputObject below
    // leads to a recursion and dead-lock if not released immediately.
    upgrading_lock_t lock(m_branchesMutex);

    // If the object already exists, return it:
    auto it = m_branches.find(sgkey);
    if (it != m_branches.end()) {
      return &(it->second);
    }
  }

  // If not, construct it now:
  BranchInfo bi;
  const xAOD::EventFormatElement* efe = getEventFormatElement(sgkey);
  if (!efe) {
    // Apparently this key is not known:
    return nullptr;
  }

  // Helper variable(s).
  static const bool SILENT = true;
  static const bool METADATA = false;

  // The name of the requested object.
  const std::string& name = getName(sgkey);
  // This is a bit perverse... In order to let the "base class" figure
  // out the exact type of this object, we ask for it with an "Event
  // pointer". I use that type because I need something that has a
  // dictionary, and which should always be available when this code
  // runs. In the end it doesn't matter that the object can't be
  // retrieved as that type (of course...), it only matters that it gets
  // "set up" following these calls.
  Event* nc_this = const_cast<Event*>(this);
  static const std::type_info& dummy = typeid(Event);
  nc_this->getInputObject(name, dummy, SILENT, METADATA);
  auto itr = m_outputObjects.find(name);
  if (itr == m_outputObjects.end()) {
    itr = m_inputObjects.find(name);
    if (itr == m_inputObjects.end()) {
      // We didn't find this object in the store...
      return nullptr;
    }
  }
  const Details::IObjectManager* mgr =
      dynamic_cast<const Details::IObjectManager*>(itr->second.get());
  if (!mgr) {
    ATH_MSG_ERROR("Internal logic error found");
    return nullptr;
  }
  bi.m_class = mgr->holder()->getClass();
  // There's no need to check whether this is a "proper" dictionary
  // at this point, since if TEvent is holding on to it, the type
  // must have a proper compiled dictionary.

#ifndef XAOD_STANDALONE
  // Create a proper proxy for the input branch:
  SG::TransientAddress* taddr = new SG::TransientAddress(
      CLID_NULL, efe->branchName(), new GenericAddress());
  taddr->setSGKey(sgkey);
  xAODPrivate::Loader* loader = new xAODPrivate::Loader(
      *nc_this, getName(sgkey), *bi.m_class->GetTypeInfo());
  bi.m_proxy.reset(new SG::DataProxy(taddr, loader));
  loader->setProxy(*bi.m_proxy.get());
#endif  // not XAOD_STANDALONE

  // Add the branch info to our list:
  upgrading_lock_t lock(m_branchesMutex);
  lock.upgrade();
  auto ret = m_branches.insert(std::make_pair(sgkey, std::move(bi)));

  // Return a pointer to the branch info:
  return &(ret.first->second);
}

}  // namespace xAOD
