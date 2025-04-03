/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_PIXELDIGITIZATION

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;


#include <boost/test/unit_test.hpp>


#include "src/PixelNoiseFunctions.h"
#include "CLHEP/Random/JamesRandom.h"
#include <cmath> //isnan
#include <utility> //pair
#include <tuple>
#include <string>
#include <ostream>
#include <memory>

std::unique_ptr<CLHEP::HepRandomEngine> rng = std::make_unique<CLHEP::HepJamesRandom>();

using namespace PixelDigitization;

BOOST_AUTO_TEST_SUITE(PixelNoiseFunctionsTest)
  BOOST_AUTO_TEST_CASE(generateToTTest){
    const double meanOk = 7.0;
    const double sdOk = 0.3;
    const double lowMean = -1.;
    const double highMean = 15.;
    const double largeSd =3.;
    using Range = std::pair<int,int>;
    const Range rangeOk{1,14};
    auto random = [ptr = rng.get()](double m,double s, const Range & r)->int{
      int i = generateToT(ptr, m,s,r);
      BOOST_TEST_MESSAGE("Generated: "+std::to_string(i));
      return i;
    };
    auto inRange = [](int v, int lo, int hi)->bool{ return (v>=lo and v<=hi);};
    //
    int sensible = random(meanOk,sdOk,rangeOk); //sensible values
    BOOST_CHECK(inRange(sensible, 1,14));
    //
    int lo = random(lowMean,sdOk,rangeOk);
    BOOST_CHECK( inRange(lo, 1, 14) );
    //
    int hi = random(highMean,sdOk,rangeOk);
    BOOST_CHECK( inRange(hi, 1, 14) );
    //
    int wild = random(meanOk,largeSd,rangeOk);
    BOOST_CHECK( inRange(wild, 1, 14) );
  }
BOOST_AUTO_TEST_SUITE_END()