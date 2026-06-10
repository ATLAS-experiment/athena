/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <EventLoop/Global.h>

#include <EventLoop/DirectDriver.h>
#include <EventLoopTest/UnitTest.h>

//
// main program
//

using namespace EL;

int main ()
{
  DirectDriver driver;
  UnitTest ut ("direct");
  // ut.cleanup = false;
  // ut.location = "$HOME/unit-test.$$";
  return ut.run (driver);
}
