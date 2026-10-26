/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <stdexcept>
#include <iostream>
#include <filesystem>
#include <sstream>
#include <memory>

#include "PersistentDataModel/Placement.h"
#include "PersistentDataModel/Token.h"

#include "PoolSvc/IContainer.h"
#include "PoolSvc/FileCatalogUtils.h"
#include "PoolSvc/ISessionTool.h"
#include "PoolSvc/ITokenIterator.h"

#include "StorageSvc/DbType.h"

#include "FileOpenWithoutCatalog.h"


int main( )
{
   try {
      TestDriver driver("DB.FOWC.pool.root", "FOWC.catalog.xml");
      return driver.execute();
   } catch ( std::exception& e ) {
      std::cerr << e.what() << std::endl;
      return 1;
   }
}


TestDriver::TestDriver(const std::string& filename, const std::string& catname)
  : PersTestBase( "FileOpenWithoutCatalog", catname ),
    m_fileName( filename )
{ }


int TestDriver::execute()
{
  ATH_MSG_INFO( "Starting test" );
  loadLibraries( {"test_TestDictionaryDict"} );

  ATH_MSG_INFO( "Writing objects in the database." );
  write();
  ATH_MSG_INFO( "...done" );

  ATH_MSG_INFO( "Reading the objects back from the database." );
  read( m_fileName );
  ATH_MSG_INFO( "...done" );

  ATH_MSG_INFO( "Trying to read the same file using a different path - should generate a warning." );
  read( std::string("./") + m_fileName );
  ATH_MSG_INFO( "...done" );

  ATH_MSG_INFO( "Exiting..." );
  return 0;
}


void TestDriver::write()
{
  Gaudi::IFileCatalog& catalog = *m_fileCatalog;
  catalog.init();
  std::string fid {"E9143E5C-FDDA-8646-9204-2E4BAE14DC00"};
  FileCatalogUtils::registerPFN(catalog, m_fileName, pool::ROOT_StorageType.storageName(), fid);

  auto dbsession = createSession( catalog, "TestSession" );
  // Start an update transaction
  if( !dbsession->start( Io::WRITE ) ) {
    throw std::runtime_error( "Could not start an update transaction" );
  }

  // Retrieving the class descriptions
  RootType class_SimpleTestClass( "SimpleTestClass" );

  // Defining the placement objects
  Placement placementHint_SimpleTestClass;
  placementHint_SimpleTestClass.setFileName( m_fileName );
  placementHint_SimpleTestClass.setContainerName( "SimpleTestClass_Container" );
  placementHint_SimpleTestClass.setTechnology( pool::ROOTKEY_StorageType.type() );

  std::vector< SimpleTestClass* > v_simpleTestClass;

  ATH_MSG_INFO( "Writing 100 objects." );
  for ( int i = 0; i < 100; ++i ) {
    SimpleTestClass* object_SimpleTestClass = new SimpleTestClass();
    v_simpleTestClass.push_back( object_SimpleTestClass );
    object_SimpleTestClass->setNonZero();
    Token* token_SimpleTestClass = dbsession->registerForWrite( placementHint_SimpleTestClass,
									   object_SimpleTestClass,
									   class_SimpleTestClass );
    if ( ! token_SimpleTestClass ) {
      throw std::runtime_error( "Could not write an object" );
    }
    token_SimpleTestClass->release();
    m_simpleTestClass.push_back( *object_SimpleTestClass );
  }


  // Committing the transaction
  ATH_MSG_INFO( "Committing the transaction." );
  if( !dbsession->commit() ) {
    throw std::runtime_error( "Could not commit the transaction." );
  }
  dbsession->closeSession();

  // Removing the "cache"
  for ( std::vector< SimpleTestClass* >::iterator iObject = v_simpleTestClass.begin();
	iObject != v_simpleTestClass.end(); ++iObject ) {
    delete *iObject;
  }
  // Need the catalog entry when testing double file opening
  catalog.commit();
}


void TestDriver::read(const std::string& fileName, pool::DatabaseSpecification::NameType nameType)
{
  Gaudi::IFileCatalog& catalog = *m_fileCatalog;
  catalog.init();
  auto dbsession = createSession( catalog, "TestSession" );
  // Starting a read transaction
  if( !dbsession->start( Io::READ ) ) {
    throw std::runtime_error( "Could not start a read transaction." );
  }

  // Opening a database
  const auto & fname = (fileName.empty()? m_fileName : fileName);
  auto db = dbsession->databaseHandle(fname, nameType);
  if( ! db ) {
    throw std::runtime_error( "Could not retrieve a database handle" );
  }
  db->setTechnology( pool::ROOT_StorageType.type() );
  db->connectForRead();

  std::vector< std::string > containers = db->containers();
  for ( std::vector< std::string >::const_iterator iContainer = containers.begin();
	iContainer != containers.end(); ++iContainer ) {
    pool::IContainer* container = db->containerHandle( *iContainer );
    if ( ! container ) {
      throw std::runtime_error( "Could not retrieve the container" );
    }
    ATH_MSG_INFO( "Container : " << container->name() );
    pool::ITokenIterator* tokenIterator = container->tokens();
    if ( ! tokenIterator ) {
      throw std::runtime_error( "Could not obtain a token iterator" );
    }

    int numberOfObjects = 0;
    Token* token = tokenIterator->next();
    while ( token ) {
      ++numberOfObjects;
      token->release();
      token = tokenIterator->next();
    }

    if ( numberOfObjects != 100 ) {
      throw std::runtime_error( "Unexpected number of objects in that container." );
    }
    else {
      ATH_MSG_INFO( numberOfObjects << " objects found in the container." );
    }

    delete tokenIterator;
    delete container;
  }

  ATH_MSG_INFO( "Committing the transaction." );
  if( !dbsession->commit() ) {
    throw std::runtime_error( "Could not commit the transaction." );
  }
  dbsession->closeSession();
  catalog.commit();
}
