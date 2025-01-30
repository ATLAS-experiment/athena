// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_Identifier
#include <boost/test/unit_test.hpp>
#include <boost/test/tools/output_test_stream.hpp>
namespace utf = boost::unit_test;

#include "Identifier/ExpandedIdentifier.h"
#include <iostream>
#include <stdexcept>
#include <sstream>

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
   struct print_log_value<ExpandedIdentifier> {
     void operator()( std::ostream& os, ExpandedIdentifier const& v){
       os<<std::string(v);//ExpandedIdentifier has string conversion
     }
   };                                                          
 }
 
 
//example of initialiser string
const std::string initialiserString{"-3/-2/-1/1/+2/3"};


BOOST_AUTO_TEST_SUITE(ExpandedIdentifierTest)

BOOST_AUTO_TEST_CASE(ExpandedIdentifierConstructors){
  BOOST_CHECK_NO_THROW(ExpandedIdentifier());
  ExpandedIdentifier e;
  BOOST_CHECK_NO_THROW(ExpandedIdentifier f(e));
  BOOST_CHECK_NO_THROW(ExpandedIdentifier(std::move(e)));
  BOOST_CHECK_NO_THROW(ExpandedIdentifier k{initialiserString});
  BOOST_CHECK_THROW(ExpandedIdentifier nonsense("piggy back"), std::invalid_argument);
  ExpandedIdentifier g(initialiserString);
  BOOST_CHECK_NO_THROW(ExpandedIdentifier h(g,3));
  BOOST_CHECK_NO_THROW(ExpandedIdentifier h = g);
  BOOST_CHECK_NO_THROW(ExpandedIdentifier i = std::move(g));
}

BOOST_AUTO_TEST_CASE(ExpandedIdentifierRepresentation){
  ExpandedIdentifier g(initialiserString);
  BOOST_TEST(std::string(g) == "-3/-2/-1/1/2/3");
  //
  boost::test_tools::output_test_stream output;
  {//scoped redirect of cout
    cout_redirect guard( output.rdbuf( ) );
    g.show();
  }
  BOOST_CHECK( output.is_equal( "[-3.-2.-1.1.2.3]" ) );
  std::ostringstream s;
  BOOST_CHECK_NO_THROW(s<<g);
  BOOST_TEST( s.str() == "-3/-2/-1/1/2/3");
}


BOOST_AUTO_TEST_CASE(ExpandedIdentifierAccessors){
  ExpandedIdentifier g(initialiserString);
  BOOST_TEST(g[1] == -2);
  BOOST_TEST(g.fields() == 6);
}

BOOST_AUTO_TEST_CASE(ExpandedIdentifierComparison){
  ExpandedIdentifier g(initialiserString);
  ExpandedIdentifier h(initialiserString);
  ExpandedIdentifier i("-3/-2/-1/1/2/2");
  ExpandedIdentifier j("-1/-2/-1/1/2/2");
  ExpandedIdentifier k("-1/-2/-1/1");
  //
  BOOST_TEST(g == h);
  BOOST_TEST(g != i);
  BOOST_TEST(i < h);
  BOOST_TEST(j > h);
  //
  BOOST_TEST(k != i);
  BOOST_TEST(j > k);
  //Is the shorter sequence at the beginning of the longer sequence?
  BOOST_TEST(k.match(j) == true);
  BOOST_TEST(k.match(i) == false);
}

BOOST_AUTO_TEST_CASE(ExpandedIdentifierModifiers){
  ExpandedIdentifier k;
  BOOST_CHECK_THROW(k.set("nonsense"), std::invalid_argument);
  k.set("-3/-2/-1/1/2/3");
  BOOST_TEST(std::string(k) == "-3/-2/-1/1/2/3");
  BOOST_CHECK_NO_THROW(k.clear());
  BOOST_TEST(k.fields() == 0);
  BOOST_CHECK_NO_THROW(k<<3<<2<<1);
  BOOST_TEST(std::string(k) == "3/2/1");
  BOOST_CHECK_NO_THROW(k.add(9));
  BOOST_TEST(std::string(k) == "3/2/1/9");
}

BOOST_AUTO_TEST_SUITE_END()

