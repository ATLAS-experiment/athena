/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file RefCountedPtr_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Oct, 2025
 * @brief Regression tests for RefCountedPtr.
 */


#undef NDEBUG
#include "CxxUtils/RefCountedPtr.h"
#include <memory>
#include <iostream>
#include <cassert>


class TestRC
{
public:
  void addRef() { ++m_rc; }
  void release() { --m_rc; }
  unsigned m_rc = 0;
};


class DerRC : public TestRC
{
public:
};


class TestRC_Const
{
public:
  void addRef() const {}
  void release() const {}
  int m_x = 42;
};


void test1()
{
  std::cout << "test1\n";
  TestRC rc1;
  assert (rc1.m_rc == 0);

  CxxUtils::RefCountedPtr<TestRC> p1;
  assert (p1.get() == nullptr);
  assert (!p1.isValid());
  assert (!static_cast<bool>(p1));
  assert (!p1);

  {
    CxxUtils::RefCountedPtr<TestRC> p2 (&rc1);
    assert (rc1.m_rc == 1);
    assert (p2.get() == &rc1);
    assert (p2.isValid());
    assert (static_cast<bool>(p2));
    assert (!!p2);
    assert ((*p2).m_rc == 1);
    assert (p2->m_rc == 1);
    TestRC* pp = p2;
    assert (pp == &rc1);
  }
  assert (rc1.m_rc == 0);

  p1 = &rc1;
  assert (rc1.m_rc == 1);
  assert (p1.get() == &rc1);

  TestRC rc2;

  p1 = &rc2;
  assert (rc1.m_rc == 0);
  assert (rc2.m_rc == 1);
  assert (p1.get() == &rc2);
  p1 = nullptr;
  assert (rc1.m_rc == 0);
  assert (rc2.m_rc == 0);
  assert (p1.get() == nullptr);

  p1.reset (&rc1);
  assert (rc1.m_rc == 1);
  assert (rc2.m_rc == 0);
  assert (p1.get() == &rc1);
  p1.reset (&rc2);
  assert (rc1.m_rc == 0);
  assert (rc2.m_rc == 1);
  assert (p1.get() == &rc2);
  p1.reset (nullptr);
  assert (rc1.m_rc == 0);
  assert (rc2.m_rc == 0);
  assert (p1.get() == nullptr);

  p1.reset (&rc1);
  CxxUtils::RefCountedPtr<TestRC> p3 (p1);
  assert (rc1.m_rc == 2);
  assert (p1.get() == &rc1);
  assert (p3.get() == &rc1);

  CxxUtils::RefCountedPtr<TestRC> p4 (std::move(p3));
  assert (rc1.m_rc == 2);
  assert (p1.get() == &rc1);
  assert (p3.get() == nullptr);
  assert (p4.get() == &rc1);

  p3 = p1;
  assert (rc1.m_rc == 3);
  assert (p1.get() == &rc1);
  assert (p3.get() == &rc1);
  assert (p4.get() == &rc1);

  p3 = &rc2;
  assert (rc1.m_rc == 2);
  assert (rc2.m_rc == 1);
  assert (p1.get() == &rc1);
  assert (p3.get() == &rc2);
  assert (p4.get() == &rc1);

  p1 = std::move(p3);
  assert (rc1.m_rc == 1);
  assert (rc2.m_rc == 1);
  assert (p1.get() == &rc2);
  assert (p3.get() == nullptr);
  assert (p4.get() == &rc1);

  DerRC rc3;
  CxxUtils::RefCountedPtr<DerRC> p5 (&rc3);
  assert (rc3.m_rc == 1);
  assert (p5.get() == &rc3);

  CxxUtils::RefCountedPtr<TestRC> p6 (p5);
  assert (rc3.m_rc == 2);
  assert (p5.get() == &rc3);
  assert (p6.get() == &rc3);

  CxxUtils::RefCountedPtr<TestRC> p7 (std::move(p5));
  assert (rc3.m_rc == 2);
  assert (p5.get() == nullptr);
  assert (p6.get() == &rc3);
  assert (p7.get() == &rc3);
  
  p5 = &rc3;
  assert (rc1.m_rc == 1);
  assert (rc2.m_rc == 1);
  assert (rc3.m_rc == 3);
  assert (p1.get() == &rc2);
  assert (p3.get() == nullptr);
  assert (p4.get() == &rc1);
  assert (p5.get() == &rc3);
  assert (p6.get() == &rc3);
  assert (p7.get() == &rc3);

  p1 = p5;
  assert (rc1.m_rc == 1);
  assert (rc2.m_rc == 0);
  assert (rc3.m_rc == 4);
  assert (p1.get() == &rc3);
  assert (p3.get() == nullptr);
  assert (p4.get() == &rc1);
  assert (p5.get() == &rc3);
  assert (p6.get() == &rc3);
  assert (p7.get() == &rc3);

  p3 = std::move(p5);
  assert (rc1.m_rc == 1);
  assert (rc2.m_rc == 0);
  assert (rc3.m_rc == 4);
  assert (p1.get() == &rc3);
  assert (p3.get() == &rc3);
  assert (p4.get() == &rc1);
  assert (p5.get() == nullptr);
  assert (p6.get() == &rc3);
  assert (p7.get() == &rc3);

  auto up = std::make_unique<DerRC>();
  CxxUtils::RefCountedPtr<TestRC> p8 (std::move(up));
  assert (p8->m_rc == 1);
  // cppcheck-suppress accessMoved; intentional
  assert (!up);
}


void test2()
{
  TestRC_Const rc1;
  CxxUtils::RefCountedPtr<const TestRC_Const> p1 (&rc1);
  const CxxUtils::RefCountedPtr<const TestRC_Const>& cp1 = p1;

  assert (p1.get() == &rc1);
  assert (cp1.get() == &rc1);

  const TestRC_Const* cp = p1;
  assert (cp == &rc1);
  cp = cp1;
  assert (cp == &rc1);

  assert (p1->m_x == 42);
  assert (cp1->m_x == 42);
  assert ((*p1).m_x == 42);
  assert ((*cp1).m_x == 42);
}


int main()
{
  std::cout << "CxxUtils/RefCountedPtr_test\n";
  test1();
  test2();
  return 0;
}
