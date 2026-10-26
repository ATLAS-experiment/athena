/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "PersistentDataModel/Placement.h"
#include "PersistentDataModel/Token.h"

#include "PoolSvc/IContainer.h"
#include "PoolSvc/FileCatalogUtils.h"
#include "PoolSvc/ISessionTool.h"
#include "PoolSvc/ITokenIterator.h"
#include "StorageSvc/DbType.h"

#include "FileCatalogOperations.h"

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <memory>


int main( )
{
   try {
      TestDriver driver;
      return driver.execute();
   } catch ( std::exception& e ) {
      std::cerr << e.what() << std::endl;
      return 1;
   }
}


TestDriver::TestDriver( const std::string& catname )
  : PersTestBase( "FileCatalogOperations", catname ),
    m_fileName1( "FCO.pool1.root" ),
    m_lfn1( "lfn1" ),
    m_fileName2( "FCO.pool2.root" ),
    m_lfn2( "lfn2" ),
    m_fileName3( "FCO.pool3.root" ),
    m_lfn3( "lfn3" ),
    m_events( 100 )
{ }


int TestDriver::execute()
{
  ATH_MSG_INFO("Starting test");
  loadLibraries( {"test_TestDictionaryDict"} );

  ATH_MSG_INFO("Writing objects in the database.");
  write();
  ATH_MSG_INFO("...done");

  ATH_MSG_INFO("Adding LFNs in the file catalog.");
  addlfns();
  ATH_MSG_INFO("...done");

  ATH_MSG_INFO("Reading the objects back from the database.");
  read();
  ATH_MSG_INFO("...done");

  ATH_MSG_INFO("Exiting...");
  return 0;
}

void TestDriver::write()
{
  Gaudi::IFileCatalog& catalog = *m_fileCatalog;
  catalog.init();
  auto dbsession = createSession( catalog, "TestSession" );

  // Start an update transaction
  if( !dbsession->start( Io::WRITE ) ) {
    throw std::runtime_error( "Could not start an update transaction" );
  }

  // Retrieving the class descriptions
  RootType class_SimpleTestClass( "SimpleTestClass" );

  // Defining the placement objects
  Placement placementHint_SimpleTestClass;
  placementHint_SimpleTestClass.setFileName( m_fileName1 );
  placementHint_SimpleTestClass.setContainerName( "SimpleTestClass_Container" );
  placementHint_SimpleTestClass.setTechnology( pool::ROOTKEY_StorageType.type() );

  std::vector< SimpleTestClass* > v_simpleTestClass;
  ATH_MSG_INFO( "Writing " << m_events << " objects" );
  for ( int i = 0; i < m_events; ++i ) {

    ///////////////////////////////////////////////////////////
    SimpleTestClass* object_SimpleTestClass = new SimpleTestClass();
    v_simpleTestClass.push_back( object_SimpleTestClass );
    object_SimpleTestClass->data = i;
    Token* token_SimpleTestClass = dbsession->registerForWrite( placementHint_SimpleTestClass,
									   object_SimpleTestClass,
									   class_SimpleTestClass );
    if ( ! token_SimpleTestClass ) {
      throw std::runtime_error( "Could not write an object" );
    }
    token_SimpleTestClass->release();
  }

  ATH_MSG_INFO( "Committing the transaction." );
  if( !dbsession->commit() ) {
    throw std::runtime_error( "Could not commit the transaction." );
  }
  dbsession->closeSession();
  catalog.commit();

  // Removing the "cache"
  for ( std::vector< SimpleTestClass* >::iterator iObject = v_simpleTestClass.begin();
	iObject != v_simpleTestClass.end(); ++iObject ) {
    delete *iObject;
  }
  v_simpleTestClass.clear();
}


void TestDriver::addlfns()
{
  m_fileCatalog->init(); 
  m_fileCatalog->registerLFN( m_fileCatalog->lookupPFN(m_fileName1), m_lfn1 );
  m_fileCatalog->commit();
}


void TestDriver::read()
{
  Gaudi::IFileCatalog& catalog = *m_fileCatalog;
  catalog.init();
  auto dbsession = createSession( catalog, "TestSession" );

  // Starting a read transaction
  if( !dbsession->start( Io::READ ) ) {
    throw std::runtime_error( "Could not start a read transaction." );
  }

  auto db = dbsession->databaseHandle( m_lfn1, pool::DatabaseSpecification::LFN );
  if ( ! db ) {
    throw std::runtime_error( "Could not retrieve a database handle" );
  }

  const std::string& pfn = db->pfn();
  ATH_MSG_INFO( "PFN1 = " << pfn );
  const std::string& fid = db->fid();
  ATH_MSG_INFO( "FID1 = " << fid );

  db->connectForRead(); // will open db in read mode.

  pool::IContainer* container = db->containerHandle( "SimpleTestClass_Container" );
  if ( ! container ) {
    throw std::runtime_error( "Could not retrieve the container" );
  }

  pool::ITokenIterator* tokenIterator = container->tokens();
  if ( ! tokenIterator ) {
    throw std::runtime_error( "Could not obtain a token iterator" );
  }

  int i = 0;

  Token* token = tokenIterator->next();
  while ( token ) {
    void* data_simpleTestClass = dbsession->readObject( *token );
    if ( data_simpleTestClass == 0 ) {
      throw std::runtime_error( "Could not read the stored data" );
    }
    SimpleTestClass* object_simpleTestClass = reinterpret_cast< SimpleTestClass* >( data_simpleTestClass );
    if ( object_simpleTestClass->data != i ) {
      throw std::runtime_error( "Unexpected data read back" );
    }
    ++i;
    delete object_simpleTestClass;
    token->release();
    token = tokenIterator->next();
  }

  delete tokenIterator;
  delete container;

  ATH_MSG_INFO( "Committing the transaction." );
  if( !dbsession->commit() ) {
    throw std::runtime_error( "Could not commit the transaction." );
  }
  dbsession->closeSession();

  catalog.commit();
}
