/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @author Shaun Roe
 * @date Nov 2024
 * @brief Some tests for IdDictMgr
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_IdDict


#include "IdDict/IdDictDefs.h"  
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

static const std::string sctDictFilename{"InDetIdDictFiles/IdDictInnerDetector_ITK-P2-RUN4-03-00-00.xml"};


BOOST_AUTO_TEST_SUITE(IdDictMgrTest)
  BOOST_AUTO_TEST_CASE(IdDictMgrConstruction){
    BOOST_CHECK_NO_THROW(IdDictMgr());
  }
  BOOST_AUTO_TEST_CASE(IdDictMgrFromParser){
    IdDictParser parser;
    parser.register_external_entity("InnerDetector", sctDictFilename);
    BOOST_CHECK_NO_THROW( [[maybe_unused]] IdDictMgr & idd = parser.parse ("IdDictParser/ATLAS_IDS.xml"));
  }
  
BOOST_AUTO_TEST_SUITE_END()
