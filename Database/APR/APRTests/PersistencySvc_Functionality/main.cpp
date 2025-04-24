/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <iostream>
#include <stdexcept>
#include "TestDriver.h"
#include "StorageSvc/DbType.h"
#include "RVersion.h"

void runTestForStorageType(const pool::DbType& storageType, pool::TestDriver& driver)
{   
   std::cout << "[OVAL] Writing objects in the database using " << storageType.storageName() << " storage type." << std::endl;
    driver.write(storageType);
    std::cout << "[OVAL] ...done" << std::endl;

    std::cout << "[OVAL] Reading the objects back from the database." << std::endl;
    driver.read();
    std::cout << "[OVAL] ...done" << std::endl;

    std::cout << "[OVAL] Clearing the tokens." << std::endl;
    driver.clearCache();
    std::cout << "[OVAL] ...done" << std::endl;

    std::cout << "[OVAL] Reading the objects back from the database as implicit collections." << std::endl;
    driver.readCollections();
    std::cout << "[OVAL] ...done" << std::endl;
}


int main( int, char** )
{
   try {
      std::cout << "[OVAL] Creating the test driver." << std::endl;
      pool::TestDriver driver;

      std::cout << "[OVAL] Loading the shared libraries." << std::endl;
      std::vector< std::string > libraries;
      libraries.push_back( "test_TestDictionaryDict" );
      driver.loadLibraries( libraries );

      runTestForStorageType(pool::ROOTKEY_StorageType, driver);
      std::cout << std::endl;
      runTestForStorageType(pool::ROOTTREE_StorageType, driver);
      std::cout << std::endl;
      runTestForStorageType(pool::ROOTTREEINDEX_StorageType, driver);
      std::cout << std::endl;
      // Enable this test once we decide how to handle commit and hold for RNTuple
      //runTestForStorageType(pool::ROOTRNTUPLE_StorageType, driver);
   } catch ( std::exception& e ) {
      std::cerr << e.what() << std::endl;
      return 1;
   }

   std::cout << "[OVAL] Exiting..." << std::endl;
   return 0;
}
