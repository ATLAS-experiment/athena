/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/*
 */
/**
 * @file AthenaServices/MetaDataSvc_test.cxx
 * @author Shaun Roe
 * @date June 2023
 * @brief Some tests for MetaDataSvc in the Boost framework
 */

#define BOOST_TEST_MODULE TEST_METADATASVC
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <string>

#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/IClassIDSvc.h"
#include "TestTools/initGaudi.h"
#include "StoreGate/StoreGateSvc.h"
#include "src/MetaDataSvc.h"
#include "MetaDataToolStub.h"
#include "PersistencySvc/PersistencySvcException.h"


/// Test fixture (run before each test)
struct TestFixture {
  TestFixture() :
    pCLIDSvc( "ClassIDSvc", appName ),
    pStore( "StoreGateSvc", appName ),
    pMetaData( "MetaDataSvc", appName ),
    pMDWithContainer( "WithMDContainer", appName )
  {
    BOOST_TEST( pCLIDSvc.retrieve().isSuccess() );
    BOOST_TEST( pStore.retrieve().isSuccess() );
    BOOST_TEST( pMetaData.retrieve().isSuccess() );
  }

  const std::string appName = "MetaDataSvc_test";
  ServiceHandle<IClassIDSvc> pCLIDSvc;
  ServiceHandle<StoreGateSvc> pStore;
  ServiceHandle<MetaDataSvc> pMetaData;
  ServiceHandle<MetaDataSvc> pMDWithContainer;
};


// Create test suite with per-test and global fixture
BOOST_FIXTURE_TEST_SUITE(
  MetaDataSvcTest,
  TestFixture,
  * boost::unit_test::fixture<Athena_test::InitGaudi>(std::string("AthenaServices/MetaDataSvc_test.txt"))
)

BOOST_AUTO_TEST_CASE( removeStreamFromKey_method){
  std::string sampleKey{"SomeTest__STREAM[ some other text]"};
  //this method *modifies* sampleKey
  std::string result = pMetaData->removeStreamFromKey(sampleKey);
  BOOST_TEST(result == " some other text");
}

//The following are private and untested:
//addProxyToInputMetaDataStore(tokenString)
//initInputMetaDataStore
//
//The following do nothing:
//preLoadAddresses
//updateAddress
BOOST_AUTO_TEST_CASE(newMetadataSource_method){
  //Note: all checks pass if no MetaDataContainer is setup
  //setup a non-"BeginInputFile" incident...
  Incident dummyIncident("test", "dummy");
  //dummy incidents fail
  BOOST_TEST(pMetaData->newMetadataSource(dummyIncident).isFailure());
  //setup a "BeginInputFile" incident and try that...
  //note the "guid" string is interpreted by the MetaDataToolStub
  FileIncident beginInputIncident("test1", "BeginInputFile", "dummy.txt", "goodGuid");
  //only a file incident succeeds
  BOOST_TEST(pMetaData->newMetadataSource(beginInputIncident).isSuccess());
  //guid not acceptable
  FileIncident failingInputIncident("test2", "BeginInputFile", "dummy.txt", "badGuid");
  BOOST_TEST(pMetaData->newMetadataSource(failingInputIncident).isFailure());
  //The following tests use a service with  MetaDataContainer defined
  //File must exist, otherwise it throws pool::PersistencySvcException
  BOOST_CHECK_THROW(pMDWithContainer->newMetadataSource(beginInputIncident).isSuccess(), pool::PersistencySvcException);
  //
  // Tests will throw unless the file is in the FileCatalog with the right info
}
  
//analogous tests for 'retire'
//Note: There is no consistency check between 'new' and 'retire' methods
BOOST_AUTO_TEST_CASE(retireMetadataSource_method){
  Incident dummyIncident("test", "dummy");
  BOOST_TEST(pMetaData->retireMetadataSource(dummyIncident).isFailure());
  //
  FileIncident beginInputIncident("test1", "EndInputFile", "dummy.txt", "goodGuid");
  BOOST_TEST(pMetaData->retireMetadataSource(beginInputIncident).isSuccess());
  //
  FileIncident failingInputIncident("test2", "EndInputFile", "dummy.txt", "badGuid");
  BOOST_TEST(pMetaData->retireMetadataSource(failingInputIncident).isFailure());
  //No check here on whether file exists
  BOOST_TEST(pMDWithContainer->retireMetadataSource(beginInputIncident).isSuccess());
}
  
BOOST_AUTO_TEST_CASE(prepareOutput_method){
  BOOST_TEST(pMDWithContainer->prepareOutput("dummy.txt").isSuccess());
}
  
BOOST_AUTO_TEST_CASE(shmProxy_method){
  //Invalid Class ID found in IOpaqueAddress @0x45abce0. IOA will not be recorded
  BOOST_TEST(pMDWithContainer->shmProxy("dummy.txt").isFailure());
}

BOOST_AUTO_TEST_SUITE_END()
