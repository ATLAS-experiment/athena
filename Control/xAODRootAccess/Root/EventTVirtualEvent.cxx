// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// File holding the implementation of the xAOD::Event functions that implement
// the TVirtualEvent interface.
//

// Local include(s).
#include "xAODRootAccess/Event.h"

#include "xAODRootAccess/TActiveStore.h"
#include "xAODRootAccess/TStore.h"
#include "xAODRootAccess/tools/TVirtualManager.h"
#include "xAODRootAccess/tools/Utils.h"

namespace xAOD {

/// This helper function is mostly needed by the smart pointers of the
/// xAOD EDM. Right now it very simply just calculates the hash just
/// based on the key given to the function. But later on it might be
/// good to do some tests here, checking if the event format knows about
/// the specified key or not. This is why it's not made to be a static
/// function
///
/// @param key String key to turn into a hash
/// @returns A hash corresponding to the specified string key
///
SG::sgkey_t Event::getHash(const std::string& key) const {

  // For empty keys let's finish quickly.
  if (key == "") {
    return 0;
  }

  // If the key is used in the input file, let's use the same hash for
  // the output file as well.
  if (m_inputEventFormat.exists(key)) {
    return m_inputEventFormat.get(key)->hash();
  }

  // If it's a new key, make a new hash for it from scratch:
  return Utils::hash(key);
}

/// This function is used by the smart pointer code to find the identifier
/// of an object that's already in the event in some way.
///
/// @param obj Pointer to the object that we want to look up
/// @returns The hashed identifier of the object, or 0 if the object was
///          not found in the event
///
SG::sgkey_t Event::getKey(const void* obj) const {

  // Make use of the getName function.
  return getHash(getName(obj));
}

/// This function is used by the smart pointer code to find the identifier
/// of an object that's already in the event in some way.
///
/// @param obj Pointer to the object that we want to look up
/// @returns The name of the object, or an empty string if the object was
///          not found in the event
///
const std::string& Event::getName(const void* obj) const {

  // First look among the output objects.
  for (const auto& [key, manager] : m_outputObjects) {
    // Check if this is our object.
    if (manager->object() == obj) {
      // If it is, let's return right away.
      return key;
    }
  }

  // Now look among the input objects.
  for (const auto& [key, manager] : m_inputObjects) {
    // Check if this is our object.
    if (manager->object() == obj) {
      // If it is, let's return.
      return key;
    }
  }

  // If it's not there either, check if it's in an active TStore object:
  const TStore* store = TActiveStore::store();
  if (store && store->contains(obj)) {
    // Get the name from the store then:
    return store->getName(obj);
  }

  // We didn't find the object in the event...
  ATH_MSG_WARNING("Didn't find object with pointer \"" << obj
                                                       << "\" in the event");
  static const std::string dummy;
  return dummy;
}

/// This function is used primarily when getting the string key of
/// a smart pointer that we read in from a file, or access it in memory.
///
/// @param hash The hashed key for the container/object
/// @returns The name of the object, or an empty string if the object was
///          not found in the event
///
const std::string& Event::getName(SG::sgkey_t hash) const {

  // If the branch is known from the input:
  if (m_inputEventFormat.exists(hash)) {
    return m_inputEventFormat.get(hash)->branchName();
  }

  // If the branch is known on the output:
  if (m_outputEventFormat && m_outputEventFormat->exists(hash)) {
    return m_outputEventFormat->get(hash)->branchName();
  }

  // If this is an object in the active store:
  const TStore* store = TActiveStore::store();
  if (store && store->contains(hash)) {
    return store->getName(hash);
  }

  // If it is unknown:
  static const std::string dummy;
  return dummy;
}

/// This function is used by the TVirtualEvent interface to access an
/// output object with a given hashed key. The function looks up the string
/// key belonging to the hash, and then calls the other GetOutputObject(...)
/// function in the class with that parameter.
///
/// @param key The hashed key of the output object
/// @param ti  The type description of the object requested
/// @returns A pointer to the requested object, or a null pointer in case
///          of failure
///
void* Event::getOutputObject(SG::sgkey_t key, const std::type_info& ti) {

  // Get a string name for this key.
  const std::string& name = getName(key);
  if (name.empty()) {
    return nullptr;
  }

  // Forward the call to the function using an std::string key.
  static const bool METADATA = false;
  return getOutputObject(name, ti, METADATA);
}

/// This function is used by the TVirtualEvent interface to access an
/// input object with a given hashed key. The function looks up the string
/// key belonging to the hash, and then calls the other GetInputObject(...)
/// function in the class with that parameter.
///
/// @param key    The hashed key of the input object
/// @param ti     The type description of the object requested
/// @param silent Switch for being silent about failures or not
/// @returns A pointer to the requested object, or a null pointer in case
///          of failure
///
const void* Event::getInputObject(SG::sgkey_t key, const std::type_info& ti,
                                  bool silent) {

  // Get a string name for this key:
  const std::string& name = getName(key);
  if (name.empty() && (silent == false)) {
    ATH_MSG_WARNING("Key 0x" << std::hex << key << " unknown");
    return nullptr;
  }

  // Forward the call to the function using an std::string key:
  static const bool METADATA = false;
  return getInputObject(name, ti, silent, METADATA);
}

}  // namespace xAOD
