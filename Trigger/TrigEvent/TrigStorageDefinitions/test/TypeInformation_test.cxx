/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

class TrigInDetTrack{};

#include "TrigStorageDefinitions/TypeInformation.h"
#include "TrigStorageDefinitions/EDM_TypeInfo.h"

#include <typeinfo>
#include <iostream>

class TrigInDetTrackCollection : public DataVector<TrigInDetTrack>{};

int main(){

  static_assert(std::is_same_v< Object2Container_t<TrigInDetTrack, TypeInfo_EDM>, TrigInDetTrackCollection >);
  static_assert(std::is_same_v< Container2Aux_t<TrigInDetTrackCollection, TypeInfo_EDM>, TrigInDetTrackCollection >);


  return 0;
}
