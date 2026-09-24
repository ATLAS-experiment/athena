/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file StoreGate/test/TypelessWriteHandle_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2019
 * @brief Tests for TypelessWriteHandle.
 */


#undef NDEBUG
#include "StoreGate/TypelessWriteHandle.h"
#include "SGTools/TestStore.h"
#include "TestTools/initGaudi.h"
#include "AthenaKernel/ExtendedEventContext.h"
#include "AthenaKernel/CLASS_DEF.h"
#include <cassert>
#include <print>


class MyObj
{
public:
  MyObj(int x=0) : x(x) {}
  ~MyObj() {} //{ deleted.push_back (x); }
  //SG::IAuxStore* getStore() const { return nullptr; }
  //void setStore (SG::IConstAuxStore* store) {aux = dynamic_cast<MyObjAux*>(store); }
  int x;
  //MyObjAux* aux {nullptr};

  //static std::vector<int> deleted ATLAS_THREAD_SAFE;
};
//std::vector<int> MyObj::deleted;
CLASS_DEF (MyObj, 293847295, 1)


void test1()
{
  std::println ("test1");
  SGTest::TestStore testStore;
  EventContext ctx;
  ctx.setExtension( Atlas::ExtendedEventContext(&testStore) );

  SG::TypelessWriteHandleKey k (ClassID_traits<MyObj>::ID(), "foo");
  assert( k.initialize().isSuccess() );
  SG::TypelessWriteHandle h (k, ctx);

  auto obj = std::make_unique<MyObj>();
  std::unique_ptr<DataObject> dobj (SG::asStorable (std::move (obj)));
  DataObject* dobjptr = dobj.get();
  assert( h.typelessPut (std::move (dobj)).isSuccess() );
  SG::DataProxy* foo_proxy = testStore.proxy (ClassID_traits<MyObj>::ID(),
                                              "foo");
  assert (foo_proxy->refCount() == 1);
  assert (foo_proxy->object() == dobjptr);

  auto obj2 = std::make_unique<MyObj>(2);
  std::unique_ptr<DataObject> dobj2 (SG::asStorable (std::move (obj2)));
  assert( h.typelessPut (std::move (dobj2)).isFailure() );

  std::ostringstream s1;
  s1 << h;
  std::ostringstream s2;
  std::print (s2, "{}", h);
  assert (s1.str() == s2.str());
}


int main()
{
  std::println ("StoreGate/TypelessWriteHandle_test");
  ISvcLocator* pDum;
  Athena_test::initGaudi(pDum); //need MessageSvc

  test1();
  return 0;
}
