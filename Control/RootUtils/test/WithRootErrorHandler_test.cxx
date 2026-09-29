/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file RootUtils/test/WithRootErrorHandler_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Mar, 2021
 * @brief Tests for WithRootErrorHandler.
 */


#undef NDEBUG
#include "RootUtils/WithRootErrorHandler.h"
#include "TInterpreter.h"
#include "TError.h"
#include <iostream>
#include <string>
#include <print>
#include <cassert>


struct TestHand
{
  TestHand (const std::string& name, bool passOn)
    : m_name (name), m_passOn (passOn)
  { }
  bool operator() (int level, Bool_t abort, const char* loc, const char* msg);

  std::string m_name;
  bool m_passOn;
};


bool TestHand::operator() (int /*level*/, Bool_t /*abort*/,
                           const char* /*loc*/, const char* msg)
{
  std::println ("{}: {}", m_name, msg);
  std::cout.flush();
  return m_passOn;
}


void test1()
{
  // If we call ::Error without having first initialized the interpreter,
  // it can deadlock.
  TInterpreter::Instance();

  std::println ("test1");
  std::cout.flush();

  ::Error ("foo1", "Bar1");
  std::cerr.flush();
  std::cout.flush();
  {
    RootUtils::WithRootErrorHandler hand { TestHand("try1", true) };
    ::Error ("foo2", "Bar2");
    std::cerr.flush();
    std::cout.flush();
  }
  {
    RootUtils::WithRootErrorHandler hand1 { TestHand("try1", true) };
    RootUtils::WithRootErrorHandler hand2 { TestHand("try2", true) };
    ::Error ("foo3", "Bar3");
    std::cerr.flush();
    std::cout.flush();
  }
  {
    RootUtils::WithRootErrorHandler hand1 { TestHand("try1", true) };
    RootUtils::WithRootErrorHandler hand2 { TestHand("try2", false) };
    ::Error ("foo4", "Bar4");
    std::cerr.flush();
    std::cout.flush();
  }
}


int main()
{
  std::println ("RootUtils/WithRootErrorHandler_test");
  test1();
  return 0;
}
