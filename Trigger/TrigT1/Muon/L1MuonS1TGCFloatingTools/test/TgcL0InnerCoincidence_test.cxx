/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TgcL0InnerCoincidenceTest

#include <boost/test/unit_test.hpp>

#include "TgcL0InnerCoincidence.h"

BOOST_AUTO_TEST_SUITE(TgcL0InnerCoincidenceTest)

BOOST_AUTO_TEST_CASE(PropagatesPreInnerResultWithoutInnerInput) {
  L0Muon::TgcL0Candidate first;
  first.preInnerCoincidencePt = 35.F;
  first.preInnerCoincidenceThreshold = 8U;
  first.pt = 0.F;
  first.threshold = 0U;
  first.hasInnerCoincidence = true;
  first.goodMagneticField = true;

  L0Muon::TgcL0Candidate second;
  second.preInnerCoincidencePt = 12.5F;
  second.preInnerCoincidenceThreshold = 4U;
  second.pt = 99.F;
  second.threshold = 15U;
  second.hasInnerCoincidence = true;
  second.goodMagneticField = false;

  L0Muon::TgcL0CandidateContainer candidates{first, second};
  const L0Muon::TgcL0Floating::InnerCoincidence innerCoincidence;
  innerCoincidence.apply(candidates);

  BOOST_REQUIRE_EQUAL(candidates.size(), 2U);
  BOOST_TEST(candidates[0].pt == 35.F);
  BOOST_TEST(candidates[1].pt == 12.5F);
  BOOST_TEST(candidates[0].threshold == 8U);
  BOOST_TEST(candidates[1].threshold == 4U);
  BOOST_TEST(!candidates[0].hasInnerCoincidence);
  BOOST_TEST(!candidates[1].hasInnerCoincidence);
  BOOST_TEST(candidates[0].goodMagneticField);
  BOOST_TEST(!candidates[1].goodMagneticField);
}

BOOST_AUTO_TEST_SUITE_END()
