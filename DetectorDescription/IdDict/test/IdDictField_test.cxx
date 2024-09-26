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
  IdDictField f;
  BOOST_CHECK_NO_THROW([[maybe_unused]] IdDictField f2(f));
  BOOST_CHECK_NO_THROW([[maybe_unused]] IdDictField f3(std::move(f)));
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

BOOST_AUTO_TEST_CASE(IdDictSetAndGet, * utf::expected_failures(1)){
  IdDictField f;
  //IdDictLabel lbl1{"label1", true, 1}; needs to be a struct
  auto lbl1 = new IdDictLabel();
  auto lbl2 = new IdDictLabel();
  lbl1->m_name = "label1";
  lbl1->m_valued = true;
  lbl1->m_value = 1;
  BOOST_CHECK_NO_THROW(f.add_label(lbl1));
  
  lbl2->m_name = "label2";
  lbl2->m_valued = false;
  BOOST_CHECK_NO_THROW(f.add_label(lbl2));
  BOOST_TEST(f.get_label_number() == 2);
  BOOST_TEST(f.get_label(1) == "label2");
  //the folowing simply crashes; there is no check
  //BOOST_TEST(f.get_label(10) == "");
  BOOST_TEST(f.get_label_value("label1") == 1);
  BOOST_TEST(f.get_label_value("label2") == 0);//fails
  BOOST_TEST(f.get_label_value("nonsense") == 0);
  BOOST_TEST(f.verify() == true);
  //f2 holds the same pointers as f1
  IdDictField f2(f);
  BOOST_TEST(f2.get_label(1) == "label2");
  // clear() deletes the label pointers
  // ... but the destructor doesn't (seems dangerous)
  BOOST_CHECK_NO_THROW(f.clear());
  //f2 holds invalid pointers now, but doesn't know
  BOOST_TEST (lbl1 == f2.m_labels[0]);
  BOOST_TEST(f2.get_label_number() == 2);
  //f knows the originals were deleted, and the vector emptied
  BOOST_TEST(f.get_label_number() == 0);
}



BOOST_AUTO_TEST_SUITE_END()

