// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_IdDict
#include <boost/test/unit_test.hpp>
namespace utf = boost::unit_test;

#include "IdDict/IdDictField.h"
#include "IdDict/IdDictLabel.h"

BOOST_AUTO_TEST_SUITE(IdDictFieldTest)
BOOST_AUTO_TEST_CASE(IdDictFieldConstructors){
  BOOST_CHECK_NO_THROW(IdDictField(""));
  IdDictField f("");
  BOOST_CHECK_NO_THROW([[maybe_unused]] IdDictField f3(std::move(f)));
}
BOOST_AUTO_TEST_CASE(EmptyIdDictFieldAccessors){
  IdDictField f("");
  BOOST_TEST(f.name() == "");
  BOOST_TEST(f.get_label_number()  == 0);
  BOOST_TEST(f.index() == 0);
  //
  BOOST_TEST(f.find_label ("name") == nullptr);
  BOOST_TEST(f.get_label_number () == 0);
  //doesnt throw, tries to access the memory and causes a crash : to be revisited
  BOOST_CHECK_THROW(f.get_label(2), std::out_of_range);
  BOOST_TEST(f.get_label_value("label") == 0);
  BOOST_TEST(f.verify() == true);
}

BOOST_AUTO_TEST_CASE(IdDictSetAndGet){
/**
  Following remain untested, as they depend on IdDictMgr:
    void resolve_references (const IdDictMgr& idd);  
    void generate_implementation (const IdDictMgr& idd, const std::string& tag = "");   
**/
  IdDictField f("");
  //the bool is "has value"
  auto lbl1_p = std::make_unique<IdDictLabel> ("label1", 2);
  IdDictLabel* lbl1 = lbl1_p.get();
  BOOST_CHECK_NO_THROW(f.add_label(std::move(lbl1_p)));
  //
  f.add_label(std::make_unique<IdDictLabel>("label2"));
  //special treatment for names which are numbers
  f.add_label(std::make_unique<IdDictLabel>("+1000", 10));
  //Now we've added three labels.
  BOOST_TEST(f.get_label_number() == 3);
  BOOST_TEST(f.get_label(1) == "label2");
  //with check
  BOOST_CHECK_THROW(f.get_label(10), std::out_of_range);
  BOOST_TEST(f.get_label_value("label1") == 2);
  //if the label has no value, it will increment the last found value and return that
  BOOST_TEST(f.get_label_value("label2") == 3);//??
  BOOST_TEST(f.get_label_value("1000") == 1000);//special treatment for numerical labels
  BOOST_TEST(f.get_label_value("nonsense") == 0);
  BOOST_TEST(f.find_label("label1") == lbl1);
  BOOST_TEST(f.verify() == true);
}



BOOST_AUTO_TEST_SUITE_END()

