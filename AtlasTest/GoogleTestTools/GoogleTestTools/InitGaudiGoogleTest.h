/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
#include "CxxUtils/checker_macros.h"

class IAppMgrUI;

namespace Athena_test {

  class ATLAS_NOT_THREAD_SAFE InitGaudiGoogleTest : public ::testing::Test {
  public:

    /**
    @brief Ctor parameter: 
    @param level controls MessageSvc
    */

    InitGaudiGoogleTest( MSG::Level level=MSG::ALWAYS );

    /** @brief dtor */

    virtual ~InitGaudiGoogleTest();

    /** @brief public members are visible in tests */

    IAppMgrUI* theApp;
    SmartIF<IProperty> propMgr;
    SmartIF<ISvcLocator> svcLoc;
    SmartIF<ISvcManager> svcMgr;
    SmartIF<IToolSvc> toolSvc;

  private:

    MSG::Level m_msgLevel;
 
 };

}

#endif
