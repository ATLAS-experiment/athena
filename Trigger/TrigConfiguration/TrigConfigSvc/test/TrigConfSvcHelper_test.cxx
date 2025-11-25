/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TEST_TrigConfSvc
#include <boost/test/unit_test.hpp>
#include "src/TrigConfSvcHelper.h"
#include <string>
namespace utf = boost::unit_test;
using namespace TrigConf;
using namespace std::string_literals;

struct InOutParams{
  //in-out parameters passed by reference
  std::string serverAddress{""};
  std::string apiVersion{""};
  std::string dbName{""};
};


BOOST_AUTO_TEST_SUITE(TrigConfSvcHelperTest)
  BOOST_AUTO_TEST_CASE(NormalCrestParametersReturnTrue){
    const std::string dbConnectionString{"https://crest.cern.ch/api-v5.0/CONF_DATA_RUN3"};
    InOutParams p;
    BOOST_REQUIRE(isCrestConnection(dbConnectionString, p.serverAddress, p.apiVersion, p.dbName));
    BOOST_CHECK_EQUAL(p.serverAddress, "https://crest.cern.ch");
    BOOST_CHECK_EQUAL(p.apiVersion, "api-v5.0");
    BOOST_CHECK_EQUAL(p.dbName, "CONF_DATA_RUN3");
  }
  
  BOOST_AUTO_TEST_CASE(NoHttpReturnsFalse){
    InOutParams p;
    const std::string dbConnectionString{"xxxx://crest.cern.ch/api-v5.0/CONF_DATA_RUN3"};
    //server name MUST start with 'http', as the protocol or not
    BOOST_REQUIRE(not isCrestConnection(dbConnectionString, p.serverAddress, p.apiVersion, p.dbName));
    BOOST_CHECK_EQUAL(p.apiVersion, "");
  }
  
  
  BOOST_AUTO_TEST_CASE(HttpButNoProtocolReturnsTrue, * utf::expected_failures(1)){
    InOutParams p;
    const std::string dbConnectionString{"http.crest.cern.ch/api-v5.0/CONF_DATA_RUN3"};
    BOOST_REQUIRE(isCrestConnection(dbConnectionString, p.serverAddress, p.apiVersion, p.dbName));
    //server name has 'http' and is parsed as part of the URL
    BOOST_CHECK_EQUAL(p.serverAddress , "http.crest.cern.ch");//fails!
    //string returned is '://http.crest.cern.ch'
    //
    //
    BOOST_CHECK_EQUAL(p.apiVersion, "api-v5.0");
    BOOST_CHECK_EQUAL(p.dbName, "CONF_DATA_RUN3");
  }
  
  //this fails
  BOOST_AUTO_TEST_CASE(MissingDbNameThrowsException, * utf::expected_failures(1)){
    InOutParams p;
    const std::string dbConnectionString{"http://crest.cern.ch/api-v5.0"};
    BOOST_CHECK_THROW(isCrestConnection(dbConnectionString, p.serverAddress, p.apiVersion, p.dbName), std::runtime_error);
    bool crest = isCrestConnection(dbConnectionString, p.serverAddress, p.apiVersion, p.dbName);
    BOOST_TEST_MESSAGE("'http://crest.cern.ch/api-v5.0' : return is "s + (crest?"true"s:"false"s));
  }
  
  BOOST_AUTO_TEST_CASE(StartsWithHttpButNoSlashesInConnectionStringThrowsException){
    InOutParams p;
    const std::string dbConnectionString{"httpcrest.cern.ch api-v5.0"};
    BOOST_CHECK_THROW(isCrestConnection(dbConnectionString, p.serverAddress, p.apiVersion, p.dbName), std::runtime_error);
  }
  
  BOOST_AUTO_TEST_CASE(StartsWithHttpButNonsenseIsAccepted){
    InOutParams p;
    const std::string dbConnectionString{"httpheffer/frogbutt5.0/900/hello/omygod"};
    BOOST_REQUIRE(isCrestConnection(dbConnectionString, p.serverAddress, p.apiVersion, p.dbName));
  }



BOOST_AUTO_TEST_SUITE_END()

