/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "PersistentDataModel/Token.h"
#include "PoolSvc/FileCatalogUtils.h"
#include "PoolSvc/ISessionTool.h"
#include "StorageSvc/DbType.h"

#include "Persistency_Parameters.h"

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <memory>



int main( )
{
   try {
      TestDriver driver( "PAR.pool.root", "" );
      return driver.execute();
   } catch ( std::exception& e ) {
      std::cerr << e.what() << std::endl;
      return 1;
   }
}

TestDriver::TestDriver(const std::string& filename, const std::string& catname)
  : PersTestBase( "Persistency_Parameters", catname ),
    m_fileName( filename ),
    m_parameters()
{ }


int TestDriver::execute()
{
   ATH_MSG_INFO( "Starting test" );
   loadLibraries( {"test_TestDictionaryDict"} );

   ATH_MSG_INFO( "Writing parameters in the database." );
   write();
   ATH_MSG_INFO( "...done" );

   ATH_MSG_INFO( "Reading the parameters back from the database." );
   read();
   ATH_MSG_INFO( "...done" );

   ATH_MSG_INFO( "Exiting..." );
   return 0;
}


void
TestDriver::write()
{
  Gaudi::IFileCatalog& catalog = *m_fileCatalog;
  catalog.init();
  auto dbsession = createSession( catalog, "TestSession" );

  // Start an update transaction
  if( !dbsession->startTransaction( Io::WRITE ) ) {
    throw std::runtime_error( "Could not start an update transaction" );
  }

  // Opening again a database
  auto db = dbsession->databaseHandle( m_fileName, pool::DatabaseSpecification::PFN );
  if ( ! db ) {
    throw std::runtime_error( "Could not retrieve a database handle" );
  }

  db->setTechnology( pool::ROOT_StorageType.type() );
  db->connectForWrite();

  // Committing the transaction
  std::cout << "Committing the transaction." << std::endl;
  if( !dbsession->commit() ) {
    throw std::runtime_error( "Could not commit the transaction." );
  }
  dbsession->closeSession();
  catalog.commit();
}


void
TestDriver::read()
{
  Gaudi::IFileCatalog& catalog = *m_fileCatalog;
  catalog.init();
  auto dbsession = createSession( catalog, "TestSession" );

  // Starting a read transaction
  if( !dbsession->startTransaction( Io::READ ) ) {
    throw std::runtime_error( "Could not start a read transaction." );
  }

  // Opening the database for reading
  auto db = dbsession->databaseHandle( m_fileName, pool::DatabaseSpecification::PFN );
  if( ! db ) {
    throw std::runtime_error( "Could not retrieve a database handle" );
  }

  db->connectForRead();

  // Committing the transaction
  std::cout << "Committing the transaction." << std::endl;
  if( !dbsession->commit() ) {
    throw std::runtime_error( "Could not commit the transaction." );
  }
  dbsession->closeSession();
  catalog.commit();
}
