/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file  SGTools/test/WithEventContext_test.cxx
 * @author scott snyder
 * @date Sep, 2026
 * @brief Unit test for WithEventContext.
 */

#undef NDEBUG


#include "SGTools/WithEventContext.h"
#include "AthenaKernel/ExtendedEventContext.h"
#include "AthenaKernel/proxyDictFromEventContext.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include <print>
#include <cassert>


void test1()
{
  std::println ("test1");

  IProxyDict* s1 = reinterpret_cast<IProxyDict*>(0x1000);
  EventContext ctx1 (1);
  ctx1.setExtension (Atlas::ExtendedEventContext (s1));
  Gaudi::Hive::setCurrentContext (ctx1);

  IProxyDict* s2 = reinterpret_cast<IProxyDict*>(0x2000);
  EventContext ctx2 (2);
  ctx2.setExtension (Atlas::ExtendedEventContext (s2));

  {
    SG::WithEventContext save;
    assert (Gaudi::Hive::currentContext().evt() == 1);
    assert (Atlas::proxyDictFromEventContext() == s1);
    Gaudi::Hive::setCurrentContext (ctx2);
    assert (Gaudi::Hive::currentContext().evt() == 2);
    assert (Atlas::proxyDictFromEventContext() == s2);
  }
  assert (Gaudi::Hive::currentContext().evt() == 1);
  assert (Atlas::proxyDictFromEventContext() == s1);

  {
    SG::WithEventContext save (ctx2);
    assert (Gaudi::Hive::currentContext().evt() == 2);
    assert (Atlas::proxyDictFromEventContext() == s2);
  }
  assert (Gaudi::Hive::currentContext().evt() == 1);
  assert (Atlas::proxyDictFromEventContext() == s1);

  {
    SG::WithEventContext save (s2);
    assert (Gaudi::Hive::currentContext().evt() == 1);
    assert (Atlas::proxyDictFromEventContext() == s2);
  }
  assert (Gaudi::Hive::currentContext().evt() == 1);
  assert (Atlas::proxyDictFromEventContext() == s1);
}



int main()
{
  std::println ("WithEventContext_test");
  test1();
  return 0;
}
