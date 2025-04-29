/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file StoreGate/test/ReadDecorHandleKey_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2017
 * @brief Tests for ReadDecorHandleKey.
 */


#undef NDEBUG
#include "StoreGate/ReadDecorHandleKey.h"
#include "TestTools/initGaudi.h"
#include "TestOwner.h"
#include <cassert>
#include <iostream>


class MyObj {};
CLASS_DEF (MyObj, 293847295, 1)


void test1()
{
  std::cout << "test1\n";

  SG::ReadDecorHandleKey<MyObj> k1 ("aaa.dec");
  assert (k1.clid() == 293847295);
  assert (k1.key() == "aaa.dec");
  assert (k1.mode() == Gaudi::DataHandle::Reader);
  assert (k1.storeHandle().name() == "StoreGateSvc");
  assert (!k1.storeHandle().isSet());
  assert (k1.initialize().isSuccess());
  assert (k1.storeHandle().isSet());

  assert (k1.contHandleKey().clid() == 293847295);
  assert (k1.contHandleKey().key() == "aaa");
  assert (k1.contHandleKey().mode() == Gaudi::DataHandle::Reader);
  assert (k1.contHandleKey().storeHandle().name() == "StoreGateSvc");
  assert (k1.contHandleKey().storeHandle().isSet());

  k1 = "bbb.foo";
  assert (k1.key() == "bbb.foo");
  assert (k1.contHandleKey().key() == "bbb");

  assert (k1.assign ("ccc.fee").isSuccess());
  assert (k1.key() == "ccc.fee");
  assert (k1.contHandleKey().key() == "ccc");

  // Test auto-declaring constructors
  auto check = [](TestOwner& owner, SG::ReadDecorHandleKey<MyObj>& k) {
    assert (k.clid() == 293847295);
    assert (k.key() == "ccc.dec");
    assert (k.mode() == Gaudi::DataHandle::Reader);
    assert (k.contHandleKey().clid() == 293847295);
    assert (k.contHandleKey().key() == "ccc");
    assert (k.contHandleKey().mode() == Gaudi::DataHandle::Reader);
    assert (owner.getProperty ("CCCKey").name() == "CCCKey");
    assert (owner.getProperty ("CCCKey").documentation() == "doc string");
    assert (owner.getProperty ("CCCKey").type_info() == &typeid(SG::ReadHandleKey<MyObj>));
    assert (owner.getProperty ("CCCKey").toString() == "'StoreGateSvc+ccc.dec'");
    assert (owner.getProperty ("CCCKey").ownerTypeName() == "TestOwner");
  };
  {
    TestOwner owner;
    SG::ReadDecorHandleKey<MyObj> k (&owner, "CCCKey", "ccc.dec", "doc string");
    check(owner, k);
  }
  {
    TestOwner owner;
    SG::ReadHandleKey<MyObj> r ("ccc");
    SG::ReadDecorHandleKey<MyObj> k (&owner, "CCCKey", r, "dec", "doc string");
    check(owner, k);
  }
  {
    TestOwner owner;
    SG::ReadHandleKey<MyObj> w ("xxx");
    SG::ReadDecorHandleKey<MyObj> k (&owner, "CCCKey", w, "dec", "doc string");
    w="ccc";
    k.initialize().ignore();
    check(owner, k);
  }
  {
    TestOwner owner;
    SG::ReadHandleKey<MyObj> w;  // empty key that gets changed later
    SG::ReadDecorHandleKey<MyObj> k (&owner, "CCCKey", w, "dec", "doc string");
    w="ccc";
    k.initialize().ignore();
    check(owner, k);
  }
}


int main()
{
  ISvcLocator* pDum;
  Athena_test::initGaudi(pDum); //need MessageSvc

  test1();
  return 0;
}
