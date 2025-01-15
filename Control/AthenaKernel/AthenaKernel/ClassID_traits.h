/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAKERNEL_CLASSID_TRAITS_H
#define ATHENAKERNEL_CLASSID_TRAITS_H
/** @file AthenaKernel/ClassID_traits.h
 * @brief  a traits class that associates a CLID to a type T
 * It also detects whether T inherits from Gaudi DataObject
 *
 * @author Paolo Calafiura <pcalafiura@lbl.gov> - ATLAS Collaboration
 */

#include "AthenaKernel/ClassName.h"
#include "GaudiKernel/ClassID.h"

#include <string>
#include <typeinfo>
#include <type_traits>
#include <concepts>


///internal use: issues a compilation error when condition B is false
#define MY_STATIC_ASSERT( B ) \
  static_assert (B, "You should use the CLASS_DEF macro to define CLID and VERSION")
class ClassID_trait_dummy{};

/** @class ClassID_traits
 * @brief  a traits class that associates a CLID to a type T
 * It also detects whether T inherits from Gaudi DataObject
 *
 * This default specialization just gives errors.
 *
 * Normally, this will get specialized by the @c CLASS_DEF macro.
 */
template <typename T>
struct ClassID_traits {
  // Always false, but needs to depend on T to prevent the assertions
  // from being instantiated too early.
  static const bool s_isDataObject = std::same_as<T, ClassID_trait_dummy>;
  using has_classID_tag = std::false_type;
  static const int s_version = 0;

  static CLID ID() {
    MY_STATIC_ASSERT(s_isDataObject);
    return CLID_NULL;
  }

  static const std::string& typeName() {
    MY_STATIC_ASSERT(s_isDataObject);
    static const std::string dummy;
    return dummy;
  }

  static const std::type_info& typeInfo() {
    MY_STATIC_ASSERT(s_isDataObject);
    return typeid(int);
  }
};


/**
 * @brief This specialization is used for classes deriving from @c DataObject.
 *        To avoid having a compile-time dependency on @c DataObject,
 *        we enable this based on whether or not @c T::classID() is defined.
 */
template <typename T>
requires requires () {
  { T::classID() } -> std::convertible_to<CLID>;
}
struct ClassID_traits<T> {
  static const bool s_isDataObject = true;

  ///the CLID of T
  static const CLID& ID() { 
    return T::classID(); 
  }

  ///the demangled type name of T
  static const std::string& typeName() {
    static const std::string tname = Athena::typeinfoName(typeid(T));
    return tname;
  }

  ///the type id of T
  static const std::type_info& typeInfo() {
    return typeid(T);
  }

  using has_version_tag = std::false_type;
  using has_classID_tag = std::false_type;

  static const int s_version = 0;

  // Is this is true, these types will automatically be made
  // const when added to StoreGate.
  static const bool s_isConst = false;
};

#undef MY_STATIC_ASSERT

#endif // not ATHENAKERNEL_CLASSID_TRAITS_H
