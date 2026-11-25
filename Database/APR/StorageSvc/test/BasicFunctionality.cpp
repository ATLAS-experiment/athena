/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "BasicFunctionality.h"
#include "SimpleTestClass.h"

#include "PersistentDataModel/Token.h"

#include "StorageSvc/IStorageTool.h"
#include "StorageSvc/DbConnection.h"
#include "StorageSvc/FileDescriptor.h"
#include "StorageSvc/DbDatabase.h"
#include "StorageSvc/DbContainer.h"
#include "StorageSvc/DbType.h"
#include "StorageSvc/DbString.h"
#include "GaudiKernel/StatusCode.h"

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <memory>


int main( )
{
   try {
      TestDriver driver("BasicFunctionalityTest");
      return driver.execute();
   } catch ( std::exception& e ) {
      std::cerr << e.what() << std::endl;
      return 1;
   }
}


void TestDriver::testTechnology( const DbType& tech, bool commit_every_row )
{
   m_storageType = tech;
   m_commitEveryRow = commit_every_row;

   ATH_MSG_INFO(" Testing StorageTool functionality for " << tech.storageName()
             << " storage technology");
   ATH_MSG_INFO(" Testing the writing operations");
   testWriting();
   ATH_MSG_INFO(" ...done");

   ATH_MSG_INFO(" Testing the reading operations");
   testReading();
   ATH_MSG_INFO(" ...done");
}


int TestDriver::execute()
{ 
   testTechnology( pool::ROOTTREE_StorageType, false );
   ATH_MSG_INFO( std::string(100,'-') );
   testTechnology( pool::ROOTKEY_StorageType, false );
   ATH_MSG_INFO( std::string(100,'-') );
   testTechnology( pool::ROOTTREEINDEX_StorageType, false );
   ATH_MSG_INFO( std::string(100,'-') );
   testTechnology( pool::ROOTRNTUPLE_StorageType, true );

   ATH_MSG_INFO(" Exiting...");
   return 0;
}



void
TestDriver::testWriting()
{
   auto storageTool = createStorageTool();
   startSession(storageTool, Io::WRITE, m_storageType);

   ATH_MSG_INFO("Session connect");
   FileDescriptor fd( m_filename, m_filename );
   if( !storageTool->connect( Io::WRITE, fd ).isSuccess() ) {
      throw std::runtime_error( "Could not start a connection." );
   }
  // Retrieve the dictionary
  RootType class_SimpleTestClass ( "SimpleTestClass" );
  if ( ! class_SimpleTestClass ) {
    throw std::runtime_error( "Could not retrieve the dictionary for class SimpleTestClass" );
  }
  // object cache
  std::vector< SimpleTestClass* > myObjects;
  myObjects.reserve(m_nObjects);

  RootType class_DbString ( "DbString" );
  if( ! class_DbString ) {
    throw std::runtime_error( "Could not retrieve the dictionary for class DbString" );
  }
  std::vector< DbString > myStrings;
  myStrings.reserve(m_nObjects);
  
  if( !class_SimpleTestClass.Properties().HasProperty( "ClassID" ) ) {
     std::ostringstream error;
     error << "There is no ClassID property for class \"SimpleTestClass\"" << std::ends;
     throw std::runtime_error( error.str() );
  }
  for ( int i = 0; i < m_nObjects; ++i ) {
    myObjects.push_back( new SimpleTestClass() );
    SimpleTestClass* myObject = myObjects.back();
    myObject->data = i;

    // Writing the object.
    Token* token = nullptr;
    if( !storageTool->write( fd, m_objContainerName, m_storageType.type(),
			myObject, class_SimpleTestClass, token ).isSuccess() ) {
      throw std::runtime_error( "Could not write a SimpleTestClass object" );
    }
    delete token; token = nullptr;

    myStrings.push_back( "This is my test DbString #" + std::to_string(i) );
    const DbString* sp = &myStrings.back();
    if( !storageTool->write( fd, m_strContainerName, m_storageType.type(),
                            sp, class_DbString, token ).isSuccess() ) {
       throw std::runtime_error( "Could not write a dbString" );
    }
    delete token; token = nullptr;
    
    if( m_commitEveryRow ) {
       if( !storageTool->endTransaction( fd, Transaction::TRANSACT_COMMIT ).isSuccess() ) {
          throw std::runtime_error( "Could not end a transaction." );
       }
    }
  }
  if( !m_commitEveryRow ) {
     // Closing the transaction.
     if( !storageTool->endTransaction( fd, Transaction::TRANSACT_COMMIT ).isSuccess() ) {
        throw std::runtime_error( "Could not end a transaction." );
     }
  }
  // Clearing the cache
  for( SimpleTestClass* myObject : myObjects ) delete myObject;
  myObjects.clear();

  endSession(storageTool, fd);
}


void
TestDriver::testReading()
{
  auto storageTool = createStorageTool();
  startSession(storageTool, Io::READ, m_storageType);

  FileDescriptor fd( m_filename, m_filename );
  if( !storageTool->connect(Io::READ, fd).isSuccess() ) {
    throw std::runtime_error( "Could not start a connection." );
  }

  DbDatabase dbH( fd.dbc()->handle() );
  if ( !dbH.isValid() )  {
    throw std::runtime_error( "Database is not valid" );
  }
  // Fetch the containers
  std::vector<const Token*> containerTokens;
  containerTokens.reserve(2);
  if( !dbH.containers( containerTokens, false ).isSuccess() or containerTokens.size() != 2 ) {
    throw std::runtime_error( "Could not retrieve the list of (two) containers" );
  }
  const Token* objContToken = nullptr;
  for( const Token* containerToken : containerTokens ) {
     if( containerToken->contID() == m_objContainerName ) {
        objContToken = containerToken;
        break;
     }
  }
  if( !objContToken )  throw std::runtime_error( "Could not find the main Container in the DB");

  const Token* strContToken = nullptr;
  for( const Token* containerToken : containerTokens ) {
     if( containerToken->contID() == m_strContainerName ) {
        strContToken = containerToken;
        break;
     }
  }
  if( !strContToken )  throw std::runtime_error( "Could not find the String container in the DB");

  // Fetch the objects in the container.
  DbContainer objCntH(objContToken->technology());
  Token::OID_t objLinkH(objContToken->oid());
  StatusCode sc = objCntH.open(dbH, m_objContainerName, 0, objContToken->technology(), Io::READ);
  int iObject = 0;
  if ( sc.isSuccess() && objCntH.isValid() ) {
    Token* objectToken = new Token(objCntH.token());
    while ( objCntH.next(objLinkH).isSuccess() ) {
      objectToken->oid() = objLinkH;
      void* ptr = 0;
      if( !storageTool->read( fd, *objectToken, &ptr ).isSuccess() ) {
	      throw std::runtime_error( "failed to read an object back from the persistency" );
      }

      SimpleTestClass* object = reinterpret_cast< SimpleTestClass* >( ptr );
      if ( object->data != iObject ) {
         throw std::runtime_error( "Object read different from object written!   Obj data=" + std::to_string(object->data) );
      }
      delete object;
      ++iObject;
    }
    objectToken->release();
  }
  if ( iObject != m_nObjects ) {
     throw std::runtime_error( std::string("Read ") + std::to_string(iObject) + " instead of " + std::to_string(m_nObjects));
  }

  // Read the Strings
  DbContainer strCntH(strContToken->technology());
  Token::OID_t strLinkH(strContToken->oid());
  sc = strCntH.open(dbH, m_strContainerName, 0, strContToken->technology(), Io::READ);
  if ( sc.isSuccess() && strCntH.isValid() ) {
     Token* stringToken = new Token(strCntH.token());
     while ( strCntH.next(strLinkH).isSuccess() ) {
        stringToken->oid() = strLinkH;
        DbString str;
        void *ptr = &str;
        void *originalPtr = ptr; // this is to check if the pointer is changed (RootKeyContainer modifies it!)
        if( !storageTool->read( fd, *stringToken, &ptr ).isSuccess() ) {
           throw std::runtime_error( "failed to read an object back from the persistency" );
        }

        if (ptr == originalPtr) {
           ATH_MSG_INFO("Read back string: " << str);
        } else {
           ATH_MSG_INFO("Read back string: " << *reinterpret_cast<std::string*>(ptr));
        }

     }
     stringToken->release();
  }

  endSession(storageTool, fd);
}
