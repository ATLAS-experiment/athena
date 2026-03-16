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

  static_assert( std::is_same_v< Object2Container_t<ElectronMuonTopoInfo, TypeInfo_EDM>,
                                 ElectronMuonTopoInfoContainer> );

  static_assert( IsKnownFeature<ElectronMuonTopoInfo>::value == 1 );
  static_assert( IsKnownFeature<xAOD::TrigElectronContainer>::value == 1);

  static_assert( IsKnownFeature<dummy>::value == 0 );

  return 0;
}
