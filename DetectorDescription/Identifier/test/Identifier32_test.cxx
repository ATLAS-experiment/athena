// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_Identifier
#include <boost/test/unit_test.hpp>
#include <boost/test/tools/output_test_stream.hpp>
namespace utf = boost::unit_test;

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
   struct print_log_value<Identifier32> {
     void operator()( std::ostream& os, Identifier32 const& v){
       os<<v.getString();
     }
   };                                                          
 }


BOOST_AUTO_TEST_SUITE(Identifier32Test)

BOOST_AUTO_TEST_CASE(Identifier32Constructors){
  //compile time test
  static_assert(std::is_trivially_destructible<Identifier32>::value);
  static_assert(std::is_trivially_copy_constructible<Identifier32>::value);
  //
  BOOST_CHECK_NO_THROW(Identifier32());
  Identifier32 e;
  BOOST_CHECK_NO_THROW([[maybe_unused]] Identifier32 f(e));
  BOOST_CHECK_NO_THROW(Identifier32(std::move(e)));
  Identifier32 g(324242);
  BOOST_CHECK_NO_THROW([[maybe_unused]] Identifier32 h = g);
  BOOST_CHECK_NO_THROW([[maybe_unused]] Identifier32 i = std::move(g));
}

BOOST_AUTO_TEST_CASE(Identifier32Accessors){
  Identifier32 e;
  BOOST_TEST(e.is_valid() ==  false);
  Identifier32 g(324242);
  BOOST_TEST(g.is_valid() ==  true);
  BOOST_TEST(g.get_compact() ==  324242);
}

BOOST_AUTO_TEST_CASE(Identifier32Representation){
  Identifier32 g(0xF1234);
  BOOST_TEST(g.getString() == "0xf1234");
  boost::test_tools::output_test_stream output;
  {//scoped redirect of cout
    cout_redirect guard( output.rdbuf( ) );
    g.show();
  }
  BOOST_CHECK( output.is_equal( "0xf1234" ) );
  //new methods, Aug 2024
  BOOST_TEST(std::string(g)  == "0xf1234");
  {//scoped redirect of cout
    cout_redirect guard( output.rdbuf( ) );
    std::cout<<g; //stream insertion operator
  }
  BOOST_TEST( output.is_equal( "0xf1234" ), "Stream insertion operator" );
}

BOOST_AUTO_TEST_CASE(Identifier32Comparison){
  Identifier32 g(0xF1234);
  Identifier32 h(0xF1234);
  Identifier32 i(0xF1233);
  Identifier32 j(0xF1235);
  Identifier32 k(0);
  //
  BOOST_TEST(g == h);
  BOOST_TEST(g != i);
  BOOST_TEST(i < h);
  BOOST_TEST(j > h);
  //
  BOOST_TEST(k != i);
  BOOST_TEST(j > k);
}

BOOST_AUTO_TEST_CASE(Identifier32Modifiers){
  Identifier32 g(0xF1234);
  BOOST_TEST(g.is_valid() ==  true);
  BOOST_CHECK_NO_THROW(g &= 0xF0000);
  BOOST_TEST(g.get_compact() ==  0xF0000);
  BOOST_CHECK_NO_THROW(g |= 1);
  BOOST_TEST(g.get_compact() ==  0xF0001);
  BOOST_CHECK_NO_THROW(g.clear());
  BOOST_TEST(g.is_valid() ==  false);
}

BOOST_AUTO_TEST_SUITE_END()
