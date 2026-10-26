/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "PersistentDataModel/Placement.h"
#include "PersistentDataModel/Token.h"

#include "PoolSvc/IContainer.h"
#include "PoolSvc/ISessionTool.h"
#include "PoolSvc/ITokenIterator.h"
#include "StorageSvc/DbType.h"

#include "Persistency_Functionality.h"

#include <stdexcept>
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



TestDriver::TestDriver( const std::string& catname ):
  PersTestBase( "Persistency_Functionality",catname ),
  m_fileName1( "PersF.pool1.root" ),
  m_fileName2( "PersF.pool2.root" ),
  m_events( 100 ),
  m_eventsToCommitAndHold( 10 )
{ }


int TestDriver::execute()
{
   ATH_MSG_INFO("Starting test");

   loadLibraries( {"test_TestDictionaryDict"} );
   runTestForStorageType(pool::ROOTKEY_StorageType);
   ATH_MSG_INFO("--------------------------------");
   runTestForStorageType(pool::ROOTTREE_StorageType);
   ATH_MSG_INFO("--------------------------------");
   runTestForStorageType(pool::ROOTTREEINDEX_StorageType);
   ATH_MSG_INFO("--------------------------------");
   runTestForStorageType(pool::ROOTRNTUPLE_StorageType);

   ATH_MSG_INFO("Exiting...");
   return  0;
}

void TestDriver::runTestForStorageType(const pool::DbType& storageType)
{
    ATH_MSG_INFO("Writing objects in the database using " << storageType.storageName() << " storage type.");
    write(storageType);
    ATH_MSG_INFO( "...done" );

    ATH_MSG_INFO("Reading the objects back from the database.");
    read();
    ATH_MSG_INFO( "...done" );

    ATH_MSG_INFO("Clearing the tokens.");
    clearCache();
    ATH_MSG_INFO( "...done" );

    ATH_MSG_INFO("Reading the objects back from the database as implicit collections.");
    readCollections();
    ATH_MSG_INFO( "...done" );
}



void
TestDriver::clearCache()
{
  for( auto iToken : m_tokens ) iToken->release();
  m_tokens.clear();
}

/*
  Test writing of several classes defined in the TestDictionary
  */
void
TestDriver::write(pool::DbType storageType)
{
  Gaudi::IFileCatalog& catalog = *m_fileCatalog;
  catalog.init();
  auto dbsession = createSession( catalog, "TestSession" );

  // Start an update transaction
  if( !dbsession->start( Io::WRITE ) ) {
    throw std::runtime_error( "Could not start an update transaction" );
  }

  // Retrieving the class descriptions
  RootType class_SimpleTestClass ( "SimpleTestClass" );
  RootType class_TestClassPrimitives ( "TestClassPrimitives" );
  RootType class_TestClassSTLContainers  ( "TestClassSTLContainers" );
  RootType class_TestClassVectors ( "TestClassVectors" );

  // Defining the placement objects
  Placement placementHint_SimpleTestClass;
  placementHint_SimpleTestClass.setFileName( m_fileName1 );
  placementHint_SimpleTestClass.setContainerName( "SimpleTestClass_Container" );
  placementHint_SimpleTestClass.setTechnology( storageType.type() );

  Placement placementHint_TestClassPrimitives;
  placementHint_TestClassPrimitives.setFileName( m_fileName1 );
  placementHint_TestClassPrimitives.setContainerName( "TestClassPrimitives_Container" );
  placementHint_TestClassPrimitives.setTechnology( storageType.type() );

  Placement placementHint_TestClassSTLContainers;
  placementHint_TestClassSTLContainers.setFileName( m_fileName2 );
  placementHint_TestClassSTLContainers.setContainerName( "TestSTLContainers_Container" );
  placementHint_TestClassSTLContainers.setTechnology( storageType.type() );

  Placement placementHint_TestClassVectors;
  placementHint_TestClassVectors.setFileName( m_fileName2 );
  placementHint_TestClassVectors.setContainerName( "TestClassVectors_Container" );
  placementHint_TestClassVectors.setTechnology( storageType.type() );

  std::vector< SimpleTestClass* > v_simpleTestClass;
  std::vector< TestClassPrimitives* > v_testClassPrimitives;
  std::vector< TestClassSTLContainers* > v_testClassSTLContainers;
  std::vector< TestClassVectors* > v_testClassVectors;

  ATH_MSG_INFO("Writing " << m_events << " objects of each type");
  for ( int i = 0; i < m_events; ++i ) {
    ///////////////////////////////////////////////////////////
    SimpleTestClass* object_SimpleTestClass = new SimpleTestClass();
    v_simpleTestClass.push_back( object_SimpleTestClass );
    object_SimpleTestClass->setNonZero();
    Token* token_SimpleTestClass = dbsession->registerForWrite( placementHint_SimpleTestClass,
                                                                           object_SimpleTestClass,
                                                                           class_SimpleTestClass );
    if ( ! token_SimpleTestClass ) {
      throw std::runtime_error( "Could not write an object" );
    }
    m_tokens.push_back( token_SimpleTestClass );
    m_simpleTestClass.push_back( *object_SimpleTestClass );

    ///////////////////////////////////////////////////////////
    TestClassPrimitives* object_TestClassPrimitives = new TestClassPrimitives();
    v_testClassPrimitives.push_back( object_TestClassPrimitives );
    object_TestClassPrimitives->setNonZero();
    Token* token_TestClassPrimitives = dbsession->registerForWrite( placementHint_TestClassPrimitives,
                                                                               object_TestClassPrimitives,
                                                                               class_TestClassPrimitives );
    if ( ! token_TestClassPrimitives ) {
      throw std::runtime_error( "Could not write an object" );
    }
    m_tokens.push_back( token_TestClassPrimitives );
    m_testClassPrimitives.push_back( *object_TestClassPrimitives );

    ///////////////////////////////////////////////////////////
    TestClassSTLContainers* object_TestClassSTLContainers = new TestClassSTLContainers();
    v_testClassSTLContainers.push_back( object_TestClassSTLContainers );
    object_TestClassSTLContainers->setNonZero();
    Token* token_TestClassSTLContainers = dbsession->registerForWrite( placementHint_TestClassSTLContainers,
                                                                            object_TestClassSTLContainers,
                                                                            class_TestClassSTLContainers );
    if ( ! token_TestClassSTLContainers ) {
       throw std::runtime_error( "Could not write an object" );
    }
    m_tokens.push_back( token_TestClassSTLContainers );
    m_testClassSTLContainers.push_back( *object_TestClassSTLContainers );

    ///////////////////////////////////////////////////////////
    TestClassVectors* object_TestClassVectors = new TestClassVectors();
    v_testClassVectors.push_back( object_TestClassVectors );
    object_TestClassVectors->setNonZero();
    Token* token_TestClassVectors = dbsession->registerForWrite( placementHint_TestClassVectors,
                                                                            object_TestClassVectors,
                                                                            class_TestClassVectors );
    if ( ! token_TestClassVectors ) {
      throw std::runtime_error( "Could not write an object" );
    }
    m_tokens.push_back( token_TestClassVectors );
    m_testClassVectors.push_back( *object_TestClassVectors );

    // Commit and hold the transaction every few rows
    if( (i+1) % m_eventsToCommitAndHold == 0 ) {
      if( !dbsession->commitAndHold() ) {
        throw std::runtime_error( "Could not commit and hold the transaction." );
      }
    }
  }
  if( !dbsession->commit() ) {
    throw std::runtime_error( "Could not commit the transaction." );
  }

  // Removing the "cache"
  for( auto ptr : v_simpleTestClass ) delete ptr;
  v_simpleTestClass.clear();
  for( auto ptr : v_testClassPrimitives ) delete ptr;
  v_testClassPrimitives.clear();
  for( auto ptr : v_testClassSTLContainers ) delete ptr;
  v_testClassSTLContainers.clear();
  for( auto ptr : v_testClassVectors ) delete ptr;
  v_testClassVectors.clear();

  // Start an update transaction
  if( !dbsession->start( Io::WRITE ) ) {
    throw std::runtime_error( "Could not start an update transaction" );
  }

  // Committing the transaction
  ATH_MSG_INFO("Committing the transaction.");
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
  if( !dbsession->start( Io::READ ) ) {
    throw std::runtime_error( "Could not start a read transaction." );
  }

  ATH_MSG_INFO("Reading back the objects.");
  const unsigned int numberOfTypes = m_tokens.size() / m_events;
  for ( int i = 0; i < m_events; ++i ) {
    int j = 0;

    ///////////////////////////////////////////////////////////
    void* data_simpleTestClass = dbsession->readObject( *( m_tokens[numberOfTypes*i + j ] ) );
    if ( data_simpleTestClass == 0 ) {
      throw std::runtime_error( "Could not read the stored data" );
    }
    std::unique_ptr<SimpleTestClass> object_simpleTestClass( static_cast< SimpleTestClass* >( data_simpleTestClass ) );
    if ( *object_simpleTestClass != m_simpleTestClass[i] ) {
      std::ostringstream error;
      error << "SimpleTestClass object written is different from object read:" << std::endl << "Original : ";
      m_simpleTestClass[i].streamOut( error );
      error << std::endl << "Read from persistency : ";
      object_simpleTestClass->streamOut( error );
      throw std::runtime_error( error.str() );
    }
    ++j;

    ///////////////////////////////////////////////////////////
    void* data_testClassPrimitives = dbsession->readObject( *( m_tokens[numberOfTypes*i + j] ) );
    if ( data_testClassPrimitives == 0 ) {
      throw std::runtime_error( "Could not read the stored data" );
    }
    std::unique_ptr<TestClassPrimitives> object_testClassPrimitives( static_cast< TestClassPrimitives* >( data_testClassPrimitives ) );
    if ( *object_testClassPrimitives != m_testClassPrimitives[i] ) {
      std::ostringstream error;
      error << "TestClassPrimitives object written is different from object read:" << std::endl << "Original : ";
      m_testClassPrimitives[i].streamOut( error );
      error << std::endl << "Read from persistency : ";
      object_testClassPrimitives->streamOut( error );
      throw std::runtime_error( error.str() );
    }
    ++j;

    //////////////////////////////////////////////////////////
    const Token& token = *( m_tokens.at(numberOfTypes*i + j) );
    Guid STLContainersClassID;
    STLContainersClassID.fromString("4E1F4DBB-1973-1974-1999-204F37331A02");
    if( token.classID() == STLContainersClassID ) {
       void* data_testClassSTLContainers = dbsession->readObject(token);
       if ( data_testClassSTLContainers == 0 ) {
          throw std::runtime_error( "Could not read the stored data" );
       }
       std::unique_ptr<TestClassSTLContainers> 
          object_testClassSTLContainers(static_cast<TestClassSTLContainers*>(data_testClassSTLContainers));
       if ( *object_testClassSTLContainers != m_testClassSTLContainers[i] ) {
          std::ostringstream error;
          error << "TestClassSTLContainers object written is different from object read:" << std::endl << "Original : ";
          m_testClassSTLContainers[i].streamOut( error );
          error << std::endl << "Read from persistency : ";
          object_testClassSTLContainers->streamOut( error );
          throw std::runtime_error( error.str() );
       }
       ++j;
    }

    ///////////////////////////////////////////////////////////
    void* data_testClassVectors = dbsession->readObject( *( m_tokens[numberOfTypes*i + j] ) );
    if ( data_testClassVectors == 0 ) {
      throw std::runtime_error( "Could not read the stored data" );
    }
    std::unique_ptr<TestClassVectors> object_testClassVectors( static_cast< TestClassVectors* >( data_testClassVectors ) );
    if ( *object_testClassVectors != m_testClassVectors[i] ) {
      std::ostringstream error;
      error << "TestClassVectors object written is different from object read:" << std::endl << "Original : ";
      m_testClassVectors[i].streamOut( error );
      error << std::endl << "Read from persistency : ";
      object_testClassVectors->streamOut( error );
      throw std::runtime_error( error.str() );
    }
  }

  // Committing the transaction
  ATH_MSG_INFO( "Committing the transaction." );
  if( !dbsession->commit() ) {
    throw std::runtime_error( "Could not commit the transaction." );
  }
  dbsession->closeSession();
  catalog.commit();
}


void
TestDriver::readCollections()
{
  Gaudi::IFileCatalog& catalog = *m_fileCatalog;
  catalog.init();
  auto dbsession = createSession( catalog, "TestSession" );

  // Starting a read transaction
  if( !dbsession->start( Io::READ ) ) {
    throw std::runtime_error( "Could not start a read transaction." );
  }

  // Opening again a database
  auto db = dbsession->databaseHandle( m_fileName1, pool::DatabaseSpecification::PFN );
  if ( ! db ) {
    throw std::runtime_error( "Could not retrieve a database handle" );
  }

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

    if ( numberOfObjects != m_events ) {
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


void
TestDriver::readFileSizes()
{
  Gaudi::IFileCatalog& catalog = *m_fileCatalog;
  catalog.init();
  auto dbsession = createSession( catalog, "TestSession" );

  // Starting a read transaction
  if( !dbsession->start( Io::READ ) ) {
    throw std::runtime_error( "Could not start a read transaction." );
  }

  // Committing
  ATH_MSG_INFO( "Committing the transaction." );
  if( !dbsession->commit() ) {
    throw std::runtime_error( "Could not commit the transaction." );
  }
  dbsession->closeSession();
  catalog.commit();
}
