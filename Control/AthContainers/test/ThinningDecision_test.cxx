/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/test/ThinningDecision_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Oct, 2019
 * @brief Tests for ThinningDecision.
 */


#undef NDEBUG
#include "AthContainers/ThinningDecision.h"
#include "AthContainers/DataVector.h"
#include "AthContainers/exceptions.h"
#include "SGTools/TestStore.h"
#include "TestTools/expect_exception.h"
#include <print>
#include <cassert>


CLASS_DEF( DataVector<int>, 194519923, 0 )


void test1 (SGTest::TestStore& store)
{
  std::println ("test1");

  auto dv = std::make_unique<DataVector<int> >();
  for (int i=0; i < 10; i++)
    dv->push_back (std::make_unique<int> (i));
  // Need this because TestStore doesn't handle inheritance.
  std::unique_ptr<SG::AuxVectorBase> base = std::move (dv);
  store.record (std::move(base), "dv");

  SG::ThinningDecision dv1 (DataLink<SG::AuxVectorBase> ("dv"));
  assert (dv1.link().dataID() == "dv");
  dv1.thin (4);
  dv1.thin (5);
  dv1.lock();
  assert (dv1.size() == 10);
  assert (dv1.index (3) == 3);
  assert (dv1.index (8) == 6);

  SG::ThinningDecision dv2 ("dv");
  dv2.thin (4);
  dv2.thin (5);
  dv2.lock();
  assert (dv2.size() == 10);
  assert (dv2.index (3) == 3);
  assert (dv2.index (8) == 6);

  EXPECT_EXCEPTION (SG::ExcInvalidThinningTarget,
                    SG::ThinningDecision dv3 ("xxx"));
}


int main()
{
  std::println ("AthContainers/ThinningDecision_test");
  std::unique_ptr<SGTest::TestStore> store = SGTest::getTestStore();
  test1 (*store);
  return 0;
}
