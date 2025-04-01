/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
 */

/**
 * @file StoreGate/test/DecorKeyHelpers_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2017
 * @brief Tests for DecorKeyHelpers.
 */


#undef NDEBUG
#include "StoreGate/DecorKeyHelpers.h"
#include "StoreGate/VarHandleKey.h"
#include "StoreGate/exceptions.h"
#include "TestTools/expect_exception.h"
#include <cassert>
#include <iostream>


void test1()
{
  std::cout << "test1\n";
  assert (SG::contKeyFromKey ("a.b") == "a");
  assert (SG::contKeyFromKey ("S+a.b") == "S+a");
  assert (SG::contKeyFromKey ("a") == "a");
  assert (SG::contKeyFromKey ("a.") == "a");
  assert (SG::contKeyFromKey ("") == "");
  assert (SG::contKeyFromKey (".b") == "");

  assert (SG::decorKeyFromKey ("a.b") == "b");
  assert (SG::decorKeyFromKey ("S+a.b") == "b");
  assert (SG::decorKeyFromKey ("a") == "");
  assert (SG::decorKeyFromKey ("a", "b") == "b");
  assert (SG::decorKeyFromKey ("a.") == "");
  assert (SG::decorKeyFromKey ("") == "");
  assert (SG::decorKeyFromKey (".b") == "b");

  assert (SG::makeContDecorKey ("a","b") == "a.b");
  assert (SG::makeContDecorKey ("a","") == "a");
  assert (SG::makeContDecorKey ("","b") == "b");
  assert (SG::makeContDecorKey ("","") == "");
}


void test2()
{
  std::cout << "test2\n";
  SG::VarHandleKey vhk(123, "a", Gaudi::DataHandle::Reader);

  assert (SG::makeContDecorKey(vhk, "b") == "StoreGateSvc+a.b");

  EXPECT_EXCEPTION (SG::ExcBadHandleKey,
                    SG::makeContDecorKey(vhk, "StoreGateSvc+x.b"));

  std::string key = "StoreGateSvc+a.b";
  SG::removeContFromDecorKey (vhk, key);
  assert (key == "b");
}


int main()
{
  test1();
  test2();
  return 0;
}
