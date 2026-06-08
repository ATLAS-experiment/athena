/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack

//
// includes
//

#include <RootCoreUtils/Global.h>

#include <iostream>
#include <RootCoreUtils/Locate.h>

//
// method implementations
//

int main (int argc, char **argv)
{
  if (argc != 2)
  {
    std::cerr << "usage: " << argv[0] << " locations" << std::endl;
    return 1;
  }

  std::cout << RCU::locate (argv[1]) << std::endl;
  return 0;
}
