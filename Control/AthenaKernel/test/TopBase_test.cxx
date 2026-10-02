/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthenaKernel/test/TopBase_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2018
 * @brief Unit test for TopBase.
 */


#undef NDEBUG
#include "AthenaKernel/TopBase.h"
#include "AthenaKernel/CLASS_DEF.h"
#include <print>
#include <cassert>


class C1 {};
class C2 : public C1 {};
class C3 : public C2 {};
CLASS_DEF(C1, 344426298, 0);
CLASS_DEF(C2, 344426299, 0);
CLASS_DEF(C3, 344426300, 0);
SG_BASES (C2, C1);
SG_BASES (C3, C2);


class C11 {};
class C12 : public C11 {};
class C13 : public C12 {};
CLASS_DEF(C11, 444426298, 0);
CLASS_DEF(C13, 444426300, 0);
SG_BASES (C12, C11);
SG_BASES (C13, C12);


void test1()
{
  std::println ("test1");

  assert ((std::is_same<SG::TopBase<C1>::type, C1>::value));
  assert ((std::is_same<SG::TopBase<C2>::type, C1>::value));
  assert ((std::is_same<SG::TopBase<C3>::type, C1>::value));

  assert ((std::is_same<SG::TopBase<C11>::type, C11>::value));
  assert ((std::is_same<SG::TopBase<C12>::type, C11>::value));
  assert ((std::is_same<SG::TopBase<C13>::type, C13>::value));
}


int main()
{
  std::println ("TopBase_test");
  test1();
  return 0;
}
