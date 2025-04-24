/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <iostream>
#include <exception>
#include "RVersion.h"

#include "TestDriver.h"
#include "StorageSvc/DbType.h"
#include "PersistentDataModel/Token.h"

void testTechnology( TestDriver& driver, const pool::DbType& tech, bool commit_every_row )
{
   driver.m_storageType = tech;
   driver.m_commitEveryRow = commit_every_row;
   
   std::cout << "[OVAL] Testing StorageSvc functionality for " << tech.storageName()
             << " storage technology" << std::endl;
   std::cout << "[OVAL] Testing the writing operations" << std::endl;
   driver.testWriting();
   std::cout << "[OVAL] ...done" << std::endl;

   std::cout << "[OVAL] Testing the reading operations" << std::endl;
   driver.testReading();
   std::cout << "[OVAL] ...done" << std::endl;
}


int main( int, char** )
{
   try {
      std::cout << "[OVAL] Creating the test driver." << std::endl;
      TestDriver driver;
      std::cout << "[OVAL] Loading the shared libraries." << std::endl;
      std::vector< std::string > libraries;
      libraries.push_back( "test_TestDictionaryDict" );
      driver.loadLibraries( libraries );
      std::cout << std::endl;
      testTechnology( driver, pool::ROOTTREE_StorageType, false );
      std::cout << std::endl;
      testTechnology( driver, pool::ROOTKEY_StorageType, false );
      std::cout << std::endl;
      testTechnology( driver, pool::ROOTTREEINDEX_StorageType, false );
      std::cout << std::endl;
      testTechnology( driver, pool::ROOTRNTUPLE_StorageType, true );
   }
   catch ( std::exception& e ) {
      std::cerr << e.what() << std::endl;
      return 1;
   }

   std::cout << "[OVAL] Exiting..." << std::endl;
   return 0;
}
