/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */


const char* const description =
  "This program does some timing tests of CachedValue vs. used std::once.\n"
  "\n"
  "Two payload classes are used: PCV used CachedValue and POnce uses std::once.\n"
  "Two tests are then run for each of these.  TestSeq accesses a set of objects\n"
  "sequentially, while TestRand accesses them randomly (so more contention would\n"
  "be expected from the first test).  The set of tests is run with the number\n"
  "of threads varying from 1 to 8.  The rightmost column printed is the\n"
  "time in seconds.\n";


#include "CxxUtils/CachedValue.h"
#include "CxxUtils/checker_macros.h"
#include "boost/timer/timer.hpp"
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <vector>
#include <cmath>
#include <cstdint>
#include <print>
#include <cstdio>
#include <string>
#include <cassert>


static const uint32_t rngmax = static_cast<uint32_t> (-1);

inline
uint32_t rng_seed (uint32_t& seed)
{
  seed = (1664525*seed + 1013904223);
  return seed;
}
double randf_seed (uint32_t& seed, float rmax, float rmin = 0)
{
  return static_cast<double>(rng_seed(seed)) / (static_cast<double>(rngmax)+1) * (rmax-rmin) + rmin;
}
inline
int randi_seed (uint32_t& seed, int rmax, int rmin = 0)
{
  return static_cast<int> (randf_seed (seed, rmax, rmin));
}


double elapsed (const boost::timer::cpu_timer& timer)
{
  boost::timer::cpu_times t = timer.elapsed();
  return static_cast<double> (t.user + t.system) / 1e9;
}


class POnce
{
public:
  static std::string name() { return "POnce"; }
  POnce (const POnce& other)
    : m_x (other.m_x),
      m_y (other.m_y),
      m_r (other.m_r)
  {
  }
  POnce (double x, double y)
    : m_x(x), m_y(y), m_r()
  {
  }
  double x() const { return m_x; }
  double y() const { return m_y; }
  double r() const
  {
    std::call_once (m_once, [this] { m_r = std::hypot (m_x, m_y); } );
    return m_r;
  }
  double m_x;
  double m_y;
  mutable double m_r ATLAS_THREAD_SAFE;
  mutable std::once_flag m_once;
};


class PCV
{
public:
  static std::string name() { return "PCV  "; }
  PCV (double x, double y)
    : m_x(x), m_y(y)
  {
  }
  double x() const { return m_x; }
  double y() const { return m_y; }
  double r() const
  {
    if (!m_r.isValid()) {
      m_r.set (std::hypot (m_x, m_y));
    }
    return *m_r.ptr();
  }
  double m_x;
  double m_y;
  CxxUtils::CachedValue<double> m_r;
};


template <class OBJ>
class TestBase
{
public:
  TestBase (size_t nobj);
  void fill (size_t nobj, uint32_t& seed);
  void run (size_t nloops, size_t nthreads);
  void report (double t, size_t nloops, size_t nthreads);

  struct runthread
  {
    runthread (TestBase& test, size_t ithread, size_t nloops)
      : m_test(test), m_ithread(ithread), m_nloops(nloops) {}
    void operator()()
    {
      m_test.m_sm.lock_shared();
      m_test.doit (m_ithread, m_nloops);
    }
    TestBase& m_test;
    size_t m_ithread;
    size_t m_nloops;
  };

  virtual double doit (size_t i, size_t nloops) = 0;
  virtual std::string name() const = 0;

  uint32_t m_seed;
  std::vector<OBJ> m_objs;
  std::shared_mutex m_sm;
};



template <class OBJ>
TestBase<OBJ>::TestBase (size_t nobj)
  : m_seed (1234)
{
  fill (nobj, m_seed);
}


template <class OBJ>
void TestBase<OBJ>::fill (size_t nobj, uint32_t& seed)
{
  m_objs.reserve (nobj);
  for (size_t i = 0; i < nobj; i++) {
    double x = randf_seed (seed, 10);
    double y = randf_seed (seed, 10);
    m_objs.emplace_back (x, y);
  }
}


template <class OBJ>
void TestBase<OBJ>::run (size_t nloops, size_t nthreads)
{
  m_sm.lock();
  std::vector<std::thread> threads;
  for (size_t i=0; i < nthreads; i++) {
    threads.emplace_back (runthread (*this, i, nloops));
  }

  m_sm.unlock();
  boost::timer::cpu_timer timer;
  for (size_t i=0; i < nthreads; i++)
    threads[i].join();

  report (elapsed (timer), nloops, nthreads);
}


template <class OBJ>
void TestBase<OBJ>::report (double t, size_t nloops, size_t nthreads)
{
  std::string title = name() + "<"  + OBJ::name() + ">";
  printf ("%s %2zu %4zu %6zu %f\n", title.c_str(), nthreads, nloops, m_objs.size(), t);
}



template <class OBJ>
class TestSeq
  : public TestBase<OBJ>
{
public:
  using TestBase<OBJ>::TestBase;

  virtual double doit (size_t i, size_t nloops) override;
  virtual std::string name() const override { return "TestSeq "; }
};


template <class OBJ>
double TestSeq<OBJ>::doit (size_t /*ithread*/, size_t nloops)
{
  double sum = 0;
  for (size_t iloop = 0; iloop < nloops; ++iloop) {
    for (const OBJ& o : this->m_objs) {
      sum += o.r();
    }
  }
  return sum;
}


template <class OBJ>
class TestRand
  : public TestBase<OBJ>
{
public:
  using TestBase<OBJ>::TestBase;

  virtual double doit (size_t i, size_t nloops) override;
  virtual std::string name() const override { return "TestRand"; }
};


template <class OBJ>
double TestRand<OBJ>::doit (size_t ithread, size_t nloops)
{
  uint32_t seed = this->m_seed + 321 * ithread;
  size_t nobj = this->m_objs.size();
  double sum = 0;
  for (size_t iloop = 0; iloop < nloops*nobj; ++iloop) {
    size_t ndx = randi_seed (seed, nobj);
    sum += this->m_objs[ndx].r();
  }
  return sum;
}


int main()
{
  std::println ("{}", description);
  std::println ();
  
  for (int nthread = 1; nthread <= 8; ++nthread) {
    TestSeq<PCV> (10000).run (10000, nthread);
    TestSeq<POnce> (10000).run (10000, nthread);
    TestRand<PCV> (10000).run (10000, nthread);
    TestRand<POnce> (10000).run (10000, nthread);
  }
  return 0;
}
