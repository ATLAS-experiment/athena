/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file IRCUSVC_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Aug, 2016
 * @brief Regression tests for RCUObject.
 */

#undef NDEBUG

#include "AthenaKernel/IRCUSvc.h"
#include <cassert>
#include <print>
#include <cstdlib>


using Athena::IRCUSvc;
using Athena::IRCUObject;
using Athena::RCUObject;
using Athena::RCURead;


class TestRCUSvc
  : public implements<IRCUSvc>
{
public:
  virtual StatusCode remove (IRCUObject* /*obj*/) override { return StatusCode::SUCCESS; }
  virtual size_t getNumSlots() const override { return 1; }
  virtual void add (IRCUObject* obj) override
  { m_added = obj; }

  IRCUObject* m_added = nullptr;
};


struct Payload
{
  Payload (int a, int b) : 
    m_a(a), m_b(b)
  {}
  Payload& operator= (const Payload&) = default;

  int m_a;
  int m_b;
};


void test1()
{
  std::println ("test1");
  TestRCUSvc svc;
  assert (svc.m_added == nullptr);
  std::unique_ptr<RCUObject<Payload> > obj = svc.newrcu<Payload> (10, 20);
  RCURead<Payload> r (*obj);
  assert (r->m_a == 10);
  assert (r->m_b == 20);
  assert (svc.m_added == obj.get());
}


int main()
{
  test1();
  return 0;
}
