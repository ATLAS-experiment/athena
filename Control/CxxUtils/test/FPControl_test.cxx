/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file CxxUtils/test/FPControl_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date May, 2021
 * @brief Regression tests for FPControl.
 */

#undef NDEBUG
#include "CxxUtils/FPControl.h"
#include "CxxUtils/checker_macros.h"
#include "CxxUtils/features.h"
#include <cassert>
#include <iostream>
#include <signal.h>
#include <setjmp.h>


const float blam = 0;
jmp_buf out ATLAS_THREAD_SAFE;   // no multi-threading here


void sighandler (int)
{
  siglongjmp (out, 1);
}


float testit (bool mask, bool expectsig)
{
  // Declaring this volatile avoids a warning that longjmp
  // might clobber it.
  CxxUtils::FPControl ctl;
  if (mask) {
    ctl.disable (CxxUtils::FPControl::Exc::divbyzero);
  }
  volatile float x = 1;
  signal (SIGFPE, sighandler);
  if (sigsetjmp (out, 1) == 0) {
    x = x / blam;
    assert (!expectsig);
  }
  else
    assert (expectsig);
  signal (SIGFPE, SIG_DFL);
  return x;
}


void test1()
{
  std::cout << "test1\n";

#if HAVE_FEENABLEEXCEPT
  fedisableexcept (FE_DIVBYZERO);
  feclearexcept (FE_DIVBYZERO);
  testit (false, false);
  assert (fetestexcept (FE_DIVBYZERO));

  fedisableexcept (FE_DIVBYZERO);
  feclearexcept (FE_DIVBYZERO);
  testit (true, false);
  assert (!fetestexcept (FE_DIVBYZERO));

  feenableexcept (FE_DIVBYZERO);
#ifdef __aarch64__
  // Some aarch64 implementations do not support trapping on FPEs :(...
  if (fegetexcept() & FE_DIVBYZERO)
#endif
  {
    feclearexcept (FE_DIVBYZERO);
    testit (false, true);

    feenableexcept (FE_DIVBYZERO);
    feclearexcept (FE_DIVBYZERO);
    testit (true, false);
    assert (!fetestexcept (FE_DIVBYZERO));
  }
#endif
}


int main()
{
  std::cout << "CxxUtils/FPControl_test\n";
  test1();
  return 0;
}
