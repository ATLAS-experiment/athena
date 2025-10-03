/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_InDetSimEvent

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include <boost/test/unit_test.hpp>
#include "InDetSimEvent/SiTotalCharge.h"

BOOST_AUTO_TEST_SUITE(SiTotalChargeTests)
  BOOST_AUTO_TEST_CASE(CanBeDefaultConstructed){
    BOOST_CHECK_NO_THROW(SiTotalCharge t);
  }
  BOOST_AUTO_TEST_CASE(CanBeCopyConstructed){
    SiTotalCharge t1;
    BOOST_CHECK_NO_THROW(SiTotalCharge t(t1));
  }
  BOOST_AUTO_TEST_CASE(CanBeAssigned){
    SiTotalCharge t1;
    BOOST_CHECK_NO_THROW(SiTotalCharge t = t1);
  }
BOOST_AUTO_TEST_SUITE_END()