/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TestDriver.h"
#include "libname.h"

#include <stdexcept>
#include <memory>
#include <filesystem>

#include "PersistentDataModel/Placement.h"
#include "PersistentDataModel/Token.h"

#include "StorageSvc/DbType.h"
#include "FileCatalog/URIParser.h"
#include "FileCatalog/IFileCatalog.h"

#include "PersistencySvc/ISession.h"
#include "PersistencySvc/ITransaction.h"
#include "PersistencySvc/DatabaseConnectionPolicy.h"
#include "PersistencySvc/IDatabase.h"
#include "PersistencySvc/ITechnologySpecificAttributes.h"
#include "PersistencySvc/IPersistencySvc.h"


pool::TestDriver::TestDriver( const std::string& catname ):
  m_fileCatalog( 0 ),
  m_fileName( "PersExtF.pool.root" ),
  m_eventsToCommitAndHold( 10 ),
  m_events( 100 )
{
  std::cout << "[OVAL] Creating a file catalog" << std::endl;
  m_fileCatalog = new pool::IFileCatalog;
  if ( ! m_fileCatalog ) {
    throw std::runtime_error( "Could not create a file catalog" );
  }
  std::filesystem::remove( {catname} );
  pool::URIParser p( std::string("file:") + catname );
  p.parse();
  m_fileCatalog->setWriteCatalog( p.contactstring() );
  m_fileCatalog->connect();
}

pool::TestDriver::~TestDriver()
{
  if ( m_fileCatalog ) delete m_fileCatalog;
  std::cout << "[OVAL] Number of floating tokens : " << Token::numInstances() << std::endl;
}

void
pool::TestDriver::clearCache()
{
  for( auto iToken : m_tokens ) iToken->release();
  m_tokens.clear();
}

/*
  Test writing TestClassSTLContainersExt class defined in the TestDictionary
  with multiple writes per commit
  */
void
pool::TestDriver::write(pool::DbType storageType)
{
  pool::IFileCatalog& catalog = *m_fileCatalog;
  catalog.start();

  std::cout << "Creating the persistency service" << std::endl;
  std::unique_ptr< pool::IPersistencySvc > persistencySvc( pool::IPersistencySvc::create(catalog) );

  // Set up the policy.
  pool::DatabaseConnectionPolicy policy;
  policy.setWriteModeForNonExisting( pool::DatabaseConnectionPolicy::CREATE );
  policy.setWriteModeForExisting( pool::DatabaseConnectionPolicy::OVERWRITE );
  persistencySvc->session().setDefaultConnectionPolicy( policy );

  // Start an update transaction
  if ( ! ( persistencySvc->session().transaction().start( pool::ITransaction::UPDATE ) ) ) {
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

  std::cout << "Writing " << m_events << " TestClassSTLContainersExt objects" << std::endl;
  for( int i = 0; i < m_events; ++i ) {
       TestClassSTLContainersExt* object_TestClassSTLContainersExt = new TestClassSTLContainersExt();
       v_testClassSTLContainersExt.push_back( object_TestClassSTLContainersExt );
       object_TestClassSTLContainersExt->setNonZero();
       Token* token_TestClassSTLContainersExt = persistencySvc->registerForWrite( placementHint_TestClassSTLContainersExt,
                                                                                  object_TestClassSTLContainersExt,
                                                                                  class_TestClassSTLContainersExt );
       if ( ! token_TestClassSTLContainersExt ) {
          throw std::runtime_error( "Could not write an object" );
       }
       m_tokens.push_back( token_TestClassSTLContainersExt );
       m_testClassSTLContainersExt.push_back( *object_TestClassSTLContainersExt );
    
    // Commit and hold the transaction every few rows
    if( ( i + 1 ) % m_eventsToCommitAndHold == 0 or storageType.exactMatch(pool::ROOTRNTUPLE_StorageType) ) {
      if( ! persistencySvc->session().transaction().commitAndHold() ) {
        throw std::runtime_error( "Could not commit and hold the transaction." );
      }
    }
  }
  // Finally, commit at the end
  std::cout << "Finished writing, committing the transaction." << std::endl;
  if ( ! persistencySvc->session().transaction().commit() ) {
    throw std::runtime_error( "Could not commit the transaction." );
  }
  for( auto ptr : v_testClassSTLContainersExt ) delete ptr;
  v_testClassSTLContainersExt.clear();

  // Start an update transaction
  if ( ! ( persistencySvc->session().transaction().start( pool::ITransaction::UPDATE ) ) ) {
    throw std::runtime_error( "Could not start an update transaction" );
  }
  // Committing the transaction
  std::cout << "Committing the transaction." << std::endl;
  if ( ! persistencySvc->session().transaction().commit() ) {
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
  std::unique_ptr< pool::IPersistencySvc > persistencySvc( pool::IPersistencySvc::create(catalog) );

  // Starting a read transaction
  if ( ! persistencySvc->session().transaction().start( pool::ITransaction::READ ) ) {
    throw std::runtime_error( "Could not start a read transaction." );
  }

  std::cout << "Reading back the objects." << std::endl;
  for( int i = 0; i < m_events; ++i ) {
    const Token& token = *( m_tokens.at(i) );
    Guid STLContainersExtClassID;
    STLContainersExtClassID.fromString("4E1F4DBB-1973-1974-2000-204F37331A02");
    if( token.classID() == STLContainersExtClassID ) {
       void* data_testClassSTLContainersExt = persistencySvc->readObject(token);
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
  std::cout << "Committing the transaction." << std::endl;
  if ( ! persistencySvc->session().transaction().commit() ) {
    throw std::runtime_error( "Could not commit the transaction." );
  }

  catalog.commit();
}
