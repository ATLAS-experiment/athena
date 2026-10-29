/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "PersistentDataModel/Placement.h"
#include "PersistentDataModel/Token.h"

#include "PoolSvc/FileCatalogUtils.h"
#include "../src/SessionTool.h"
#include "StorageSvc/DbType.h"

#include "Persistency_NoClassID.h"
#include "MyTestClass.h"

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <memory>


int main( )
{
   try {
      TestDriver driver( "NCI.pool.root", "NCI.catalog.xml" );
      return driver.execute();
   } catch ( std::exception& e ) {
      std::cerr << e.what() << std::endl;
      return 1;
   }
}



TestDriver::TestDriver(const std::string& filename, const std::string& catname)
  : PersTestBase( "Persistency_NoClassID", catname ),
    m_fileName( filename ),
    m_events( 100 )
{ }


int TestDriver::execute()
{
   ATH_MSG_INFO( "Starting test" );
   loadLibraries( {"test_TestDictionaryDict"} );

   ATH_MSG_INFO( "Writing objects in the database." );
   write();
   ATH_MSG_INFO( "...done" );

   ATH_MSG_INFO( "Reading the objects back from the database." );
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

  // Retrieving the class descriptions
  RootType class_MyTestClass ( "MyTestClass" );

  // Defining the placement objects
  Placement placementHint_MyTestClass;
  placementHint_MyTestClass.setFileName( m_fileName );
  placementHint_MyTestClass.setContainerName( "MyTestClass_Container" );
  placementHint_MyTestClass.setTechnology( pool::ROOTKEY_StorageType.type() );

  std::vector< MyTestClass* > v_myTestClass;
  ATH_MSG_INFO( "Writing " << m_events << " objects" );
  for ( int i = 0; i < m_events; ++i ) {
    MyTestClass* object_MyTestClass = new MyTestClass();
    v_myTestClass.push_back( object_MyTestClass );
    object_MyTestClass->data = i;
    Token* token_MyTestClass = dbsession->registerForWrite( placementHint_MyTestClass,
								       object_MyTestClass,
								       class_MyTestClass );
    if ( ! token_MyTestClass ) {
      throw std::runtime_error( "Could not write an object" );
    }
    m_tokens.push_back( token_MyTestClass );
  }

  ATH_MSG_INFO( "Committing the transaction." );
  if( !dbsession->commit() ) {
    throw std::runtime_error( "Could not commit the transaction." );
  }
  dbsession->closeSession();
  // Removing the "cache"
  for ( std::vector< MyTestClass* >::iterator iObject = v_myTestClass.begin();
	iObject != v_myTestClass.end(); ++iObject ) {
    delete *iObject;
  }

  catalog.commit();
}


void
TestDriver::read()
{
  Gaudi::IFileCatalog& catalog = *m_fileCatalog;
  catalog.init();
  auto dbsession = createSession( catalog, "ReadTestSession" );

  // Starting a read transaction
  if( !dbsession->startTransaction( Io::READ ) ) {
    throw std::runtime_error( "Could not start a read transaction." );
  }

  ATH_MSG_INFO( "Reading back the objects." );
  for ( int i = 0; i < m_events; ++i ) {
    void* data_myTestClass = dbsession->readObject( *( m_tokens[ i ] ) );
    if ( data_myTestClass == 0 ) {
      throw std::runtime_error( "Could not read the stored data" );
    }
    MyTestClass* object_myTestClass = reinterpret_cast< MyTestClass* >( data_myTestClass );
    if ( object_myTestClass->data != i ) {
      std::ostringstream error;
      error << "MyTestClassy object written is different from object read";
      throw std::runtime_error( error.str() );
    }
    delete object_myTestClass;
  }

  ATH_MSG_INFO( "Committing the transaction." );
  if( !dbsession->commit() ) {
    throw std::runtime_error( "Could not commit the transaction." );
  }
  dbsession->closeSession();
  for ( std::vector< Token* >::iterator iToken = m_tokens.begin();
	iToken != m_tokens.end(); ++iToken ) {
    (*iToken)->release();
  }
  m_tokens.clear();

  catalog.commit();
}
