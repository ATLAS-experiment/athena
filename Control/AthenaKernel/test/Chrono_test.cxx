/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// $Id$
/**
 * @file AthenaKernel/Chrono_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Aug, 2014
 * @brief Regression tests for Chrono.
 */

#undef NDEBUG


#include "AthenaKernel/Chrono.h"
#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/ChronoEntity.h"
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <cassert>


ATLAS_NO_CHECK_FILE_THREAD_SAFETY;


int count = 0;


class ChronoSvcTest
  : public implements<IChronoSvc>
{
public:
  virtual       ChronoEntity* chronoStart   ( const std::string& /*t*/ ) override
  {++count; return 0;}
  virtual       ChronoEntity* chronoStop   ( const std::string& /*t*/ ) override
  {--count; return 0;}

  virtual ChronoTime  chronoDelta   ( const ChronoTag& , ChronoType ) override { std::abort(); }
  virtual void  chronoPrint   ( const ChronoTag& ) override { std::abort(); }
  virtual ChronoStatus  chronoStatus ( const ChronoTag& ) override { std::abort(); }
  virtual const ChronoEntity* chrono ( const ChronoTag& ) const override { std::abort(); }
};


void testit (bool throw_p)
{
  assert (count == 1);
  if (throw_p)
    throw std::runtime_error("test");
}


void test1()
{
  std::cout << "test1\n";
  ChronoSvcTest svc;

  {
    Athena::Chrono chrono ("test", &svc);
    testit(false);
  }
  assert (count == 0);

  try {
    Athena::Chrono chrono ("test", &svc);
    testit(true);
  }
  catch (const std::runtime_error&)
  {
  }
  assert (count == 0);
}


int main()
{
  test1();
  return 0;
}

          
