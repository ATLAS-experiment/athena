/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_InDetSimEvent

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include <boost/test/unit_test.hpp>
#include "InDetSimEvent/SiCharge.h"

SiCharge::Process process{SiCharge::Process::no};

BOOST_AUTO_TEST_SUITE(SiChargeTests)
  BOOST_AUTO_TEST_CASE(CanBeConstructed){
    BOOST_CHECK_NO_THROW(SiCharge c(0,0,process));
  }
  BOOST_AUTO_TEST_CASE(CanBeCopyConstructed){
    SiCharge c1(0,0,process);
    //coverity [copy_constructor_call]
    BOOST_CHECK_NO_THROW(SiCharge c(c1));
  }
  BOOST_AUTO_TEST_CASE(CanBeMoveConstructed){
    SiCharge c1(0,0,process);
    BOOST_CHECK_NO_THROW(SiCharge c(std::move(c1)));
  }
  BOOST_AUTO_TEST_CASE(CanBeAssigned){
    SiCharge c1(0,0,process);
    //coverity [copy_constructor_call]
    BOOST_CHECK_NO_THROW(SiCharge c = c1);
  }
  BOOST_AUTO_TEST_CASE(CanBeMoveAssigned){
    SiCharge c1(0,0,process);
    BOOST_CHECK_NO_THROW(SiCharge c = std::move(c1));
  }
  BOOST_AUTO_TEST_CASE(HasExpectedProperties){
    SiCharge c1(1.0,2.0,process);
    BOOST_CHECK_EQUAL(c1.charge(), 1.0);
    BOOST_CHECK_EQUAL(c1.time(), 2.0);
    BOOST_CHECK_EQUAL(c1.processType(), process);
    HepMcParticleLink emptyLink;
    BOOST_CHECK_EQUAL(c1.particleLink(), emptyLink);

  }
BOOST_AUTO_TEST_SUITE_END()