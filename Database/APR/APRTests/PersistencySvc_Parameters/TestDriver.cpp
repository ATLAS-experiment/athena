/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TestDriver.h"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <memory>
#include <filesystem>

#include "PersistentDataModel/Token.h"
#include "PoolSvc/IFileCatalog.h"
#include "PoolSvc/ISession.h"

#include "StorageSvc/DbType.h"

pool::TestDriver::TestDriver():
  m_fileCatalog( 0 ),
  m_fileName( "PAR.pool.root" ),
  m_parameters()
{
  
  std::cout << "[OVAL] Creating a file catalog" << std::endl;
  m_fileCatalog = new pool::IFileCatalog;
  if ( ! m_fileCatalog ) {
    throw std::runtime_error( "Could not create a file catalog" );
  }
  const std::string catname = "PAR.catalog.xml";
  std::filesystem::remove( {catname} );
  m_fileCatalog->setWriteCatalog( catname );
}

pool::TestDriver::~TestDriver()
{
  if ( m_fileCatalog ) delete m_fileCatalog;
  std::cout << "[OVAL] Number of floating tokens : " << Token::numInstances() << std::endl;
}

void
pool::TestDriver::write()
{
  pool::IFileCatalog& catalog = *m_fileCatalog;
  catalog.start();

  std::cout << "Creating the persistency service" << std::endl;
  auto dbsession = pool::createSession(catalog);

  // Start an update transaction
  if( !dbsession->start( Io::WRITE ) ) {
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

  catalog.commit();
}


void
pool::TestDriver::read()
{
  pool::IFileCatalog& catalog = *m_fileCatalog;
  catalog.start();

  std::cout << "Creating the persistency service" << std::endl;
  auto dbsession = pool::createSession(catalog);

  // Starting a read transaction
  if( !dbsession->start( Io::READ ) ) {
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

  catalog.commit();
}
