/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file xAODCore/test/AuxInfoBase_test.cxx
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
}


int main()
{
  std::cout << "ut_xaodcore_auxinfobase_test\n";
  test1();
  test_linked();
  return 0;
}
