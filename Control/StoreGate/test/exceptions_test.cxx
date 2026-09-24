/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file StoreGate/test/exceptions_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Feb, 2016
 * @brief Regression tests for exceptions.
 */


#undef NDEBUG
#include "StoreGate/exceptions.h"
#include "GaudiKernel/EventContext.h"
#include <print>


void test1()
{
  std::println ("test1");

  EventContext ctx;

  std::println ("{}", SG::ExcNullHandleKey().what());
  std::println ("{}", SG::ExcBadHandleKey("xkey").what());
  std::println ("{}", SG::ExcForbiddenMethod("meth").what());
  std::println ("{}", SG::ExcHandleInitError(123, "foo", "FooSvc").what());
  std::println ("{}", SG::ExcUninitKey (123, "foo", "FooSvc").what());
  std::println ("{}", SG::ExcUninitKey (123, "foo", "FooSvc", "holder", "Flooby").what());
  std::println ("{}", SG::ExcConstObject(123, "foo", "FooSvc").what());
  std::println ("{}", SG::ExcNullWriteHandle(123, "foo", "FooSvc").what());
  std::println ("{}", SG::ExcNullReadHandle(123, "foo", "FooSvc").what());
  std::println ("{}", SG::ExcNullUpdateHandle(123, "foo", "FooSvc").what());
  std::println ("{}", SG::ExcNonConstHandleKey (123, "foo", "FooSvc").what());
  std::println ("{}", SG::ExcInvalidIterator().what());
  std::println ("{}", SG::ExcBadInitializedReadHandleKey().what());
  std::println ("{}", SG::ExcBadContext(ctx, "foo").what());
  std::println ("{}", SG::ExcNoCondCont("foo", "because").what());
  std::println ("{}", SG::ExcBadReadCondHandleInit().what());
  std::println ("{}", SG::ExcNoRange().what());
  std::println ("{}", SG::ExcBadDecorElement(Gaudi::DataHandle::Writer,
                                             1234, "foo.bar").what());
}


int main()
{
  test1();
  return 0;
}
