/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthenaKernel/test/ILockableTool_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2022
 * @brief Unit test for ILockableTool.
 */


#undef NDEBUG
#include "AthenaKernel/ILockableTool.h"
#include <print>
#include <cassert>


class TestTool : public ILockableTool
{
public:
  virtual void lock_shared() const override { std::println ("lock"); }
  virtual void unlock_shared() const override  { std::println ("unlock"); }
};


void test1()
{
  std::println ("test1");
  TestTool tool;
  {
    Athena::ToolLock lock (tool);
  }
  std::println ("test1 exiting");
}


int main()
{
  std::println ("AthenaKernel/ILockableTool_test");
  test1();
  return 0;
}
