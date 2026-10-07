/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/atomic_bounded_decrement_test.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2023
 * @brief Unit tests for atomic_bounded_decrement.
 */


#undef NDEBUG
#include "CxxUtils/atomic_bounded_decrement.h"
#include "TestTools/random.h"
#include <mutex>
#include <thread>
#include <shared_mutex>
#include <limits>
#include <print>
#include <cassert>

using CxxUtils::atomic_bounded_decrement;


template <class T>
void test1_test()
{
  std::atomic<T> v;
  v = 10;
  assert (atomic_bounded_decrement (&v, static_cast<T>(0)) == 10);
  assert (v == 9);
  assert (atomic_bounded_decrement (&v, static_cast<T>(0), static_cast<T>(5)) == 9);
  assert (v == 4);
  assert (atomic_bounded_decrement (&v, static_cast<T>(0), static_cast<T>(5)) == 4);
  assert (v == 4);
  assert (atomic_bounded_decrement (&v, static_cast<T>(0), static_cast<T>(3)) == 4);
  assert (v == 1);
  assert (atomic_bounded_decrement (&v, static_cast<T>(0), static_cast<T>(3)) == 1);
  assert (v == 1);
  assert (atomic_bounded_decrement (&v, static_cast<T>(0)) == 1);
  assert (v == 0);
  assert (atomic_bounded_decrement (&v, static_cast<T>(0)) == 0);
  assert (v == 0);
  assert (atomic_bounded_decrement (&v, static_cast<T>(10)) == 0);
  assert (v == 0);
}


void test1()
{
  std::println ("test1");

  test1_test<char>();
  test1_test<signed char>();
  test1_test<unsigned char>();
  test1_test<short>();
  test1_test<unsigned short>();
  test1_test<int>();
  test1_test<unsigned int>();
  test1_test<long>();
  test1_test<unsigned long>();
  test1_test<long long>();
  test1_test<unsigned long long>();
  test1_test<char16_t>();
  test1_test<char32_t>();
  test1_test<wchar_t>();
}


std::shared_timed_mutex start_mutex;


template <class T>
class test2_Thread
{
public:
  test2_Thread (int ithread, std::atomic<T>& x);
  void operator()();

private:
  int m_ithread;
  uint32_t m_seed;
  std::atomic<T>& m_x;
};


template <class T>
test2_Thread<T>::test2_Thread (int ithread, std::atomic<T>& x)
  : m_ithread (ithread),
    m_seed (ithread * 123),
    m_x (x)
{
}


template <class T>
void test2_Thread<T>::operator()()
{
  std::shared_lock<std::shared_timed_mutex> lock (start_mutex);

  const int niter = 1000000;

  for (int i=0; i < niter; i++) {
    T val = static_cast<T> (Athena_test::randi_seed (m_seed, 10));
    T orig = atomic_bounded_decrement (&m_x, static_cast<T>(0), val);
    T x = m_x;
    assert (orig >= 0 && orig < 60000);
    assert (x >= 0 && x < 60000);
    if (m_ithread == 0 && (i%1000)==0) m_x = 5;
  }
}


template <class T>
void test2_test()
{
  const int nthread = 4;
  std::thread threads[nthread];
  start_mutex.lock();

  std::atomic<T> x = 5;

  threads[0] = std::thread (test2_Thread<T> (0, x));
  threads[1] = std::thread (test2_Thread<T> (1, x));
  threads[2] = std::thread (test2_Thread<T> (2, x));
  threads[3] = std::thread (test2_Thread<T> (3, x));

  // Try to get the threads starting as much at the same time as possible.
  start_mutex.unlock();
  for (int i=0; i < nthread; i++)
    threads[i].join();
}


void test2()
{
  std::println ("test2");

  test2_test<char>();
  test2_test<signed char>();
  test2_test<unsigned char>();
  test2_test<short>();
  test2_test<unsigned short>();
  test2_test<int>();
  test2_test<unsigned int>();
  test2_test<long>();
  test2_test<unsigned long>();
  test2_test<long long>();
  test2_test<unsigned long long>();
  test2_test<char16_t>();
  test2_test<char32_t>();
  test2_test<wchar_t>();
}


int main()
{
  test1();
  test2();
  return 0;
}
