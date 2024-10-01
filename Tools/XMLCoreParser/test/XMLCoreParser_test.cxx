/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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
#include "src/DOMNode.h"
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
    BOOST_CHECK_NO_THROW( [[maybe_unused]] XMLCoreNode n = p.parse(file));
    XMLCoreNode n{p.parse(file)};
    const auto & m = n.get_node();
    BOOST_TEST(m.get_type() ==  CoreParser::DOMNode::NodeType::DOCUMENT_NODE);
  }
  
  BOOST_AUTO_TEST_CASE(XMLCoreParserErrorCondition, *utf::expected_failures(2)){
    XMLCoreParser s;
    const std::string noFile= "Inexistent.xml";
    BOOST_CHECK_THROW( [[maybe_unused]] XMLCoreNode n = s.parse(noFile), std::runtime_error);
    //
    XMLCoreParser p;
    const std::string fName= "XMLCoreParser/IllFormed.xml";
    std::string file = PathResolver::find_file (fName, "DATAPATH");
    BOOST_TEST_MESSAGE("Filename: "+ file);
    BOOST_CHECK_THROW( [[maybe_unused]] XMLCoreNode n = p.parse(file), std::runtime_error);
    //the following should not work, as the xml file is ill-formed
    XMLCoreNode n{p.parse(file)};
    const auto & m = n.get_node();
    BOOST_TEST(m.get_type() ==  CoreParser::DOMNode::NodeType::DOCUMENT_NODE);
  }
  
  
BOOST_AUTO_TEST_SUITE_END()