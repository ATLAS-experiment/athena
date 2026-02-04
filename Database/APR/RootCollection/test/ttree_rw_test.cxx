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
   try {
      std::cout << "Read test starting..." << std::endl;
      TestDriver driver1( "Writer", pool::ROOTTREEINDEX_StorageType.type(), "test_collection.ttree.root" );
      driver1.write();
      TestDriver driver2( "Input", pool::ROOT_StorageType.type(), "test_collection.ttree.root" );
      driver2.read();

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

