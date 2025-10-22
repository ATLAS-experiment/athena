// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

// Local include(s).
#include "xAODRootAccess/TPyEvent.h"

#include "xAODRootAccess/tools/ReturnCheck.h"

// Project include(s).
#include "CxxUtils/checker_macros.h"

// ROOT include(s).
#include <TClass.h>
#include <TError.h>
#include <TPython.h>

namespace xAOD {

TPyEvent::TPyEvent(EAuxMode mode) : TEvent(mode) {}

PyObject* TPyEvent::pyRetrieve(const std::string& key) {

  // Try to find the event format corresponding to this key.
  const EventFormat* ef = this->inputEventFormat();
  const EventFormatElement* efe = ((ef != nullptr) ? ef->get(key) : nullptr);
  if (efe == nullptr) {
    ef = this->outputEventFormat();
    efe = ((ef != nullptr) ? ef->get(key) : nullptr);
    if (efe == nullptr) {
      // Fail if we can't find the type.
      Py_INCREF(Py_None);
      return Py_None;
    }
  }

  // Convert the type to a std::type_info.
  ::TClass* cl = TClass::GetClass(efe->className().c_str());
  if (cl == nullptr) {
    Py_INCREF(Py_None);
    return Py_None;
  }
  const std::type_info* ti = cl->GetTypeInfo();
  if (ti == nullptr) {
    Py_INCREF(Py_None);
    return Py_None;
  }

  // Get the object from the store as a void*.
  static constexpr bool SILENT = true;
  static constexpr bool METADATA = false;
  void* obj ATLAS_THREAD_SAFE =
      const_cast<void*>(this->getInputObject(key, *ti, SILENT, METADATA));
  if (obj == nullptr) {
    obj = this->getOutputObject(key, *ti, METADATA);
  }
  if (obj == nullptr) {
    Py_INCREF(Py_None);
    return Py_None;
  }

  // Convert to a PyObject.
  return TPython::CPPInstance_FromVoidPtr(obj, efe->className().c_str());
}

bool TPyEvent::pyContains(const std::string& key, const std::string& type) {

  // Try to access the dictionary of this type.
  ::TClass* cl = ::TClass::GetClass(type.c_str());
  if (cl == nullptr) {
    ::Warning("xAOD::TPyEvent::pyContains", "Type name \"%s\" not known",
              type.c_str());
    return false;
  }

  // Check if the dictionary can return a type_info.
  const std::type_info* ti = cl->GetTypeInfo();
  if (ti == nullptr) {
    ::Warning("xAOD::TPyEvent::pyContains",
              "Type \"%s\" doesn't have a proper dictionary", type.c_str());
    return false;
  }

  // Let the base class do the work.
  static constexpr bool METADATA = false;
  return TEvent::contains(key, *ti, METADATA);
}

bool TPyEvent::pyTransientContains(const std::string& key,
                                   const std::string& type) const {

  // Try to access the dictionary of this type.
  ::TClass* cl = ::TClass::GetClass(type.c_str());
  if (cl == nullptr) {
    ::Warning("xAOD::TPyEvent::pyTransientContains",
              "Type name \"%s\" not known", type.c_str());
    return false;
  }

  // Check if the dictionary can return a type_info:
  const std::type_info* ti = cl->GetTypeInfo();
  if (ti == nullptr) {
    ::Warning("xAOD::TPyEvent::pyTransientContains",
              "Type \"%s\" doesn't have a proper dictionary", type.c_str());
    return false;
  }

  // Let the base class do the work:
  return TEvent::transientContains(key, *ti, kFALSE);
}

/// This function is designed to be callable from PyROOT in order to record
/// hand-made containers into an output file.
///
/// Notice that unline TEvent::record(...), this function doesn't take
/// ownership of the object that's given to it. That's because all objects
/// created by the Python interpreter are garbage collected by the Python
/// interpreter. And we don't want to run into double-deletes.
///
/// @param obj  Typeless pointer to the object that is to be recorded
/// @param key  The key with which the object is to be recorded
/// @param type The type name of the object being recorded
/// @returns The usual @c StatusCode types
///
StatusCode TPyEvent::pyRecord(void* obj, const std::string& key,
                              const std::string& type) {

  // Simply forward the call to the base class:
  static constexpr bool OVERWRITE = false;
  static constexpr bool METADATA = false;
  static constexpr bool IS_OWNER = false;
  static constexpr int BASKET_SIZE = 32000;
  static constexpr int SPLIT_LEVEL = 0;
  RETURN_CHECK("xAOD::TPyEvent::pyRecord",
               record(obj, type, key, BASKET_SIZE, SPLIT_LEVEL, OVERWRITE,
                      METADATA, IS_OWNER));
  // Return gracefully:
  return StatusCode::SUCCESS;
}

}  // namespace xAOD
