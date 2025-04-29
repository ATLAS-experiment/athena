// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_IdDict
#include <boost/test/unit_test.hpp>
namespace utf = boost::unit_test;

#include "IdDict/IdDictDefs.h"

BOOST_AUTO_TEST_SUITE(IdDictLabelTest)
BOOST_AUTO_TEST_CASE(IdDictLabelConstructors){
  BOOST_CHECK_NO_THROW(IdDictLabel());
  IdDictLabel i1;
  BOOST_CHECK_NO_THROW([[maybe_unused]] IdDictLabel i2(i1));
  BOOST_CHECK_NO_THROW([[maybe_unused]] IdDictLabel i3(std::move(i1)));
  [[maybe_unused]] IdDictLabel i4{"test", true, 100};
  [[maybe_unused]] IdDictLabel i5{.m_name="sroe", .m_valued=true, .m_value=1};
}
BOOST_AUTO_TEST_CASE(EmptyIdDictLabelAccessors){
  IdDictLabel f;
  BOOST_TEST(f.m_name == "");
  BOOST_TEST(f.m_valued == false);
  BOOST_TEST(f.m_value == 0);
}

BOOST_AUTO_TEST_SUITE_END()
