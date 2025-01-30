// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_Identifier
#include <boost/test/unit_test.hpp>
#include <boost/test/tools/output_test_stream.hpp>
namespace utf = boost::unit_test;

#include "Identifier/IdentifierHash.h"
#include <type_traits>


BOOST_AUTO_TEST_SUITE(IdentifierHashTest)

BOOST_AUTO_TEST_CASE(IdentifierHashConstructors){
  //compile time tests
  static_assert(std::is_trivially_destructible<IdentifierHash>::value);
  static_assert(std::is_trivially_copy_constructible<IdentifierHash>::value);
  //
  BOOST_CHECK_NO_THROW(IdentifierHash());
  IdentifierHash e;
  BOOST_CHECK_NO_THROW([[maybe_unused]] IdentifierHash f(e));
  BOOST_CHECK_NO_THROW([[maybe_unused]] IdentifierHash g(std::move(e)));
  IdentifierHash g;
  BOOST_CHECK_NO_THROW([[maybe_unused]] IdentifierHash h = g);
  BOOST_CHECK_NO_THROW([[maybe_unused]] IdentifierHash i = std::move(g));
  BOOST_CHECK_NO_THROW([[maybe_unused]] IdentifierHash j(352));
  
}

BOOST_AUTO_TEST_CASE(IdentifierHashAccessors){
  IdentifierHash g(356);
  BOOST_TEST(int(g) == 356);//might be dangerous, but..
  BOOST_TEST(g+4 == 360);//probably intended
  BOOST_TEST(g.value() == 356);
  BOOST_TEST(g.is_valid());
  IdentifierHash h;
  BOOST_TEST(h.is_valid() ==  false);
  h = 200;
  BOOST_TEST(h.value() == 200); 
}

BOOST_AUTO_TEST_CASE(IdentifierHashAssignment){
  IdentifierHash e(356);
  BOOST_CHECK_NO_THROW(e = 200);
  IdentifierHash g(356);
  BOOST_CHECK_NO_THROW(g+=10);
  BOOST_TEST(g == 366);
  BOOST_CHECK_NO_THROW(g-=10);
  BOOST_TEST(g == 356);
}
BOOST_AUTO_TEST_CASE(IdentifierHashHashFunction){
  std::hash<IdentifierHash> h;
  BOOST_TEST( h(999) == 999);
}


BOOST_AUTO_TEST_SUITE_END()

