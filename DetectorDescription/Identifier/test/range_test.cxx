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
  BOOST_TEST_MESSAGE(std::string(f2));
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

