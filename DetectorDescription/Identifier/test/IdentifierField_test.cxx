// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_Identifier
#include <boost/test/unit_test.hpp>

namespace utf = boost::unit_test;

#include "Identifier/IdentifierField.h"
#include "Identifier/ExpandedIdentifier.h"
#include <sstream>


BOOST_AUTO_TEST_SUITE(IdentifierFieldTest)


//IdentifierField is a publicly accessible class defined in the Range class
//Range holds a vector of these as a private data member
BOOST_AUTO_TEST_CASE(IdentifierFieldConstructors){
  BOOST_CHECK_NO_THROW(IdentifierField());
  IdentifierField f1;
  BOOST_CHECK_NO_THROW(IdentifierField f2(f1));
  BOOST_CHECK_NO_THROW(IdentifierField f3(std::move(f1)));
  IdentifierField::element_type e1{};
  BOOST_CHECK_NO_THROW(IdentifierField f4(e1));
  IdentifierField::element_type e2(10);
  BOOST_CHECK_NO_THROW(IdentifierField f5(e1, e2));
  IdentifierField::element_vector e3{1,2,3};
  BOOST_CHECK_NO_THROW(IdentifierField f6(e3));
}

BOOST_AUTO_TEST_CASE(DefaultIdentifierFieldProperties){
  IdentifierField f1;//default constructed...
  BOOST_CHECK(not f1.wrap_around());
  BOOST_CHECK(f1.empty());
  BOOST_CHECK(f1.get_minimum() == 0); //is this sensible? optional for this case would have been better?
  BOOST_CHECK(f1.get_maximum() == 0);
  const auto & elementVec = f1.get_values();//should be empty
  BOOST_CHECK(elementVec.empty());
  //These won't compile because second arguments are non-const (in/out) references
  //BOOST_CHECK(f1.get_previous(0,1));
  //BOOST_CHECK(f1.get_next(0,1));
  IdentifierField::element_type e1{};
  IdentifierField::element_type e2(10);
  //these are badly named
  BOOST_CHECK(f1.get_previous(e1,e2));
  BOOST_CHECK(e2 == -1); //0 --
  BOOST_CHECK(e1 == 0);
  BOOST_CHECK(f1.get_next(e1,e2));
  BOOST_CHECK(e2 == 1); //0 ++
  BOOST_CHECK(e1 == 0);
  //
  BOOST_CHECK(f1.get_indices() == 0);
  //name not only grammatically wrong, but confusing
  BOOST_CHECK(f1.get_indexes().empty());
  //how many bits needed to express get_indices result?
  BOOST_TEST(f1.get_bits() == 64);
  //these will cause problems later
  //they should not return possibly valid entries when the object is empty
  BOOST_TEST(f1.get_value_at(0) == 0);
  BOOST_TEST(f1.get_value_at(1) == 0);
  BOOST_TEST(f1.get_value_index(0) == 0);
  BOOST_TEST(f1.get_value_index(1) == 0);
  //
  IdentifierField f2;//default constructed...
  BOOST_TEST(f1.overlaps_with(f2));
}

BOOST_AUTO_TEST_CASE(UniqueIdentifierFieldProperties){
  const IdentifierField::element_type e =565;//randomly chosen unique value
  IdentifierField f1(e);
  BOOST_CHECK(not f1.wrap_around());
  BOOST_TEST(not f1.empty());
  BOOST_TEST(f1.get_minimum() == e); //is this sensible? optional for this case would have been better?
  BOOST_TEST(f1.get_maximum() == e);
  BOOST_CHECK_NO_THROW([[maybe_unused]] auto & elementVec = f1.get_values());
  //BOOST_CHECK(elementVec.empty());
  //
  IdentifierField::element_type e1{};
  IdentifierField::element_type e2(10);
  //these are badly named
  BOOST_CHECK(f1.get_previous(e1,e2));
  BOOST_CHECK(e2 == -1); //0 --
  BOOST_CHECK(e1 == 0);
  BOOST_CHECK(f1.get_next(e1,e2));
  BOOST_CHECK(e2 == 1); //0 ++
  BOOST_CHECK(e1 == 0);
  //
  BOOST_CHECK(f1.get_indices() == 0);
  //name not only grammatically wrong, but confusing
  BOOST_CHECK(f1.get_indexes().empty());
  //how many bits needed to express get_indices result?
  BOOST_TEST(f1.get_bits() == 64);
  //these will cause problems later
  //they should not return possibly valid entries when the object is empty?
  BOOST_TEST(f1.get_value_at(0) == e);
  BOOST_CHECK_THROW(f1.get_value_at(1), std::out_of_range);
  //wrapped unsigned integer
  const std::size_t oddVal = static_cast<std::size_t>(0 - e);
  BOOST_TEST(f1.get_value_index(0) == oddVal); //-565
  BOOST_TEST(f1.get_value_index(1) == oddVal + 1); //-564
  //
  IdentifierField f2(0);
  BOOST_TEST(not f1.overlaps_with(f2));//doesnt overlap with another unique value
  IdentifierField f3;
  BOOST_TEST(f1.overlaps_with(f3));//overlaps with an unbounded value
}

BOOST_AUTO_TEST_CASE(BoundedIdentifierFieldProperties){
  const IdentifierField::element_type e1 =-3;//randomly chosen unique value
  const IdentifierField::element_type e2 =5;//randomly chosen unique value
  IdentifierField f1(e1,e2);
  BOOST_CHECK(not f1.wrap_around());
  BOOST_TEST(f1.isBounded());
  BOOST_TEST(f1.get_minimum() == e1); 
  BOOST_TEST(f1.get_maximum() == e2);
  BOOST_CHECK_NO_THROW([[maybe_unused]] auto & elementVec = f1.get_values());
  BOOST_TEST(f1.get_indices() == 9);
  //
  IdentifierField::element_type e{};
  IdentifierField::element_type eReturn(10);
  //at zero, the adjacent entries are -1
  BOOST_CHECK(f1.get_previous(e,eReturn));
  BOOST_TEST(e == 0); 
  BOOST_TEST(eReturn == -1);
  //...  and 1
  BOOST_TEST(f1.get_next(e,eReturn));
  BOOST_TEST(eReturn == 1);
  //at 5, the next entry doesnt exist 
  e=5;
  BOOST_TEST(not f1.get_next(e,eReturn)); //false returned
  BOOST_TEST(e == 5); //always unchanged
  BOOST_TEST(eReturn == 5); //we are stuck at max value
  //
  BOOST_TEST(f1.get_indices() == 9);
  //name not only grammatically wrong, but confusing
  BOOST_CHECK(f1.get_indexes().empty());
  //how many bits needed to express get_indices result?
  BOOST_TEST(f1.get_bits() == 4);
  BOOST_TEST(f1.get_value_at(0) == e1);
  BOOST_CHECK_THROW(f1.get_value_at(10), std::out_of_range);
  //wrapped unsigned integer
  const std::size_t oddVal = static_cast<std::size_t>(0 - e1);
  BOOST_TEST(f1.get_value_index(0) == oddVal); 
  BOOST_TEST(f1.get_value_index(1) == oddVal + 1);
  //
  IdentifierField f2(10);
  BOOST_TEST(not f1.overlaps_with(f2));//doesnt overlap with another unique value
  IdentifierField f3(1);
  BOOST_TEST(f1.overlaps_with(f3));//overlap with another unique value in range
  IdentifierField f4;
  BOOST_TEST(f1.overlaps_with(f4));//overlaps with an unbounded value
}



BOOST_AUTO_TEST_CASE(SetAndTestRangeFieldProperties){
   //
   IdentifierField f1(9,20);
   //lets check...
   BOOST_TEST(f1.isBounded());
   BOOST_TEST(f1.overlaps_with(10));
   //but 100 doesnt overlap now
   BOOST_TEST(not f1.overlaps_with(100));
   //add an individual value at -1
   BOOST_CHECK_NO_THROW(f1.add_value(-1));
   //what mode is it now? its *enumerated* 
   BOOST_TEST(f1.isEnumerated());
   //The "add_value method" cleared everything and changed the mode. 
   // Its not an additive procedure
   //does an overlap still pass for this type?
   //BOOST_TEST(f1.overlaps_with(10)); //no, it doesnt
   BOOST_TEST(not f1.overlaps_with(100));//expected
   BOOST_TEST(f1.get_values() == std::vector<IdentifierField::element_type>(1,-1), boost::test_tools::per_element());
   BOOST_TEST(f1.overlaps_with(-1));//only this is active now
   //The adding an enumerated value 'clears' first, like this:
   BOOST_CHECK_NO_THROW(f1.clear());
   //and then it is back to virgin state
   IdentifierField f2;
   BOOST_TEST((f1==f2));//also test equality
}

BOOST_AUTO_TEST_CASE(EnumeratedIdentiferFieldProperties){
  //these are like SCT barrel eta values
  const std::vector<IdentifierField::element_type> ev{-6, -5, -4, -3, -2, -1 , 1, 2, 3, 4, 5, 6};
  IdentifierField f1(ev);
  BOOST_TEST(f1.get_values() == ev,boost::test_tools::per_element());
  BOOST_TEST(f1.isEnumerated());
  BOOST_CHECK(not f1.wrap_around());
  BOOST_TEST(f1.get_minimum() == -6);
  BOOST_TEST(f1.get_maximum() == 6);
  BOOST_TEST(f1.get_indices() == 12);
  BOOST_TEST(f1.check_for_both_bounded() == false);
  //
  IdentifierField::element_type e1{-1};
  IdentifierField::element_type e2{};
  BOOST_CHECK(f1.get_previous(e1,e2));
  BOOST_CHECK(e2 == -2);
  BOOST_CHECK(f1.get_next(e1,e2));
  BOOST_CHECK(e2 == 1);
  BOOST_CHECK(not f1.get_next(6,e2));//there is no 'next'
  BOOST_CHECK(e2 == 6);
  BOOST_CHECK(not f1.get_previous(-6,e2));//there is no 'previous'
  BOOST_CHECK(f1.wrap_around() == false);
  //unless we wrap around
  BOOST_CHECK_NO_THROW(f1.set(true));//sets wrap_around
  BOOST_CHECK(f1.wrap_around() == true);//confirm
  BOOST_CHECK(f1.get_next(6,e2));//after 6 we wrap to -6
  BOOST_CHECK(e2 == -6);
  BOOST_CHECK(f1.get_previous(-6,e2));//before -6 we wrap to 6
  BOOST_CHECK(e2 == 6);
}

BOOST_AUTO_TEST_CASE(NextPreviousBounded){
  IdentifierField f1 (0, 10);
  IdentifierField f2 (0, 10);
  f2.set_next (7);
  IdentifierField f3 (0, 10);
  f3.set_previous (3);
  IdentifierField f4 (0, 10);
  f4.set_next (7);
  f4.set_previous (3);
  IdentifierField f5 (0, 10);
  f5.set (true);

  // Nothing set -- next/previous from the ends fail.
  IdentifierField::element_type e = -1;
  BOOST_CHECK (!f1.get_next (10, e));
  BOOST_CHECK (!f1.get_previous (0, e));

  // Next set.
  BOOST_CHECK (f2.get_next (10, e));
  BOOST_CHECK (e == 7);
  BOOST_CHECK (!f2.get_previous (0, e));

  // Previous set.
  BOOST_CHECK (!f3.get_next (10, e));
  BOOST_CHECK (f3.get_previous (0, e));
  BOOST_CHECK (e == 3);

  // Both set.
  BOOST_CHECK (f4.get_next (10, e));
  BOOST_CHECK (e == 7);
  BOOST_CHECK (f4.get_previous (0, e));
  BOOST_CHECK (e == 3);

  // Wraparound.
  BOOST_CHECK (f5.get_next (10, e));
  BOOST_CHECK (e == 0);
  BOOST_CHECK (f5.get_previous (0, e));
  BOOST_CHECK (e == 10);
}

BOOST_AUTO_TEST_CASE(NextPreviousEnum){
  IdentifierField::element_vector elts {0, 3, 5, 7, 10};
  IdentifierField f1 (elts);
  IdentifierField f2 (elts);
  f2.set_next (7);
  IdentifierField f3 (elts);
  f3.set_previous (3);
  IdentifierField f4 (elts);
  f4.set_next (7);
  f4.set_previous (3);
  IdentifierField f5 (elts);
  f5.set (true);

  // Nothing set -- next/previous from the ends fail.
  IdentifierField::element_type e = -1;
  BOOST_CHECK (!f1.get_next (10, e));
  BOOST_CHECK (!f1.get_previous (0, e));

  // Next set.
  BOOST_CHECK (f2.get_next (10, e));
  BOOST_CHECK (e == 7);
  BOOST_CHECK (!f2.get_previous (0, e));

  // Previous set.
  BOOST_CHECK (!f3.get_next (10, e));
  BOOST_CHECK (f3.get_previous (0, e));
  BOOST_CHECK (e == 3);

  // Both set.
  BOOST_CHECK (f4.get_next (10, e));
  BOOST_CHECK (e == 7);
  BOOST_CHECK (f4.get_previous (0, e));
  BOOST_CHECK (e == 3);

  // Wraparound.
  BOOST_CHECK (f5.get_next (10, e));
  BOOST_CHECK (e == 0);
  BOOST_CHECK (f5.get_previous (0, e));
  BOOST_CHECK (e == 10);
}

BOOST_AUTO_TEST_CASE(IdentifierFieldOperators){
  //start with a virgin field
  IdentifierField f1;
  IdentifierField f2;
  IdentifierField f3;
  const std::vector<IdentifierField::element_type> ev{0,1,2,3,4,5};
  f1.set(ev);
  f1.set(true); //will wrap around now
  //assignment
  BOOST_CHECK_NO_THROW(f2 = f1);
  //underlying enumeration should be the same now
  BOOST_CHECK(f2.get_values() == ev);
  //equality operator
  BOOST_CHECK(f2 == f1);
  //...and inequality
  BOOST_CHECK(f2 != f3);
  //conversion to string, with conversion operator
  BOOST_TEST(std::string(f2) == std::string("0,1,2,3,4,5"));
  //using stream insertion
  std::ostringstream os;
  os<<f2;
  BOOST_TEST(os.str() == std::string("0,1,2,3,4,5"));
}
BOOST_AUTO_TEST_CASE(IdentifierFieldOrOperators, * utf::expected_failures(2)){
  //Field 'or' is the method for combining range fields, giving a superset of valid indices
  //start with an unbounded virgin field
  //UNBOUNDED
  const std::vector<IdentifierField::element_type> ev{0,1,2,3,4,5};
   IdentifierField  f1(ev); //f1 is now enumerated (check...)
  const auto enum1 = f1;//use this later
  BOOST_CHECK(f1.isEnumerated());
  const IdentifierField f2;//unbounded
  BOOST_CHECK_NO_THROW(f1 |= f2); //or them into f1
  BOOST_CHECK(f1.empty());
  //BOTH bounded
  //add overlapping bounded
  IdentifierField both1(0,6);//both bounded
  BOOST_TEST(both1.isBounded());
  IdentifierField both2(3,8);//both bounded
  BOOST_CHECK_NO_THROW(both1 |= both2);
  BOOST_TEST(both1.isBounded());
  BOOST_TEST(both1.get_minimum() == 0);
  BOOST_TEST(both1.get_maximum() == 8);
  //add a disjoint bounded region
  IdentifierField both3(11,20);//both bounded
  BOOST_CHECK_NO_THROW(both1 |= both2);
  BOOST_TEST(both1.get_minimum() == 0);
  //the following would fail (as per comment in the code)
  BOOST_TEST(both1.get_maximum() == 20);
  BOOST_TEST(both1.isBounded());
  //check mode
  const std::vector<IdentifierField::element_type> ev2{8,9,10,11};
  IdentifierField enum2(ev2);
  int e{};
  //ENUMERATED
  IdentifierField enumField(ev);
  //...disjoint enumerated region
  BOOST_CHECK_NO_THROW(enumField |= enum2); //{8,9,10,11}, enumerated with enumerated
  BOOST_TEST(enumField.get_values().size() == 10); //disjoint; add entries
  BOOST_TEST(enumField.isEnumerated());
  BOOST_TEST(enumField.get_maximum() == 11);
  BOOST_TEST(enumField.get_minimum() == 0);
  BOOST_TEST(enumField.get_next(5,e));
  BOOST_TEST(e == 8);
  //overlapping enumerated region
  enumField.clear();
  enumField.set(ev);//{0,1,2,3,4,5}
  IdentifierField enum3;
  const std::vector<IdentifierField::element_type> ev3{5,6,7,8};
  enum3.set(ev3);
  BOOST_CHECK_NO_THROW(enumField |= enum3);//{0,1,2,3,4,5,6,7,8}; it removes duplicates
  BOOST_TEST(enumField.get_values().size() == 9);
  BOOST_TEST(enumField.isEnumerated());
 
  enumField.clear();
 
  //with a both_bounded field encompassing the enumerated values
  enumField.clear();
  enumField.set(ev);//{0,1,2,3,4,5}
  IdentifierField bounded(-1,10);
  BOOST_CHECK_NO_THROW(enumField |= bounded);
  BOOST_TEST(enumField.isBounded());
  BOOST_TEST(enumField.get_maximum() == 10);
  BOOST_TEST(enumField.get_minimum() == -1);
  //with a disjoint both_bounded field
  enumField.clear();
  enumField.set(ev);//{0,1,2,3,4,5}
  bounded.clear();
  bounded.set(10, 20);
  BOOST_CHECK_NO_THROW(enumField |= bounded);
  BOOST_TEST(enumField.isBounded());
  BOOST_TEST(enumField.get_maximum() == 20);
  BOOST_TEST(enumField.get_minimum() == 0);
  
}
BOOST_AUTO_TEST_CASE(RangeFieldStreamExtraction){
  IdentifierField f;
  std::istringstream asterisk("  *");
  BOOST_CHECK_NO_THROW(asterisk>>f);
  BOOST_TEST(std::string(f) == "*");
  f.clear();
  std::istringstream bound("  -10:100 ");
  BOOST_CHECK_NO_THROW(bound>>f);
  BOOST_TEST(std::string(f) == "-10:100");
  f.clear();
  std::istringstream singleValue("  -5 ");
  BOOST_CHECK_NO_THROW(singleValue>>f);
  BOOST_TEST(std::string(f) == "-5");
  f.clear();
  std::istringstream list("  -1, 2, 3,5 ");
  BOOST_CHECK_NO_THROW(list>>f);
  BOOST_TEST(std::string(f) == "-1,2,3,5");
  f.clear();
  std::istringstream list2("-1,2,3,5");
  BOOST_CHECK_NO_THROW(list2>>f);
  BOOST_TEST(std::string(f) == "-1,2,3,5");
  f.clear();
  std::istringstream nonsense("ghghghgh");
  BOOST_CHECK_THROW(nonsense>>f, std::invalid_argument);
  f.clear();
  std::istringstream fullString("4/1/-1,1/3/0/0:19/0:255");
  BOOST_CHECK_NO_THROW(fullString>>f);//should parse the first field
  BOOST_TEST(std::string(f) == "4"); //..and then the string has '4' popped
  //BOOST_TEST(fullString.str() == "/1/-1,1/3/0/0:19/0:255");
  
}

BOOST_AUTO_TEST_SUITE_END()

