/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 test the data store clear
 -------------------------------------------
 ATLAS Collaboration
 ***************************************************************************/


#include <iostream>

#undef NDEBUG

#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/SGtests.h"
#include "TestTools/initGaudi.h"

using namespace std;

int main() {
  cout << "*** ClearStore_test BEGINS ***" <<endl;
  ISvcLocator* pSvcLoc;
  if (!Athena_test::initGaudi("StoreGate/StoreGate_jobOptions.txt", pSvcLoc)) {
    cerr << "This test can not be run" << endl;
    return 0;
  }  
  assert(pSvcLoc);

  SmartIF<StoreGateSvc> pStore(pSvcLoc->service("StoreGateSvc"));
  assert(pStore);
  
  cout << "*** ClearStore_test run standard testRecord a first time ***" << endl;
  Athena_test::testRecord(*pStore);
  cout << "*** ClearStore_test clear the store ***" << endl;
  assert(pStore->clearStore().isSuccess());
  cout << "Testing dump: store should contain no data now \n -------->>\n" 
       << pStore->dump() 
       << "\n<<--------" << endl;

  cout << "\n\n\n*** ClearStore_test run standard testRecord a second time ***" << endl;
  Athena_test::testRecord(*pStore);
  cout << "*** ClearStore_test clear the store again ***" << endl;
  assert(pStore->clearStore().isSuccess());

  cout << "Testing dump: store should contain no data now \n -------->>\n" 
       << pStore->dump() 
       << "\n<<--------" << endl;

  cout << "*** ClearStore_test clear the store one last time forcing proxy removal (the way we do in finalize()) ***" << endl;
  const bool FORCEREMOVE(true);
  assert(pStore->clearStore(FORCEREMOVE).isSuccess());

  cout << "Testing dump: store should contain no proxy now \n -------->>\n" 
       << pStore->dump() 
       << "\n<<--------" << endl;


  cout << "*** ClearStore_test OK ***" <<endl;
  return 0;
}
