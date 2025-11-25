/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#define BOOST_TEST_MODULE ActsLoggerTest
#include <boost/test/data/test_case.hpp>
#include <boost/test/included/unit_test.hpp>

#include <iostream>
#include <memory>
#include <stdexcept>

#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/IMessageSvc.h"
#include "GaudiKernel/SmartIF.h"
#include "TestTools/initGaudi.h"

#include "ActsInterop/Logger.h"
#include "ActsInterop/LoggerUtils.h"

namespace ActsTrk::Test {
  static const std::array<MSG::Level, 6> gaudiLogLevels {MSG::FATAL, MSG::ERROR, MSG::WARNING, MSG::INFO, MSG::DEBUG, MSG::VERBOSE};
  static const std::array<Acts::Logging::Level, 6> actsLogLevels {Acts::Logging::Level::FATAL, Acts::Logging::Level::ERROR, Acts::Logging::Level::WARNING, Acts::Logging::Level::INFO, Acts::Logging::Level::DEBUG, Acts::Logging::Level::VERBOSE};
}

BOOST_AUTO_TEST_SUITE( loglevel_consistency )

BOOST_AUTO_TEST_CASE( loglevel_conversion )
{
  for (std::size_t i(0); i<6; ++i) {
    auto gaudiLevel = ActsTrk::Test::gaudiLogLevels[i];
    auto actsLevel = ActsTrk::Test::actsLogLevels[i];
    
    BOOST_CHECK_EQUAL(actsLevel, ActsTrk::actsLevelVector(gaudiLevel));
    BOOST_CHECK_EQUAL(ActsTrk::athLevelVector(actsLevel), gaudiLevel);
    BOOST_CHECK_EQUAL(actsLevel, ActsTrk::actsLevelVector(ActsTrk::athLevelVector(actsLevel)));
    BOOST_CHECK_EQUAL(gaudiLevel, ActsTrk::athLevelVector(ActsTrk::actsLevelVector(gaudiLevel)));
  }
}

BOOST_AUTO_TEST_CASE( loglevel_propagation ) 
{
  ISvcLocator* svcLoc = nullptr;
  if (!Athena_test::initGaudi(svcLoc))
    throw std::runtime_error("Failure in initializing Gaudi");

  SmartIF<IMessageSvc> msgSvc(svcLoc->service("MessageSvc"));
  if (!msgSvc)
    throw std::runtime_error("Failure in retrieving MessageSvc");

  for (MSG::Level lvl : ActsTrk::Test::gaudiLogLevels) {
    std::string name = "MsgStream";
    auto msg = std::make_shared<::MsgStream>(msgSvc, name);
    msg->setLevel(lvl);
    BOOST_CHECK_EQUAL(lvl, msg->level());

    auto filter = std::make_unique<ActsAthenaFilterPolicy>(msg);
    auto print = std::make_unique<ActsAthenaPrintPolicy>(msg, name);
    const auto logger = std::make_unique<const Acts::Logger>(std::move(print), std::move(filter));
    
    BOOST_CHECK_EQUAL(logger->name(), name);
    BOOST_CHECK_EQUAL(logger->level(), ActsTrk::actsLevelVector(msg->level()));

    // make a clone
    auto clonedLogger = logger->clone("ClonedLogger");
    BOOST_CHECK_EQUAL(logger->level(), clonedLogger->level());
    BOOST_CHECK_EQUAL(clonedLogger->name(), "ClonedLogger");

    // make a clone with suffix
    auto clonedWithSuffixLogger = logger->cloneWithSuffix("Cloned");
    BOOST_CHECK_EQUAL(logger->level(), clonedWithSuffixLogger->level());
    BOOST_CHECK_EQUAL(clonedWithSuffixLogger->name(), logger->name()+"Cloned");
  }
} 

BOOST_AUTO_TEST_SUITE_END()

