// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

// Local include(s).
#include "xAODRootAccess/Event.h"
#include "xAODRootAccess/TActiveStore.h"
#include "xAODRootAccess/TStore.h"
#include "xAODRootAccess/tools/IObjectManager.h"

// Project include(s).
#include "AthContainers/normalizedTypeinfoName.h"

// System include(s).
#include <regex>
#include <string>
#include <vector>

namespace xAOD {

/// This function can be used to easily copy a given (set of)
/// object/container(s) to the output, without modifying the contents of
/// it/them. It only needs to be called on the interface object/container(s),
/// the copying of the auxiliary data is done automatically, and is steered by
/// the @c xAOD::Event::setAuxItemList function.
///
/// @param pattern Regular expression for the key(s)/name(s) of the
///                object(s)/container(s) to copy
///
StatusCode Event::copy(const std::string& pattern) {

  // Tell the user what's happening.
  ATH_MSG_DEBUG("Copying objects matching pattern \"" << pattern
                                                      << "\" to the output");

  // Collect a list of keys to copy.
  std::set<std::string> keys;

  // The regular expression to use.
  std::regex re{pattern};

  // Loop over the known input containers.
  for (const auto& [key, efe] : m_inputEventFormat) {

    // Tell the user what's happening.
    ATH_MSG_VERBOSE("Considering input object with key \"" << key << "\"");

    // Check if the class in question matches the requested pattern.
    if (std::regex_match(key, re) == false) {
      continue;
    }
    // Ignore objects that don't exist on the input.
    static const bool SILENT = true;
    if (connectObject(key, SILENT).isSuccess() == false) {
      continue;
    }
    // Skip all branches ending in "Aux.":
    if (key.ends_with("Aux.")) {
      continue;
    }
    // Also skip dynamic branches:
    if (efe.parentName() != "") {
      continue;
    }
    // Add the key to the list.
    ATH_MSG_VERBOSE("Matched key \"" << key << "\"");
    keys.insert(key);
  }

  // Check if the pattern matches any of the name remapping rules.
  for (const auto& [newname, onfile] : m_nameRemapping) {

    // Tell the user what's happening.
    ATH_MSG_VERBOSE("Considering remapped key \"" << newname << "\"");

    // Check if the remapped name matches the pattern.
    if (std::regex_match(newname, re) == false) {
      continue;
    }
    // Ignore objects that don't exist on the input.
    static const bool SILENT = true;
    if (connectObject(onfile, SILENT).isSuccess() == false) {
      continue;
    }
    // Add the remapped name to the list.
    ATH_MSG_VERBOSE("Matched remapped key \"" << newname << "\"");
    keys.insert(newname);
  }

  // Now loop over all of the found keys.
  for (const std::string& key : keys) {

    // Check if a name re-mapping should be applied or not.
    std::string keyToUse = key;
    auto remap_itr = m_nameRemapping.find(key);
    if ((remap_itr != m_nameRemapping.end()) &&
        (!m_inputEventFormat.exists(key)) &&
        m_inputEventFormat.exists(remap_itr->second)) {
      keyToUse = remap_itr->second;
    }

    // Make sure that the input object got connected to.
    static const bool SILENT = false;
    ATH_CHECK(connectObject(keyToUse, SILENT));

    // Make sure that the input object is properly updated.
    Object_t::const_iterator vobjMgr = m_inputObjects.find(keyToUse);
    if (vobjMgr == m_inputObjects.end()) {
      ATH_MSG_FATAL("Internal logic error detected");
      return StatusCode::FAILURE;
    }
    Details::IObjectManager* objMgr =
        dynamic_cast<Details::IObjectManager*>(vobjMgr->second.get());
    if (objMgr == nullptr) {
      ATH_MSG_FATAL("Internal logic error detected");
      return StatusCode::FAILURE;
    }
    static const bool METADATA = false;
    if (getInputObject(keyToUse, *(objMgr->holder()->getClass()->GetTypeInfo()),
                       SILENT, METADATA) == nullptr) {
      ATH_MSG_FATAL("Internal logic error detected");
      return StatusCode::FAILURE;
    }

    // Put the interface object into the output.
    static const bool OVERWRITE = true;
    static const bool IS_OWNER = true;
    ATH_CHECK(record(objMgr->object(), objMgr->holder()->getClass()->GetName(),
                     key, OVERWRITE, METADATA, IS_OWNER));

    // If there is also an auxiliary store for this object/container, copy that
    // as well.
    const std::string auxKey = keyToUse + "Aux.";
    if (m_inputObjects.contains(auxKey)) {
      ATH_CHECK(
          recordAux(*(m_inputObjects.at(auxKey)), key + "Aux.", METADATA));
    }
  }

  // Return gracefully:
  return StatusCode::SUCCESS;
}

/// This function does the heavy lifting of retrieving object from the list
/// of output objects. While it returns a typeless pointer, that pointer can
/// be cast directly to the type described by the second parameter in the
/// caller code.
///
/// @param key The key (branch name) of the object to retrieve
/// @param ti  The type as which the object is to be retrieved
/// @param metadata Flag deciding whether we're looking for a metadata or
///                 event data object
/// @returns A pointer to the output object if successful, or a null pointer
///          if not
///
void* Event::getOutputObject(const std::string& key, const std::type_info& ti,
                             bool metadata) const {

  // Select which object container to use:
  const Object_t& objects = (metadata ? m_outputMetaObjects : m_outputObjects);

  // Check if the object can be found:
  auto itr = objects.find(key);
  if (itr == objects.end()) {
    // Do the following only for event data:
    if (!metadata) {
      // It's not in the event. Let's check if we find it in an active
      // TStore object...
      TStore* store = TActiveStore::store();
      if ((!store) || (!store->contains(key, ti)) || store->isConst(key, ti)) {
        // Nope, not there either...
        return nullptr;
      }
      // Let's return the object from the TStore:
      void* result = store->getObject(key, ti);
      return result;
    } else {
      // For metadata we don't use external resources.
      return nullptr;
    }
  }

  // If the object is not set in this event yet, we can't continue:
  if (itr->second->isSet() == false) {
    return nullptr;
  }

  // If it does exist, check if it's the right kind of object:
  Details::IObjectManager* mgr =
      dynamic_cast<Details::IObjectManager*>(itr->second.get());
  if (mgr == nullptr) {
    ATH_MSG_ERROR("Object of wrong type found for key \"" << key << "\"");
    return nullptr;
  }

  // Ask the holder object for the object of this type:
  void* result = mgr->holder()->getAs(ti);
  if (result == nullptr) {
    ATH_MSG_WARNING("Couldn't retrieve object as \""
                    << SG::normalizedTypeinfoName(ti) << "\"");
    return nullptr;
  }

  // Return the object:
  return result;
}

/// This is the function doing the heavy lifting to retrieve objects
/// from the input file.
///
/// @param key The key (branch name) of the object to retrieve
/// @param ti  The type as which the object is to be retrieved
/// @param silent Set to <code>kTRUE</code> to make the code fail silently
///               in case the object can't be retrieved
/// @param metadata Flag deciding whether we're looking for a metadata or
///                 event data object
/// @returns A pointer to the input object if successful, or a null pointer
///          if not
///
const void* Event::getInputObject(const std::string& key,
                                  const std::type_info& ti, bool silent,
                                  bool metadata) {

  // Check if a name remapping should be applied or not:
  std::string keyToUse = key;
  auto remap_itr = m_nameRemapping.find(key);
  if ((remap_itr != m_nameRemapping.end()) &&
      (!m_inputEventFormat.exists(key)) &&
      m_inputEventFormat.exists(remap_itr->second)) {
    keyToUse = remap_itr->second;
  }

  // The following catches the cases when we ask for a transient
  // ConstDataVector object to be returned as "const DataVector".
  TStore* store = TActiveStore::store();
  if (store && store->contains(keyToUse, ti) && store->isConst(keyToUse, ti)) {
    const void* result = store->getConstObject(keyToUse, ti);
    return result;
  }

  // A sanity check before checking for an object from the input file.
  if (hasInput() == false) {
    if (silent == false) {
      ATH_MSG_WARNING("No input file connected to the Event object");
    }
    return nullptr;
  }

  // Make sure that the requested input is connected to.
  const StatusCode sc = (metadata ? connectMetaObject(keyToUse, silent)
                                  : connectObject(keyToUse, silent));
  if (sc.isSuccess() == false) {
    return nullptr;
  }

  // Select which object container to use:
  Object_t& objects = (metadata ? m_inputMetaObjects : m_inputObjects);

  // Access the object's manager:
  auto itr = objects.find(keyToUse);
  if (itr == objects.end()) {
    ATH_MSG_FATAL("There is an internal logic error in the code...");
    return nullptr;
  }

  // This has to be an ObjectManager object:
  Details::IObjectManager* mgr =
      dynamic_cast<Details::IObjectManager*>(itr->second.get());
  if (mgr == nullptr) {
    if (key == keyToUse) {
      ATH_MSG_ERROR("Object of wrong type found for key \"" << key << "\"");
    } else {
      ATH_MSG_ERROR("Object of wrong type found for key \""
                    << key << "\"/\"" << keyToUse << "\"");
    }
    return nullptr;
  }

  // Make sure that the current entry is loaded for event data objects.
  if (metadata == false) {
    const Int_t readBytes = mgr->getEntry();
    if (readBytes > 0) {
      // Connect the auxiliary store to objects needing it. This call also
      // takes care of updating the dynamic store of auxiliary containers,
      // when they are getting accessed directly.
      static const bool IS_METADATA = false;
      if (setAuxStore(key, *mgr, IS_METADATA).isSuccess() == false) {
        ATH_MSG_ERROR("Failed to set the auxiliary store for "
                      << mgr->holder()->getClass()->GetName() << "/"
                      << keyToUse);
        return nullptr;
      }
    } else if (readBytes < 0) {
      ATH_MSG_ERROR("Failed to load current entry for object "
                    << mgr->holder()->getClass()->GetName() << "/" << keyToUse);
      return nullptr;
    }
  }

  // Ask the holder object for the object of this type:
  const void* result = mgr->holder()->getAsConst(ti, silent);
  if (result == nullptr) {
    if (!silent) {
      ATH_MSG_WARNING("Could not retrieve object with key \""
                      << keyToUse << "\" as \""
                      << SG::normalizedTypeinfoName(ti) << "\"");
    }
    return nullptr;
  }

  // We succeeded:
  return result;
}

/// Non-template version of the @c xAOD::Event::contains function
///
/// @param key The key to check
/// @param ti  The type of the object to check
/// @param metadata Flag deciding whether we are looking for a metadata
///                 object or not
/// @returns @c true if such an object is held by the
///          event object at the moment, @c false otherwise
///
::Bool_t Event::contains(const std::string& key, const std::type_info& ti,
                         ::Bool_t metadata) {

  static const bool SILENT = true;
  return ((getOutputObject(key, ti, metadata) != nullptr) ||
          (getInputObject(key, ti, SILENT, metadata) != nullptr));
}

/// Internal function checking if an object of a given type, with a given
/// key is held in memory by the event object. It's this function doing
/// the heavy lifting for the public template function.
///
/// @param key The key/branch name of the object
/// @param ti The type description of the object
/// @param metadata Flag deciding whether we are looking for a metadata
///                 object or not
/// @returns @c true if such a modifiable object is held by the
///          event object at the moment, @c false otherwise
///
::Bool_t Event::transientContains(const std::string& key,
                                  const std::type_info& ti,
                                  ::Bool_t metadata) const {

  return (getOutputObject(key, ti, metadata) != nullptr);
}

}  // namespace xAOD
