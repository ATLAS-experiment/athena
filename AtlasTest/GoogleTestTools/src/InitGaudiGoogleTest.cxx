/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GoogleTestTools/InitGaudiGoogleTest.h"

#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/IAppMgrUI.h"

#include <iostream>
#include <fstream>

namespace Athena_test {

  InitGaudiGoogleTest::InitGaudiGoogleTest( const std::string& jobOptsPath, MSG::Level level ) :
    // create a new ApplicationMgr w/o singleton behaviour
    theApp( Gaudi::createApplicationMgrEx( "GaudiCoreSvc", "ApplicationMgr" ) ),
    propMgr( theApp ),
    svcLoc( theApp ),
    svcMgr( theApp ),
    toolSvc( svcLoc->service("ToolSvc") )
  {
    EXPECT_TRUE( theApp != nullptr );
    EXPECT_TRUE( propMgr.isValid() );
    EXPECT_TRUE( svcLoc.isValid() );
    EXPECT_TRUE( svcMgr.isValid() );
    EXPECT_TRUE( toolSvc.isValid() );
    // set the new ApplicationMgr as instance in Gaudi
    Gaudi::setInstance( theApp );

    EXPECT_TRUE( propMgr->setProperty( "OutputLevel", std::to_string( level ) ).isSuccess() );
    if (jobOptsPath.empty()) {
      EXPECT_TRUE( propMgr->setProperty( "JobOptionsType", "NONE" ).isSuccess() );
    }
    else {
      EXPECT_TRUE( propMgr->setProperty( "JobOptionsType", "FILE" ).isSuccess() );
      EXPECT_TRUE( propMgr->setProperty( "JobOptionsPath", jobOptsPath ).isSuccess() );
    }
    EXPECT_TRUE( theApp->configure().isSuccess() );
    EXPECT_TRUE( theApp->initialize().isSuccess() );
  }

  // Finalize and terminate Gaudi, more to make sure that things
  // went clean than anything else because we are throwing away the
  // ApplicationMgr anyway
  InitGaudiGoogleTest::~InitGaudiGoogleTest() {
    EXPECT_TRUE( theApp->finalize().isSuccess() );
    EXPECT_TRUE( theApp->terminate().isSuccess() );
  }

  InitGaudiGoogleTest::InitGaudiGoogleTest( const std::string& jobOptsPath ) :
    InitGaudiGoogleTest(jobOptsPath, MSG::INFO) {}

  InitGaudiGoogleTest::InitGaudiGoogleTest( MSG::Level level ) :
    InitGaudiGoogleTest({}, level) {}

}

