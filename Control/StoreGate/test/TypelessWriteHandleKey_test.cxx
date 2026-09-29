/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file StoreGate/test/TypelessWriteHandleKey_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2019
 * @brief Tests for TypelessWriteHandleKey.
 */


#undef NDEBUG
#include "StoreGate/TypelessWriteHandleKey.h"
#include "StoreGate/exceptions.h"
#include "TestTools/initGaudi.h"
#include "TestTools/expect_exception.h"
#include "TestOwner.h"
#include <cassert>
#include <print>


void test1()
{
  std::println ("test1");

  SG::TypelessWriteHandleKey k1;
  assert (k1.clid() == CLID_NULL);
  assert (k1.key() == "");
  assert (k1.mode() == Gaudi::DataHandle::Writer);
  assert (k1.storeHandle().name() == "StoreGateSvc");

  SG::TypelessWriteHandleKey k2 (123, "aaa", "FooSvc");
  assert (k2.clid() == 123);
  assert (k2.key() == "aaa");
  assert (k1.mode() == Gaudi::DataHandle::Writer);
  assert (k2.storeHandle().name() == "FooSvc");

  {
    std::ostringstream s1;
    s1 << k2;
    assert (s1.str() == "'FooSvc+aaa'");
    std::ostringstream s2;
    std::print (s2, "{}", k2);
    assert (s2.str() == "'FooSvc+aaa'");
  }

  k2 = "FeeSvc+bbb";
  assert (k2.clid() == 123);
  assert (k2.key() == "bbb");
  assert (k1.mode() == Gaudi::DataHandle::Writer);
  assert (k2.storeHandle().name() == "FeeSvc");

  k2 = "BarSvc+xAOD::EventInfo/ccc";
  assert (k2.clid() == 45903698);
  assert (k2.key() == "ccc");
  assert (k1.mode() == Gaudi::DataHandle::Writer);
  assert (k2.storeHandle().name() == "BarSvc");

  EXPECT_EXCEPTION (SG::ExcBadHandleKey,
                    k2 = "BarSvc+xAOD::EventInfo/ddd/");


  TestOwner owner;
  SG::TypelessWriteHandleKey k3 (&owner, "CCCKey", 123, "ccc", "doc string");
  assert (k3.clid() == 123);
  assert (k3.key() == "ccc");
  assert (k3.mode() == Gaudi::DataHandle::Writer);
  assert (owner.getProperty ("CCCKey").name() == "CCCKey");
  assert (owner.getProperty ("CCCKey").documentation() == "doc string");
  assert (owner.getProperty ("CCCKey").type_info() == &typeid(SG::TypelessWriteHandleKey));
  assert (owner.getProperty ("CCCKey").toString() == "'StoreGateSvc+ccc'");
  assert (owner.getProperty ("CCCKey").ownerTypeName() == "TestOwner");
}


int main()
{
  std::println ("StoreGateSvc/TypelessWriteHandleKey_test");

  ISvcLocator* pDum;
  Athena_test::initGaudi(pDum); //need MessageSvc

  test1();
  return 0;
}

  
