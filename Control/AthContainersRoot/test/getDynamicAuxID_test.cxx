/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainersRoot/test/getDynamicAuxID_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Oct, 2016
 * @brief Regression test for getDynamicAuxID.
 */


#undef NDEBUG
#include "AthContainersRoot/getDynamicAuxID.h"
#include "AthContainersRoot/AthContainersRootTestDict.h"
#include "AthContainersRoot/RootAuxVectorFactory.h"
#include "AthContainers/AuxElement.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthenaKernel/errorcheck.h"
#include "TInterpreter.h"
#include "TROOT.h"
#include "TClass.h"
#include <print>
#include <cassert>


void test1 (SG::auxid_t a1_id)
{
  std::println ("test1");
  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();

  assert (SG::getDynamicAuxID (typeid(int),
                               "a1",
                               "int",
                               "std::vector<int>",
                               false,
                               SG::null_auxid) == a1_id);

  const SG::IAuxTypeVectorFactory* fac;
  fac = r.getFactory (typeid(AthContainersRootTest::Foo),
                      typeid(std::allocator<AthContainersRootTest::Foo>));
  assert  (fac == nullptr);
  SG::auxid_t foo_id = SG::getDynamicAuxID (typeid(AthContainersRootTest::Foo),
                                            "foo",
                                            "AthContainersRootTest::Foo",
                                            "std::vector<AthContainersRootTest::Foo>",
                                            false,
                                            SG::null_auxid);
  assert (foo_id != SG::null_auxid);
  fac = r.getFactory (typeid(AthContainersRootTest::Foo),
                      typeid(std::allocator<AthContainersRootTest::Foo>));
  assert  (fac != nullptr);
  assert (dynamic_cast<const SG::AuxTypeVectorFactory<AthContainersRootTest::Foo>*>(fac) != nullptr);
  assert (r.getName(foo_id) == "foo");
  assert (r.getTypeName(foo_id) == "AthContainersRootTest::Foo");
  assert (r.getVecTypeName(foo_id) == "std::vector<AthContainersRootTest::Foo>");
  assert (!r.isLinked(foo_id));
  assert (r.linkedVariable(foo_id) == SG::null_auxid);

  fac = r.getFactory (typeid(AthContainersRootTest::Bar),
                      typeid(std::allocator<AthContainersRootTest::Bar>));
  assert  (fac == nullptr);
  SG::auxid_t bar_id = SG::getDynamicAuxID (typeid(AthContainersRootTest::Bar),
                                            "bar",
                                            "AthContainersRootTest::Bar",
                                            "std::vector<AthContainersRootTest::Bar>",
                                            false,
                                            SG::null_auxid);
  assert (bar_id != SG::null_auxid);
  fac = r.getFactory (typeid(AthContainersRootTest::Bar),
                      typeid(std::allocator<AthContainersRootTest::Bar>));
  assert  (fac != nullptr);
  assert (dynamic_cast<const SG::RootAuxVectorFactory*>(fac) != nullptr);
  assert (r.getName(bar_id) == "bar");
  assert (r.getTypeName(bar_id) == "AthContainersRootTest::Bar");
  assert (r.getVecTypeName(bar_id) == "std::vector<AthContainersRootTest::Bar>");
  assert (!r.isLinked(bar_id));
  assert (r.linkedVariable(bar_id) == SG::null_auxid);

  fac = r.getFactory (typeid(AthContainersRootTest::Baz),
                      typeid(std::allocator<AthContainersRootTest::Baz>));
  assert  (fac == nullptr);
  SG::auxid_t baz_id = SG::getDynamicAuxID (typeid(AthContainersRootTest::Baz),
                                            "baz",
                                            "AthContainersRootTest::Baz",
                                            "AthContainersRootTest::Baz",
                                            true,
                                            SG::null_auxid);
  assert (baz_id != SG::null_auxid);
  fac = r.getFactory (typeid(AthContainersRootTest::Baz),
                      typeid(std::allocator<AthContainersRootTest::Baz>));
  assert  (fac != nullptr);
  assert (dynamic_cast<const SG::RootAuxVectorFactory*>(fac) != nullptr);
  assert (r.getName(baz_id) == "baz");
  assert (r.getTypeName(baz_id) == "AthContainersRootTest::Baz");
  assert (r.getVecTypeName(baz_id) == "std::vector<AthContainersRootTest::Baz>");
  assert (!r.isLinked(baz_id));
  assert (r.linkedVariable(baz_id) == SG::null_auxid);
}


// Test handling linked variables
void test_linked()
{
  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  std::println ("test_linked");

  SG::auxid_t l1_id = SG::getDynamicAuxID (typeid(AthContainersRootTest::L1),
                                           "linked1_linked",
                                           "AthContainersRootTest::L1",
                                           "std::vector<AthContainersRootTest::L1>",
                                           false,
                                           SG::null_auxid);
  assert (l1_id != SG::null_auxid);
  assert (r.getName(l1_id) == "linked1_linked");
  assert (r.getTypeName(l1_id) == "AthContainersRootTest::L1");
  assert (r.getVecTypeName(l1_id) == "std::vector<AthContainersRootTest::L1>");
  assert (r.isLinked(l1_id));
  assert (r.linkedVariable(l1_id) == SG::null_auxid);
  assert (r.getFactory(l1_id) != nullptr);
  assert (r.getFactory(l1_id)->isDynamic());

  SG::auxid_t l2_id = SG::getDynamicAuxID (typeid(AthContainersRootTest::L2),
                                           "linked2_linked",
                                           "AthContainersRootTest::L2",
                                           "std::vector<AthContainersRootTest::L2>",
                                           false,
                                           SG::null_auxid);
  assert (l2_id != SG::null_auxid);
  assert (r.getName(l2_id) == "linked2_linked");
  assert (r.getTypeName(l2_id) == "AthContainersRootTest::L2");
  assert (r.getVecTypeName(l2_id) == "std::vector<AthContainersRootTest::L2>");
  assert (r.isLinked(l2_id));
  assert (r.linkedVariable(l2_id) == SG::null_auxid);
  assert (r.getFactory(l2_id) != nullptr);
  assert (!r.getFactory(l2_id)->isDynamic());

  SG::auxid_t l3_id = SG::getDynamicAuxID (typeid(AthContainersRootTest::L3),
                                           "linked3",
                                           "AthContainersRootTest::L3",
                                           "std::vector<AthContainersRootTest::L3>",
                                           false,
                                           l1_id);
  assert (l3_id == SG::null_auxid);

  SG::auxid_t l4_id = SG::getDynamicAuxID (typeid(AthContainersRootTest::L2),
                                           "linked4",
                                           "AthContainersRootTest::L2",
                                           "std::vector<AthContainersRootTest::L2>",
                                           false,
                                           l1_id);
  assert (l4_id != SG::null_auxid);
  assert (r.getName(l4_id) == "linked4");
  assert (r.getTypeName(l4_id) == "AthContainersRootTest::L2");
  assert (r.getVecTypeName(l4_id) == "std::vector<AthContainersRootTest::L2>");
  assert (!r.isLinked(l4_id));
  assert (r.linkedVariable(l4_id) == l1_id);
  assert (r.getFactory(l4_id) != nullptr);
  assert (!r.getFactory(l4_id)->isDynamic());
}


SG::auxid_t init()
{
  static const SG::Accessor<int> a1 ("a1");
  return a1.auxid();
}


int main()
{
  errorcheck::ReportMessage::hideErrorLocus();
  errorcheck::ReportMessage::hideFunctionNames();
  SG::auxid_t a1_auxid = init();
  test1(a1_auxid);
  test_linked();
  return 0;
}
