// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_IdDict
#include <boost/test/unit_test.hpp>
namespace utf = boost::unit_test;

#include "IdDict/IdDictDefs.h"
#include "Identifier/Range.h" 

BOOST_AUTO_TEST_SUITE(IdDictRegionTest)
BOOST_AUTO_TEST_CASE(IdDictRegionConstructors){
  BOOST_CHECK_NO_THROW(IdDictRegion());
  IdDictRegion i1;
  BOOST_CHECK_NO_THROW([[maybe_unused]] IdDictRegion i2(i1));
  BOOST_CHECK_NO_THROW([[maybe_unused]] IdDictRegion i3(std::move(i1)));
  
}

BOOST_AUTO_TEST_CASE(EmptyIdDictRegionAccessors){
  IdDictRegion f;
  BOOST_TEST(f.group_name() == "");
  BOOST_TEST(f.verify() == true);
}

BOOST_AUTO_TEST_CASE(IdDictRegionModifyMembers){
  IdDictRegion f;
  f.m_name = "sroe";
  f.m_group = "groupName";
  BOOST_TEST(f.group_name() == "groupName");
  //IdDictRange _isa_ IdDictRegionEntry
  IdDictRange * dRange= new  IdDictRange;
  //by minmax
  dRange->m_specification = IdDictRange::by_minmax;
  dRange->m_minvalue = -1;
  dRange->m_maxvalue = 5;
  BOOST_CHECK_NO_THROW(f.add_entry(dRange));//what is the ownership policy?
  BOOST_CHECK(f.size() == 1);
  BOOST_TEST(f.fieldSize() == 0);
  Range r("-1:5");
  BOOST_CHECK_NO_THROW([[maybe_unused]] auto s = f.build_range());
  BOOST_TEST(r == f.build_range());
}

BOOST_AUTO_TEST_SUITE_END()
