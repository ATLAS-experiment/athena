/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 test the store ID setting
 -------------------------------------------
 ATLAS Collaboration
 ***************************************************************************/


#include <iostream>
#include <print>

#undef NDEBUG

#include "StoreGate/StoreGateSvc.h"
#include "TestTools/initGaudi.h"

using namespace std;

int main() {
  std::println ("*** StoreID_test BEGINS ***");
  ISvcLocator* pSvcLoc;
  if (!Athena_test::initGaudi("StoreGate/StoreID_test.txt", pSvcLoc)) {
    std::println (std::cerr, "This test can not be run");
    return 0;
  }  
  assert(pSvcLoc);

  SmartIF<StoreGateSvc> pStore(pSvcLoc->service("StoreGateSvc"));
  assert(pStore);
  assert(pStore->storeID() == StoreID::EVENT_STORE);

  pStore = pSvcLoc->service("DetectorStore");
  assert(pStore);
  assert(pStore->storeID() == StoreID::DETECTOR_STORE);
  
  pStore = pSvcLoc->service("ConditionStore");
  assert(pStore);
  assert(pStore->storeID() == StoreID::CONDITION_STORE);
  
  std::println ("*** StoreID_test OK ***");
  return 0;
}
