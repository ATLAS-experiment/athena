/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 test the store ID setting
 -------------------------------------------
 ATLAS Collaboration
 ***************************************************************************/


#include <iostream>

#undef NDEBUG

#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/SGtests.h"


#ifndef NOGAUDI
#include "TestTools/initGaudi.h"
using namespace Athena_test;
using namespace std;

int main() {
  cout << "*** StoreID_test BEGINS ***" <<endl;
  ISvcLocator* pSvcLoc;
  if (!initGaudi("StoreGateTestCommon.txt", pSvcLoc)) {
    cerr << "This test can not be run" << endl;
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
  
  cout << "*** StoreID_test OK ***" <<endl;
  return 0;
}
#endif /*NOGAUDI*/













