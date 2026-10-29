/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#undef NDEBUG
#include "AuxStore.h"
#include "AuxStoreTest.h"

#include "PersistentDataModel/Guid.h"
#include "PersistentDataModel/Token.h"

#include "StorageSvc/Shape.h"
#include "StorageSvc/IStorageSvc.h"
#include "StorageSvc/DbReflex.h"
#include "StorageSvc/DbConnection.h"
#include "StorageSvc/FileDescriptor.h"
#include "StorageSvc/DbDatabase.h"
#include "StorageSvc/DbContainer.h"
#include "StorageSvc/DbType.h"
#include "StorageSvc/DbOption.h"
#include "GaudiKernel/StatusCode.h"

#include <stdexcept>
#include <iostream>
#include <memory>

#include "AthContainers/AuxTypeRegistry.h"
#include "AthenaKernel/errorcheck.h"

using namespace pool;

static const bool test_nodict = false;

static const std::string container = "DataContainer(container_Aux.)";
static const int nObjects = 10;

class TestClassNoDict {
public:
  explicit TestClassNoDict(int v=12345) { _val=v; };
  int _val;
};

using APRTest::AClassWithDict;


int main( )
{
   try {
      TestDriver driver("AuxStoreTest");
      return driver.execute();
   } 
   catch( std::exception& e ) {
      std::cerr << e.what() << std::endl;
      return 1;
   }
}



int TestDriver::execute() 
{
   errorcheck::ReportMessage::hideFunctionNames (true);

   runTestForStorageType("AUX.pool_test.root", pool::ROOTTREE_StorageType);
   ATH_MSG_INFO( std::string(100,'-') );
   runTestForStorageType("AUX.rntuple_test.root", pool::ROOTRNTUPLE_StorageType);
   
   return 0;
}

void TestDriver::runTestForStorageType(const std::string& filename, const pool::DbType& storageType)
{
   m_fileName = filename;
   m_storageType = storageType;

   // hardcode here in case we only test reading, remember to keep in sync with classes.xml
   std::string testTypeID = "A2222222-B111-C111-D111-E22134511111";

   ATH_MSG_INFO("Testing writing, using " << storageType.storageName() << " storage type.");
   std::string id = testWriting();
   assert(testTypeID == id);
   ATH_MSG_INFO(" ...done");

   ATH_MSG_INFO(" Testing the reading operations");
   testReading(testTypeID);
   ATH_MSG_INFO(" ...done");
}



inline int getVal1(int objn, int idx) { return objn*1000 + idx; }
inline int getVal2(int objn, int idx) { return objn*5000 + (idx<<1); }


std::string TestDriver::testWriting()
{
   ATH_MSG_INFO("createStorageSvc");
   IStorageSvc* storSvc = createStorageSvc("StorageSvc");
   if ( ! storSvc ) {
      throw std::runtime_error( "Could not create a StorageSvc object" );
   }
   storSvc->addRef();
   ATH_MSG_INFO("startSession");
   if( !storSvc->startSession( Io::WRITE, m_storageType.type() ).isSuccess() ) {
      throw std::runtime_error( "Could not start a session." );
   }

   ATH_MSG_INFO("Session connect");
   FileDescriptor fd( m_fileName, m_fileName );
   if ( ! ( storSvc->connect( Io::WRITE, fd ).isSuccess() ) ) {
      throw std::runtime_error( "Could not start a connection." );
   }
   DbOption opt("TREE_AUTO_FLUSH", "CollectionTree", 10);
   DbDatabase dbH( fd.dbc()->handle() );
   if( !dbH.setOption(opt).isSuccess() ) {
      throw std::runtime_error( "Could not set the TTree autoflush option." );
   }

   SG::auxid_t ityp1 = SG::AuxTypeRegistry::instance().getAuxID<int> ("anInt");
   SG::auxid_t ityp2 = SG::AuxTypeRegistry::instance().getAuxID<int> ("int2");
   SG::auxid_t dict_type = SG::AuxTypeRegistry::instance().getAuxID<AClassWithDict>("withdict");
   SG::auxid_t nodict_type = SG::AuxTypeRegistry::instance().getAuxID<TestClassNoDict>("nodict");

   ATH_MSG_INFO("Creating objects");
   std::vector<APRTest::AuxStore*>  objs;
   for( int objn=0; objn <nObjects; objn++ ) {
      APRTest::AuxStore   *object = new APRTest::AuxStore();
      object->m_data = 123456;
      const int intN = 10;
      int* i1 = reinterpret_cast<int*>( object->getData(ityp1, intN, 20) );
      int* i2 = reinterpret_cast<int*>( object->getData(ityp2, intN, 20) );
      AClassWithDict* wd_vect = reinterpret_cast<AClassWithDict*>( object->getData(dict_type, intN, 20) );
      TestClassNoDict* nodict = 0;
      if( test_nodict )
         nodict = reinterpret_cast<TestClassNoDict*>( object->getData(nodict_type, intN, 20) );
      assert (object->size() == intN);
      for( int i=0; i<intN; i++ ) {
         i1[i] = getVal1(objn, i);
         i2[i] = getVal2(objn, i);
         wd_vect[i] = AClassWithDict(objn);
         if( test_nodict )
            nodict[i] = TestClassNoDict(objn);
      } 
      objs.push_back( object );
   }
   //ATH_MSG_INFO("****  m_data offset=" << (char*)&(objs[0]->m_data) - (char*)objs[0]);

   
   // Creating the persistent shape.
   ATH_MSG_INFO("Creating Shape");
   // Retrieve the dictionary
   RootType testtype ( "APRTest::AuxStore" );
   if ( ! testtype ) {
      throw std::runtime_error( "Could not retrieve the dictionary for class APRTest::AuxStore" );
   }
   Guid guid = DbReflex::guid(testtype);
   const Shape* shape = storSvc->createShape(guid);
   if( !shape ) {
      throw std::runtime_error( "Could not create a persistent shape." );
   }
   std::string testTypeID = shape->shapeID().toString();

   // Writing the objects.
   ATH_MSG_INFO("Writing objects");
   for( int objn=0; objn <nObjects; objn++ ) {
      Token* token;
      if( ! ( storSvc->allocate( fd, container, m_storageType.type(),
                                 objs[objn], shape, token ).isSuccess() ) ) {
         throw std::runtime_error( "Could not write an object" );
      }
      ATH_MSG_INFO("Committing transaction");
      if( ! ( storSvc->endTransaction( fd, Transaction::TRANSACT_COMMIT ).isSuccess() ) ) {
         throw std::runtime_error( "Commit ERROR" );
      }
      delete token;
   }

   // Closing the transaction.
   ATH_MSG_INFO("Committing transaction");
   if ( ! ( storSvc->endTransaction( fd, Transaction::TRANSACT_COMMIT ).isSuccess() ) ) {
      throw std::runtime_error( "Could not end a transaction." );
   }

// Clearing the cache
   ATH_MSG_INFO("Deleting written data objects");
   for( int objn=0; objn <nObjects; objn++ ) 
      delete objs[objn];
   
   ATH_MSG_INFO("Disconnecting StorageSvc");
   if ( ! ( storSvc->disconnect( fd ).isSuccess() ) ) {
      throw std::runtime_error( "Could not disconnect." );
   }
   ATH_MSG_INFO("Ending Session");
   if ( ! ( storSvc->endSession().isSuccess() ) ) {
      throw std::runtime_error( "Could not end correctly the session." );
   }
   ATH_MSG_INFO("Releasing StorageSvc");
   storSvc->release();

   return testTypeID;
}


void
TestDriver::testReading(const std::string& testTypeID)
{
  IStorageSvc* storSvc = createStorageSvc("StorageSvc");
  if ( ! storSvc ) {
    throw std::runtime_error( "Could not create a StorageSvc object" );
  }
  storSvc->addRef();

  SG::auxid_t ityp1 = SG::AuxTypeRegistry::instance().getAuxID<int> ("anInt");
  SG::auxid_t ityp2 = SG::AuxTypeRegistry::instance().getAuxID<int> ("int2");
  SG::auxid_t dict_type = SG::AuxTypeRegistry::instance().getAuxID<AClassWithDict>("withdict");
  SG::auxid_t nodict_type = SG::AuxTypeRegistry::instance().getAuxID<TestClassNoDict>("nodict");

  if( !storSvc->startSession( Io::READ, pool::ROOT_StorageType.type() ).isSuccess() ) {
    throw std::runtime_error( "Could not start a session." );
  }

  FileDescriptor* fd = new FileDescriptor( m_fileName, m_fileName );
  if( !storSvc->connect( Io::READ, *fd ).isSuccess() ) {
    throw std::runtime_error( "Could not start a connection." );
  }
  DbDatabase dbH( fd->dbc()->handle() );
  if ( !dbH.isValid() )  {
    throw std::runtime_error( "Database is not valid" );
  }
  // Fetch the containers
  std::vector<const Token*> containerTokens;
  if( !dbH.containers(containerTokens, false ).isSuccess() or containerTokens.size() != 1 ) {
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
    const Guid& guid = objectToken->classID();
    while ( cntH.next(linkH).isSuccess() ) {
      objectToken->oid() = linkH;
      const Shape* shape = 0;
      if( !storSvc->getShape( *fd, guid, shape ).isSuccess() ) {
	      throw std::runtime_error( "Could not fetch the persistent shape" );
      }
      RootType classType = DbReflex::forGuid(guid);
      if(!classType){
	      throw std::runtime_error( "Could not resolve the class by guid" );
      }
      void* ptr = 0;
      if ( ! ( storSvc->read( *fd, *objectToken, shape, &ptr ) ).isSuccess() ) {
	      throw std::runtime_error( "failed to read an object back from the persistency" );
      }

      if ( shape->shapeID().toString() != testTypeID ) {
	      throw std::runtime_error( std::string("read wrong class type: ") + shape->shapeID().toString()
                                  + "  while expecting: " + testTypeID );
      }

      APRTest::AuxStore   * object = reinterpret_cast< APRTest::AuxStore* >( ptr );
      if( object->m_data != 123456 ) {
         throw std::runtime_error( "Object m_data read different from object written" );
      }

      const int intN = 10;
      const int* i1 = reinterpret_cast<const int*>( object->getData(ityp1) );
      const int* i2 = reinterpret_cast<const int*>( object->getData(ityp2) );
      TestClassNoDict const* nodict = 0;
      if( test_nodict )
         nodict = reinterpret_cast<TestClassNoDict const*>( object->getData(nodict_type) );
      auto wd_vect = reinterpret_cast<AClassWithDict const*>( object->getData(dict_type) );
      assert (object->size() == intN);
      for( int i=0; i<intN; i++ ) {
         ATH_MSG_INFO("" << i  << ": " << i1[i] << "   : " << i2[i]
              << "  : withdict[" <<i<< "]:  val=" << wd_vect[i]._val);
         if( test_nodict )
            ATH_MSG_INFO("  : nodict[]:  val=" << nodict[i]._val );
         ATH_MSG_INFO(" ");
         if( i1[i] != getVal1(iObject, i) || i2[i] != getVal2(iObject,i) )
            throw std::runtime_error( "Objects AUX data read different from objects written" );
      }
      
      ++iObject;
      delete object; 
    }
    objectToken->release();
  }
  ATH_MSG_INFO("Objects read: " << iObject);
  if ( iObject != nObjects ) {
    throw std::runtime_error( "Objects read different from objects written" );
  }


  if ( ! ( storSvc->disconnect( *fd ).isSuccess() ) ) {
    throw std::runtime_error( "Could not disconnect." );
  }
  delete fd;

  ATH_MSG_INFO("Closing the session");
  if ( ! ( storSvc->endSession().isSuccess() ) ) {
    throw std::runtime_error( "Could not end correctly the session." );
  }
  storSvc->release();
}
