// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_IdDict
#include <boost/test/unit_test.hpp>
namespace utf = boost::unit_test;

#include "IdDict/IdDictDefs.h"


BOOST_AUTO_TEST_SUITE(IdDictDictionaryTest)
BOOST_AUTO_TEST_CASE(IdDictDictionaryConstructors){
  BOOST_CHECK_NO_THROW(IdDictDictionary());
}
BOOST_AUTO_TEST_CASE(EmptyIdDictDictionaryAccessors){
  IdDictDictionary d;
  BOOST_TEST(d.find_field ("nonsense") == nullptr);  
  BOOST_TEST(d.find_label ("field", "label") == nullptr);
  int value;
  BOOST_TEST(d.get_label_value ("field", "label", value) > 0);  // > 0 == error
  BOOST_TEST(d.find_subregion ("subregion_name") == nullptr);  
  BOOST_TEST(d.find_region ("region_name") == nullptr);  
  BOOST_TEST(d.find_region ("region_name", "group_name") == nullptr);  
  BOOST_TEST(d.find_group ("group_name") == nullptr);
}
BOOST_AUTO_TEST_CASE(IdDictDictionaryExposedMemberAccess){
  IdDictDictionary d;
  //just checking this compiles, really
  d.m_name = "someName";
  d.m_version = "1.0";
  d.m_date = "32/9/2024";
  d.m_author = "sroe";
  BOOST_TEST_MESSAGE("m_name, m_version, m_date, m_author are exposed members");
}



BOOST_AUTO_TEST_SUITE_END()
