// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_IdDict
#include <boost/test/unit_test.hpp>
namespace utf = boost::unit_test;
#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;


#include "src/get_bits.h"

BOOST_AUTO_TEST_SUITE(get_bits_functions)
BOOST_AUTO_TEST_CASE(fn_compute_bits){
 BOOST_TEST(true);
  
}

BOOST_AUTO_TEST_CASE(fn_get_bits){
 BOOST_TEST(true);
}

BOOST_AUTO_TEST_SUITE_END()
