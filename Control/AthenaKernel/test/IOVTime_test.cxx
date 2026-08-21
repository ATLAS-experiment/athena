/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthenaKernel/test/IOVTime_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Aug, 2026
 * @brief Regression tests for IOVTime.
 */

#undef NDEBUG

#include "AthenaKernel/IOVTime.h"
#include "GaudiKernel/EventIDBase.h"
#include <cassert>
#include <print>
#include <sstream>
#include <stdexcept>


void teststr (const IOVTime& t, const std::string& exp)
{
  assert ((std::string)t == exp);
  std::ostringstream ss1;
  ss1 << t;
  assert (ss1.str() == exp);
  std::ostringstream ss2;
  std::print (ss2, "{}", t);
  assert (ss2.str() == exp);
}


void test1()
{
  std::println ("test1");
  IOVTime t1 (1234);
  assert (t1.isValid());
  assert (t1.isTimestamp());
  assert (!t1.isRunEvent());
  assert (!t1.isBoth());
  assert (t1.run() == IOVTime::MAXEVENT);
  assert (t1.event() == IOVTime::MAXEVENT);
  assert (t1.timestamp() == 1234);
  assert (t1.re_time() == IOVTime::UNDEFRETIME);
  teststr (t1, "[1234]");
  EventIDBase e1 = t1;
  assert (e1.run_number() == EventIDBase::UNDEFNUM);
  assert (e1.lumi_block() == EventIDBase::UNDEFNUM);
  assert (e1.event_number() == EventIDBase::UNDEFEVT);
  assert (e1.time_stamp() == 0);
  assert (e1.time_stamp_ns_offset() == 1234);

  IOVTime t2 (2, 3);
  assert (t2.isValid());
  assert (!t2.isTimestamp());
  assert (t2.isRunEvent());
  assert (!t2.isBoth());
  assert (t2.run() == 2);
  assert (t2.event() == 3);
  assert (t2.timestamp() == IOVTime::UNDEFTIMESTAMP);
  assert (t2.re_time() == (2ull<<32) + 3);
  teststr (t2, "[2,3]");
  EventIDBase e2 = t2;
  assert (e2.run_number() == 2);
  assert (e2.event_number() == EventIDBase::UNDEFEVT);
  assert (e2.lumi_block() == 3);
  assert (e2.time_stamp() == EventIDBase::UNDEFNUM);
  assert (e2.time_stamp_ns_offset() == 0);

  IOVTime t3 (3, 4, 5678);
  assert (t3.isValid());
  assert (t3.isTimestamp());
  assert (t3.isRunEvent());
  assert (t3.isBoth());
  assert (t3.run() == 3);
  assert (t3.event() == 4);
  assert (t3.timestamp() == 5678);
  assert (t3.re_time() == (3ull<<32) + 4);
  teststr (t3, "[3,4:5678]");
  EventIDBase e3 = t3;
  assert (e3.run_number() == 3);
  assert (e3.lumi_block() == 4);
  assert (e3.event_number() == EventIDBase::UNDEFEVT);
  assert (e3.time_stamp() == 0);
  assert (e3.time_stamp_ns_offset() == 5678);

  IOVTime t4 (EventIDBase (5, 6));
  assert (t4.isValid());
  assert (!t4.isTimestamp());
  assert (t4.isRunEvent());
  assert (!t4.isBoth());
  assert (t4.run() == 5);
  assert (t4.event() == 6);
  assert (t4.re_time() == (5ull<<32) + 6);
  teststr (t4, "[5,6]");

  IOVTime t5;
  assert (!t5.isValid());
  assert (!t5.isTimestamp());
  assert (!t5.isRunEvent());
  assert (!t5.isBoth());
  assert (t5.run() == IOVTime::MAXEVENT);
  assert (t5.event() == IOVTime::MAXEVENT);
  assert (t5.timestamp() == IOVTime::UNDEFTIMESTAMP);
  assert (t5.re_time() == IOVTime::UNDEFRETIME);
  teststr (t5, "[]");

  assert (t2 == t2);
  assert (t2 != t4);
  assert (t2 < t4);
  assert (t4 > t2);
  assert (t2 <= t4);
  assert (t2 <= t2);
  assert (t4 >= t2);
  assert (t2 >= t2);

  IOVTime::SortByTimeStamp s1;
  assert (s1 (t3, t1));
  assert (s1 (&t3, &t1));
  IOVTime::SortByRunEvent s2;
  assert (s2 (t3, t2));
  assert (s2 (&t3, &t2));

  t1.setTimestamp (9999);
  assert (t1.timestamp() == 9999);
  t1.setRETime ((4ull<<32) + 10);
  assert (t1.run() == 4);
  assert (t1.event() == 10);
  t1.setRunEvent (5, 11);
  assert (t1.run() == 5);
  assert (t1.event() == 11);
  t1.reset();
  assert (!t1.isValid());
}


int main()
{
  std::println ("AthenaKernel/IOVTime_test");
  test1();
  return 0;
}
