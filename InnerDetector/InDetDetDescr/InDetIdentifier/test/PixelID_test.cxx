/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/*
 */
/**
 * @file InDetIdentifier/test/PixelID_test.cxx
 * @author Shaun Roe
 * @date Jan 2026
 * @brief Some tests for PixelID 
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE InDetIdentifier

#include "IdDictParser/IdDictParser.h"  
#include "InDetIdentifier/PixelID.h"
#include "Identifier/IdContext.h"
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

//
//would prefer to use a local file in the package
static const std::string idDictFilename{"InDetIdDictFiles/IdDictInnerDetector_IBL3D25-03.xml"};
static const std::string rangeError{"ERROR SCT_ID::wafer_id_checks  result is NOT ok. ID, range 2/2/10/3/3/1/02/2/0/0/0:31/-6:-1/0:1 | 2/2/0/1/0:39/-6:-1/0:1 | 2/2/0/2/0:47/-6:-1/0:1 | 2/2/0/3/0:55/-6:-1/0:1 | 2/2/0/0/0:31/1:6/0:1 | 2/2/0/1/0:39/1:6/0:1 | 2/2/0/2/0:47/1:6/0:1 | 2/2/0/3/0:55/1:6/0:1 | 2/2/-2,2/0:8/0:51/0/0:1 | 2/2/-2,2/0/0:39/1/0:1 | 2/2/-2,2/1:5/0:39/1:2/0:1 | 2/2/-2,2/6:7/0:39/1/0:1\n"};

BOOST_AUTO_TEST_SUITE(PixelID_Test)
  BOOST_AUTO_TEST_CASE(IdentifierMethods){
    IdDictParser parser;
    parser.register_external_entity("InnerDetector", idDictFilename);
    IdDictMgr& idd = parser.parse ("IdDictParser/ATLAS_IDS.xml");
    PixelID pixelId;
    BOOST_TEST(pixelId.initialize_from_dictionary (idd) == 0);
    BOOST_TEST((pixelId.helper() == AtlasDetectorID::HelperType::Pixel));
  }

  
 
  
BOOST_AUTO_TEST_SUITE_END()
