/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "rw_test.h"

#include <string>
#include <iostream>
#include <stdexcept>


/*
  NOTE: Output level selected with CORAL_MSGLEVEL envvar
*/

int main ATLAS_NOT_THREAD_SAFE ()
{
   (void)remove("test_collection.root");
   try {
      std::cout << "Read test starting..." << std::endl;
      TestDriver driver( "PFN:test_collection.rntup", "RNTCollection", "" );
      driver.write();
      driver.read();

   }
   catch ( std::exception& e ) {
      std::cerr << "---> Exception:" << std::endl;
      std::cerr << e.what() << std::endl;
      return 1;
   }
   catch (...) {
      std::cerr << "Unhandled exception..." << std::endl;
      return 1;
   }
   
   return 0;
}

