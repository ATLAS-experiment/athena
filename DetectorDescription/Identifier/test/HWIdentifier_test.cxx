/* 
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */
 
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_HWIdentifier
#include <boost/test/unit_test.hpp>
#include <boost/test/tools/output_test_stream.hpp>
#include <boost/core/demangle.hpp>
namespace utf = boost::unit_test;

#include "Identifier/HWIdentifier.h"
#include "Identifier/Identifier.h"
#include <iostream>
#include <stdexcept>
#include <type_traits>


//redirect cout buffer for test
struct cout_redirect {
  cout_redirect( std::streambuf * new_buffer ) 
      : m_old( std::cout.rdbuf( new_buffer ) ){
      /*nop*/ 
  }
  //restore buffer
  ~cout_redirect() {
      std::cout.rdbuf( m_old );
  }

private:
  std::streambuf * m_old{};
};



//ensure BOOST knows how to represent an ExpandedHWIdentifier
namespace boost::test_tools::tt_detail {
   template<>           
   struct print_log_value<HWIdentifier> {
     void operator()( std::ostream& os, HWIdentifier const& v){
       os<<v.getString();
     }
   };                                                          
 }


BOOST_AUTO_TEST_SUITE(HWIdentifierTest)

BOOST_AUTO_TEST_CASE(HWIdentifierConstructors){
  //compile time test
  static_assert(std::is_trivially_destructible<HWIdentifier>::value);
  static_assert(std::is_trivially_copy_constructible<HWIdentifier>::value);
  //
  BOOST_CHECK_NO_THROW(HWIdentifier());//default
  HWIdentifier e;
  BOOST_CHECK_NO_THROW([[maybe_unused]] HWIdentifier f(e));//copy
  BOOST_CHECK_NO_THROW(HWIdentifier(std::move(e)));//move
  BOOST_CHECK_NO_THROW(HWIdentifier(90));//from int
  HWIdentifier g(324242);
  BOOST_CHECK_NO_THROW([[maybe_unused]] HWIdentifier h = g);//assignment
  BOOST_CHECK_NO_THROW([[maybe_unused]] HWIdentifier i = std::move(g));//move assigned
  //
  HWIdentifier::value_type vt{7647634};
  std::string real_name = boost::core::demangle(typeid(vt).name());
  BOOST_TEST_MESSAGE("HWIdentifier::value_type is " + real_name);//what is value_type?
  boost::test_tools::output_test_stream output;
  {//scoped redirect of cout
    cout_redirect guard( output.rdbuf() );
    BOOST_CHECK_NO_THROW([[maybe_unused]] HWIdentifier a(vt));//from value type
  }
  //should have put a warning in cout
  BOOST_TEST( output.str().find("WARNING")!= std::string::npos);
  output.flush();
  //
  Identifier id(454545);
  BOOST_CHECK_NO_THROW([[maybe_unused]] HWIdentifier hwid(id));//from Identifier
}

BOOST_AUTO_TEST_CASE(HWIdentifierHash){
   HWIdentifier g(0x4F292);
   size_t i = std::hash<HWIdentifier>{}(g);
   BOOST_TEST(i == 0x4F29200000000);
}

BOOST_AUTO_TEST_SUITE_END()
