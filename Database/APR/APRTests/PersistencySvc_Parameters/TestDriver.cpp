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
#include "PoolSvc/FileCatalogUtils.h"
#include "PoolSvc/ISession.h"
#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"

#include "StorageSvc/DbType.h"

pool::TestDriver::TestDriver():
  m_fileCatalogMgr( Gaudi::svcLocator()->service<Gaudi::IFileCatalogMgr>( "Gaudi::MultiFileCatalog" ) ),
  m_fileCatalog( m_fileCatalogMgr ),
  m_fileName( "PAR.pool.root" ),
  m_parameters()
{
  
  std::cout << "[OVAL] Creating a file catalog" << std::endl;
  if ( ! m_fileCatalog.isValid() ) {
    throw std::runtime_error( "Could not create a file catalog" );
  }
  const std::string catname = "PAR.catalog.xml";
  std::filesystem::remove( {catname} );
  FileCatalogUtils::addCatalog( *m_fileCatalogMgr, catname, true );
}

pool::TestDriver::~TestDriver()
{
  std::cout << "[OVAL] Number of floating tokens : " << Token::numInstances() << std::endl;
}

void
pool::TestDriver::write()
{
  Gaudi::IFileCatalog& catalog = *m_fileCatalog;
  catalog.init();

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
  Gaudi::IFileCatalog& catalog = *m_fileCatalog;
  catalog.init();

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
