// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_IdDict
#include <boost/test/unit_test.hpp>
namespace utf = boost::unit_test;

#include <boost/test/tools/output_test_stream.hpp>
#include "IdDict/IdDictFieldImplementation.h"
#include "IdDict/IdDictRange.h"
#include "Identifier/IdentifierField.h"
#include "TestTools/initGaudi.h"
#include "AthenaKernel/getMessageSvc.h"
#include <string>
#include <sstream>
#include <vector>

struct cout_redirect {
    cout_redirect( std::streambuf * new_buffer ) 
        : old( std::cout.rdbuf( new_buffer ) )
    { }

    ~cout_redirect( ) {
        std::cout.rdbuf( old );
    }
    std::streambuf * old;
};

BOOST_AUTO_TEST_SUITE(IdDictFieldImplementationTest)
  BOOST_AUTO_TEST_CASE(IdDictFieldImplementationConstructors){
    BOOST_CHECK_NO_THROW(IdDictFieldImplementation());
  }
  BOOST_AUTO_TEST_CASE(show_to_stringHasKeywordsForEmptyObject){
    IdDictFieldImplementation impl;
    std::string s = impl.show_to_string();
    BOOST_TEST(!s.empty());
    BOOST_TEST_MESSAGE(s);
    // The canonical headings we expect regardless of content:
    BOOST_TEST(s.find("decode ") != std::string::npos);
    BOOST_TEST(s.find("vals ") != std::string::npos);
    BOOST_TEST(s.find("mask/zero mask/shift/bits/offset") != std::string::npos);
    BOOST_TEST(s.find("indexes ") != std::string::npos);
    BOOST_TEST(s.find("mode") != std::string::npos);
    const std::string expected{"decode 0 vals 0               mask/zero mask/shift/bits/offset 0   0   0   0   0   indexes                      mode  both_bounded  "};
    BOOST_CHECK_EQUAL(s, expected);
  }
  BOOST_AUTO_TEST_CASE(show_to_stringGivesExpectedOutputForInitialisedObject){
    IdDictFieldImplementation impl;
    impl.set_decode_index(true);
    impl.set_bits(1,9);
    const std::vector<IdentifierField::element_type> ev{ -4, -3, -2, -1 , 1, 2, 3, 4};
    IdentifierField f1(ev);
    impl.set_ored_field(f1);
    std::string s = impl.show_to_string();
    const std::string expected{"decode 1 vals -4,-3,-2,-1,1,2,3,4 mask/zero mask/shift/bits/offset 7   ff8fffffffffffff 52  3   9   indexes                      mode  enumerated  "};
    BOOST_TEST_MESSAGE(s);
    BOOST_CHECK_EQUAL(s, expected);
  }
  BOOST_AUTO_TEST_CASE(streamInsertionGivesExpectedOutputForInitialisedObject){
    IdDictFieldImplementation impl;
    impl.set_decode_index(true);
    impl.set_bits(1,9);
    const std::vector<IdentifierField::element_type> ev{ -4, -3, -2, -1 , 1, 2, 3, 4};
    IdentifierField f1(ev);
    impl.set_ored_field(f1);
    std::ostringstream s;
    s<<impl;
    const std::string expected{"decode 1 vals -4,-3,-2,-1,1,2,3,4 mask/zero mask/shift/bits/offset 7   ff8fffffffffffff 52  3   9   indexes                      mode  enumerated  "};
    BOOST_TEST_MESSAGE(s.str());
    BOOST_CHECK_EQUAL(s.str(), expected);
  }
  BOOST_AUTO_TEST_CASE(MsgStreamInsertionGivesExpectedOutputForInitialisedObject){
    //init Gaudi and setup messaging
    ISvcLocator* pDum{};
    IMessageSvc *pMS{};
    BOOST_CHECK(Athena_test::initGaudi(pDum));
    pMS = Athena::getMessageSvc();
    BOOST_CHECK(pMS != nullptr);
    pMS->addRef();
    //prefix will be truncated to 'IdDictFieldImpl...'
    MsgStream mlog(pMS, "IdDictFieldImplementation_test");
    //
    IdDictFieldImplementation impl;
    impl.set_decode_index(true);
    impl.set_bits(1,9);
    const std::vector<IdentifierField::element_type> ev{ -4, -3, -2, -1 , 1, 2, 3, 4};
    IdentifierField f1(ev);
    impl.set_ored_field(f1);
    const std::string essential{"decode 1 vals -4,-3,-2,-1,1,2,3,4 mask/zero mask/shift/bits/offset 7   ff8fffffffffffff 52  3   9   indexes                      mode  enumerated  \n"};
    const std::string info{"INFO"};
    boost::test_tools::output_test_stream output;
    //capture 'cout' inside this scope
    {
      cout_redirect guard( output.rdbuf( ) );
      mlog << MSG::INFO << impl <<endmsg;
    }
    bool containsEssentialInfo = (output.str().find(essential) != std::string::npos) and (output.str().find(info) != std::string::npos);
    BOOST_CHECK(containsEssentialInfo);
  }



BOOST_AUTO_TEST_SUITE_END()
