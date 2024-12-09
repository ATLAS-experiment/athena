
//
//  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
//

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include "GoogleTestTools/InitGaudiGoogleTest.h"

#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/Algorithm.h"
#include "Gaudi/Interfaces/IOptionsSvc.h"

#include "AthenaBaseComps/AthAlgorithm.h"
#include "AthenaBaseComps/AthService.h"

#include "AthAsgExUnittest/IAthAsgExUnittestTool.h"

#include <string>
#include <iostream>
#include <fstream>

namespace Athena_test {

  // Algorithm test suite:

  class AthAsgExUnittestAlgTest : public InitGaudiGoogleTest {
  public:

    AthAsgExUnittestAlgTest()
    //  : InitGaudiGoogleTest( MSG::INFO ) // get usual message blurb
      : myAlg(nullptr)
    {}

    virtual void SetUp() override {

      ServiceHandle<Gaudi::Interfaces::IOptionsSvc> joSvc( "JobOptionsSvc",
                                                           "AthAsgExUnittestAlgTest" );
      joSvc->set( "AthAsgExUnittestAlg.MyProperty", Gaudi::Utils::toString( 21 ) );
      joSvc->set( "AthAsgExUnittestAlg.MyTool", "AthAsgExUnittestTool/AnotherName" );
      joSvc->set( "AthAsgExUnittestAlg.AnotherName.Property", Gaudi::Utils::toString( 42.0 ) );
      IAlgorithm* ialg= Algorithm::Factory::create( "AthAsgExUnittestAlg",
                                                    "AthAsgExUnittestAlg",
                                                    Gaudi::svcLocator() ).release();
      myAlg= dynamic_cast<Gaudi::Algorithm*>( ialg );

    }

    IAthAsgExUnittestTool* getMyTool() {
      ToolHandle<IAthAsgExUnittestTool> toolHandle( "", myAlg );
      toolHandle.setTypeAndName( myAlg->getProperty( "MyTool" ).toString() );
      EXPECT_TRUE( toolHandle.retrieve().isSuccess() );
      IAthAsgExUnittestTool* impt= toolHandle.get();
      return impt;
    }

    int getIntProperty( const std::string & name ) {
      std::string prop;
      EXPECT_TRUE( myAlg->getProperty( name, prop ) );
      return std::stoi( prop );
    }

    Gaudi::Algorithm* myAlg;

  };

  TEST_F( AthAsgExUnittestAlgTest, getDefaultPropertyValue ) {
    int prop= getIntProperty( "MyProperty" );
    EXPECT_EQ( prop, 1 );
  }

  TEST_F( AthAsgExUnittestAlgTest, initialise ) {
    EXPECT_TRUE( myAlg->initialize().isSuccess() );
  }

  TEST_F( AthAsgExUnittestAlgTest, setProperty ) {
    EXPECT_TRUE( myAlg->setProperty( "MyProperty", 5 ).isSuccess() );
    EXPECT_TRUE( myAlg->initialize().isSuccess() );
    int prop= getIntProperty( "MyProperty" );
    EXPECT_EQ( prop, 5 );
  }

  TEST_F( AthAsgExUnittestAlgTest, getPropertyFromCatalogue ) {
    EXPECT_TRUE( myAlg->sysInitialize().isSuccess() );
    int prop= getIntProperty( "MyProperty" );
    EXPECT_EQ( prop, 21 );
  }

  TEST_F( AthAsgExUnittestAlgTest, toolProperty ) {
    // sysInitialize() gets properties then calls initialize()
    EXPECT_TRUE( myAlg->sysInitialize().isSuccess() );
    IAthAsgExUnittestTool* mpt= getMyTool();
    double prop= mpt->useTheProperty();
    EXPECT_EQ( prop, 42.0 );
  }

}

int main( int argc, char **argv ) {
  ::testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}
