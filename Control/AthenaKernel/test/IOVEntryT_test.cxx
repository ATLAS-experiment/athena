/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthenaKernel/test/IOVEntryT_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Aug, 2026
 * @brief Regression tests for IOVEntryT.
 */

#undef NDEBUG

#include "AthenaKernel/IOVEntryT.h"
#include <cassert>
#include <print>
#include <stdexcept>


void test1()
{
  std::println ("test1");
  int i1 = 1;
  IOVEntryT<int> t1 (&i1, EventIDRange(EventIDBase (1, 0),EventIDBase (8, 0)));
  int i2 = 2;
  IOVEntryT<int> t2 (&i2, EventIDRange(EventIDBase (2, 0),EventIDBase (9, 0)));

  assert (t1.objPtr() == &i1);
  assert (t1.range().start() == EventIDBase (1, 0));

  IOVEntryT<int>::IOVEntryTStartCritereon tstart;
  IOVEntryT<int>::IOVEntryTStopCritereon tstop;

  assert (tstart(t2, t1));
  assert (tstart(&t2, &t1));
  assert (!tstart(t1, t2));

  assert (tstop(t1, t2));
  assert (tstop(&t1, &t2));
  assert (!tstop(t2, t1));

  std::ostringstream exp;
  std::print (exp, "{{[1,0] - [8,0]}} {}", static_cast<void*> (&i1));
  std::ostringstream ss1;
  ss1 << t1;
  assert (ss1.str() == exp.str());
  std::ostringstream ss2;
  std::print (ss2, "{}", t1);
  assert (ss2.str() == exp.str());

  int i3 = 3;
  t1.setPtr (&i3);
  t1.setRange (EventIDRange(EventIDBase (2, 0), EventIDBase (6, 0)));
  assert (t1.objPtr() == &i3);
  assert (t1.range().stop() == EventIDBase (6, 0));
}


int main()
{
  std::println ("AthenaKernel/IOVEntryT_test");
  test1();
  return 0;
}
