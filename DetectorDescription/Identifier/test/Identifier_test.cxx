// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_Identifier
#include <boost/test/unit_test.hpp>
#include <boost/test/tools/output_test_stream.hpp>
#include <boost/core/demangle.hpp>
namespace utf = boost::unit_test;

#include "Identifier/Identifier.h"
#include "Identifier/Identifier32.h"
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



//ensure BOOST knows how to represent an ExpandedIdentifier
namespace boost::test_tools::tt_detail {
   template<>           
   struct print_log_value<Identifier> {
     void operator()( std::ostream& os, Identifier const& v){
       os<<v.getString();
     }
   };                                                          
 }


BOOST_AUTO_TEST_SUITE(IdentifierTest)

BOOST_AUTO_TEST_CASE(IdentifierConstructors){
  //compile time test
  static_assert(std::is_trivially_destructible<Identifier>::value);
  static_assert(std::is_trivially_copy_constructible<Identifier>::value);
  //
  BOOST_CHECK_NO_THROW(Identifier());
  Identifier e;
  BOOST_CHECK_NO_THROW([[maybe_unused]] Identifier f(e));
  BOOST_CHECK_NO_THROW(Identifier(std::move(e)));
  BOOST_CHECK_NO_THROW(Identifier(90));
  Identifier g(324242);
  BOOST_CHECK_NO_THROW([[maybe_unused]] Identifier h = g);
  BOOST_CHECK_NO_THROW([[maybe_unused]] Identifier i = std::move(g));
  //
  Identifier::value_type vt{7647634};
  std::string real_name = boost::core::demangle(typeid(vt).name());
  BOOST_TEST_MESSAGE("Identifier::value_type is " + real_name);
  boost::test_tools::output_test_stream output;
  {//scoped redirect of cout
    cout_redirect guard( output.rdbuf() );
    BOOST_CHECK_NO_THROW([[maybe_unused]] Identifier a(vt));
  }
  //should have put a warning in cout
  BOOST_TEST( output.str().find("WARNING")!= std::string::npos);
  output.flush();
  //
  Identifier32 i32(345);
  BOOST_CHECK_NO_THROW([[maybe_unused]] Identifier j(i32));
  Identifier32::value_type vt32{98988};
  real_name = boost::core::demangle(typeid(vt32).name());
  BOOST_TEST_MESSAGE("Identifier32::value_type is " + real_name);
  BOOST_CHECK_NO_THROW([[maybe_unused]] Identifier k(vt32));
  
}

BOOST_AUTO_TEST_CASE(IdentifierAccessors){
  Identifier e;
  BOOST_TEST(e.is_valid() ==  false);
  Identifier g(0x4F292);
  BOOST_TEST(g.is_valid() ==  true);
  BOOST_TEST(g.get_compact() ==  0x4F29200000000);
  Identifier32 i32(0x4F292);
  BOOST_TEST(g.get_identifier32() == i32);
}

BOOST_AUTO_TEST_CASE(IdentifierModifiers){
  Identifier e;
  BOOST_CHECK_NO_THROW(e.set("0x4F292"));
  BOOST_TEST(e.get_compact() ==  0x4F292);
  BOOST_CHECK_NO_THROW(e.set_literal(9));
  BOOST_CHECK_THROW(e.set("hghghgh"), std::runtime_error);
  BOOST_TEST(e.get_compact() ==  9);
  BOOST_CHECK_NO_THROW(e.clear());
  BOOST_TEST(e.is_valid() == false);
  
}

BOOST_AUTO_TEST_CASE(IdentifierRepresentation){
  Identifier g(0xF1234);
  BOOST_TEST(g.getString() == "0xf123400000000");
  boost::test_tools::output_test_stream output;
  {//scoped redirect of cout
    cout_redirect guard( output.rdbuf( ) );
    g.show();
  }
  BOOST_CHECK( output.is_equal( "0xf123400000000" ) );
}

BOOST_AUTO_TEST_CASE(IdentifierSelfComparison){
  Identifier g(0xF1234);
  Identifier h(0xF1234);
  Identifier i(0xF1233);
  Identifier j(0xF1235);
  Identifier k(0);
  //
  BOOST_TEST(g == h);
  BOOST_TEST(g != i);
  BOOST_TEST(i < h);
  BOOST_TEST(j > h);
  //
  BOOST_TEST(k != i);
  BOOST_TEST(j > k);
}

BOOST_AUTO_TEST_CASE(IdentifierValueComparison){
  Identifier g(0xF1234);
  //
  Identifier::value_type h(0xf123400000000);
  Identifier::value_type k(0xF12);
  BOOST_TEST(g == h);
  BOOST_TEST(g != k);
  //
  Identifier32::value_type j(0xF1234);
  Identifier32::value_type l(0xF12);
  BOOST_TEST(g == j);
  BOOST_TEST(g != l);
  //
  int m(0xF1234);
  int n(0xF12);
  BOOST_TEST(g == m);
  BOOST_TEST(g != n);
}

BOOST_AUTO_TEST_CASE(IdentifierAssignment){ 
  Identifier z;
  Identifier::value_type vt{7647634};
  boost::test_tools::output_test_stream output;
  {//scoped redirect of cout
    cout_redirect guard( output.rdbuf() );
    BOOST_CHECK_NO_THROW(z = vt);
  }
  //should have put a warning in cout
  BOOST_TEST( output.str().find("WARNING")!= std::string::npos);
  //
  Identifier32 i32(0x159);
  BOOST_CHECK_NO_THROW(z = i32);
  BOOST_TEST(z.get_compact() == 0x15900000000);
  //
  Identifier y;
  Identifier32::value_type value{0x159};
  BOOST_CHECK_NO_THROW(y = value);
  BOOST_TEST(y.get_compact() == 0x15900000000);
  //
  Identifier x;
  int i{0xFE};
  BOOST_CHECK_NO_THROW(x = i);
  BOOST_TEST(x.get_compact() == 0xFE00000000);//ok
  //no residual state
  BOOST_CHECK_NO_THROW(y = i);
  BOOST_TEST(y.get_compact() == 0xFE00000000);
}



BOOST_AUTO_TEST_SUITE_END()
