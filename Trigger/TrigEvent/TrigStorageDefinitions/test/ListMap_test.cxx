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
     HLT_REGISTER_TYPE(struct ObjectB, struct ObjectB2, struct ContainerB, struct AuxContainerB)
HLT_END_TYPE_REGISTRATION(Test)

// Declare testing EDM map
TYPEMAPCLASS(Test)

struct TypeInfo_EDM {
  using map = class_Test::map;
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
  static_assert(featuresA::size == 1);  // one feature
  static_assert(std::is_same_v< featuresA::at<0>, ObjectA >);

  using featuresB = Object2Features_t<ObjectB, TypeInfo_EDM>;
  static_assert(featuresB::size == 2);  // two features
  static_assert(std::is_same_v< featuresB::at<0>, ObjectB >);
  static_assert(std::is_same_v< featuresB::at<1>, ObjectB2 >);

  return 0;
}
