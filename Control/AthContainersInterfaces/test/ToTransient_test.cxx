/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainersInterfaces/test/ToTransient_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jul, 2025
 * @brief Regression test for ToTransient.
 */


#undef NDEBUG
#include "AthContainersInterfaces/ToTransient.h"
#include <cassert>
#include <print>


class C {} ;
namespace SG {
template<> class ToTransient<C>
{
public:
};
}


int main()
{
  std::println ("AthContainersInterfaces/ToTransient_test");
  assert( SG::noToTransient<int>() );
  assert( !SG::noToTransient<C>() );
  return 0;
}
