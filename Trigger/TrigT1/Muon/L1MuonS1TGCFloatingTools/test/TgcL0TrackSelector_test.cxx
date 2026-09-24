/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TgcL0TrackSelectorTest

#include <boost/test/unit_test.hpp>

#include "TgcL0TrackSelector.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "AthContainers/AuxStoreInternal.h"

namespace {

constexpr std::uint16_t sideA = 0x65U;
constexpr std::uint16_t sideC = 0x66U;
constexpr std::uint16_t currentBc = 0x2U;

bool close(float left, float right, float tolerance) {
  return std::abs(left - right) <= tolerance;
}

L0Muon::TgcL0Candidate candidate(std::uint16_t subdetectorId,
                                 std::uint16_t sectorId, std::uint16_t bcTag,
                                 float pt, std::uint8_t threshold,
                                 std::uint8_t selectorPriority,
                                 std::uint32_t marker) {
  L0Muon::TgcL0Candidate result;
  result.subdetectorId = subdetectorId;
  result.sectorId = sectorId;
  result.bcTag = bcTag;
  result.eta = 1.2F;
  result.phi = 0.7F;
  result.deltaTheta = -0.025F;
  result.deltaPhi = 0.012F;
  result.pt = pt;
  result.threshold = threshold;
  result.charge = 1;
  result.hasInnerCoincidence = true;
  result.goodMagneticField = true;
  result.selectorPriority = selectorPriority;
  result.nswSegment = marker;
  return result;
}

// Keep the expected encoding owned by the xAOD setter used in production.
std::uint8_t expectedPtCode(float pt) {
  xAOD::TGCCandDataContainer candidates;
  SG::AuxStoreInternal candidatesAux;
  candidates.setStore(&candidatesAux);
  xAOD::TGCCandData *referenceCandidate =
      candidates.push_back(std::make_unique<xAOD::TGCCandData>());
  referenceCandidate->setPt(pt);
  return referenceCandidate->pt();
}

std::size_t countGroup(const xAOD::TGCCandDataContainer &candidates,
                       std::uint16_t subdetectorId, std::uint16_t sectorId) {
  return std::count_if(
      candidates.begin(), candidates.end(),
      [subdetectorId, sectorId](const xAOD::TGCCandData *selected) {
        return selected->subdetectorId() == subdetectorId &&
               selected->sectorId() == sectorId;
      });
}

struct CandidateFixture {
  CandidateFixture()
      : candidates{
            candidate(sideA, 5U, currentBc, 10.F, 2U, 2U, 10U),
            candidate(sideA, 5U, currentBc, 50.F, 8U, 2U, 50U),
            candidate(sideA, 5U, currentBc, 30.F, 6U, 2U, 30U),
            candidate(sideA, 5U, currentBc, 20.F, 5U, 2U, 20U),
            candidate(sideA, 5U, currentBc, 40.F, 7U, 2U, 40U),
            candidate(sideA, 5U, currentBc, 60.F, 4U, 2U, 60U),
            candidate(sideA, 5U, currentBc, 25.F, 9U, 2U, 25U),
            candidate(sideA, 5U, currentBc, 5.F, 1U, 2U, 5U),
            candidate(sideA, 5U, currentBc, 100.F, 0U, 2U, 100U),
            candidate(sideA, 6U, currentBc, 15.F, 4U, 2U, 615U),
            candidate(sideA, 6U, currentBc, 35.F, 6U, 2U, 635U),
            candidate(sideC, 5U, currentBc, 45.F, 7U, 2U, 545U),
            candidate(sideC, 7U, currentBc, 20.F, 5U, 1U, 701U),
            candidate(sideC, 7U, currentBc, 20.F, 5U, 3U, 703U),
            candidate(sideC, 7U, currentBc, 20.F, 5U, 3U, 704U)} {
    candidates[6].charge = -1;
    candidates[6].hasInnerCoincidence = false;
    candidates[6].goodMagneticField = false;
    output.setStore(&outputAux);
  }

  void select() { selector.select(candidates, output); }

  L0Muon::TgcL0CandidateContainer candidates;
  xAOD::TGCCandDataContainer output;
  SG::AuxStoreInternal outputAux;
  L0Muon::TgcL0Floating::TrackSelector selector;
};

}  // namespace

BOOST_AUTO_TEST_SUITE(TgcL0TrackSelectorTest)

BOOST_FIXTURE_TEST_CASE(SelectsAtMostSixCandidatesPerSector,
                        CandidateFixture) {
  select();

  BOOST_REQUIRE_EQUAL(output.size(), 12U);
  BOOST_TEST(countGroup(output, sideA, 5U) == 6U);
  BOOST_TEST(countGroup(output, sideA, 6U) == 2U);
  BOOST_TEST(countGroup(output, sideC, 5U) == 1U);
  BOOST_TEST(countGroup(output, sideC, 7U) == 3U);
}

BOOST_AUTO_TEST_CASE(RejectsZeroThreshold) {
  L0Muon::TgcL0CandidateContainer candidates{
      candidate(sideA, 5U, currentBc, 100.F, 0U, 2U, 100U),
      candidate(sideA, 5U, currentBc, 10.F, 1U, 2U, 10U)};
  xAOD::TGCCandDataContainer output;
  SG::AuxStoreInternal outputAux;
  output.setStore(&outputAux);

  const L0Muon::TgcL0Floating::TrackSelector selector;
  selector.select(candidates, output);

  BOOST_REQUIRE_EQUAL(output.size(), 1U);
  BOOST_TEST(output[0]->nswSegment() == 10U);
  BOOST_TEST(output[0]->threshold() == 1U);
}

BOOST_FIXTURE_TEST_CASE(OrdersCandidatesAndAssignsTcIds, CandidateFixture) {
  select();
  BOOST_REQUIRE_EQUAL(output.size(), 12U);

  const std::vector<float> firstGroupPt{25.F, 50.F, 40.F,
                                        30.F, 20.F, 60.F};
  const std::vector<std::uint8_t> firstGroupThreshold{9U, 8U, 7U,
                                                      6U, 5U, 4U};
  const std::vector<std::uint32_t> firstGroupMarker{25U, 50U, 40U,
                                                    30U, 20U, 60U};
  for (std::size_t index = 0U; index < firstGroupPt.size(); ++index) {
    const xAOD::TGCCandData &selected = *output[index];
    BOOST_TEST(selected.subdetectorId() == sideA);
    BOOST_TEST(selected.sectorId() == 5U);
    BOOST_TEST(selected.bcTag() == currentBc);
    BOOST_TEST(selected.tcId() == index + 1U);
    BOOST_TEST(selected.threshold() == firstGroupThreshold[index]);
    BOOST_TEST(selected.pt() == expectedPtCode(firstGroupPt[index]));
    BOOST_TEST(selected.nswSegment() == firstGroupMarker[index]);
  }

  BOOST_TEST(output[6]->sectorId() == 6U);
  BOOST_TEST(output[6]->tcId() == 1U);
  BOOST_TEST(output[6]->pt() == expectedPtCode(35.F));
  BOOST_TEST(output[8]->subdetectorId() == sideC);
  BOOST_TEST(output[8]->tcId() == 1U);
  BOOST_TEST(output[9]->nswSegment() == 703U);
  BOOST_TEST(output[10]->nswSegment() == 704U);
  BOOST_TEST(output[11]->nswSegment() == 701U);
}

BOOST_FIXTURE_TEST_CASE(MapsCandidateFields, CandidateFixture) {
  select();
  BOOST_REQUIRE_EQUAL(output.size(), 12U);

  const xAOD::TGCCandData &mapped = *output[0];
  BOOST_TEST(mapped.threshold() == 9U);
  BOOST_TEST(mapped.candCharge() == 0U);
  BOOST_TEST(output[1]->candCharge() == 1U);
  BOOST_TEST(mapped.mdtFlag() == 0U);
  BOOST_TEST(!mapped.hasInnerCoincidence());
  BOOST_TEST(output[1]->hasInnerCoincidence());
  BOOST_TEST(!mapped.goodMagneticField());
  BOOST_TEST(output[1]->goodMagneticField());
  BOOST_TEST(static_cast<int>(mapped.candQuality()) ==
             static_cast<int>(xAOD::ICandData_v1::Quality::Q_UNDEFINED));
  BOOST_TEST(close(mapped.deltaPhi(), 0.012F, 0.005F));
  BOOST_TEST(close(mapped.deltaTheta(), -0.025F, 0.003F));
}

BOOST_AUTO_TEST_SUITE_END()
