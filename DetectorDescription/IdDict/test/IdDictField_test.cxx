// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_IdDict
#include <boost/test/unit_test.hpp>
namespace utf = boost::unit_test;
#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include "IdDict/IdDictDefs.h"

BOOST_AUTO_TEST_SUITE(IdDictFieldTest)
BOOST_AUTO_TEST_CASE(IdDictFieldConstructors){
  BOOST_CHECK_NO_THROW(IdDictField());
}
BOOST_AUTO_TEST_CASE(EmptyIdDictFieldAccessors){
  IdDictField f;
  //exposed members
  BOOST_TEST(f.m_name == "");
  BOOST_TEST(f.m_labels.empty()  == true);
  BOOST_TEST(f.m_index == 0);
  //
  BOOST_TEST(f.find_label ("name") == nullptr);
  BOOST_TEST(f.get_label_number () == 0);
  //doesnt throw, tries to access the memory and causes a crash : to be revisited
  //BOOST_CHECK_THROW(f.get_label(2), std::out_of_range);
  BOOST_TEST(f.get_label_value("label") == 0);
  BOOST_TEST(f.verify() == true);
}

BOOST_AUTO_TEST_SUITE_END()

