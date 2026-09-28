/*
 * Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file AthenaKernel/test/TypelessDataBucket_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2026
 * @brief Regression test for TypelessDataBucket.
 */

#undef NDEBUG
#include "AthenaKernel/TypelessDataBucket.h"
#include "AthenaKernel/BaseInfo.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/ClassID_traits.h"
#include <atomic>
#include <cassert>
#include <print>


struct X1
{
  X1(int the_a) : a(the_a) { ++count; }
  virtual ~X1() { --count; }
  int a;
  static std::atomic<int> count;
};
std::atomic<int> X1::count = 0;

struct X2
  : public virtual X1 // Make it virtual so that hard casting won't work.
{
  X2(int the_a, int the_b) : X1(the_a), b(the_b) {}
  int b;
};

CLASS_DEF(X1, 8011, 1)
CLASS_DEF(X2, 8012, 1)
SG_BASES(X2, X1);


void test1()
{
  std::println ("test1");

  X2 x2 (10, 20);
  X1& x1 = x2;
  assert (X1::count == 1);

  {
    TypelessDataBucket db (&x2, SG::BaseInfo<X2>::baseinfo());
    assert (db.clID() == ClassID_traits<X2>::ID());
    assert (db.object() == &x2);

    assert (db.cast (ClassID_traits<X2>::ID(), nullptr, true) == &x2);
    assert (db.cast (ClassID_traits<X2>::ID(), nullptr, false) == nullptr);
    assert (db.cast (ClassID_traits<X1>::ID(), nullptr, true) == &x1);
    assert (db.cast (ClassID_traits<X1>::ID(), nullptr, false) == nullptr);
    assert (db.cast (123, nullptr, true) == nullptr);

    assert (db.cast (typeid(X2), nullptr, true) == &x2);
    assert (db.cast (typeid(X2), nullptr, false) == nullptr);
    assert (db.cast (typeid(X1), nullptr, true) == &x1);
    assert (db.cast (typeid(X1), nullptr, false) == nullptr);
    assert (db.cast (typeid(int), nullptr, true) == nullptr);

    db.lock();
    db.relinquish();
    assert (db.object() == nullptr);
  }
  assert (X1::count == 1);

  {
    TypelessDataBucket db (new X2(20, 30), SG::BaseInfo<X2>::baseinfo());
    assert (X1::count == 2);
  }
  assert (X1::count == 1);
}


int main()
{
  std::println ("AthenaKernel/TypelessDataBucket_test");
  test1();
  return 0;
}
