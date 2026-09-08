/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TgcL0SegmentReconstructionTest

#include <boost/test/unit_test.hpp>

#include "TgcL0FloatingData.h"
#include "TgcL0SegmentReconstruction.h"

#include <cmath>

BOOST_AUTO_TEST_SUITE(TgcL0SegmentReconstructionTest)

BOOST_AUTO_TEST_CASE(WrapsPhysicalDeltaThetaAcrossAtan2BranchCut) {
  using namespace L0Muon::TgcL0Floating;

  const HitGroupKey key{103U, 1U, 0x2U};
  StationCoincidenceContainer coincidences;

  coincidences.emplace_back(key, Station::M1, false, 1U, -1, 1U, 10U, 0x7U,
                            3U, 3U, -1.50F, 0.20F, 1000.F, -10000.F);
  coincidences.emplace_back(key, Station::M3, false, 1U, -1, 1U, 11U, 0x7U,
                            3U, 3U, -1.52F, 0.20F, 900.F, -11000.F);
  coincidences.emplace_back(key, Station::M1, true, 1U, -1, 1U, 20U, 0x7U,
                            3U, 3U, -1.50F, 0.20F, 1000.F, -10000.F);
  coincidences.emplace_back(key, Station::M3, true, 1U, -1, 1U, 21U, 0x7U,
                            3U, 3U, -1.52F, 0.20F, 900.F, -11000.F);

  L0Muon::TgcL0CandidateContainer candidates;
  L0Muon::TgcL0SegmentContainer segments;
  SegmentStatistics statistics;
  const SegmentReconstruction reconstruction;

  const StatusCode status =
      reconstruction.build(coincidences, candidates, statistics, &segments);

  BOOST_REQUIRE(status.isSuccess());
  BOOST_REQUIRE_EQUAL(candidates.size(), 1U);
  BOOST_REQUIRE_EQUAL(segments.size(), 2U);

  const float segmentTheta = std::atan2(-100.F, -1000.F);
  const float pivotTheta = std::atan2(900.F, -11000.F);
  const float rawDifference = segmentTheta - pivotTheta;

  BOOST_TEST(rawDifference < -6.F);
  BOOST_TEST(std::abs(candidates.front().deltaTheta - 0.181305F) < 2.E-6F);
  BOOST_TEST(candidates.front().deltaTheta > 0.F);
  BOOST_TEST(candidates.front().deltaTheta < 0.2F);
  BOOST_TEST(candidates.front().wireStationMask == 0x5U);
  BOOST_TEST(candidates.front().stripStationMask == 0x5U);
}

BOOST_AUTO_TEST_SUITE_END()
