/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//
//

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include "GoogleTestTools/InitGaudiGoogleTest.h"

#include <string>

#include "AsgTools/StandaloneToolHandle.h"
#include "AthAnalysisBaseComps/AthAnalysisHelper.h"

#include "AthAsgExUnittest/IAthAsgExUnittestTool.h"
#include "AthAsgExUnittest/AthAsgExUnittestTool.h"

#include "GaudiKernel/IAlgManager.h"
#include "Gaudi/Algorithm.h"

#include <iostream>
#include <fstream>

// Tool test suite:

namespace Athena_test {

  // Tool test suite:

  class AthAsgExUnittestToolTest : public ::testing::Test  {
  public:
    AthAsgExUnittestToolTest() : myTool( "MyTool" ) {}
    ~AthAsgExUnittestToolTest() {}
    AthAsgExUnittestTool myTool;
  };

  TEST_F( AthAsgExUnittestToolTest, initialise ) {
    EXPECT_TRUE( myTool.initialize().isSuccess() );
  }

  TEST_F( AthAsgExUnittestToolTest, property ) {
    EXPECT_TRUE( myTool.setProperty( "Property", 42.0 ).isSuccess() );
    EXPECT_TRUE( myTool.initialize().isSuccess() );
    std::string prop;
    EXPECT_TRUE( myTool.getProperty( "Property", prop ).isSuccess() );
    EXPECT_EQ( std::stod( prop ), 42.0 );
  }

  TEST_F( AthAsgExUnittestToolTest, enumProperty ) {
    EXPECT_TRUE( myTool.setProperty( "EnumProperty", IAthAsgExUnittestTool::Val2 ).isSuccess() );
    EXPECT_TRUE( myTool.initialize().isSuccess() );
    std::string prop;
    EXPECT_TRUE( myTool.getProperty( "EnumProperty", prop ).isSuccess() );
    EXPECT_EQ( std::stoi( prop ), IAthAsgExUnittestTool::Val2 );
  }

  // Algorithm test suite:

  class AthAsgExUnittestAlgTest : public InitGaudiGoogleTest {
  public:
    virtual void SetUp() override {
      // Algorithm and Tool properties via service:
      // see: Control/AthAnalysisBaseComps/AthAnalysisBaseComps/AthAnalysisHelper.h
      EXPECT_TRUE( AthAnalysisHelper::addPropertyToCatalogue( "AthAsgExUnittestAlg",
                                                              "MyProperty",
                                                              21 ).isSuccess() );
      EXPECT_TRUE( AthAnalysisHelper::addPropertyToCatalogue( "AthAsgExUnittestAlg",
                                                              "MyTool",
                                                              "AthAsgExUnittestTool/AnotherName" ).isSuccess() );
      // Set property on the tool
      EXPECT_TRUE( AthAnalysisHelper::addPropertyToCatalogue( "AthAsgExUnittestAlg.AnotherName",
                                                              "Property",
                                                              42.0 ).isSuccess() );
      // Create instance of my algorithm through Gaudi.
      IAlgManager* algMgr = svcMgr.as< IAlgManager >();
      EXPECT_TRUE( algMgr != nullptr );
      IAlgorithm* alg = nullptr;
      EXPECT_TRUE( algMgr->createAlgorithm( "AthAsgExUnittestAlg", "AthAsgExUnittestAlg",
                                            alg ).isSuccess() );
      EXPECT_TRUE( alg != nullptr );
      myAlg = dynamic_cast< Algorithm* >( alg );
      EXPECT_TRUE( myAlg != nullptr );
    }

    AthAsgExUnittestTool* getMyTool() {
      ToolHandle<IAthAsgExUnittestTool> toolHandle( "", myAlg );
      toolHandle.setTypeAndName( myAlg->getProperty( "MyTool" ).toString() );
      EXPECT_TRUE( toolHandle.retrieve().isSuccess() );
      IAthAsgExUnittestTool* impt= toolHandle.get();
      AthAsgExUnittestTool* mpt= dynamic_cast<AthAsgExUnittestTool*>( impt );
      return mpt;
    }

    Algorithm* myAlg = nullptr;

  };

  TEST_F( AthAsgExUnittestAlgTest, initialise ) {
    EXPECT_TRUE( myAlg->initialize().isSuccess() );
  }

  TEST_F( AthAsgExUnittestAlgTest, setProperty ) {
    EXPECT_TRUE( myAlg->setProperty( "MyProperty", 5 ).isSuccess() );
    EXPECT_TRUE( myAlg->initialize().isSuccess() );
    std::string prop;
    EXPECT_TRUE( myAlg->getProperty( "MyProperty", prop ).isSuccess() );
    EXPECT_EQ( prop, "5" );
  }

  TEST_F( AthAsgExUnittestAlgTest, sysInitialize ) {
    EXPECT_TRUE( myAlg->sysInitialize().isSuccess() );
    std::string prop;
    EXPECT_TRUE( myAlg->getProperty( "MyProperty", prop ).isSuccess() );
    EXPECT_EQ( std::stoi( prop ), 21 );
  }

  TEST_F( AthAsgExUnittestAlgTest, toolProperty ) {
    // sysInitialize() gets properties then call initialize()
    EXPECT_TRUE( myAlg->sysInitialize().isSuccess() );
    AthAsgExUnittestTool* mpt= getMyTool();
    std::string prop;
    EXPECT_TRUE( mpt->getProperty( "Property", prop ).isSuccess() );
    EXPECT_EQ( std::stod( prop ), 42.0 );
  }

}

int main( int argc, char **argv ) {
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}
