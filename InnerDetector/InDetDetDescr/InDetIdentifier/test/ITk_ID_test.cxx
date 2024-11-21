/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/
/*
 */
/**
 * @file InDetIdentifier/test/ITk_ID_test.cxx
 * @author Shaun Roe
 * @date Nov 2024
 * @brief Some tests for SCT_ID using ITk input
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE InDetIdentifier

#include "IdDictParser/IdDictParser.h"  
#include "InDetIdentifier/SCT_ID.h"
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
static const std::string itkDictFilename{"InDetIdDictFiles/IdDictInnerDetector_ITK-P2-RUN4-03-00-00.xml"};

BOOST_AUTO_TEST_SUITE(ITk_ID_Test)
  BOOST_AUTO_TEST_CASE(IdentifierMethods){
    IdDictParser parser;
    parser.register_external_entity("InnerDetector", itkDictFilename);
    IdDictMgr& idd = parser.parse ("IdDictParser/ATLAS_IDS.xml");
    SCT_ID itkId;
    BOOST_TEST(itkId.initialize_from_dictionary (idd) == 0);
    BOOST_TEST((itkId.helper() == AtlasDetectorID::HelperType::SCT));
    //check valid module identifier
    Identifier::value_type v = 0xac1b70000000000;
    const auto barrelIdentifier = itkId.module_id(0,3,3,-1);
    BOOST_TEST(barrelIdentifier == Identifier(v));
    BOOST_TEST_MESSAGE("Module (0,3,3,-1) : "<<barrelIdentifier);
    BOOST_TEST(barrelIdentifier == Identifier(v));
    //the following case doesn't generate any error, even with 'checks on'
    const auto nonsenseIdentifier = itkId.module_id(0,3,3,0);//eta=0 modules don't exist in the barrel
    BOOST_TEST_MESSAGE("Module (0,3,3,0) : "<<nonsenseIdentifier<<", which is nonsense.");//but this doesn't prevent it generating a number
    //check wafer Identifier for side 0
    const auto barrelWaferId0 = itkId.wafer_id(0,3,3,-1,0);
    //which is just the same as the module Id...
    BOOST_TEST(barrelWaferId0 == Identifier(v));
    //whereas "side 1" ...
    Identifier::value_type s1 = 0xac1b78000000000;
    const auto barrelWaferId1 = itkId.wafer_id(0,3,3,-1,1);
    BOOST_TEST(barrelWaferId1 == Identifier(s1));
    //strip 0 of side 0...
    const auto barrelStrip0id = itkId.strip_id(0,3,3,-1,0,0);
    //has the same id as the module...
    BOOST_TEST(barrelStrip0id == Identifier(v));
    //but strip 1 does not...
    Identifier::value_type strip1 = 0xac1b70008000000;
    const auto barrelStrip1id = itkId.strip_id(0,3,3,-1,0,1);
    BOOST_TEST(barrelStrip1id == Identifier(strip1));
    //alternatively can specify the wafer Id and a strip number
    BOOST_TEST(itkId.strip_id(barrelWaferId0, 1) == barrelStrip1id);
    //..and I'm not going to test the 'row' alternative
    //int base_bit(void) const;
    BOOST_TEST(itkId.base_bit() == 27);//?why?
    //Identifier::diff_type calc_offset(const Identifier& base,const Identifier& target) const;
    const auto barrelStrip99id = itkId.strip_id(0,3,3,-1,0,99);
    auto offset = itkId.calc_offset(barrelIdentifier,barrelStrip99id );
    BOOST_TEST(offset == 99);
    //Identifier strip_id_offset(const Identifier& base,Identifier::diff_type offset) const;
    BOOST_TEST(itkId.strip_id_offset(barrelIdentifier, 99) == barrelStrip99id);
    //expanded identifier methods
    //void get_expanded_id(const Identifier& id,ExpandedIdentifier& exp_id, const IdContext* context = 0) const;
    ExpandedIdentifier expId;
    itkId.get_expanded_id(barrelIdentifier, expId);//who uses this?
    BOOST_TEST(std::string(expId) == "2/2/0/3/3/-1/0/0/0");
    // Identifier strip_id(const ExpandedIdentifier& strip_id) const;
    BOOST_TEST(itkId.strip_id(expId) == barrelIdentifier);
    //Iterator usage
    const int nWafersUsingIterator = std::accumulate(itkId.wafer_begin(), itkId.wafer_end(), 0,[](int s, Identifier ){return s+1;});
    BOOST_TEST(nWafersUsingIterator == 49536);
    //expanded iterator usage
    const int nStripsUsingIterator = std::accumulate(itkId.strip_begin(), itkId.strip_end(), 0,[](int s, const ExpandedIdentifier& ){return s+1;});
    BOOST_TEST(nStripsUsingIterator == 59'867'136);//~60 million channels
    //complete test, more-or-less equivalent to "test wafer packing" in original code
    auto context = itkId.wafer_context();
    bool allIdentifiersAndExpandedIdentifiersAreEquivalent{true};
    enum indices{INDET, ITk, BARREL_EC, LAYER_DISK, PHI, ETA, SIDE, STRIP};
    for(auto pWaferId = itkId.wafer_begin(); pWaferId != itkId.wafer_end(); ++pWaferId){
      Identifier id = (*pWaferId);
      ExpandedIdentifier expId;
      itkId.get_expanded_id(id, expId, &context);
      Identifier returnedId = itkId.wafer_id(expId[BARREL_EC],
                                   expId[LAYER_DISK],
                                   expId[PHI],
                                   expId[ETA],
                                   expId[SIDE]);
      allIdentifiersAndExpandedIdentifiersAreEquivalent &=(returnedId == id);
    }
    BOOST_TEST(allIdentifiersAndExpandedIdentifiersAreEquivalent);
  }
  
 
  
 
  
 
  
BOOST_AUTO_TEST_SUITE_END()
