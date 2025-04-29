// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_Identifier
#include <boost/test/unit_test.hpp>
#include <boost/test/tools/output_test_stream.hpp>
namespace utf = boost::unit_test;

#include "Identifier/IdContext.h"
#include "Identifier/ExpandedIdentifier.h"


//ensure BOOST knows how to represent an ExpandedIdentifier
namespace boost::test_tools::tt_detail {
   template<>           
   struct print_log_value<ExpandedIdentifier> {
     void operator()( std::ostream& os, ExpandedIdentifier const& v){
       os<<std::string(v);//ExpandedIdentifier has string conversion
     }
   };                                                          
 }

BOOST_AUTO_TEST_SUITE(IdContextTest)

BOOST_AUTO_TEST_CASE(IdContextConstructors){
  BOOST_CHECK_NO_THROW(IdContext());
  BOOST_CHECK_NO_THROW([[maybe_unused]] IdContext f(-5,5));
  ExpandedIdentifier e("-1/2/3");
  BOOST_CHECK_NO_THROW([[maybe_unused]] IdContext g(e,-5,5));
  BOOST_CHECK_NO_THROW([[maybe_unused]] IdContext h(std::move(e),-3,2));
  //
  //what if..
  BOOST_CHECK_NO_THROW([[maybe_unused]] IdContext f(10,5));//no checking is done
}

BOOST_AUTO_TEST_CASE(IdContextAccessors){
  ExpandedIdentifier b;
  IdContext a;
  BOOST_TEST(a.begin_index() == 0);
  BOOST_TEST(a.end_index() == 0);
  BOOST_TEST(a.prefix_id() == b);
  //
  ExpandedIdentifier e("-1/2/3");
  IdContext g(e,-5,5);
  BOOST_TEST(g.prefix_id() == e);
  BOOST_TEST(g.begin_index() == -5);
  BOOST_TEST(g.end_index() == 5);
  //
  ExpandedIdentifier f("-1/2/3");
  IdContext h(e,0,5);
  BOOST_TEST(h.prefix_id() == f);
  BOOST_TEST(h.begin_index() == 0);
  BOOST_TEST(h.end_index() == 5);
}

BOOST_AUTO_TEST_CASE(IdContextModifiers){
  ExpandedIdentifier b("-1/2/3");;
  IdContext a;
  BOOST_CHECK_NO_THROW(a.set(b,-1,1));
  BOOST_TEST(a.prefix_id() == b);
  BOOST_TEST(a.begin_index() == -1);
  BOOST_TEST(a.end_index() == 1);
  //what if..
  BOOST_CHECK_NO_THROW(a.set(b,1,1));//no checking is done
  BOOST_TEST(a.begin_index() == 1);
  BOOST_TEST(a.end_index() == 1);
}


BOOST_AUTO_TEST_SUITE_END()


