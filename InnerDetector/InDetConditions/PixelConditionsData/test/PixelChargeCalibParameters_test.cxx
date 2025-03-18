/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file PixelChargeCalibParameters_test.cxx
 * @author Shaun Roe
 * @date October, 2023
 * @brief basic tests in the Boost framework for PixelChargeCalibParameters
 */
 
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_PIXELCONDITIONSDATA

#include "CxxUtils/checker_macros.h"
//
#include <boost/test/unit_test.hpp>
#include <boost/test/tools/output_test_stream.hpp>
//
#include "PixelConditionsData/ChargeCalibParameters.h"

namespace utf = boost::unit_test;
using namespace PixelChargeCalib;

ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

BOOST_AUTO_TEST_SUITE(PixelChargeCalibParametersTest)
  BOOST_AUTO_TEST_CASE( Construction ){
    BOOST_CHECK_NO_THROW(LegacyFitParameters());
    BOOST_CHECK_NO_THROW(LegacyFitParameters(2.3f, 2.f, 3.f, 32.f));
    //
    BOOST_CHECK_NO_THROW(LinearFitParameters());
    BOOST_CHECK_NO_THROW(LinearFitParameters(1.f, 2.f));
    //
    BOOST_CHECK_NO_THROW(Thresholds());
    BOOST_CHECK_NO_THROW(Thresholds(2, 2, 3, 4));
    //
    BOOST_CHECK_NO_THROW(Resolutions());
    BOOST_CHECK_NO_THROW(Resolutions(1.3f, 2.f));
  }
  BOOST_AUTO_TEST_CASE( Streaming ){
    boost::test_tools::output_test_stream output;
    LegacyFitParameters leg(2.3f, 2.f, 3.f, 32.f);
    LinearFitParameters lin(1.f, 2.f);
    Thresholds t(2, 2, 3, 4);
    Resolutions r(1.3f, 2.f);
    //
    output<<leg;
    BOOST_CHECK(output.is_equal("(2.3, 2, 3, 32)"));
    //note: stream is flushed by default
    output<<lin;
    BOOST_CHECK(output.is_equal("(1, 2)"));
    //
    output<<t;
    BOOST_CHECK(output.is_equal("(2, 2, 3, 4)"));
    //
    output<<r;
    BOOST_CHECK(output.is_equal("(1.3, 2)"));
  }
  BOOST_AUTO_TEST_CASE( Equality ){
    LegacyFitParameters leg1(2.3f, 2.f, 3.f, 32.f);
    LegacyFitParameters leg2(2.3f, 2.f, 3.f, 32.f);
    LegacyFitParameters leg3(2.2f, 2.f, 3.f, 32.f);
    BOOST_CHECK(leg1 == leg2);
    BOOST_CHECK(leg1 != leg3);
    //
    LinearFitParameters lin1(1.f, 2.f);
    LinearFitParameters lin2(1.f, 2.f);
    LinearFitParameters lin3(1.2f, 2.f);
    BOOST_CHECK(lin1 == lin2);
    BOOST_CHECK(lin1 != lin3);
    //
    Thresholds t1(2, 2, 3, 4);
    Thresholds t2(2, 2, 3, 4);
    Thresholds t3(2, 2, 3, 5);
    BOOST_CHECK(t1 == t2);
    BOOST_CHECK(t1 != t3);
    //
    Resolutions r1(1.3f, 2.f);
    Resolutions r2(1.3f, 2.f);
    Resolutions r3(1.3f, 2.1f);
    BOOST_CHECK(r1 == r2);
    BOOST_CHECK(r1 != r3);
  }
BOOST_AUTO_TEST_SUITE_END();
