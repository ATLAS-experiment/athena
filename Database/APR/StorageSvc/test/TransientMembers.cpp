/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TransientMembers.h"
#include "TestClassWithTransients.h"

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


static const std::string file = "TM.test.pool.root";
static const std::string container = "container";
static const int nObjects = 100;


class TestClassTransient {
public:
  TestClassTransient() : data(0) {}
  virtual ~TestClassTransient() {};
  int data;
};


int main( )
{
   try {
      TestDriver driver("TransientMembersTest");
      return driver.execute();
   } catch ( std::exception& e ) {
      std::cerr << e.what() << std::endl;
      return 1;
   }
}


int TestDriver::execute()
{
  ATH_MSG_INFO("Testing the writing operations");
  testWriting();
  ATH_MSG_INFO(" ...done");

  ATH_MSG_INFO(" Testing the reading operations");
  testReading();
  ATH_MSG_INFO(" ...done");
  
  ATH_MSG_INFO(" Exiting...");
  return 0;
}


void
TestDriver::testWriting()
{
  auto storageTool = createStorageTool();
  startSession(storageTool, Io::WRITE, pool::ROOT_StorageType);
  FileDescriptor fd( file, file );
  if ( ! ( storageTool->connect( Io::WRITE, fd ).isSuccess() ) ) {
    throw std::runtime_error( "Could not start a connection." );
  }
  // Retrieve the dictionary
  RootType class_TestClassWithTransients  ( "TestClassWithTransients" );
  if ( ! class_TestClassWithTransients ) {
    throw std::runtime_error( "Could not retrieve the dictionary for class TestClassWithTransients" );
  }

  std::vector< TestClassWithTransients* > myObjects;
  std::vector< TestClassTransient* > myTransientObjects;

  for ( int i = 0; i < nObjects; ++i ) {
    myObjects.push_back( new TestClassWithTransients() );
    TestClassWithTransients* myObject = myObjects.back();
    myObject->data1 = i;
    for ( int j = 0; j < 4; ++j ) {
      myObject->transientData[j].data1 = 4*i + j;
      myObject->transientData[j].data2 = 4*i + j + 0.5;
      myObject->transientData[j].data3 = (4*i + j)%99;
    }
    myTransientObjects.push_back( new TestClassTransient() );
    myObject->transientPointer = myTransientObjects.back();
    myObject->transientPointer->data = 3*i;
    myObject->transientObject.data1 = 1;
    myObject->transientObject.data2 = 2;
    myObject->data2 = 2*i + 0.3;

    // Writing the object.
    Token* token;
    if( !storageTool->write( fd, container, pool::ROOTTREE_StorageType.type(),
                         myObject, class_TestClassWithTransients, token ).isSuccess() ) {
      throw std::runtime_error( "Could not write an object" );
    }
    delete token;
  }

  // Closing the transaction.
  if ( ! ( storageTool->endTransaction( fd, Transaction::TRANSACT_COMMIT ).isSuccess() ) ) {
    throw std::runtime_error( "Could not end a transaction." );
  }

  // Clearing the cache
  for ( std::vector< TestClassWithTransients* >::iterator iObject = myObjects.begin();
	iObject != myObjects.end(); ++iObject ) delete *iObject;
  myObjects.clear();

  for ( std::vector< TestClassTransient* >::iterator iObject = myTransientObjects.begin();
	iObject != myTransientObjects.end(); ++iObject ) delete *iObject;
  myTransientObjects.clear();

  endSession(storageTool, fd);
}


void
TestDriver::testReading()
{
  auto storageTool = createStorageTool();
  startSession(storageTool, Io::READ, pool::ROOT_StorageType);

  FileDescriptor fd( file, file );
  if ( !storageTool->connect( Io::READ, fd ).isSuccess() ) {
    throw std::runtime_error( "Could not start a connection." );
  }

  // Fetch the containers
  std::vector<const Token*> containerTokens;
  DbDatabase dbH( fd.dbc()->handle() );
  if( !dbH.containers(containerTokens, false).isSuccess() or containerTokens.size() != 1 ) {
    throw std::runtime_error( "Could not fetch the containers" );
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

      TestClassWithTransients* object = reinterpret_cast< TestClassWithTransients* >(ptr);
      int expectedData1 = iObject;
      if ( object->data1 != expectedData1 ) {
	      std::ostringstream error;
	      error << "data1 expected: " << expectedData1 << " and got " << object->data1 << std::endl;
	      throw std::runtime_error( error.str() );
      }

      for ( int j = 0; j < 4; ++j ) {
	if ( object->transientData[j].data1 != 0 ||
	     object->transientData[j].data2 != 0 ||
	     object->transientData[j].data3 != 0 ) {
	  throw std::runtime_error( "Object (transientData) read different from object written" );
	}
      }

      if ( object->transientPointer != 0 ) {
	      throw std::runtime_error( "Object (transient pointer) read different from object written" );
      }

      if ( object->transientObject.data1 != 0 ||
	   object->transientObject.data2 != 0 ) {
	      throw std::runtime_error( "Object (transient object) read different from object written" );
      }

      double expectedData2 = 2*iObject + 0.3;
      if ( object->data2 != expectedData2 ) {
	      std::ostringstream error;
	      error << "data2 expected: " << expectedData2 << " and got " << object->data2 << std::endl;
	      throw std::runtime_error( error.str() );
      }

      delete object;
      ++iObject;
    }
    objectToken->release();
  }

  if ( iObject != nObjects ) {
    throw std::runtime_error( "Objects read different from objects written" );
  }
  endSession(storageTool, fd);
}
