/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/** @file ProxyProviderSvc_test.cxx
 * @brief unit test for the proxy provider mechanism
 * @author ATLAS Collaboration
 ***************************************************************************/


#include "AthenaKernel/IProxyProviderSvc.h"

#include <cassert>
#include <iostream>
#include <string>

#include "TestTools/initGaudi.h"
#include "TestTools/FLOATassert.h"
#include "TestTools/SGassert.h"
#include "ToyConversion/FooBar.h"
#include "ToyConversion/ToyConversionSvc.h"

#include "SGTools/TransientAddress.h"
#include "SGTools/DataProxy.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/StoreGateSvc.h"
#include "AthenaKernel/ClassID_traits.h"
#include "AthenaKernel/IAddressProvider.h"
#include "AthenaKernel/StoreID.h"

#include "GaudiKernel/GenericAddress.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/StatusCode.h"
#include "GaudiKernel/ClassID.h"

#include "CxxUtils/checker_macros.h"


using namespace Athena_test;
using std::cerr;
using std::cout;
using std::endl;
using std::string;
using namespace SG;

class FooBar {
public:
  FooBar(): m_i(s_i++){}
  int i() const { return m_i; }
  ~FooBar() { 
    cout << "FooBar i=" << i() << " deleted" << endl;
  }
private:
  int m_i;
  inline static std::atomic<int> s_i{0};
};

#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF(FooBar, 8109, 0)

template <typename PROXIED>
class TestProvider : public implements<IAddressProvider> {
public:
  TestProvider(const std::string& key) :
    m_ID(ClassID_traits<PROXIED>::ID()), m_key(key)
  {  }
  ~TestProvider() override {}
  ///get all addresses that the provider wants to preload in SG maps

  StatusCode loadAddresses(StoreID::type /*id*/, tadList& /* tList */) override 
  {return StatusCode::SUCCESS;}

  StatusCode preLoadAddresses(StoreID::type /*id*/, tadList& tList) override 
  {
    TransientAddress* tad = new TransientAddress(m_ID, m_key, new GenericAddress(ToyConversionSvc::storageType(), m_ID,m_key));
    tList.push_back(tad);
    return StatusCode::SUCCESS;
  }

  ///get a specific address, plus all others  the provider wants to load in SG maps
  StatusCode updateAddress(StoreID::type /*sID*/, TransientAddress* tad,
                           const EventContext& /*ctx*/) override
  { 
    StatusCode sc;
    if ((tad->clID() != m_ID) || (tad->name() != m_key)) {
      //      cout <<"store id"<< sID << endl;
      //      cout << "m_ID: " << m_ID << " " << m_key << endl;
      //      cout << "id: " << tad->clID() << " " << tad->name() << endl;
      sc=StatusCode::FAILURE;
    } else {
      tad->setAddress(new GenericAddress(ToyConversionSvc::storageType(), m_ID,m_key));
      sc=StatusCode::SUCCESS;
    }
    return sc;
  }
      
private:
  CLID m_ID;
  std::string m_key;
};

void testRecordBeforeRead(StoreGateSvc& rSG, IProxyProviderSvc& rPPS) {
  cout << "*** ProxyProviderSvc_test RecordBeforeRead BEGINS ***" <<endl;
  rSG.clearStore().ignore();
  rPPS.addProvider(new TestProvider<Foo>("existingFoo"));
  const Foo *pFoo(nullptr);
  //NOT YET SGASSERTERROR(rSG.record(new Foo(6.28), "existingFoo").isSuccess());
  assert(rSG.record(new Foo(6.28), "existingFoo").isSuccess());
  assert(rSG.retrieve(pFoo, "existingFoo").isSuccess());
  cout << pFoo->a() << endl;
  cout << "*** ProxyProviderSvc_test RecordBeforeRead OK ***\n\n" <<endl;
}

void testHLTAutoKeyReset(StoreGateSvc& rSG, IProxyProviderSvc& rPPS) {
  cout << "*** ProxyProviderSvc_test HLTAutoKeyReset BEGINS ***" <<endl;
  assert(rSG.clearStore(true).isSuccess());
  std::list<DataProxy*> pl;
  assert(rSG.proxies().empty());
  rPPS.addProvider(new TestProvider<Foo>("HLTAutoKey_1"));
  rPPS.addProvider(new TestProvider<Foo>("HLTAutoKey_2"));
  rPPS.addProvider(new TestProvider<Foo>("HLTAutoKey_3"));
  rPPS.addProvider(new TestProvider<Foo>("NOT_HLTAutoKey_3"));
  assert(rSG.contains<Foo>("HLTAutoKey_1"));
  assert(rSG.contains<Foo>("HLTAutoKey_2"));
  assert(rSG.contains<Foo>("HLTAutoKey_3"));
  assert(rSG.contains<Foo>("NOT_HLTAutoKey_3"));
  pl.clear();
  assert(rSG.proxies().size() == 4);
  assert(rSG.clearStore().isSuccess());
  pl.clear();
  assert(rSG.proxies().size() == 1);
  assert(rSG.contains<Foo>("HLTAutoKey_1"));
  assert(rSG.contains<Foo>("NOT_HLTAutoKey_3"));
  pl.clear();
  assert(rSG.proxies().size() == 2);
  
  cout << "*** ProxyProviderSvc_test HLTAutoKeyReset OK ***\n\n" <<endl;
}


void testOverwrite(StoreGateSvc& rSG, IProxyProviderSvc& rPPS) {
  cout << "*** ProxyProviderSvc_test Overwrite starts ***\n\n" <<endl;
  
  assert(rSG.clearStore(true).isSuccess());
  rPPS.addProvider(new TestProvider<Foo>("toOverwrite"));
  //key
  const std::string KEY("toOverwrite");
  //our "data members"
  WriteHandle<FooBar> wFB(KEY);
  ReadHandle<FooBar> rFB(KEY);
  //simulate an event loop
  for (int i=0;i<3;++i) {
    cout << "=============Event #" << i << " starts" << std::endl;
    //check we can not retrieve a non const pointer from the PPS
    assert(!(wFB.isValid()));
    //overwrite an object coming from the PPS
    assert(rSG.overwrite(std::make_unique<FooBar>(), KEY, true));
    //check contents of the overwritten object
    assert(rFB.isValid());
    cout << "Overwritten FooBar i="<< rFB->i() << endl;
    assert(i == rFB->i());
    //clear the store
    assert(rSG.clearStore().isSuccess());
  }
  cout << "*** ProxyProviderSvc_test Overwrite OK ***\n\n" <<endl;
}

int main ATLAS_NOT_THREAD_SAFE () {
  ISvcLocator* pSvcLoc(nullptr);
  if (!initGaudi("ProxyProviderSvc_test.txt", pSvcLoc)) {
    cerr << "This test can not be run" << endl;
    return 0;
  }  
  assert( pSvcLoc );

  SmartIF<StoreGateSvc> pStore(pSvcLoc->service("StoreGateSvc"));
  assert( pStore );
  SmartIF<IProxyProviderSvc> pIPPSvc(pSvcLoc->service("ProxyProviderSvc"));
  assert( pIPPSvc );

  pIPPSvc->addProvider(new TestProvider<Foo>("aFoo"));
  pIPPSvc->addProvider(new TestProvider<Foo>("diskFoo"));
  pIPPSvc->addProvider(new TestProvider<Foo>("privFoo"));
  pIPPSvc->addProvider(new TestProvider<Bar>("aBar"));
  pIPPSvc->addProvider(new TestProvider<FooBar>("aFooBar"));


  assert( !(pStore->transientContains<Foo>("aFoo")) );
  assert( pStore->contains<Foo>("aFoo") );

  const Foo* pFoo(nullptr);
  assert( (pStore->retrieve(pFoo, "aFoo")).isSuccess() );
  assert( pFoo );


  const FooBar* pFooBar(nullptr);
  SGASSERTERROR( (pStore->retrieve(pFooBar, "aFooBar")).isSuccess() );
  assert( nullptr == pFooBar );

  cout << "*** ProxyProviderSvc_test OK ***" <<endl;


  testRecordBeforeRead(*pStore, *pIPPSvc);
  testHLTAutoKeyReset(*pStore, *pIPPSvc);
  testOverwrite(*pStore, *pIPPSvc);
  assert(pStore->clearStore(true).isSuccess());
  return 0;
}

