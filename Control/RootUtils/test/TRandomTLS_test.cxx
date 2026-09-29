/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */

#undef NDEBUG

#include "CxxUtils/checker_macros.h"
#include "RootUtils/TRandomTLS.h"
#include "TRandom3.h"

#include <cassert>
#include <mutex>
#include <print>
#include <set>
#include <thread>

/// Helper class
struct Test {
  void rnd() const
  {
    const int v = m_rnd->Integer(1000);
    std::scoped_lock lock(m_mutex);
    m_values.insert(v);
  }
  RootUtils::TRandomTLS<TRandom3> m_rnd;
  mutable std::mutex m_mutex;
  mutable std::set<int> m_values ATLAS_THREAD_SAFE;
};


void test_compilation()
{
  RootUtils::TRandomTLS<TRandom3> rnd(42);
  rnd->Rndm();
  (*rnd).Rndm();
  assert(rnd.get());
}


void test_unique()
{
  Test test;
  std::vector<std::thread> threads;
  constexpr int Nthreads = 4;

  // Launch threads and wait
  for (size_t i = 0; i < Nthreads; i++) {
    threads.emplace_back(&Test::rnd, &test);
  }
  for (auto& th : threads) th.join();

  // Each thread should have created a unique random number
  assert(test.m_values.size() == Nthreads);

  for (int v : test.m_values) {
    std::println ("{}", v);
  }
}


int main()
{
  test_compilation();
  test_unique();

  return 0;
}
