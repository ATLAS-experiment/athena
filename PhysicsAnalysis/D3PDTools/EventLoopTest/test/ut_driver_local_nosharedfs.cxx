/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <EventLoop/Global.h>

#include <EventLoop/Job.h>
#include <EventLoop/LocalDriver.h>
#include <EventLoopTest/UnitTest.h>

//
// main program
//

using namespace EL;

int main ()
{
  LocalDriver driver;
  driver.options()->setBool (Job::optBatchSharedFileSystem, true);
  UnitTest ut ("local");
  ut.cleanup = false;
  return ut.run (driver);
}
