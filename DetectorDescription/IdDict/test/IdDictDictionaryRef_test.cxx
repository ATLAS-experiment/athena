// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_IdDict
#include <boost/test/unit_test.hpp>
namespace utf = boost::unit_test;

#include "IdDict/IdDictDictionaryRef.h"
#include "IdDict/IdDictDictionary.h"


BOOST_AUTO_TEST_SUITE(IdDictDictionaryRefTest)
BOOST_AUTO_TEST_CASE(IdDictDictionaryRefConstructors){
  BOOST_CHECK_NO_THROW(IdDictDictionaryRef(""));
  IdDictDictionaryRef i1("");
  BOOST_CHECK_NO_THROW([[maybe_unused]] IdDictDictionaryRef i2(i1));
  BOOST_CHECK_NO_THROW([[maybe_unused]] IdDictDictionaryRef i3(std::move(i1)));
}


BOOST_AUTO_TEST_CASE(IdDictDictionaryRefBuildRange){
  auto pDictionary = std::make_unique<IdDictDictionary>(); 
  IdDictDictionaryRef f (pDictionary.get());
  BOOST_TEST(f.build_range() == Range());
}

BOOST_AUTO_TEST_SUITE_END()
