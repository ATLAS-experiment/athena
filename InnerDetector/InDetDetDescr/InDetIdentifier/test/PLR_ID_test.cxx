/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/*
 */
/**
 * @file InDetIdentifier/test/PLR_ID_test.cxx
 * @author Shaun Roe
 * @date Nov 2024
 * @brief Some tests for PLR_ID using PLR input
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE InDetIdentifier

#include "IdDictParser/IdDictParser.h"  
#include "InDetIdentifier/PLR_ID.h"
#include <string>

#include <boost/test/unit_test.hpp>

#include <boost/test/tools/output_test_stream.hpp>
#include <iostream>
#include <algorithm>

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

//https://gitlab.cern.ch/atlas/athena/-/blob/main/InnerDetector/InDetDetDescr/InDetIdDictFiles/data/IdDictInnerDetector_ITK-P2-RUN4-03-00-00.xml
//would prefer to use a local file in the package
static const std::string PLR_DictFilename{"InDetIdDictFiles/IdDictInnerDetector_ITK-P2-RUN4-03-00-00.xml"};

BOOST_AUTO_TEST_SUITE(PLR_ID_Test)
  BOOST_AUTO_TEST_CASE(IdentifierMethods){
    IdDictParser parser;
    parser.register_external_entity("InnerDetector", PLR_DictFilename);
    IdDictMgr& idd = parser.parse ("IdDictParser/ATLAS_IDS.xml");
    PLR_ID PLR_Id;
    BOOST_TEST(PLR_Id.initialize_from_dictionary (idd) == 0);
    BOOST_TEST((PLR_Id.helper() == AtlasDetectorID::HelperType::PLR));
  }
  
 
  
 
  
 
  
BOOST_AUTO_TEST_SUITE_END()
