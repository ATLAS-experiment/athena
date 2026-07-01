/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @author Shaun Roe
 * @date Sept 2024
 * @brief Some tests for IdDictParser 
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_IdDictParser

#include "IdDictParser/IdDictParser.h"  
#include <string>

#include <boost/test/unit_test.hpp>
#include <boost/test/tools/output_test_stream.hpp>
#include <iostream>
#include <algorithm>
namespace utf = boost::unit_test;


//in case we want to catch the debug output
struct cout_redirect {
  cout_redirect( std::streambuf * new_buffer ) 
      : m_old( std::cout.rdbuf( new_buffer ) )
  { }

  ~cout_redirect( ) {
      std::cout.rdbuf( m_old );
  }

private:
    std::streambuf * m_old;
};

static const std::string sctDictFilename{"IdDictInnerDetector.xml"};


BOOST_AUTO_TEST_SUITE(IdDictParserTest)
  BOOST_AUTO_TEST_CASE(IdDictParserConstruction){
    BOOST_CHECK_NO_THROW(IdDictParser());
  }
  BOOST_AUTO_TEST_CASE(IdDictParser_parse){
    IdDictParser parser;
    BOOST_TEST(parser.m_dictionary == nullptr);
    BOOST_CHECK_NO_THROW(parser.register_external_entity("InnerDetector", sctDictFilename));
    //nb: idd cannot be copied
    BOOST_CHECK_NO_THROW( [[maybe_unused]] IdDictMgr & idd = parser.parse ("IdDictParser/ATLAS_IDS.xml"));
    boost::test_tools::output_test_stream output;
    { 
      //note that the nonsense identifiers are still numbers which might be used
      cout_redirect guard( output.rdbuf( ) ); //catch the cout output for comparison
      [[maybe_unused]] IdDictMgr & idd = parser.parse ("IdDictParser/ATLAS_IDS.xml");
    }
    if (::getenv("XMLDEBUG")){
      //presence of IdDictParser::parse3> in the debug output indicates it finished ok
      BOOST_TEST( output.str().find("IdDictParser::parse3>")!= std::string::npos);
    } else {
      BOOST_TEST_MESSAGE("XML Debug output not checked outside of ctest");
    }
  }
  
  BOOST_AUTO_TEST_CASE(IdDictParserErrorCases, * utf::expected_failures(1)){
    IdDictParser parser;
    BOOST_CHECK_THROW( [[maybe_unused]] IdDictMgr & idd = parser.parse ("IdDictParser/IllFormedXmlTest.xml"), std::runtime_error);
    //the following simply crashes with memory access violation
    BOOST_CHECK_THROW( [[maybe_unused]] IdDictMgr & idd = parser.parse ("IdDictParser/Inexistent.xml"), std::runtime_error);
  }
  
BOOST_AUTO_TEST_SUITE_END()
