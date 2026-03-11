/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigStorageDefinitions/TypeInformation.h"
#include "TrigStorageDefinitions/EDM_TypeInformation.h"
#include "TrigStorageDefinitions/EDM_TypeInfoMethods.h"

#include <typeinfo>
#include <iostream>


HLT_BEGIN_TYPE_REGISTRATION
     HLT_REGISTER_TYPE(struct ObjectA,struct ObjectA,struct ContainerA)
     HLT_REGISTER_TYPE(struct ObjectB,struct ObjectB,struct ContainerB,struct AuxContainerB)
HLT_END_TYPE_REGISTRATION(Test)

TYPEMAPCLASS(Test)

struct TypeInfo_EDM {
  using map = HLT::TypeInformation::newlist
    ::add<class_Test>::go
    ::done;
};

struct AuxContainerB{};


int main() {
  std::cout << "ListMap_test" << std::endl;

  std::cout << typeid(Container2Aux<ContainerB,TypeInfo_EDM>::type).name() << std::endl;
  std::cout << typeid(Container2Aux<ContainerA,TypeInfo_EDM>::type).name() << std::endl;

  return 0;
}
