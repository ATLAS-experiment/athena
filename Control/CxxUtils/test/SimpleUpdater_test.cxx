/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file CxxUtils/test/SimpleUpdater_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Dec, 2020
 * @brief Tests for SimpleUpdater.
 */


#undef NDEBUG
#include "CxxUtils/SimpleUpdater.h"
#include <algorithm>
#include <vector>
#include <memory>
#include <print>
#include <cassert>


class Payload
{
public:
  typedef std::vector<int> vec_t;
  Payload (vec_t& vec, int x) : m_vec(vec), m_x (x)
  { vec.push_back (x); }
  ~Payload()
  {
    vec_t::iterator i = std::find (m_vec.begin(), m_vec.end(), m_x);
    if (i != m_vec.end()) m_vec.erase (i);
  }
  vec_t& m_vec;
  int m_x;
};


void test1()
{
  std::println ("test1");
  std::vector<int> log;

  {
    using SU = CxxUtils::SimpleUpdater<Payload>;
    SU su;
    su.update (std::make_unique<Payload> (log, 1), SU::defaultContext());
    assert (su.get().m_x == 1);
    su.discard (std::make_unique<Payload> (log, 2));
    su.update (std::make_unique<Payload> (log, 3), SU::defaultContext());
    assert (su.get().m_x == 3);
    su.quiescent (SU::defaultContext());
    assert (log == (std::vector<int> {1, 2, 3}));
  }
  assert (log.empty());

  {
    using SU = CxxUtils::SimpleUpdater<Payload>;
    SU su;
    su.update (std::make_unique<Payload> (log, 1), SU::defaultContext());
    assert (su.get().m_x == 1);
    su.discard (std::make_unique<Payload> (log, 2));
    su.update (std::make_unique<Payload> (log, 3), SU::defaultContext());
    assert (su.get().m_x == 3);
    su.discard (std::make_unique<Payload> (log, 4));
    assert (su.get().m_x == 3);
    assert (log == (std::vector<int> {1, 2, 3, 4}));

    su.clean();
    assert (su.get().m_x == 3);
    assert (log == (std::vector<int> {3}));
  }
  assert (log.empty());
}


int main()
{
  std::println ("CxxUtils/SimpleUpdater_test");
  test1();
  return 0;
}
