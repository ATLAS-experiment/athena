/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/test/LinkedVarAccessorBase_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jun, 2024
 * @brief Regression tests for LinkedVarAccessorBase.
 */

#undef NDEBUG
#include "AthContainers/tools/LinkedVarAccessorBase.h"
#include "AthContainers/AuxElement.h"
#include "AthContainers/AuxVectorData.h"
#include "CxxUtils/as_const_ptr.h"
#include <iostream>
#include <cassert>


class AuxVectorDataTest
  : public SG::AuxVectorData
{
public:
  AuxVectorDataTest()
  {
    setCache (1, m_arr);
    setCache (2, m_arr);
    setCache (1, CxxUtils::as_const_ptr (m_arr));
    setCache (2, CxxUtils::as_const_ptr (m_arr));
  }
  virtual size_t size_v() const override { return 10; }
  virtual size_t capacity_v() const override { return 10; }
  int m_arr[10] = {0};
};


class LinkedVarAccessorBaseTest
  : public SG::detail::LinkedVarAccessorBase
{
public:
  LinkedVarAccessorBaseTest (SG::auxid_t auxid, SG::auxid_t linkedAuxid)
  {
    m_auxid = auxid;
    m_linkedAuxid = linkedAuxid;
  }
};


void test1()
{
  std::cout << "test1\n";

  AuxVectorDataTest c;
  SG::ConstAuxElement e (&c, 0);
  LinkedVarAccessorBaseTest b (1, 2);
  assert (b.auxid() == 1);
  assert (b.linkedAuxid() == 2);
  assert (b.isAvailable (e));
  assert (b.isAvailable (c));
  LinkedVarAccessorBaseTest b2 (1, 3);
  assert (!b2.isAvailable (e));
  assert (!b2.isAvailable (c));
}


int main()
{
  std::cout << "CxxUtils/LinkedVarAccessorBase_test\n";
  test1();
  return 0;
}
