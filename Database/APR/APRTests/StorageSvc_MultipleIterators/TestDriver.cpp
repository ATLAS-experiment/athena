/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TestDriver.h"
#include "SimpleTestClass.h"

#include "libname.h"

#include "PersistentDataModel/Guid.h"
#include "PersistentDataModel/Token.h"
#include "GaudiKernel/StatusCode.h"

#include "StorageSvc/Shape.h"
#include "StorageSvc/IStorageSvc.h"
#include "StorageSvc/DbReflex.h"
#include "StorageSvc/DatabaseConnection.h"
#include "StorageSvc/FileDescriptor.h"
#include "StorageSvc/DbDatabase.h"
#include "StorageSvc/DbContainer.h"
#include "StorageSvc/DbType.h"
#include "StorageSvc/pool.h"

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <memory>

using namespace pool;

static const std::string file = "MI.test.pool.root";
static const std::string container = "container";
static const int nObjects = 100;


TestDriver::TestDriver()
{
}

TestDriver::~TestDriver()
{
   std::cout << "[OVAL] Number of floating tokens : " << Token::numInstances() << std::endl;
}


void
TestDriver::testWriting()
{
  pool::IStorageSvc* storSvc = pool::createStorageSvc("StorageSvc");
  if ( ! storSvc ) {
    throw std::runtime_error( "Could not create a StorageSvc object" );
  }
  storSvc->addRef();
  pool::Session* sessionHandle = 0;
  if ( ! ( storSvc->startSession( pool::RECREATE, pool::ROOT_StorageType.type(), sessionHandle ).isSuccess() ) ) {
    throw std::runtime_error( "Could not start a session." );
  }

  pool::FileDescriptor fd( file, file );
  if ( ! ( storSvc->connect( sessionHandle, pool::RECREATE, fd ).isSuccess() ) ) {
    throw std::runtime_error( "Could not start a connection." );
  }
  pool::DatabaseConnection* connection = fd.dbc();

  // Retrieve the dictionary
  const RootType class_SimpleTestClass  ( "SimpleTestClass" );
  if ( ! class_SimpleTestClass ) {
    throw std::runtime_error( "Could not retrieve the dictionary for class SimpleTestClass" );
  }

  std::vector< SimpleTestClass* > myObjects_SimpleTestClass;
  std::vector< SimpleTestClass* > myObjects_SimpleTestClass2;
  for ( int i = 0; i < nObjects; ++i ) {
    myObjects_SimpleTestClass.push_back( new SimpleTestClass() );
    SimpleTestClass* myObject_SimpleTestClass = myObjects_SimpleTestClass.back();
    myObject_SimpleTestClass->data = i;

    // Creating the persistent shape.
    Guid guid = pool::DbReflex::guid(class_SimpleTestClass);
    const pool::Shape* shape_SimpleTestClass = storSvc->createShape(guid);
    if( !shape_SimpleTestClass ) {
        throw std::runtime_error( "Could not create a persistent shape." );
    }
  
    // Writing the object.
    Token* token_SimpleTestClass;
    if ( ! ( storSvc->allocate( fd,
				container, pool::ROOTKEY_StorageType.type(),
				myObject_SimpleTestClass, shape_SimpleTestClass, token_SimpleTestClass ).isSuccess() ) ) {
      throw std::runtime_error( "Could not write an object" );
    }
    //token_SimpleTestClass->setClassID( guid_SimpleTestClass );
    delete token_SimpleTestClass;

    // The second class
    myObjects_SimpleTestClass2.push_back( new SimpleTestClass() );
    SimpleTestClass* myObject_SimpleTestClass2 = myObjects_SimpleTestClass2.back();
    myObject_SimpleTestClass2->setNonZero();

    // Creating the persistent shape.
    const pool::Shape* shape_SimpleTestClass2 = storSvc->createShape(guid);
    if( !shape_SimpleTestClass2 ) {
      throw std::runtime_error( "Could not create a persistent shape." );
    }
  
    // Writing the object.
    Token* token_SimpleTestClass2;
    if ( ! ( storSvc->allocate( fd,
				container, pool::ROOTKEY_StorageType.type(),
				myObject_SimpleTestClass2, shape_SimpleTestClass2, token_SimpleTestClass2 ).isSuccess() ) ) {
      throw std::runtime_error( "Could not write an object" );
    }
    //token_SimpleTestClass2->setClassID( guid_SimpleTestClass2 );
    delete token_SimpleTestClass2;
  }

  // Closing the transaction.
  if ( ! ( storSvc->endTransaction( connection, pool::Transaction::TRANSACT_COMMIT ).isSuccess() ) ) {
    throw std::runtime_error( "Could not end a transaction." );
  }

  // Clearing the cache
  for ( std::vector< SimpleTestClass* >::iterator iObject_SimpleTestClass = myObjects_SimpleTestClass.begin();
	iObject_SimpleTestClass != myObjects_SimpleTestClass.end(); ++iObject_SimpleTestClass ) delete *iObject_SimpleTestClass;
  myObjects_SimpleTestClass.clear();

  for ( std::vector< SimpleTestClass* >::iterator iObject_SimpleTestClass2 = myObjects_SimpleTestClass2.begin();
	iObject_SimpleTestClass2 != myObjects_SimpleTestClass2.end(); ++iObject_SimpleTestClass2 ) delete *iObject_SimpleTestClass2;
  myObjects_SimpleTestClass2.clear();

  // Disconnecting
  if ( ! ( storSvc->disconnect( fd ).isSuccess() ) ) {
    throw std::runtime_error( "Could not disconnect." );
  }
  if ( ! ( storSvc->endSession( sessionHandle ).isSuccess() ) ) {
    throw std::runtime_error( "Could not end correctly the session." );
  }
  storSvc->release();
}


void
TestDriver::testReadingParallelSameContainer()
{
  pool::IStorageSvc* storSvc = pool::createStorageSvc("StorageSvc");
  if ( ! storSvc ) {
    throw std::runtime_error( "Could not create a StorageSvc object" );
  }
  storSvc->addRef();

  pool::Session* sessionHandle = 0;
  if ( ! storSvc->startSession( pool::READ, pool::ROOT_StorageType.type(), sessionHandle ).isSuccess() ) {
    throw std::runtime_error( "Could not start a session." );
  }

  pool::FileDescriptor* fd = new pool::FileDescriptor( file, file );
  if ( !storSvc->connect( sessionHandle, pool::READ, *fd ).isSuccess() ) {
    throw std::runtime_error( "Could not start a connection." );
  }

  pool::DatabaseConnection* connection = fd->dbc();
  DbDatabase dbH(static_cast<DbDatabaseObj*>(connection->handle()));
  if ( !dbH.isValid() )  {
    throw std::runtime_error( "Database is not valid" );
  }
  // Fetch the containers
  std::vector<const Token*> containerTokens;
  if ( !dbH.containers( containerTokens, false).isSuccess() || containerTokens.size() != 1 ) {
    throw std::runtime_error( "Unexpected number of containers" );
  }
  const Token* containerToken = containerTokens.front();
  const std::string containerName = containerToken->contID();
  if ( containerName != container ) {
    throw std::runtime_error( "Container name read is different from the container name written" );
  }

  // Fetch the objects in the container (Initialize the iterators)
  DbContainer cnt1H(containerToken->technology());
  Token::OID_t link1H(containerToken->oid());
  if ( ! cnt1H.open(dbH, containerToken->contID(), 0, containerToken->technology(), pool::READ).isSuccess() && cnt1H.isValid() ) {
    throw std::runtime_error( "Could not start an implicit collection iteration" );
  }
  DbContainer cnt2H(containerToken->technology());
  Token::OID_t link2H(containerToken->oid());
  if ( ! cnt2H.open(dbH, containerToken->contID(), 0, containerToken->technology(), pool::READ).isSuccess() && cnt2H.isValid() ) {
    throw std::runtime_error( "Could not start an implicit collection iteration" );
  }

  // Start the parallel iteration
  int iObject = 0;
  while ( iObject < nObjects ) {

    // First iterator
    Token* objectToken_SimpleTestClass1 = new Token(cnt1H.token());
    if ( ! ( cnt1H.next( link1H ).isSuccess() ) ) {
      throw std::runtime_error( "Could not retrieve the object token" );
    }
    objectToken_SimpleTestClass1->oid() = link1H;


    const pool::Shape* shape_SimpleTestClass1 = 0;
    if ( !storSvc->getShape( *fd, objectToken_SimpleTestClass1->classID(), shape_SimpleTestClass1 ).isSuccess() ) {
      throw std::runtime_error( "Could not fetch the persistent shape for SimpleTestClass1" );
    }
    void* ptr_SimpleTestClass1 = 0;
    if ( !storSvc->read( *fd, *objectToken_SimpleTestClass1, shape_SimpleTestClass1, &ptr_SimpleTestClass1 ).isSuccess() ) {
      throw std::runtime_error( "failed to read a SimpleTestClass1 object back from the persistency" );
    }
    
    if ( shape_SimpleTestClass1->shapeID().toString() != "4E1F4DBB-1973-1974-1999-204F37331A01" ) {
      throw std::runtime_error( std::string("read wrong class type: ") + shape_SimpleTestClass1->shapeID().toString());
    }

    SimpleTestClass* object_SimpleTestClass1 = reinterpret_cast< SimpleTestClass* >(ptr_SimpleTestClass1);
    if ( object_SimpleTestClass1->data != iObject ) {
      throw std::runtime_error( "Object read different from object written" );
    }
    delete object_SimpleTestClass1;

    // Skip the next object for this iterator
    if ( ! ( cnt1H.next( link1H ).isSuccess() ) ) {
      throw std::runtime_error( "Could not retrieve the object token" );
    }
    objectToken_SimpleTestClass1->release();


    // Second iterator
    // Skip the next object for this iterator
    Token* objectToken_SimpleTestClass2 = new Token(cnt2H.token());
    if ( ! ( cnt2H.next( link2H ).isSuccess() ) ) {
      throw std::runtime_error( "Could not retrieve the object token" );
    }
    objectToken_SimpleTestClass2->oid() = link2H;

    // Read the next object for this iterator
    if ( ! ( cnt2H.next( link2H ).isSuccess() ) ) {
      throw std::runtime_error( "Could not retrieve the object token" );
    }
    objectToken_SimpleTestClass2->oid() = link2H;

    const pool::Shape* shape_SimpleTestClass2 = 0;
    if(! storSvc->getShape( *fd, objectToken_SimpleTestClass2->classID(), shape_SimpleTestClass2 ).isSuccess() ) {
      throw std::runtime_error( "Could not fetch the persistent shape for SimpleTestClass2" );
    }
    void* ptr_SimpleTestClass2 = 0;
    if ( ! ( storSvc->read( *fd, *objectToken_SimpleTestClass2, shape_SimpleTestClass2, &ptr_SimpleTestClass2 ) ).isSuccess() ) {
      throw std::runtime_error( "failed to read a SimpleTestClass2 object back from the persistency" );
    }
    
    if ( shape_SimpleTestClass2->shapeID().toString() != "4E1F4DBB-1973-1974-1999-204F37331A01" ) {
      throw std::runtime_error( std::string("read wrong class type: ") + shape_SimpleTestClass2->shapeID().toString());
    }

    SimpleTestClass referenceObject;
    referenceObject.setNonZero();

    SimpleTestClass* object_SimpleTestClass2 = reinterpret_cast< SimpleTestClass* >(ptr_SimpleTestClass2);
    if ( *object_SimpleTestClass2 != referenceObject ) {
      throw std::runtime_error( "Object read different from object written" );
    }
    delete object_SimpleTestClass2;
    objectToken_SimpleTestClass2->release();

    // Continue the iteration
    ++iObject;
  }


  if ( iObject != nObjects ) {
    throw std::runtime_error( "Objects read different from objects written" );
  }

  // Closing the transaction.
  if ( ! ( storSvc->endTransaction( connection, pool::Transaction::TRANSACT_COMMIT ).isSuccess() ) ) {
    throw std::runtime_error( "Could not end a transaction." );
  }

  if ( ! ( storSvc->disconnect( *fd ).isSuccess() ) ) {
    throw std::runtime_error( "Could not disconnect." );
  }
  delete fd;

  std::cout << "Closing the session" << std::endl;
  if ( ! ( storSvc->endSession( sessionHandle ).isSuccess() ) ) {
    throw std::runtime_error( "Could not end correctly the session." );
  }
  storSvc->release();
}
