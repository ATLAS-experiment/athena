//  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#include <iostream>
#include <stdexcept>
#include "../PersistencySvc_FileOpenWithoutCatalog/TestDriver.h"
#include "AthenaKernel/getMessageSvc.h"

int main( int, char** ) {
  try {
    
    // Suppress Athena MessageSvc warnings about not finding Gaudi MessageSvc
    Athena::getMessageSvcQuiet = true;
    std::cout << "Creating the test driver." << std::endl;
    pool::TestDriver driver("DB.FODP.pool.root", "FODP.catalog.xml");
    driver.loadLibraries( { "test_TestDictionaryDict" } );

    std::cout << "Creating a database" << std::endl;
    driver.write();
    std::cout << "...done" << std::endl;

    std::cout << "Reading objects back from the database." << std::endl;
    driver.read( std::string("./") + driver.m_fileName );
    std::cout << "...done" << std::endl;

  }
  catch ( std::exception& e ) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  std::cout << "Exiting..." << std::endl;
  return 0;
}
