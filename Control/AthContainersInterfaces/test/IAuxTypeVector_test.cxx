/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainersInterfaces/test/IAuxTypeVector_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jun, 2024
 * @brief Regression test for IAuxTypeVector.
 */


#undef NDEBUG
#include "AthContainersInterfaces/IAuxTypeVector.h"
#include "CxxUtils/checker_macros.h"
#include <iostream>
#include <cassert>


int xx ATLAS_THREAD_SAFE = 0;

class TestAuxVector
  : public SG::IAuxTypeVector
{
public:
  TestAuxVector() : SG::IAuxTypeVector (1, false) {}
  virtual std::unique_ptr<SG::IAuxTypeVector> clone() const { std::abort(); }
  virtual void* toPtr() { std::abort(); }
  virtual const void* toPtr() const { std::abort(); }
  virtual void* toVector() { std::abort(); }
  virtual size_t size() const { std::abort(); }
  virtual bool resize (size_t) { std::abort(); }
  virtual void reserve (size_t){ std::abort(); }
  virtual bool shift (size_t, ptrdiff_t){ std::abort(); }
  virtual bool insertMove (size_t, void*, size_t, size_t,
                           SG::IAuxStore&){ std::abort(); }
  virtual SG::AuxDataSpanBase getDataSpanImpl() const
  { ++xx; return SG::AuxDataSpanBase (&xx, 1); }

  using SG::IAuxTypeVector::storeDataSpan;
  using SG::IAuxTypeVector::resetDataSpan;
};


void test1()
{
  std::cout << "test1\n";
  TestAuxVector tav;
  tav.storeDataSpan (&xx, 10);

  const SG::AuxDataSpanBase& sp = tav.getDataSpan();
  assert (xx == 1);
  assert (sp.beg == &xx);
  assert (sp.size == 1);
  tav.storeDataSpan (&xx, 10);
  assert (sp.beg == &xx);
  assert (sp.size == 10);
  tav.getDataSpan();
  assert (xx == 1);
  tav.resetDataSpan();
  tav.getDataSpan();
  assert (xx == 2);
}


int main()
{
  std::cout << "AthContainersInterfaces/IAuxTypeVector_test\n";
  test1();
}
