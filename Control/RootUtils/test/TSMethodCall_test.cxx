/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// $Id$
/**
 * @file RootUtils/test/TSMethodCall_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2017
 * @brief Regression tests for TSMethodCall.
 */


#undef NDEBUG
#include "RootUtils/TSMethodCall.h"
#include "TClass.h"
#include <shared_mutex>
#include <thread>
#include <iostream>
#include <cassert>


struct TestThread
{
  TestThread (RootUtils::TSMethodCall& meth,
              int n,
              std::shared_mutex& sm)
    : m_meth(meth), m_n(n), m_sm(sm)
  {}

  static void test1 (RootUtils::TSMethodCall& tsmeth)
  {
    std::string a = "asdf";
    std::string b;
    TMethodCall* meth = tsmeth.call();
    meth->ResetParam();
    meth->SetParam (reinterpret_cast<Long_t>(&a));
    meth->Execute (&b);
    assert (b == "asdf");
    assert (a == "asdf");
  }

  void operator() ()
  {
    std::shared_lock<std::shared_mutex> guard (m_sm);
    for (int i = 0; i < m_n; i++) {
      test1 (m_meth);
    }
  }

  RootUtils::TSMethodCall& m_meth;
  int m_n;
  std::shared_mutex& m_sm;
};


void test1()
{
  std::cout << "test1\n";
  RootUtils::TSMethodCall meth;
  TClass* cl = TClass::GetClass("std::string");
  meth.setProto (cl, "operator=", "const std::string&");
  assert (meth.call() != nullptr);

  TestThread::test1 (meth);
  const int nthread = 10;
  const int niter = 10000;
  std::thread threads[nthread];

  // Declare the mutex in an outer scope
  std::shared_mutex sm;
  {
    // Lock in a nested scope to avoid triggering localMutex warning
    std::unique_lock<std::shared_mutex> lock(sm);

    // Create threads while holding the lock
    for (int i = 0; i < nthread; i++)
      threads[i] = std::thread(TestThread(meth, niter, sm));
    // Lock is released automatically when this block ends
    // Try to get the threads starting as much at the same time as possible.
  }
  for (int i=0; i < nthread; i++)
    threads[i].join();
}


int main()
{
  test1();
  return 0;
}

