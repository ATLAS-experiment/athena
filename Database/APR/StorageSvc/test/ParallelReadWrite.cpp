/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "ParallelReadWrite.h"
#include "SimpleTestClass.h"

#include "PersistentDataModel/Token.h"
#include "GaudiKernel/StatusCode.h"

#include "StorageSvc/IStorageTool.h"
#include "StorageSvc/DbConnection.h"
#include "StorageSvc/FileDescriptor.h"
#include "StorageSvc/DbDatabase.h"
#include "StorageSvc/DbContainer.h"
#include "StorageSvc/DbType.h"

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <memory>


static const std::string file1 = "PARR.test1.pool.root";
static const std::string file2 = "PARR.test2.pool.root";
static const std::string container = "container";
static const int nObjects = 100;


int main( )
{
   try {
      TestDriver driver("ParallelReadWriteTest");
      return driver.execute();
   } catch ( std::exception& e ) {
      std::cerr << e.what() << std::endl;
      return 1;
   }
}


int TestDriver::execute() 
{
   ATH_MSG_INFO("] Testing the writing operations");
   testWriting();
   ATH_MSG_INFO("] ...done");

   ATH_MSG_INFO("] Testing the parrallel reading and writing operations");
   testParallelReadWrite();
   ATH_MSG_INFO("] ...done");

   ATH_MSG_INFO("] Testing the reading operations");
   testReading();

   return 0;
}

void
TestDriver::testWriting()
{
  auto storageTool = createStorageTool();
  startSession(storageTool, Io::WRITE, pool::ROOT_StorageType);

  FileDescriptor fd( file1, file1 );
  if( !storageTool->connect( Io::WRITE, fd ).isSuccess() ) {
    throw std::runtime_error( "Could not start a connection." );
  }
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

    // Writing the object.
    Token* token;
    if( !storageTool->write( fd,	container, pool::ROOTTREE_StorageType.type(),
                         myObject, class_SimpleTestClass, token ).isSuccess() ) {
      throw std::runtime_error( "Could not write an object" );
    }
    delete token;
  }

  // Closing the transaction.
  if ( ! ( storageTool->endTransaction( fd, Transaction::TRANSACT_COMMIT ).isSuccess() ) ) {
    throw std::runtime_error( "Could not end a transaction." );
  }

  // Clearing the cache
  for ( std::vector< SimpleTestClass* >::iterator iObject = myObjects.begin();
	iObject != myObjects.end(); ++iObject ) delete *iObject;
  myObjects.clear();

  endSession(storageTool, fd);
}


void
TestDriver::testParallelReadWrite()
{
  auto storageTool = createStorageTool();
  startSession(storageTool, Io::WRITE, pool::ROOT_StorageType);

  // Open the file to read
  FileDescriptor fd1( file1, file1 );
  if( !storageTool->connect( Io::READ, fd1 ).isSuccess() ) {
    throw std::runtime_error( "Could not start a connection." );
  }

  // Open the file to write
  FileDescriptor fd2( file2, file2 );
  if ( ! ( storageTool->connect( Io::WRITE, fd2 ).isSuccess() ) ) {
    throw std::runtime_error( "Could not start a connection." );
  }

  DbDatabase dbH( fd1.dbc()->handle() );
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
  StatusCode sc = cntH.open(dbH, containerToken->contID(), 0, containerToken->technology(), Io::READ);
  int iObject = 0;
  if ( sc.isSuccess() && cntH.isValid() ) {
    Token* objectToken = new Token(cntH.token());
    while ( cntH.next(linkH).isSuccess() ) {
      objectToken->oid() = linkH;
      // Read the object from one file
      void* ptr = 0;
      if( !storageTool->read( fd1, *objectToken, &ptr ).isSuccess() ) {
	      throw std::runtime_error( "failed to read an object back from the persistency" );
      }
      SimpleTestClass* object = reinterpret_cast< SimpleTestClass* >(ptr);
      if ( object->data != iObject ) {
	      throw std::runtime_error( "Object read different from object written" );
      }

      // Write a new object into the other file.
      myObjects.push_back( new SimpleTestClass() );
      SimpleTestClass* myObject = myObjects.back();
      myObject->data = object->data;

      // Writing the object.
      Token* tokenw;
      if( !storageTool->write( fd2, container, pool::ROOTTREE_StorageType.type(),
                           myObject, class_SimpleTestClass, tokenw ).isSuccess() ) {
	      throw std::runtime_error( "Could not write an object" );
      }
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
  if ( ! ( storageTool->endTransaction( fd1, Transaction::TRANSACT_COMMIT ).isSuccess() ) ) {
    throw std::runtime_error( "Could not end a transaction." );
  }
  if ( ! ( storageTool->endTransaction( fd2, Transaction::TRANSACT_COMMIT ).isSuccess() ) ) {
    throw std::runtime_error( "Could not end a transaction." );
  }

  // Clearing the cache
  for( SimpleTestClass* myObject : myObjects ) delete myObject;
  myObjects.clear();

  if( !storageTool->disconnect( fd1 ).isSuccess() ) {
    throw std::runtime_error( "Could not disconnect FD1." );
  }
  if( !storageTool->disconnect( fd2 ).isSuccess() ) {
    throw std::runtime_error( "Could not disconnect FD2." );
  }
  endSession(storageTool);
}



void
TestDriver::testReading()
{
  auto storageTool = createStorageTool();
  startSession(storageTool, Io::READ, pool::ROOT_StorageType);
  FileDescriptor fd( file2, file2 );
  if( !storageTool->connect( Io::READ, fd ).isSuccess() ) {
    throw std::runtime_error( "Could not start a connection." );
  }

  DbDatabase dbH( fd.dbc()->handle() );
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
  StatusCode sc = cntH.open(dbH, containerToken->contID(), 0, containerToken->technology(), Io::READ);
  int iObject = 0;
  if ( sc.isSuccess() && cntH.isValid() ) {
    Token* objectToken = new Token(cntH.token());
    while ( cntH.next(linkH).isSuccess() ) {
      objectToken->oid() = linkH;
      void* ptr = 0;
      if( !storageTool->read( fd, *objectToken, &ptr ).isSuccess() ) {
	      throw std::runtime_error( "failed to read an object back from the persistency" );
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

  if ( ! ( storageTool->endTransaction( fd, Transaction::TRANSACT_COMMIT ).isSuccess() ) ) {
    throw std::runtime_error( "Could not end a transaction." );
  }
  endSession(storageTool, fd);
}
