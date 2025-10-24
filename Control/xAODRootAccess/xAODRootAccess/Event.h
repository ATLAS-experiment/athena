// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#ifndef XAODROOTACCESS_EVENT_H
#define XAODROOTACCESS_EVENT_H

// Local include(s):
#include "xAODRootAccess/tools/IProxyDict.h"

// Project include(s):
#include "AsgMessaging/AsgMessaging.h"
#include "AsgMessaging/StatusCode.h"
#include "AthContainers/tools/threading.h"
#include "AthContainers/tools/upgrade_mutex.h"
#include "CxxUtils/checker_macros.h"
#include "xAODEventFormat/EventFormat.h"
#include "xAODRootAccessInterfaces/TVirtualEvent.h"

// System include(s).
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

// Forward declaration(s).
class TClass;
namespace xAODPrivate {
class HolderBucket;
class Loader;
}  // namespace xAODPrivate

namespace xAOD {
namespace Details {

/// @c IProxyDict base class to use in a specific build environment
using IProxyDictBase =
#ifdef XAOD_STANDALONE
    IProxyDict;
#else
    implements<IProxyDict>;
#endif

// Forward declaration(s).
class IObjectManager;

}  // namespace Details

// Forward declaration(s).
class TVirtualIncidentListener;
class TVirtualManager;

/// Base class for the event (@c xAOD::TEvent and @c xAOD::REvent) classes
///
/// It implements all the common functionality used by the different storage
/// technologies. Leaving just the technology specific parts to the derived
/// classes.
///
class Event : public TVirtualEvent,
              public Details::IProxyDictBase,
              public asg::AsgMessaging {

  // Declare the friend functions/classes.
  friend class xAODPrivate::HolderBucket;
  friend class xAODPrivate::Loader;

 public:
  /// Constructor with a name
  Event(std::string_view name);
  /// Virtual destructor
  virtual ~Event();

  /// @name Setup functions
  /// @{

  /// Set this event object as the currently active one
  void setActive() const;

  /// Configure which dynamic variables to write out for a given store
  void setAuxItemList(const std::string& containerKey,
                      const std::string& itemList);

  /// Register an incident listener object
  StatusCode addListener(TVirtualIncidentListener* listener);
  /// Remove an incident listener object
  StatusCode removeListener(TVirtualIncidentListener* listener);
  /// Remove all listeners from the object
  void clearListeners();

  /// Add a name re-mapping rule
  StatusCode addNameRemap(const std::string& onfile,
                          const std::string& newName);
  /// Clear the current name re-mapping
  void clearNameRemap();
  /// Print the current name re-mapping rules
  void printNameRemap() const;

  /// Enable warnings associated with broken element links
  void printProxyWarnings(bool value = true);

  /// @}

  /// @name Persistent data accessor/modifier functions
  /// @{

  /// Get information about the input objects
  const EventFormat* inputEventFormat() const;
  /// Get information about the output objects
  const EventFormat* outputEventFormat() const;

  /// @}

  /// @name Event data accessor/modifier functions
  /// @{

  /// Function creating a user-readable dump of the current input
  std::string dump();

  /// Function printing the I/O statistics of the current process
  void printIOStats() const;

  /// Function checking if an object is available from the store
  template <typename T>
  bool contains(const std::string& key);
  /// Function checking if an object is already in memory
  template <typename T>
  bool transientContains(const std::string& key) const;

  /// Provide a list of all data object keys associated with a specific type
  template <typename T>
  StatusCode keys(std::vector<std::string>& vkeys, bool metadata) const;

  /// Retrieve either an input or an output object from the event
  template <typename T>
  StatusCode retrieve(const T*& obj, const std::string& key);
  /// Retrieve an output object from the event
  template <typename T>
  StatusCode retrieve(T*& obj, const std::string& key);

  /// Add an output object to the event
  template <typename T>
  StatusCode record(T* obj, const std::string& key);
  /// Add an output object to the event, explicitly taking ownership of it
  template <typename T>
  StatusCode record(std::unique_ptr<T> obj, const std::string& key);

  /// Copy an object directly from the input to the output
  StatusCode copy(const std::string& pattern = ".*");

  /// @}

  /// @name Metadata accessor/modifier functions
  /// @{

  /// Function checking if a meta-object is available from the store
  template <typename T>
  bool containsMeta(const std::string& key);
  /// Function checking if a meta-object is already in memory
  template <typename T>
  bool transientContainsMeta(const std::string& key) const;

  /// Provide a list of all metadata object keys associated with a specific type
  template <typename T>
  StatusCode metaKeys(std::vector<std::string>& vkeys) const;

  /// Retrieve an input metadata object
  template <typename T>
  StatusCode retrieveMetaInput(const T*& obj, const std::string& key);

  /// Retrieve an output metadata object
  template <typename T>
  StatusCode retrieveMetaOutput(const T*& obj, const std::string& key);
  /// Retrieve an output metadata object
  template <typename T>
  StatusCode retrieveMetaOutput(T*& obj, const std::string& key);

  /// Add an object to the output file's metadata
  template <typename T>
  StatusCode recordMeta(T* obj, const std::string& key);
  /// Add an object to the output file's metadata, explicitly taking
  /// ownership of it
  template <typename T>
  StatusCode recordMeta(std::unique_ptr<T> obj, const std::string& key);

  /// @}

  /// @name Functions implementing the @c xAOD::TVirtualEvent interface
  /// @{

  /// Function returning the hash describing an object name
  SG::sgkey_t getHash(const std::string& key) const override;
  /// Function returning the hash describing a known object
  SG::sgkey_t getKey(const void* obj) const override;
  /// Function returning the key describing a known object
  const std::string& getName(const void* obj) const override;
  /// Function returning the key describing a known object
  const std::string& getName(SG::sgkey_t hash) const override;

 protected:
  /// Function for retrieving an output object in a non-template way
  void* getOutputObject(SG::sgkey_t key, const std::type_info& ti) override;
  /// Function for retrieving an input object in a non-template way
  const void* getInputObject(SG::sgkey_t key, const std::type_info& ti,
                             bool silent) override;

  /// @}

  /// @name Functions implementing the IProxyDict interface
  /// @{

  /// get proxy for a given data object address in memory
  SG::DataProxy* proxy(const void* const pTransient) const override;

  /// get proxy with given id and key. Returns 0 to flag failure
  SG::DataProxy* proxy(const CLID& id, const std::string& key) const override;

  /// Get proxy given a hashed key+clid.
  SG::DataProxy* proxy_exact(SG::sgkey_t sgkey) const override;

  /// Add a new proxy to the store.
  StatusCode addToStore(CLID id, SG::DataProxy* proxy) override;

  /// return the list of all current proxies in store
  std::vector<const SG::DataProxy*> proxies() const override;

  /// Find the string corresponding to a given key.
  SG::sgkey_t stringToKey(const std::string& str, CLID clid) override;

  /// Find the string corresponding to a given key.
  const std::string* keyToString(SG::sgkey_t key) const override;

  /// Find the string and CLID corresponding to a given key.
  const std::string* keyToString(SG::sgkey_t key, CLID& clid) const override;

  /// Remember an additional mapping from key to string/CLID.
  void registerKey(SG::sgkey_t key, const std::string& str, CLID clid) override;

  /// Record an object in the store
  SG::DataProxy* recordObject(SG::DataObjectSharedPtr<DataObject> obj,
                              const std::string& key, bool allowMods,
                              bool returnExisting) override;

  /// Get the name of the instance
  const std::string& name() const override;

  /// @}

  /// Internal function for recording an object into the output
  StatusCode recordTypeless(void* obj, const std::string& typeName,
                            const std::string& key, bool overwrite = false,
                            bool metadata = true, bool isOwner = true);

  /// @name Functions to be implemented by the derived types
  /// @{

  /// Check if an input file is connected to the object
  virtual bool hasInput() const = 0;
  /// Check if an output file is connected to the object
  virtual bool hasOutput() const = 0;

  /// Function determining the list keys associated with a type name
  virtual StatusCode getNames(const std::string& targetClassName,
                              std::vector<std::string>& vkeys,
                              bool metadata = false) const = 0;

  /// Function setting up access to a particular object
  virtual StatusCode connectObject(const std::string& key, bool silent) = 0;
  /// Function setting up access to a particular metadata object
  virtual StatusCode connectMetaObject(const std::string& key, bool silent) = 0;
  /// Function setting up access to a set of auxiliary branches
  virtual StatusCode connectAux(const std::string& prefix, bool standalone) = 0;
  /// Function setting up access to a set of auxiliary branches for a
  /// metadata object
  virtual StatusCode connectMetaAux(const std::string& prefix,
                                    bool standalone) = 0;

  /// Function connecting a DV object to its auxiliary store
  virtual StatusCode setAuxStore(const std::string& key,
                                 Details::IObjectManager& mgr,
                                 bool metadata) = 0;

  /// Record an object into a connected output file
  virtual StatusCode record(void* obj, const std::string& typeName,
                            const std::string& key, bool overwrite,
                            bool metadata, bool isOwner) = 0;
  /// Record an auxiliary store into a connected output file
  virtual StatusCode recordAux(TVirtualManager& mgr, const std::string& key,
                               bool metadata) = 0;

  /// @}

  /// Function for retrieving an output object in a non-template way
  void* getOutputObject(const std::string& key, const std::type_info& ti,
                        bool metadata) const;
  /// Function for retrieving an input object in a non-template way
  const void* getInputObject(const std::string& key, const std::type_info& ti,
                             bool silent, bool metadata);
  /// Internal function checking if an object is in the input
  bool contains(const std::string& key, const std::type_info& ti,
                bool metadata);
  /// Internal function checking if an object is already in memory
  bool transientContains(const std::string& key, const std::type_info& ti,
                         bool metadata) const;

  /// Definition of the internal data structure type
  using Object_t =
      std::unordered_map<std::string, std::unique_ptr<TVirtualManager>>;

  /// Collection of all the managed input objects
  Object_t m_inputObjects;
  /// Objects that have been asked for, but were found to be missing
  /// in the current input
  std::set<std::string> m_inputMissingObjects;
  /// Collection of all the managed output object
  Object_t m_outputObjects;

  /// Collection of all the managed input meta-objects
  Object_t m_inputMetaObjects;
  /// Collection of all the managed output meta-objects
  Object_t m_outputMetaObjects;

  /// Format of the current input file
  EventFormat m_inputEventFormat;
  /// Format of the current output file
  EventFormat* m_outputEventFormat = nullptr;

  /// Rules for selecting which auxiliary branches to write
  std::unordered_map<std::string, std::set<std::string>> m_auxItemList;

  /// Listeners who should be notified when certain incidents happen
  std::vector<TVirtualIncidentListener*> m_listeners;

  /// Container name re-mapping rules
  std::unordered_map<std::string, std::string> m_nameRemapping;

  /// Option to silence common warnings that seem to be harmless
  bool m_printEventProxyWarnings = false;

  /// @name Variable(s) used in the @c IProxyDict implementation
  /// @{

  /// Helper struct used by the @c IProxyDict code
  struct BranchInfo {
    /// Data proxy describing this branch/object
    std::unique_ptr<SG::DataProxy> m_proxy;
    /// Dictionary describing this branch/object
    const ::TClass* m_class = 0;
  };  // struct BranchInfo

  /// Mutex type for multithread synchronization
  using upgrade_mutex_t = AthContainers_detail::upgrade_mutex;
  /// Lock type for multithread synchronization
  using upgrading_lock_t =
      AthContainers_detail::upgrading_lock<upgrade_mutex_t>;
  /// Mutex for multithread synchronization
  mutable upgrade_mutex_t m_branchesMutex;

  /// Map from hashed sgkey to BranchInfo.
  mutable SG::SGKeyMap<BranchInfo> m_branches
      ATLAS_THREAD_SAFE;  // protected by mutex

  /// @}

  /// @name Helper functions for the IProxyDict interface
  /// @{

  /// Get the metadata object for a given "SG key"
  const xAOD::EventFormatElement* getEventFormatElement(
      SG::sgkey_t sgkey) const;

  /// Get the object describing one object/branch
  const BranchInfo* getBranchInfo(SG::sgkey_t sgkey) const;

  /// @}

};  // class Event

}  // namespace xAOD

// Include the implementation of the template functions.
#include "xAODRootAccess/Event.icc"

#endif  // XAODROOTACCESS_EVENT_H
