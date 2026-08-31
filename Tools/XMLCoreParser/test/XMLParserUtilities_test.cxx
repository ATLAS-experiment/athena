/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @author Shaun Roe
 * @date April 2026
 * @brief Some tests for xmlFileName 
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_IdDictParser

#include "src/XMLParserUtilities.h"  
#include "PathResolver/PathResolver.h"
#include <expat.h>
#include <string>

#include <boost/test/unit_test.hpp>

namespace utf = boost::unit_test;
using namespace XmlParser;


BOOST_AUTO_TEST_SUITE(XMLParserUtilitiesTest)
  BOOST_AUTO_TEST_CASE(nonExistentFile){
    const std::string nonExistent{"Nonexistent.xml"};
    BOOST_CHECK(xmlFileName(nonExistent).empty());
  }
  
  BOOST_AUTO_TEST_CASE(fileIsOnDataPath){
    const std::string fName= "XMLCoreParser/WellFormed.xml";
    const std::string existingFile = PathResolver::find_file (fName, "DATAPATH");
    BOOST_CHECK(not xmlFileName(existingFile).empty());
  }
  BOOST_AUTO_TEST_CASE(rtrimWorks){
    const XML_Char* normalStringWithNewLine("hello\n");
    auto l1 = 6;
    const XML_Char* normalStringWithoutNewLine("hello");
    auto l2= 5;
    BOOST_CHECK(rtrim(normalStringWithNewLine, l1) == std::string("hello"));
    BOOST_CHECK(rtrim(normalStringWithoutNewLine, l2) == std::string("hello"));
    //
    const XML_Char* onlySpacesWithNewLine("     \n");
    const XML_Char* onlySpacesWithoutNewLine("     ");
    BOOST_CHECK(rtrim(onlySpacesWithNewLine, l1) == std::string("     "));
    BOOST_CHECK(rtrim(onlySpacesWithoutNewLine, l2) == std::string("     "));
  }
  BOOST_AUTO_TEST_CASE(isDebugEnabled){
    if (debug_enabled()){
      BOOST_TEST_MESSAGE("XMLDEBUG is set and debugging enabled");
    } else {
      BOOST_TEST_MESSAGE("XMLDEBUG is not set and debugging is disabled");
    }
  }
  BOOST_AUTO_TEST_CASE(labelReportsFunction){
    const std::string lbl = label();
    BOOST_TEST_MESSAGE("label() reports: "<< lbl);
  }
 
  
BOOST_AUTO_TEST_SUITE_END()
