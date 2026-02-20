/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file xAODRootAccess/src/RPyEvent.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2025
 * @brief Python interface to xAOD::REvent
 */

#include "xAODRootAccess/RPyEvent.h"
#include "xAODRootAccess/tools/ReturnCheck.h"
#include "CxxUtils/checker_macros.h"
#include "TPython.h"


namespace xAOD::Experimental {


/// Return the object with a given key as a PyObject.
PyObject* RPyEvent::pyRetrieve(const std::string& key) {

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

/// Function checking if an object is available from the store
bool RPyEvent::pyContains(const std::string& key, const std::string& type) {

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
  return REvent::contains(key, *ti, METADATA);
}

/// Function checking if an object is already in memory
bool RPyEvent::pyTransientContains(const std::string& key,
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
  return REvent::transientContains(key, *ti, kFALSE);
}


} // namespace xAOD::Experimental
