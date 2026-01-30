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

static const std::string file1 = "PARR.test1.pool.root";
static const std::string file2 = "PARR.test2.pool.root";
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

  pool::FileDescriptor fd( file1, file1 );
  if ( ! ( storSvc->connect( sessionHandle, pool::RECREATE, fd ).isSuccess() ) ) {
    throw std::runtime_error( "Could not start a connection." );
  }
  pool::DatabaseConnection* connection = fd.dbc();

  // Retrieve the dictionary
  RootType class_SimpleTestClass  ( "SimpleTestClass" );
  if ( ! class_SimpleTestClass ) {
    throw std::runtime_error( "Could not retrieve the dictionary for class SimpleTestClass" );
  }

  std::vector< SimpleTestClass* > myObjects;
  for ( int i = 0; i < nObjects; ++i ) {
    myObjects.push_back( new SimpleTestClass() );
    SimpleTestClass* myObject = myObjects.back();
    myObject->data = i;

    // Creating the persistent shape.
    Guid guid = pool::DbReflex::guid(class_SimpleTestClass);
    const pool::Shape* shape = storSvc->createShape(guid);
    if( !shape ) {
        throw std::runtime_error( "Could not create a persistent shape." );
    }
  
    // Writing the object.
    Token* token;
    if ( ! ( storSvc->allocate( fd,
				container, pool::ROOTTREE_StorageType.type(),
				myObject, shape, token ).isSuccess() ) ) {
      throw std::runtime_error( "Could not write an object" );
    }
    //token->setClassID( guid );
    delete token;
  }

  // Closing the transaction.
  if ( ! ( storSvc->endTransaction( connection, pool::Transaction::TRANSACT_COMMIT ).isSuccess() ) ) {
    throw std::runtime_error( "Could not end a transaction." );
  }

  // Clearing the cache
  for ( std::vector< SimpleTestClass* >::iterator iObject = myObjects.begin();
	iObject != myObjects.end(); ++iObject ) delete *iObject;
  myObjects.clear();

  if ( ! ( storSvc->disconnect( fd ).isSuccess() ) ) {
    throw std::runtime_error( "Could not disconnect." );
  }
  if ( ! ( storSvc->endSession( sessionHandle ).isSuccess() ) ) {
    throw std::runtime_error( "Could not end correctly the session." );
  }
  storSvc->release();
}


void
TestDriver::testParallelReadWrite()
{
  pool::IStorageSvc* storSvc = pool::createStorageSvc("StorageSvc");
  if ( ! storSvc ) {
    throw std::runtime_error( "Could not create a StorageSvc object" );
  }
  storSvc->addRef();
  pool::Session* sessionHandle = 0;
  if ( ! ( storSvc->startSession( pool::UPDATE, pool::ROOT_StorageType.type(), sessionHandle ).isSuccess() ) ) {
    throw std::runtime_error( "Could not start a session." );
  }


  // Open the file to read
  pool::FileDescriptor fd1( file1, file1 );
  if( !storSvc->connect( sessionHandle, pool::READ, fd1 ).isSuccess() ) {
    throw std::runtime_error( "Could not start a connection." );
  }

  // Open the file to write
  pool::FileDescriptor fd2( file2, file2 );
  if ( ! ( storSvc->connect( sessionHandle, pool::RECREATE, fd2 ).isSuccess() ) ) {
    throw std::runtime_error( "Could not start a connection." );
  }

  pool::DatabaseConnection* connection = fd1.dbc();
  DbDatabase dbH(static_cast<DbDatabaseObj*>(connection->handle()));
  if ( !dbH.isValid() )  {
    throw std::runtime_error( "Database is not valid" );
  }
  // Fetch the containers
  std::vector<const Token*> containerTokens;
  if( !dbH.containers( containerTokens, false ).isSuccess() or containerTokens.size() != 1 ) {
    throw std::runtime_error( "Unexpected number of containers" );
  }
  const Token* containerToken = containerTokens.front();
  const std::string containerName = containerToken->contID();
  if ( containerName != container ) {
    throw std::runtime_error( "Container name read is different from the container name written" );
  }


  // Retrieve the dictionary
  RootType class_SimpleTestClass  ( "SimpleTestClass" );
  if ( ! class_SimpleTestClass ) {
    throw std::runtime_error( "Could not retrieve the dictionary for class SimpleTestClass" );
  }

  std::vector< SimpleTestClass* > myObjects;
  // Fetch the objects in the container.
  DbContainer cntH(containerToken->technology());
  Token::OID_t linkH(containerToken->oid());
  StatusCode sc = cntH.open(dbH, containerToken->contID(), 0, containerToken->technology(), pool::READ);
  int iObject = 0;
  if ( sc.isSuccess() && cntH.isValid() ) {
    Token* objectToken = new Token(cntH.token());
    const Guid& guid = objectToken->classID();
    while ( cntH.next(linkH).isSuccess() ) {
      objectToken->oid() = linkH;
      // Read the object from one file
      const pool::Shape* shape = 0;
      if( !storSvc->getShape( fd1, guid, shape ).isSuccess() ) {
	      throw std::runtime_error( "Could not fetch the persistent shape" );
      }
      RootType classType = pool::DbReflex::forGuid(guid);
      if(!classType){
	      throw std::runtime_error( "Could not resolve the class by guid" );
      }
      void* ptr = 0;
      if ( ! ( storSvc->read( fd1, *objectToken, shape, &ptr ) ).isSuccess() ) {
	      throw std::runtime_error( "failed to read an object back from the persistency" );
      }

      if ( shape->shapeID().toString() != "4E1F4DBB-1973-1974-1999-204F37331A01" ) {
        throw std::runtime_error( std::string("read wrong class type: ") + shape->shapeID().toString());
      }
      SimpleTestClass* object = reinterpret_cast< SimpleTestClass* >(ptr);
      if ( object->data != iObject ) {
	      throw std::runtime_error( "Object read different from object written" );
      }

      // Write a new object into the other file.
      myObjects.push_back( new SimpleTestClass() );
      SimpleTestClass* myObject = myObjects.back();
      myObject->data = object->data;

      // Creating the persistent shape.
      Guid guidw = pool::DbReflex::guid(class_SimpleTestClass);
      const pool::Shape* shapew = storSvc->createShape(guidw);
      if( !shapew ) {
	        throw std::runtime_error( "Could not create a persistent shape." );
      }
      
      // Writing the object.
      Token* tokenw;
      if ( ! ( storSvc->allocate( fd2,
				  container, pool::ROOTTREE_StorageType.type(),
				  myObject, shapew, tokenw ).isSuccess() ) ) {
	      throw std::runtime_error( "Could not write an object" );
      }
      tokenw->setClassID( guidw );
      tokenw->release();

      delete object;
      ++iObject;
    }
    objectToken->release();
  }
  if ( iObject != nObjects ) {
    throw std::runtime_error( "Objects read different from objects written" );
  }


  // Closing the transaction.
  if ( ! ( storSvc->endTransaction( fd1.dbc(), pool::Transaction::TRANSACT_COMMIT ).isSuccess() ) ) {
    throw std::runtime_error( "Could not end a transaction." );
  }
  if ( ! ( storSvc->endTransaction( fd2.dbc(), pool::Transaction::TRANSACT_COMMIT ).isSuccess() ) ) {
    throw std::runtime_error( "Could not end a transaction." );
  }

  // Clearing the cache
  for ( std::vector< SimpleTestClass* >::iterator iObject = myObjects.begin();
	iObject != myObjects.end(); ++iObject ) delete *iObject;
  myObjects.clear();


  if ( ! ( storSvc->disconnect( fd1 ).isSuccess() ) ) {
    throw std::runtime_error( "Could not disconnect." );
  }

  if ( ! ( storSvc->disconnect( fd2 ).isSuccess() ) ) {
    throw std::runtime_error( "Could not disconnect." );
  }

  std::cout << "Closing the session" << std::endl;
  if ( ! ( storSvc->endSession( sessionHandle ).isSuccess() ) ) {
    throw std::runtime_error( "Could not end correctly the session." );
  }
  storSvc->release();
}




void
TestDriver::testReading()
{
  pool::IStorageSvc* storSvc = pool::createStorageSvc("StorageSvc");
  if ( ! storSvc ) {
    throw std::runtime_error( "Could not create a StorageSvc object" );
  }

  pool::Session* sessionHandle = 0;
  if ( ! ( storSvc->startSession( pool::READ, pool::ROOT_StorageType.type(), sessionHandle ).isSuccess() ) ) {
    throw std::runtime_error( "Could not start a session." );
  }

  pool::FileDescriptor fd( file2, file2 );
  if( !storSvc->connect( sessionHandle, pool::READ, fd ).isSuccess() ) {
    throw std::runtime_error( "Could not start a connection." );
  }

  pool::DatabaseConnection* connection = fd.dbc();
  DbDatabase dbH(static_cast<DbDatabaseObj*>(connection->handle()));
  if ( !dbH.isValid() )  {
    throw std::runtime_error( "Database is not valid" );
  }
  // Fetch the containers
  std::vector<const Token*> containerTokens;
  if( !dbH.containers( containerTokens, false ).isSuccess() or containerTokens.size() != 1 ) {
    throw std::runtime_error( "Unexpected number of containers" );
  }
  const Token* containerToken = containerTokens.front();
  const std::string containerName = containerToken->contID();
  if ( containerName != container ) {
    throw std::runtime_error( "Container name read is different from the container name written" );
  }

  // Fetch the objects in the container.
  DbContainer cntH(containerToken->technology());
  Token::OID_t linkH(containerToken->oid());
  StatusCode sc = cntH.open(dbH, containerToken->contID(), 0, containerToken->technology(), pool::READ);
  int iObject = 0;
  if ( sc.isSuccess() && cntH.isValid() ) {
    Token* objectToken = new Token(cntH.token());
    const Guid& guid = objectToken->classID();
    while ( cntH.next(linkH).isSuccess() ) {
      objectToken->oid() = linkH;
      const pool::Shape* shape = 0;
      if( !storSvc->getShape( fd, guid, shape ).isSuccess() ) {
	      throw std::runtime_error( "Could not fetch the persistent shape" );
      }
      RootType classType = pool::DbReflex::forGuid(guid);
      if(!classType){
	      throw std::runtime_error( "Could not resolve the class by guid" );
      }      
      void* ptr = 0;
      if ( ! ( storSvc->read( fd, *objectToken, shape, &ptr ) ).isSuccess() ) {
	      throw std::runtime_error( "failed to read an object back from the persistency" );
      }

      if ( shape->shapeID().toString() != "4E1F4DBB-1973-1974-1999-204F37331A01" ) {
        throw std::runtime_error( std::string("read wrong class type: ") + shape->shapeID().toString());
      }
      SimpleTestClass* object = reinterpret_cast< SimpleTestClass* >(ptr);
      if ( object->data != iObject ) {
	throw std::runtime_error( "Object read different from object written" );
      }
      delete object;
      ++iObject;
    }
    objectToken->release();
  }
  if ( iObject != nObjects ) {
    throw std::runtime_error( "Objects read different from objects written" );
  }

  if ( ! ( storSvc->endTransaction( fd.dbc(), pool::Transaction::TRANSACT_COMMIT ).isSuccess() ) ) {
    throw std::runtime_error( "Could not end a transaction." );
  }

  if ( ! ( storSvc->disconnect( fd ).isSuccess() ) ) {
    throw std::runtime_error( "Could not disconnect." );
  }

  std::cout << "Closing the session" << std::endl;
  if ( ! ( storSvc->endSession( sessionHandle ).isSuccess() ) ) {
    throw std::runtime_error( "Could not end correctly the session." );
  }
  storSvc->release();
}
