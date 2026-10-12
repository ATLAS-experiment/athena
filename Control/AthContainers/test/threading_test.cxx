/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/test/threading_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2013
 * @brief Regression tests for threading.
 */


#undef NDEBUG
#include "AthContainers/tools/threading.h"
#include "AthContainers/tools/upgrade_mutex.h"
#include "AthContainers/tools/thread_specific_ptr.h"
#include <print>
#include <cassert>


class test_mutex
{
public:
  void lock_shared() const { std::println ("lock_shared"); }
  void unlock_shared() const { std::println ("unlock_shared"); }
  void lock_upgrade() { std::println ("lock_upgrade"); }
  void unlock_upgrade() { std::println ("unlock_upgrade"); }
  void lock() { std::println ("lock"); }
  void unlock() { std::println ("unlock"); }
  void unlock_upgrade_and_lock() { std::println ("unlock_upgrade_and_lock"); }
};


void test1()
{
  using AthContainers_detail::strict_shared_lock;

  std::println ("test1");
  test_mutex m;
  strict_shared_lock<test_mutex> lock (m);
}


void test2()
{
  using AthContainers_detail::upgrading_lock;
  using AthContainers_detail::upgrade_mutex;

  std::println ("test2");
  test_mutex m;
  {
    upgrading_lock<test_mutex> lock (m);
  }
  {
    upgrading_lock<test_mutex> lock (m);
    lock.upgrade();
  }
  {
    upgrading_lock<test_mutex> lock (m);
    lock.upgrade();
    lock.upgrade();
  }

  upgrade_mutex mm;
  upgrading_lock<upgrade_mutex> lock (mm);
  lock.upgrade();
  lock.upgrade();
}


void test3()
{
  using AthContainers_detail::fence_acq_rel;
  using AthContainers_detail::fence_seq_cst;
  using AthContainers_detail::mutex;
  using AthContainers_detail::lock_guard;

  std::println ("test3");

  fence_acq_rel();
  fence_seq_cst();

  mutex m;
  {
    lock_guard<mutex> g (m);
  }
}


struct Payload
{
  Payload(int i) : m_i(i) { std::println ("ctor {}", i); }
  ~Payload() { std::println ("dtor {}", m_i); }
  Payload& operator= (const Payload&) = default;
  int m_i;
};

void test4()
{
  using AthContainers_detail::thread_specific_ptr;

  std::println ("test4");
  
  thread_specific_ptr<Payload> p;
  assert (p.get() == 0);
  assert (p.release() == 0);
  p.reset (new Payload(1));
  assert (p.get()->m_i == 1);
  assert (p->m_i == 1);
  assert ((*p).m_i == 1);
  Payload* pp = p.release();
  assert (pp->m_i == 1);
  assert (p.get() == 0);
  p.reset (new Payload(2));
}


int main()
{
  test1();
  test2();
  test3();
  test4();
  return 0;
}
