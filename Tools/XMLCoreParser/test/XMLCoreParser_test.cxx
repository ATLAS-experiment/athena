/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


/**
 * @author Shaun Roe
 * @date Sept 2024
 * @brief Some tests for XMLCoreParser 
 */
 
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_XMLCoreParser

#include <boost/test/unit_test.hpp>

#include "XMLCoreParser/XMLCoreParser.h"

#include "PathResolver/PathResolver.h"
#include <string>
namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE(XMLCoreParserTest)

  BOOST_AUTO_TEST_CASE(XMLCoreParserConstructors){
    BOOST_CHECK_NO_THROW([[maybe_unused]] XMLCoreParser s);
  }
  
  BOOST_AUTO_TEST_CASE(XMLCoreParserParse){
    XMLCoreParser p;
    const std::string fName= "XMLCoreParser/WellFormed.xml";
    std::string file = PathResolver::find_file (fName, "DATAPATH");
    BOOST_TEST_MESSAGE("Filename: "+ file);
    BOOST_CHECK_NO_THROW( [[maybe_unused]] std::unique_ptr<XMLCoreNode> n = p.parse(file));
    std::unique_ptr<XMLCoreNode> n{p.parse(file)};
    BOOST_TEST(n->get_type() ==  XMLCoreNode::DOCUMENT_NODE);
  }
  
  BOOST_AUTO_TEST_CASE(XMLCoreParserErrorCondition, *utf::expected_failures(1)){
    XMLCoreParser s;
    const std::string noFile= "Inexistent.xml";
    BOOST_CHECK_THROW( [[maybe_unused]] std::unique_ptr<XMLCoreNode> n = s.parse(noFile), std::runtime_error);
    //
    XMLCoreParser p;
    const std::string fName= "XMLCoreParser/IllFormed.xml";
    std::string file = PathResolver::find_file (fName, "DATAPATH");
    BOOST_TEST_MESSAGE("Filename: "+ file);
    BOOST_CHECK_THROW( [[maybe_unused]] std::unique_ptr<XMLCoreNode> n = p.parse(file), std::runtime_error);
    //the following should not work, as the xml file is ill-formed
    //but expat is a *stream* parser, so might not fail until it explicitly tries
    //to parse the ill-formed element
    std::unique_ptr<XMLCoreNode> n{p.parse(file)};
    BOOST_TEST(n->get_type() ==  XMLCoreNode::DOCUMENT_NODE);
  }
  
  
BOOST_AUTO_TEST_SUITE_END()
