/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <iostream>
#include <exception>
#include <string>
#include "TestDriver.h"
#include "TestTools/initGaudi.h"
#include "AthenaKernel/errorcheck.h"

int main( int argc, char** argv ) {
  errorcheck::ReportMessage::hideFunctionNames (true);

  // hardcode here in case we only test reading, remember to keep in sync with classes.xml
  std::string testTypeID = "A2222222-B111-C111-D111-E22134511111";

  ISvcLocator* svcloc = nullptr;
  if (!Athena_test::initGaudi (svcloc)) std::abort();
  try {
    std::cout << "[OVAL] Creating the test driver." << std::endl;
    TestDriver test_ttree("AUX.pool_test.root", pool::ROOTTREE_StorageType );
    TestDriver test_rntuple("AUX.rntuple_test.root", pool::ROOTRNTUPLE_StorageType );

    std::cout << "[OVAL] Loading the shared libraries." << std::endl;
    std::vector< std::string > libraries;
    libraries.push_back( "test_StorageSvc_AuxStoreDict" );
    TestDriver::loadLibraries( libraries );

    if( argc<2 || *argv[1] == 'w' ) {
       std::cout << "[OVAL] Testing the writing operations" << std::endl;
       std::cout << "[OVAL]   - TTree" << std::endl;
       std::string id1 = test_ttree.testWriting();
       assert(testTypeID == id1);
       std::cout << "[OVAL]   - RNTuple" << std::endl;
       std::string id2 = test_rntuple.testWriting();
       assert(testTypeID == id2);
       std::cout << "[OVAL] ...done" << std::endl;
    }
    if( argc<2 || *argv[1] == 'r' ) {
       std::cout << "[OVAL] Testing the reading operations" << std::endl;
       std::cout << "[OVAL]   - TTree" << std::endl;
       test_ttree.testReading(testTypeID);
       std::cout << "[OVAL]   - RNTuple" << std::endl;
       test_rntuple.testReading(testTypeID);
       std::cout << "[OVAL] ...done" << std::endl;
    }
  }
  catch ( std::exception& e ) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  std::cout << "[OVAL] Exiting..." << std::endl;
  return 0;
}
