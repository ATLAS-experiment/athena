/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Local include(s).
#include "xAODRootAccess/TPyStore.h"

#include "xAODRootAccess/tools/ReturnCheck.h"

// ROOT include(s).
#include <TClass.h>
#include <TError.h>

namespace xAOD {

/// This function can be used in the same manner as
/// @c TStore::contains<...>(...), but instead of providing a type, one
/// gives it a type name.
///
/// Note that only types that have a proper ROOT dictionary can be used.
/// Unlike C++, which allows one to insert any type of object into TStore.
///
/// @param key  The key of the object that we're looking for
/// @param type The type name of the object we're trying to access
/// @returns @c true if the object is acessible,
///          @c false otherwise
///
bool TPyStore::pyContains(const std::string& key,
                          const std::string& type) const {

  // Try to access the dictionary of this type.
  ::TClass* cl = ::TClass::GetClass(type.c_str());
  if (cl == nullptr) {
    ::Warning("xAOD::TPyStore::pyContains", "Type name \"%s\" not known",
              type.c_str());
    return false;
  }

  // Check if the dictionary can return a type_info.
  const std::type_info* ti = cl->GetTypeInfo();
  if (!ti) {
    ::Warning("xAOD::TPyStore::pyContains",
              "Type \"%s\" doesn't have a proper dictionary", type.c_str());
    return false;
  }

  // Use the base class to answer the question.
  return TStore::contains(key, *ti);
}

/// This function can be used in the same manner as
/// @c TStore::isConst<...>(...), but instead of providing a type, one gives it
/// a type name.
///
/// Note that only types that have a proper ROOT dictionary can be used.
/// Unlike C++, which allows one to insert any type of object into TStore.
///
/// @param key  The key of the object that we're looking for
/// @param type The type name of the object we're trying to access
/// @returns @c true if the object is acessible,
///          @c false otherwise
///
bool TPyStore::pyIsConst(const std::string& key,
                         const std::string& type) const {

  // Try to access the dictionary of this type.
  ::TClass* cl = ::TClass::GetClass(type.c_str());
  if (cl == nullptr) {
    ::Warning("xAOD::TPyStore::pyIsConst", "Type name \"%s\" not known",
              type.c_str());
    return false;
  }

  // Check if the dictionary can return a type_info.
  const std::type_info* ti = cl->GetTypeInfo();
  if (ti == nullptr) {
    ::Warning("xAOD::TPyStore::pyIsConst",
              "Type \"%s\" doesn't have a proper dictionary", type.c_str());
    return false;
  }

  // Use the base class to answer the question.
  return TStore::isConst(key, *ti);
}

/// This function can be used in the same manner as
/// @c TStore::record<...>(...), but instead of providing a type, one gives it
/// a type name and a typeless pointer.
///
/// Note that only types that have a proper ROOT dictionary can be used.
/// Unlike C++, which allows one to insert any type of object into TStore.
///
/// Also note that this function doesn't take ownership of the recorded
/// object. In Python all the objects created by the interpreter are managed
/// by the interpreter. So this code is not supposed to delete them,
/// otherwise all hell breaks loose.
///
/// @param obj  Pointer to the object to be put into the store
/// @param key  Key of the object in the store
/// @param type The type name of the object we are inserting
/// @returns The usual @c StatusCode types
///
StatusCode TPyStore::pyRecord(void* obj, const std::string& key,
                              const std::string& type) {

  // Simply forward the call to the appropriate function from the base
  // class.
  static constexpr bool IS_OWNER = true;
  static constexpr bool IS_CONST = false;
  RETURN_CHECK("xAOD::TPyStore::pyRecord",
               TStore::record(obj, key, type, IS_OWNER, IS_CONST));

  // Return gracefully:
  return StatusCode::SUCCESS;
}

/// This is just a convenience function, to make it easier to print the
/// contents of such objects in Python. Since the base class's print()
/// function has a special meaning in Python.
///
void TPyStore::dump() const {

  print();
  return;
}

}  // namespace xAOD
