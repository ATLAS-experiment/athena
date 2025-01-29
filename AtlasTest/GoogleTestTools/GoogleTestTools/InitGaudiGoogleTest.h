/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
// -*- mode: c++ -*-

/** 
@file TestTools/InitGaudiGoogleTest.h
@author S. Kluth
@date July 2016
@brief Gaudi initialisation fixture base class for googletest based unit tests.
Test fixture classes for tests which need a fully configured Gaudi should inherit
from this class, and then provide further Gaudi configuration in their SetUp()
method if needed.
*/

#ifndef GOOGLETESTTOOLS_INITGAUDIGOOGLETEST_H
#define GOOGLETESTTOOLS_INITGAUDIGOOGLETEST_H

#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/IProperty.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/ISvcManager.h"
#include "GaudiKernel/IToolSvc.h"

#include "gtest/gtest.h"

class IAppMgrUI;

namespace Athena_test {

  class InitGaudiGoogleTest : public ::testing::Test {
  public:

    /**
     * @brief Create GoogleTest fixture
     * @param level MessageSvc OutputLevel
     */
    InitGaudiGoogleTest( MSG::Level level=MSG::INFO );

    /**
     * @brief Create GoogleTest fixture
     * @param jobOptsPath Path to job options
     */
    InitGaudiGoogleTest( const std::string& jobOptsPath );

    /** @brief dtor */
    virtual ~InitGaudiGoogleTest();

    /** @brief public members are visible in tests
     **@{*/
    IAppMgrUI* theApp;
    SmartIF<IProperty> propMgr;
    SmartIF<ISvcLocator> svcLoc;
    SmartIF<ISvcManager> svcMgr;
    SmartIF<IToolSvc> toolSvc;
    /**@}*/

  private:
    InitGaudiGoogleTest( const std::string& jobOptsPath, MSG::Level level );
 
 };

}

#endif
