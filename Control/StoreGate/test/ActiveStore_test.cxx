/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
  test ActiveStoreSvc functionality
 ------------------------------
 ATLAS Collaboration
 ***************************************************************************/

#include <string>
#include <print>
#include "TestTools/initGaudi.h"
#include "TestTools/SGassert.h"
#include "StoreGate/ActiveStoreSvc.h"
#include "StoreGate/StoreGateSvc.h"


using namespace std;
using namespace Athena_test;

class Foo {
public:
  Foo() : m_i(0), m_d(0.0) {}
  Foo(int i) : m_i(i), m_d(0.0) {}
  int i() const { return m_i; }
  ~Foo() {
  }

  // Members public to avoid clang warning about unused private members.
  int m_i;
  double m_d;
};


#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF(Foo, 8101, 1)

int main() {
  std::println ("*** ActiveStoreTest BEGINS ***");
  ISvcLocator* pSvcLoc;
  if (!initGaudi("StoreGate/ActiveStore_test.txt", pSvcLoc)) {
    return 1;
  }
  SmartIF<ActiveStoreSvc> pASS{pSvcLoc->service("ActiveStoreSvc")};
  assert( pASS.isValid() );
  assert( pASS->activeStore() == pASS->operator->() );
  //as set in ActiveStore_test.txt
  assert( pASS->activeStore()->name() == "E1" ); 
  SmartIF<StoreGateSvc> pE2{pSvcLoc->service("E2")};
  assert( pE2.isValid() );
  pASS->setStore(pE2);
  assert( pASS->activeStore()->name() == "E2" ); 
  assert( pASS->finalize().isSuccess() );
  std::println ("*** ActiveStoreTest OK ***\n\n");

  return 0;
}

