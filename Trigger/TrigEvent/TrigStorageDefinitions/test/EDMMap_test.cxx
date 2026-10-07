/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// Test with real trigger EDM

#include "TrigStorageDefinitions/EDM_TypeInfo.h"
#include "TrigStorageDefinitions/EDM_TypeInfoMethods.h"

#include <iostream>
#include <string>

struct dummy{};


// Just a "compilation test"
int main(){

  static_assert( std::is_same_v< Object2Container_t<TrigRoiDescriptor, TypeInfo_EDM>,
                                 TrigRoiDescriptorCollection> );

  static_assert( IsKnownFeature<TrigRoiDescriptor>::value );
  static_assert( IsKnownFeature<xAOD::TrigElectronContainer>::value );

  static_assert( !IsKnownFeature<dummy>::value );

  auto print = []<typename T>() { std::cout << typeid(T).name() << std::endl; };
  TypeInfo_EDM::map::for_each(print);


  return 0;
}
