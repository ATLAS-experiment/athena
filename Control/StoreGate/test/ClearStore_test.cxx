/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 test the data store clear
 -------------------------------------------
 ATLAS Collaboration
 ***************************************************************************/


#include <iostream>
#include <print>

#undef NDEBUG

#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/SGtests.h"
#include "TestTools/initGaudi.h"

using namespace std;

int main() {
  std::println ("*** ClearStore_test BEGINS ***");
  ISvcLocator* pSvcLoc;
  if (!Athena_test::initGaudi("StoreGate/StoreGate_jobOptions.txt", pSvcLoc)) {
    std::println (std::cerr, "This test can not be run");
    return 0;
  }  
  assert(pSvcLoc);

  SmartIF<StoreGateSvc> pStore(pSvcLoc->service("StoreGateSvc"));
  assert(pStore);
  
  std::println ("*** ClearStore_test run standard testRecord a first time ***");
  Athena_test::testRecord(*pStore);
  std::println ("*** ClearStore_test clear the store ***");
  assert(pStore->clearStore().isSuccess());
  std::println ("Testing dump: store should contain no data now \n -------->>\n{}\n<<--------",
                pStore->dump());

  std::println ("\n\n\n*** ClearStore_test run standard testRecord a second time ***");
  Athena_test::testRecord(*pStore);
  std::println ("*** ClearStore_test clear the store again ***");
  assert(pStore->clearStore().isSuccess());

  std::println ("Testing dump: store should contain no data now \n -------->>\n{}\n<<--------",
                pStore->dump());

  std::println ("*** ClearStore_test clear the store one last time forcing proxy removal (the way we do in finalize()) ***");
  const bool FORCEREMOVE(true);
  assert(pStore->clearStore(FORCEREMOVE).isSuccess());

  std::println ("Testing dump: store should contain no proxy now \n -------->>\n{}\n<<--------",
                pStore->dump());


  std::println ("*** ClearStore_test OK ***");
  return 0;
}
