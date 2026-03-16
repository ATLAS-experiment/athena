/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// Test of type registration and query

#include "TrigStorageDefinitions/TypeInformation.h"
#include "TrigStorageDefinitions/EDM_TypeInformation.h"
#include "TrigStorageDefinitions/EDM_TypeInfoMethods.h"


// Declare testing types
HLT_BEGIN_TYPE_REGISTRATION
     HLT_REGISTER_TYPE(struct ObjectA, struct ObjectA, struct ContainerA)
     HLT_REGISTER_TYPE(struct ObjectB, struct ObjectB, struct ContainerB, struct AuxContainerB)
HLT_END_TYPE_REGISTRATION(Test)

// Declare testing EDM map
TYPEMAPCLASS(Test)

struct TypeInfo_EDM {
  using map = HLT::TypeInformation::newlist
    ::add<class_Test>::go
    ::done;
};


// Just a "compilation test"
int main() {

  static_assert(std::is_same_v< Object2Container_t<ObjectA, TypeInfo_EDM>, ContainerA >);
  static_assert(std::is_same_v< Container2Object_t<ContainerA, TypeInfo_EDM>, ObjectA >);
  static_assert(std::is_same_v< Container2Aux_t<ContainerA, TypeInfo_EDM>, HLT::TypeInformation::no_aux >);
  static_assert(std::is_same_v< Container2Aux_t<ContainerB, TypeInfo_EDM>, AuxContainerB >);
  static_assert(std::is_same_v< Features2Container_t<ObjectA, TypeInfo_EDM>, ContainerA >);
  static_assert(std::is_same_v< Features2Object_t<ObjectA, TypeInfo_EDM>, ObjectA >);

  using featuresA = Object2Features_t<ObjectA, TypeInfo_EDM>;
  static_assert(std::is_same_v< HLT::TypeInformation::at<featuresA,0>::type, ObjectA >);

  return 0;
}
