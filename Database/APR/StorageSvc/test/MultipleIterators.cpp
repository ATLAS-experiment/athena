/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MultipleIterators.h"
#include "SimpleTestClass.h"

#include "PersistentDataModel/Token.h"
#include "GaudiKernel/StatusCode.h"

#include "StorageSvc/IStorageTool.h"
#include "StorageSvc/DbConnection.h"
#include "StorageSvc/FileDescriptor.h"
#include "StorageSvc/DbDatabase.h"
#include "StorageSvc/DbContainer.h"
#include "StorageSvc/DbType.h"
#include "StorageSvc/pool.h"

#include "AthenaKernel/getMessageSvc.h"

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <memory>


static const std::string file = "MI.test.pool.root";
static const std::string container = "container";
static const int nObjects = 100;


int main( )
{
   try {
      TestDriver driver("MultipleIteratorsTest");
      return driver.execute();
   } catch ( std::exception& e ) {
      std::cerr << e.what() << std::endl;
      return 1;
   }
}



int TestDriver::execute() 
{
    ATH_MSG_INFO(" Testing the writing operations");
    testWriting();
    ATH_MSG_INFO(" ...done");

    ATH_MSG_INFO(" Testing the parallel iterators in the same container");
    testReadingParallelSameContainer();
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
  if( !storageTool->connect(Io::WRITE, fd ).isSuccess() ) {
    throw std::runtime_error( "Could not start a connection." );
  }
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

    // Writing the object.
    Token* token_SimpleTestClass;
    if( !storageTool->write( fd, container, pool::ROOTKEY_StorageType.type(),
                         myObject_SimpleTestClass, class_SimpleTestClass, token_SimpleTestClass ).isSuccess() ) {
      throw std::runtime_error( "Could not write an object" );
    }
    delete token_SimpleTestClass;

    // The second class
    myObjects_SimpleTestClass2.push_back( new SimpleTestClass() );
    SimpleTestClass* myObject_SimpleTestClass2 = myObjects_SimpleTestClass2.back();
    myObject_SimpleTestClass2->setNonZero();

    // Writing the object.
    Token* token_SimpleTestClass2;
    if( !storageTool->write( fd, container, pool::ROOTKEY_StorageType.type(),
                         myObject_SimpleTestClass2, class_SimpleTestClass, token_SimpleTestClass2 ).isSuccess() ) {
      throw std::runtime_error( "Could not write an object" );
    }
    delete token_SimpleTestClass2;
  }

  // Closing the transaction.
  if ( ! ( storageTool->endTransaction( fd, Transaction::TRANSACT_COMMIT ).isSuccess() ) ) {
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
  endSession(storageTool, fd);
}


void
TestDriver::testReadingParallelSameContainer()
{
  auto storageTool = createStorageTool();
  startSession(storageTool, Io::READ, pool::ROOT_StorageType);

  FileDescriptor fd( file, file );
  if( !storageTool->connect( Io::READ, fd ).isSuccess() ) {
    throw std::runtime_error( "Could not start a connection." );
  }

  DbDatabase dbH( fd.dbc()->handle() );
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
  if ( ! cnt1H.open(dbH, containerToken->contID(), 0, containerToken->technology(), Io::READ).isSuccess() && cnt1H.isValid() ) {
    throw std::runtime_error( "Could not start an implicit collection iteration" );
  }
  DbContainer cnt2H(containerToken->technology());
  Token::OID_t link2H(containerToken->oid());
  if ( ! cnt2H.open(dbH, containerToken->contID(), 0, containerToken->technology(), Io::READ).isSuccess() && cnt2H.isValid() ) {
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

    void* ptr_SimpleTestClass1 = 0;
    if ( !storageTool->read( fd, *objectToken_SimpleTestClass1, &ptr_SimpleTestClass1 ).isSuccess() ) {
      throw std::runtime_error( "failed to read a SimpleTestClass1 object back from the persistency" );
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

    void* ptr_SimpleTestClass2 = 0;
    if( !storageTool->read( fd, *objectToken_SimpleTestClass2, &ptr_SimpleTestClass2 ).isSuccess() ) {
      throw std::runtime_error( "failed to read a SimpleTestClass2 object back from the persistency" );
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
  if( !storageTool->endTransaction( fd, Transaction::TRANSACT_COMMIT ).isSuccess() ) {
    throw std::runtime_error( "Could not end a transaction." );
  }

  endSession(storageTool, fd);
}
