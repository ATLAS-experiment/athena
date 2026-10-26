/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "PersistentDataModel/Placement.h"
#include "PersistentDataModel/Token.h"

#include "PoolSvc/FileCatalogUtils.h"
#include "PoolSvc/ISessionTool.h"
#include "StorageSvc/DbType.h"

#include "Persistency_ExtraFunctionality.h"

#include <stdexcept>
#include <memory>

using namespace pool;

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
  : PersTestBase( "Persistency_ExtraFunctionality", catname ),
    m_fileName( "PersExtF.pool.root" ),
    m_eventsToCommitAndHold( 10 ),
    m_events( 100 )
{ }


/* Run Extra Fiunctionality test for TTree and Tkey based Storage Services onlu (no RNTuple) */
int TestDriver::execute()
{
   ATH_MSG_INFO( "Starting test" );

   loadLibraries( {"test_TestDictionaryDict"} );
   runTestForStorageType(pool::ROOTKEY_StorageType);
   ATH_MSG_INFO( "-------------------------------------" );
   runTestForStorageType(pool::ROOTTREE_StorageType);
   ATH_MSG_INFO( "-------------------------------------" );
   runTestForStorageType(pool::ROOTTREEINDEX_StorageType);

   ATH_MSG_INFO( "Exiting..." );
   return 0;
}


void TestDriver::runTestForStorageType(const pool::DbType& storageType)
{
   ATH_MSG_INFO( "Writing objects in the database using " << storageType.storageName() << " storage type." );
   write(storageType);
   ATH_MSG_INFO( "...done" );

   ATH_MSG_INFO( "Reading the objects back from the database." );
   read();
   ATH_MSG_INFO( "...done" );

   ATH_MSG_INFO( "Clearing the tokens." );
   clearCache();
   ATH_MSG_INFO( "...done" );
}


void TestDriver::clearCache()
{
  for( auto iToken : m_tokens ) iToken->release();
  m_tokens.clear();
}

/*
  Test writing TestClassSTLContainersExt class defined in the TestDictionary
  with multiple writes per commit
  */
void TestDriver::write(pool::DbType storageType)
{
  Gaudi::IFileCatalog& catalog = *m_fileCatalog;
  catalog.init();
  auto dbsession = createSession( catalog, "TestSession" );

  // Start an update transaction
  if( !dbsession->start( Io::WRITE ) ) {
    throw std::runtime_error( "Could not start an update transaction" );
  }

  // Retrieving the class descriptions
  RootType class_TestClassSTLContainersExt  ( "TestClassSTLContainersExt" );
  
  // Defining the placement objects
  Placement placementHint_TestClassSTLContainersExt;
  placementHint_TestClassSTLContainersExt.setFileName( m_fileName );
  placementHint_TestClassSTLContainersExt.setContainerName( "TestSTLContainersExt_Container" );
  placementHint_TestClassSTLContainersExt.setTechnology( storageType.type() );

  std::vector< TestClassSTLContainersExt* > v_testClassSTLContainersExt;

  ATH_MSG_INFO( "Writing " << m_events << " TestClassSTLContainersExt objects" );
  for( int i = 0; i < m_events; ++i ) {
       TestClassSTLContainersExt* object_TestClassSTLContainersExt = new TestClassSTLContainersExt();
       v_testClassSTLContainersExt.push_back( object_TestClassSTLContainersExt );
       object_TestClassSTLContainersExt->setNonZero();
       Token* token_TestClassSTLContainersExt = dbsession->registerForWrite( placementHint_TestClassSTLContainersExt,
                                                                                  object_TestClassSTLContainersExt,
                                                                                  class_TestClassSTLContainersExt );
       if ( ! token_TestClassSTLContainersExt ) {
          throw std::runtime_error( "Could not write an object" );
       }
       m_tokens.push_back( token_TestClassSTLContainersExt );
       m_testClassSTLContainersExt.push_back( *object_TestClassSTLContainersExt );
    
    // Commit and hold the transaction every few rows
    if( ( i + 1 ) % m_eventsToCommitAndHold == 0 or storageType.exactMatch(pool::ROOTRNTUPLE_StorageType) ) {
      if( ! dbsession->commitAndHold() ) {
        throw std::runtime_error( "Could not commit and hold the transaction." );
      }
    }
  }
  // Finally, commit at the end
  ATH_MSG_INFO( "Finished writing, committing the transaction." );
  if( !dbsession->commit() ) {
    throw std::runtime_error( "Could not commit the transaction." );
  }
  for( auto ptr : v_testClassSTLContainersExt ) delete ptr;
  v_testClassSTLContainersExt.clear();

  // Start an update transaction
  if( !dbsession->start( Io::WRITE ) ) {
    throw std::runtime_error( "Could not start an update transaction" );
  }
  // Committing the transaction
  ATH_MSG_INFO( "Committing the transaction." );
  if( !dbsession->commit() ) {
    throw std::runtime_error( "Could not commit the transaction." );
  }
  dbsession->closeSession();
  catalog.commit();
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

  ATH_MSG_INFO( "Reading back the objects." );
  for( int i = 0; i < m_events; ++i ) {
    const Token& token = *( m_tokens.at(i) );
    Guid STLContainersExtClassID;
    STLContainersExtClassID.fromString("4E1F4DBB-1973-1974-2000-204F37331A02");
    if( token.classID() == STLContainersExtClassID ) {
       void* data_testClassSTLContainersExt = dbsession->readObject(token);
       if ( data_testClassSTLContainersExt == 0 ) {
          throw std::runtime_error( "Could not read the stored data" );
       }
       std::unique_ptr<TestClassSTLContainersExt>
          object_testClassSTLContainersExt(static_cast<TestClassSTLContainersExt*>(data_testClassSTLContainersExt));
       if ( *object_testClassSTLContainersExt != m_testClassSTLContainersExt[i] ) {
          std::ostringstream error;
          error << "TestClassSTLContainersExt object written is different from object read:" << std::endl << "Original : ";
          m_testClassSTLContainersExt[i].streamOut( error );
          error << std::endl << "Read from persistency : ";
          object_testClassSTLContainersExt->streamOut( error );
          throw std::runtime_error( error.str() );
       }
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
