/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file xAODCore/test/ut_xaodcore_auxinfobase_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Apr, 2018
 * @brief Unit tests for AuxInfoBase.  (sadly incomplete)
 */


#undef NDEBUG
#include "xAODCore/AuxInfoBase.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/exceptions.h"
#include "TestTools/expect_exception.h"
#include <iostream>
#include <sstream>
#include <cassert>


class AuxInfoTest
  : public xAOD::AuxInfoBase
{
public:
  AuxInfoTest();

  int i1 = 0;
  int a1 = 0;
};


AuxInfoTest::AuxInfoTest()
{
  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();

  AUX_VARIABLE(i1);
  SG::auxid_t i1aux = getAuxID( "i1", i1 );
  AUX_VARIABLE(a1, SG::AuxTypeRegistry::Flags::Atomic);
  SG::auxid_t a1aux = getAuxID( "a1", a1, SG::AuxTypeRegistry::Flags::Atomic );

  assert (i1aux == r.findAuxID ("i1"));
  assert (a1aux == r.findAuxID ("a1"));
}


class AuxContainerLinkTest
  : public xAOD::AuxInfoBase
{
public:
  AuxContainerLinkTest();

  std::vector<int> ltest1;
  float ltest2 = 0;
};


AuxContainerLinkTest::AuxContainerLinkTest()
{
  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t id1 = getAuxID( "ltest1", ltest1, SG::AuxVarFlags::Linked );
  SG::auxid_t id2 = getAuxID( "ltest2", ltest2, SG::AuxVarFlags::None, id1 );

  regAuxVar (id1, "ltest1", ltest1);
  regAuxVar (id2, "ltest2", ltest2);

  assert (id1 == r.findAuxID ("ltest1"));
  assert (id2 == r.findAuxID ("ltest2"));
}


void test1()
{
  std::cout << "test1\n";
  AuxInfoTest s1;

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t ityp1 = r.getAuxID<int> ("i1");

  EXPECT_EXCEPTION (SG::ExcFlagMismatch,
                    r.getAuxID<int> ("a1"));

  SG::auxid_t atyp1 = r.getAuxID<int> ("a1", "",
                                       SG::AuxTypeRegistry::Flags::Atomic);
  assert (r.getFlags (atyp1) == SG::AuxTypeRegistry::Flags::Atomic);

  int* i1 = reinterpret_cast<int*> (s1.getData(ityp1, 1, 1));
  int* a1 = reinterpret_cast<int*> (s1.getData(atyp1, 1, 1));
  assert (i1 == &s1.i1);
  assert (a1 == &s1.a1);

  assert (s1.getVector(ityp1)->toPtr() == i1);
  assert (s1.getVector(ityp1)->size() == 1);
}


// Test handling of linked variables.
void test_linked()
{
  std::cout << "test_linked\n";

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  AuxContainerLinkTest s;
  SG::auxid_t auxid1 = r.findAuxID ("ltest1");
  SG::auxid_t auxid2 = r.findAuxID ("ltest2");
  assert (r.isLinked (auxid1));
  assert (!r.isLinked (auxid2));

  assert (s.linkedVector (auxid2)->size() == 0);

  (void)s.getData (auxid1, 10, 10);
  (void)s.getData (auxid2, 1, 1);

  const AuxContainerLinkTest& cs = s;
  assert (cs.linkedVector (auxid2)->size() == 10);

  auto v1 = reinterpret_cast<const std::vector<int>*> (s.getIOData (auxid1));
  assert (v1->size() == 10);
  assert (v1->capacity() == 10);
  assert (s.size() == 1);

  s.resize (1);
  assert (s.size() == 1);
  assert (v1->size() == 10);
  assert (v1->capacity() == 10);

  s.reserve (1);
  assert (s.size() == 1);
  assert (v1->size() == 10);
  assert (v1->capacity() == 10);
}


// Test getCopyIDs()
void test_copyIDs()
{
  std::cout << "test_copyIDs\n";
  AuxInfoTest s1;

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t i1 = r.getAuxID<int> ("i1");
  SG::auxid_t i2 = r.getAuxID<int> ("i2");
  SG::auxid_t i3 = r.getAuxID<int> ("i3");
  SG::auxid_t m1 = r.findAuxID ("m1");
  SG::auxid_t a1 = r.findAuxID ("a1");

  (void)s1.getData(i1, 1, 1);
  (void)s1.getData(i3, 1, 1);
  s1.lock();
  (void)s1.getDecoration(i2, 1, 1);

  SG::auxid_set_t exp;
  exp.set (i1);
  exp.set (i3);
  exp.set (m1);
  exp.set (a1);

  {
    SG::auxid_set_t out = s1.getCopyIDs();
    assert (out == exp);
  }

  {
    std::cout << "Expect a warning here (except in standalone):\n";
    SG::auxid_set_t out = s1.getCopyIDs (true);
    assert (out == exp);
  }
}


int main()
{
  std::cout << "ut_xaodcore_auxinfobase_test\n";
  test1();
  test_linked();
  test_copyIDs();
  return 0;
}
