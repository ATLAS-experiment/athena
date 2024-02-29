/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthenaKernel/test/ExtendedEventContext_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2019
 * @brief Regression tests for ExtendedEventContext (partial).
 */

#undef NDEBUG

#include "AthenaKernel/ExtendedEventContext.h"
#include "GaudiKernel/EventContext.h"
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <memory_resource>


class TestMemResource
  : public std::pmr::memory_resource
{
public:
  virtual void* do_allocate (size_t, std::size_t) override
  {
    std::abort();
  }
  virtual void do_deallocate (void*, std::size_t, std::size_t) override
  {
    std::abort();
  }
  virtual bool do_is_equal (const std::pmr::memory_resource&) const noexcept override
  {
    std::abort();
  }
};



void test1()
{
  std::cout << "test1\n";

  TestMemResource tmr;

  EventContext ctx;

  Atlas::ExtendedEventContext ectx;

  assert (!Atlas::hasExtendedEventContext (ctx));
  ectx.setConditionsRun (42);
  ectx.setMemResource (&tmr);
  Atlas::setExtendedEventContext (ctx, std::move (ectx));
  assert (Atlas::hasExtendedEventContext (ctx));
  assert (Atlas::getExtendedEventContext (ctx).conditionsRun() == 42);
  assert (Atlas::getExtendedEventContext (ctx).memResource() == &tmr);
}


int main()
{
  std::cout << "AthenaKernel/ExtendedEventContext_test\n";
  test1();
  return 0;
}
