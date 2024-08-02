// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_Identifier
#include <boost/test/unit_test.hpp>

namespace utf = boost::unit_test;
#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include "Identifier/Range.h"
#include "Identifier/ExpandedIdentifier.h"


BOOST_AUTO_TEST_SUITE(RangeTest)
BOOST_AUTO_TEST_CASE(RangeConstructors){
  BOOST_CHECK_NO_THROW(Range());
  Range r1;
  BOOST_CHECK_NO_THROW(Range r2(r1));
  BOOST_CHECK_NO_THROW(Range r3(std::move(r1)));
  ExpandedIdentifier e;
  BOOST_CHECK_NO_THROW(Range r4(e));
}

//Range::field is a publicly accessible class defined in the Range class
//Range holds a vector of these as a private data member
BOOST_AUTO_TEST_CASE(RangeFieldConstructors){
  BOOST_CHECK_NO_THROW(Range::field());
  Range::field f1;
  BOOST_CHECK_NO_THROW(Range::field f2(f1));
  BOOST_CHECK_NO_THROW(Range::field f3(std::move(f1)));
  Range::element_type e1{};
  BOOST_CHECK_NO_THROW(Range::field f4(e1));
  Range::element_type e2(10);
  BOOST_CHECK_NO_THROW(Range::field f5(e1, e2));
}

BOOST_AUTO_TEST_CASE(DefaultRangeFieldProperties){
  Range::field f1;//default constructed...
  BOOST_CHECK(not f1.has_minimum());
  BOOST_CHECK(not f1.has_maximum());
  BOOST_CHECK(not f1.wrap_around());
  BOOST_CHECK(f1.get_mode() == Range::field::unbounded);
  BOOST_CHECK(f1.get_minimum() == 0); //is this sensible? optional for this case would have been better?
  BOOST_CHECK(f1.get_maximum() == 0);
  const auto & elementVec = f1.get_values();//should be empty
  BOOST_CHECK(elementVec.empty());
  //These won't compile because second arguments are non-const (in/out) references
  //BOOST_CHECK(f1.get_previous(0,1));
  //BOOST_CHECK(f1.get_next(0,1));
  Range::element_type e1{};
  Range::element_type e2(10);
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
  //this doesn't seem correct, and is due to unsigned integer wrap
  BOOST_TEST(f1.get_bits() == 64);
  //these will cause problems later
  //they should not return possibly valid entries when the object is empty
  BOOST_TEST(f1.get_value_at(0) == 0);
  BOOST_TEST(f1.get_value_at(1) == 0);
  BOOST_TEST(f1.get_value_index(0) == 0);
  BOOST_TEST(f1.get_value_index(1) == 0);
  //
  BOOST_TEST(f1.match_any());
  Range::field f2;//default constructed...
  BOOST_TEST(f1.overlaps_with(f2));
  BOOST_TEST(f1.match(3));
}

BOOST_AUTO_TEST_CASE(UniqueRangeFieldProperties){
  const Range::element_type e =565;//randomly chosen unique value
  Range::field f1(e);
  BOOST_CHECK(f1.has_minimum());
  BOOST_CHECK(f1.has_maximum());
  BOOST_CHECK(not f1.wrap_around());
  BOOST_TEST(f1.get_mode() == Range::field::both_bounded);
  BOOST_TEST(f1.get_minimum() == e); //is this sensible? optional for this case would have been better?
  BOOST_TEST(f1.get_maximum() == e);
  const auto & elementVec = f1.get_values();//should be empty
  BOOST_CHECK(elementVec.empty());
  //
  Range::element_type e1{};
  Range::element_type e2(10);
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
  BOOST_TEST(not f1.match_any());
  Range::field f2(0);
  BOOST_TEST(not f1.overlaps_with(f2));//doesnt overlap with another unique value
  Range::field f3;
  BOOST_TEST(f1.overlaps_with(f3));//overlaps with an unbounded value
}

BOOST_AUTO_TEST_CASE(BoundedRangeFieldProperties){
  const Range::element_type e1 =-3;//randomly chosen unique value
  const Range::element_type e2 =5;//randomly chosen unique value
  Range::field f1(e1,e2);
  BOOST_CHECK(f1.has_minimum());
  BOOST_CHECK(f1.has_maximum());
  BOOST_CHECK(not f1.wrap_around());
  BOOST_TEST(f1.get_mode() == Range::field::both_bounded);
  BOOST_TEST(f1.get_minimum() == e1); 
  BOOST_TEST(f1.get_maximum() == e2);
  const auto & elementVec = f1.get_values();//should be empty
  BOOST_CHECK(elementVec.empty());
  //
  Range::element_type e{};
  Range::element_type eReturn(10);
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
  BOOST_TEST(not f1.match_any());
  Range::field f2(10);
  BOOST_TEST(not f1.overlaps_with(f2));//doesnt overlap with another unique value
  Range::field f3(1);
  BOOST_TEST(f1.overlaps_with(f3));//overlap with another unique value in range
  Range::field f4;
  BOOST_TEST(f1.overlaps_with(f4));//overlaps with an unbounded value
}

BOOST_AUTO_TEST_CASE(SetAndTestRangeFieldProperties){
  //start with a virgin field
   Range::field f1;
   BOOST_CHECK_NO_THROW(f1.set_minimum(9));//now it's lower bounded at 9?
   //lets check...
   BOOST_TEST(f1.get_mode() == Range::field::low_bounded);
   //can't overlap with 0
   BOOST_TEST(not f1.overlaps_with(0));
   //but 100 should be ok
   BOOST_TEST(f1.overlaps_with(100));
   //now add an upper bound 
   BOOST_CHECK_NO_THROW(f1.set_maximum(20));//now it's also upper bounded at 20?
   //lets check...
   BOOST_TEST(f1.get_mode() == Range::field::both_bounded);
   //but 100 doesnt overlap now
   BOOST_TEST(not f1.overlaps_with(100));
   //add an individual value at -1
   BOOST_CHECK_NO_THROW(f1.add_value(-1));
   //what mode is it now? its *enumerated* 
   BOOST_TEST(f1.get_mode() == Range::field::enumerated);
   //The "add_value method" cleared everything and changed the mode. 
   // Its not an additive procedure
   //does an overlap still pass for this type?
   //BOOST_TEST(f1.overlaps_with(10)); //no, it doesnt
   BOOST_TEST(not f1.overlaps_with(100));//expected
   BOOST_TEST(f1.overlaps_with(-1));//only this is active now
   //The adding an enumerated value 'clears' first, like this:
   BOOST_CHECK_NO_THROW(f1.clear());
   //and then it is back to virgin state
   Range::field f2;
   BOOST_TEST((f1==f2));//also test equality
}

BOOST_AUTO_TEST_CASE(EnumeratedRangeFieldProperties){
  //start with a virgin field
  Range::field f1;
  //these are like SCT barrel eta values
  const std::vector<Range::element_type> ev{-6, -5, -4, -3, -2, -1 , 1, 2, 3, 4, 5, 6};
  BOOST_CHECK_NO_THROW(f1.set(ev));
  BOOST_TEST(f1.get_values() == ev);
  BOOST_TEST(f1.get_mode() == Range::field::enumerated);
  BOOST_CHECK(f1.has_minimum());
  BOOST_CHECK(f1.has_maximum());
  BOOST_CHECK(not f1.wrap_around());
  BOOST_TEST(f1.get_minimum() == -6);
  BOOST_TEST(f1.get_maximum() == 6);
  //
  Range::element_type e1{-1};
  Range::element_type e2{};
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

BOOST_AUTO_TEST_CASE(RangeFieldOperators){
  //start with a virgin field
  Range::field f1;
  Range::field f2;
  Range::field f3;
  const std::vector<Range::element_type> ev{0,1,2,3,4,5};
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
}
BOOST_AUTO_TEST_CASE(RangeFieldOrOperators, * utf::expected_failures(9)){
  //Field 'or' is the method for combining range fields, giving a superset of valid indices
  //start with an unbounded virgin field
  //UNBOUNDED
  Range::field f1;
  const std::vector<Range::element_type> ev{0,1,2,3,4,5};
  f1.set(ev); //f1 is now enumerated (check...)
  const auto enum1 = f1;//use this later
  BOOST_CHECK(f1.get_mode() == Range::field::enumerated);
  const Range::field f2;//unbounded
  BOOST_CHECK_NO_THROW(f1 |= f2); //or them into f1
  BOOST_CHECK(f1.get_mode() == Range::field::unbounded);
  //BOTH bounded
  //add overlapping bounded
  Range::field both1(0,6);//both bounded
  BOOST_TEST(both1.get_mode() == Range::field::both_bounded);
  Range::field both2(3,8);//both bounded
  BOOST_CHECK_NO_THROW(both1 |= both2);
  BOOST_TEST(both1.get_mode() == Range::field::both_bounded);
  BOOST_TEST(both1.get_minimum() == 0);
  BOOST_TEST(both1.get_maximum() == 8);
  //add a disjoint bounded region
  Range::field both3(11,20);//both bounded
  BOOST_CHECK_NO_THROW(both1 |= both2);
  BOOST_TEST(both1.get_minimum() == 0);
  //the following would fail (as per comment in the code)
  BOOST_TEST(both1.get_maximum() == 20);
  BOOST_TEST(both1.get_mode() == Range::field::both_bounded);
  //HIGH bounded
  Range::field hi1;
  hi1.set_maximum(10);
  //check mode
  BOOST_TEST(hi1.get_mode() == Range::field::high_bounded);
  Range::field hi2;
  hi2.set_maximum(12);
  BOOST_CHECK_NO_THROW(hi1 |= hi2);
  BOOST_TEST(hi1.get_mode() == Range::field::high_bounded);
  BOOST_TEST(hi1.get_maximum() == 12);
  //'or' a high bounded with a both bounded
  BOOST_CHECK_NO_THROW(hi1 |= both3);
  BOOST_TEST(hi1.get_mode() == Range::field::high_bounded);
  BOOST_TEST(hi1.get_maximum() == 20);
  //'or' a high bounded with a low bounded
  Range::field lo1;
  lo1.set_minimum(3);
  BOOST_CHECK_NO_THROW(hi1 |= lo1);
  BOOST_TEST(hi1.get_mode() == Range::field::unbounded);
  //'or' a high bounded with an enumerated
  hi1.set_maximum(2);
  BOOST_TEST(hi1.get_maximum() == 2);
  BOOST_CHECK_NO_THROW(hi1 |= enum1);//{0,1,2,3,4,5}
  BOOST_TEST(hi1.get_mode() == Range::field::high_bounded);
  BOOST_TEST(hi1.get_maximum() == 5);
  //'or' a high bounded with disjoint enumerated
  const std::vector<Range::element_type> ev2{8,9,10,11};
  Range::field enum2;
  enum2.set(ev2);
  BOOST_CHECK_NO_THROW(hi1 |= enum2);
  BOOST_TEST(hi1.get_maximum() == 11);
  Range::element_type e{};
  BOOST_TEST(hi1.get_next(5,e));
  //fails, it treats the regions as one contiguous field, giving 6 as next to 5
  BOOST_TEST(e == 8);
  //ENUMERATED
  Range::field enumField;
  enumField.set(ev);//{0,1,2,3,4,5}
  //...disjoint enumerated region
  BOOST_CHECK_NO_THROW(enumField |= enum2); //{8,9,10,11}, enumerated with enumerated
  BOOST_TEST(enumField.get_values().size() == 10); //disjoint; add entries
  BOOST_TEST(enumField.get_mode() == Range::field::enumerated);
  BOOST_TEST(enumField.get_maximum() == 11);
  BOOST_TEST(enumField.get_minimum() == 0);
  BOOST_TEST(enumField.get_next(5,e));
  BOOST_TEST(e == 8);
  //overlapping enumerated region
  enumField.clear();
  enumField.set(ev);//{0,1,2,3,4,5}
  Range::field enum3;
  const std::vector<Range::element_type> ev3{5,6,7,8};
  enum3.set(ev3);
  BOOST_CHECK_NO_THROW(enumField |= enum3);//{0,1,2,3,4,5,6,7,8}; it removes duplicates
  BOOST_TEST(enumField.get_values().size() == 9);
  BOOST_TEST(enumField.get_mode() == Range::field::enumerated);
  BOOST_TEST(hi1.get_next(5,e));
  BOOST_TEST(e == 6);
  enumField.clear();
  //..with a high bound field which encompasses all values
  Range::field hi;
  enumField.clear();
  enumField.set(ev);
  hi.set_maximum(20);
  BOOST_CHECK_NO_THROW(enumField |= hi);
  //whats its type?
  BOOST_TEST(enumField.get_mode() == Range::field::enumerated);//unexpectedly, it is a both_bounded
  //whats the possible value?
  BOOST_TEST(enumField.get_maximum() == 20);
  BOOST_TEST(enumField.has_minimum() == false);//failing
  BOOST_TEST(enumField.get_minimum() == 0);//it doesnt have a minimum
  //..with a high bound field which is disjoint
  enumField.clear();
  hi.clear();
  enumField.set(ev);//{0,1,2,3,4,5}
  hi.set_maximum(-2);
  BOOST_CHECK_NO_THROW(enumField |= hi);
  //whats its type?
  BOOST_TEST(enumField.get_mode() == Range::field::enumerated);//unexpectedly, it is a both_bounded
  BOOST_TEST(enumField.get_maximum() == 5);
  BOOST_TEST(enumField.get_minimum() == 0);
  //with a lo bound field encompassing all values
  enumField.clear();
  Range::field lo;
  enumField.set(ev);//{0,1,2,3,4,5}
  lo.set_minimum(-1);
  BOOST_CHECK_NO_THROW(enumField |= lo);
  //whats its type?
  BOOST_TEST(enumField.get_mode() == Range::field::enumerated);//unexpectedly, it is a both_bounded
  BOOST_TEST(enumField.has_maximum() == false);//failing
  BOOST_TEST(enumField.get_maximum() == 0);//fails, gives 5
  BOOST_TEST(enumField.get_minimum() == -1);
  //with a disjoint lo bound field
  enumField.clear();
  lo.clear();
  enumField.set(ev);//{0,1,2,3,4,5}
  lo.set_minimum(8);
  BOOST_CHECK_NO_THROW(enumField |= lo);
  //whats its type?
  BOOST_TEST(enumField.get_mode() == Range::field::enumerated);//unexpectedly, it is a both_bounded
  BOOST_TEST(enumField.has_maximum() == false);//failing
  BOOST_TEST(enumField.get_maximum() == 0);//fails, gives 5
  BOOST_TEST(enumField.get_minimum() == 0);
  //with a both_bounded field encompassing the enumerated values
  enumField.clear();
  enumField.set(ev);//{0,1,2,3,4,5}
  Range::field bounded(-1,10);
  BOOST_CHECK_NO_THROW(enumField |= bounded);
  BOOST_TEST(enumField.get_mode() == Range::field::both_bounded);
  BOOST_TEST(enumField.has_maximum() == true);
  BOOST_TEST(enumField.get_maximum() == 10);
  BOOST_TEST(enumField.get_minimum() == -1);
  //with a disjoint both_bounded field
  enumField.clear();
  enumField.set(ev);//{0,1,2,3,4,5}
  bounded.clear();
  bounded.set_minimum(10);
  bounded.set_maximum(20);
  BOOST_CHECK_NO_THROW(enumField |= bounded);
  BOOST_TEST(enumField.get_mode() == Range::field::both_bounded);
  BOOST_TEST(enumField.has_maximum() == true);
  BOOST_TEST(enumField.get_maximum() == 20);
  BOOST_TEST(enumField.get_minimum() == 0);
  
}

//Range::identifier_factory is a publicly accessible class defined in the Range class
BOOST_AUTO_TEST_CASE(RangeIdentifier_factoryConstructors){
  BOOST_CHECK_NO_THROW(Range::identifier_factory());
  Range::identifier_factory f1;
  BOOST_CHECK_NO_THROW(Range::identifier_factory f2(f1));
  BOOST_CHECK_NO_THROW(Range::identifier_factory f3(std::move(f1)));
  Range r1;
  BOOST_CHECK_NO_THROW(Range::identifier_factory f4(r1));
}

//const_identifier_factory
//Range::const_identifier_factory is a publicly accessible class defined in the Range class
BOOST_AUTO_TEST_CASE(RangeConst_identifier_factoryConstructors){
  BOOST_CHECK_NO_THROW(Range::const_identifier_factory());
  Range::const_identifier_factory f1;
  BOOST_CHECK_NO_THROW(Range::const_identifier_factory f2(f1));
  Range r1;
  BOOST_CHECK_NO_THROW(Range::const_identifier_factory f3(r1));
}

BOOST_AUTO_TEST_SUITE_END()

