/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/** @file read_athena_statm_test.cxx
 * @brief  unit test for read_athena_statm 
 *
 * @author Paolo Calafiura <pcalafiura@lbl.gov> -ATLAS Collaboration
 **/

#undef NDEBUG

#include <cassert>
#include <print>

#include "CxxUtils/read_athena_statm.h"


int main() {
  const std::string appName = "read_statm_test";
  std::println ("*** {} starts ***", appName);

  athena_statm s = read_athena_statm();
  std::println ("read_athena_statm reports process size (in pages): VM {} RSS {}",
                s.vm_pages, s.rss_pages);
  //let's do something...
  assert(s.vm_pages >= s.rss_pages);
  //all done
  std::println ("*** {} OK ***", appName);
  return 0;
}
