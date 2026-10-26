/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include <iostream>
#include <exception>
#include "TestDriver.h"
#include "AthenaKernel/getMessageSvc.h"

int main( int, char** ) {
  Athena::getMessageSvcQuiet = true;
  try {
    std::cout << "[OVAL] Creating the test driver." << std::endl;
    TestDriver driver;
    driver.loadLibraries( {"test_APRTestDictionary"} );

    std::cout << "[OVAL] Testing the writing operations" << std::endl;
    driver.testWriting();
    std::cout << "[OVAL] ...done" << std::endl;

    std::cout << "[OVAL] Testing the parallel iterators in the same container" << std::endl;
    driver.testReadingParallelSameContainer();
    std::cout << "[OVAL] ...done" << std::endl;
  }
  catch ( std::exception& e ) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  std::cout << "[OVAL] Exiting..." << std::endl;
  return 0;
}
