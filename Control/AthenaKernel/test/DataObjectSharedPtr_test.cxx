/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthenaKernel/test/DataObjectSharedPtr_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Feb, 2016
 * @brief Regression tests for DataObjectSharedPtr.
 */


#if __GNUC__==13
// gcc13 produces a bogus warning for the atomic operations on DataObject.
// This was fixed as of gcc14.
# pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif

#undef NDEBUG
#include "AthenaKernel/DataObjectSharedPtr.h"
#include <cassert>
#include <iostream>


class MyObj : public DataObject
{
public:
  virtual ~MyObj() override { std::cout << "MyObj dtor\n"; }
};


int f (SG::DataObjectSharedPtr<DataObject> ptr)
{
  return ptr->refCount();
}


void test1()
{
  std::cout << "test1\n";
  {
    SG::DataObjectSharedPtr<MyObj> ptr (new MyObj);
    assert (ptr->refCount() == 1);
    {
      SG::DataObjectSharedPtr<MyObj> ptr2 (ptr);
      assert (ptr->refCount() == 2);
    }
    assert (ptr->refCount() == 1);

    SG::DataObjectSharedPtr<MyObj> pp1;
    SG::DataObjectSharedPtr<DataObject> pp2 (pp1);
    assert (f (ptr) == 2);
    assert (ptr->refCount() == 1);
    
    std::cout << "should call dtor now\n";
  }
}


void test2()
{
  std::cout << "test2\n";
  {
    auto uptr = std::make_unique<MyObj>();
    SG::DataObjectSharedPtr<MyObj> ptr (std::move (uptr));
    assert (ptr->refCount() == 1);
    std::cout << "should call dtor now\n";
  }
}


int main()
{
  test1();
  test2();
  return 0;
}
