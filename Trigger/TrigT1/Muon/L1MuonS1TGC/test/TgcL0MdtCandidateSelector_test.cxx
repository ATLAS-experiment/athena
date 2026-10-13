/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TgcL0MdtCandidateSelectorTest

#include <boost/test/unit_test.hpp>

#include "TgcL0MdtCandidateSelector.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>

#include "AthContainers/AuxStoreInternal.h"
#include "AthContainers/OwnershipPolicy.h"
#include "xAODTrigL1Muon/L1TGCCandData.h"

namespace {

constexpr std::uint16_t sideA = 0x65U;
constexpr std::uint16_t sideC = 0x66U;
constexpr std::uint16_t currentBc = 0x2U;

void addCandidate(xAOD::L1TGCCandDataContainer &candidates,
                  std::uint16_t subdetectorId, std::uint16_t sectorId,
                  std::uint16_t bcTag, float pt, std::uint8_t tcId,
                  std::uint32_t marker) {
  xAOD::L1TGCCandData *candidate =
      candidates.push_back(std::make_unique<xAOD::L1TGCCandData>());
  candidate->initialize(subdetectorId, sectorId, bcTag);
  candidate->setL1Pt(pt);
  candidate->setTcId(tcId);
  candidate->setL1NswSegment(marker);
}

std::size_t countGroup(const xAOD::L1TGCCandDataContainer &candidates,
                       std::uint16_t subdetectorId, std::uint16_t sectorId) {
  return std::count_if(
      candidates.begin(), candidates.end(),
      [subdetectorId, sectorId](const xAOD::L1TGCCandData *candidate) {
        return candidate->l1SubdetectorId() == subdetectorId &&
               candidate->l1SectorId() == sectorId;
      });
}

struct CandidateFixture {
  CandidateFixture() {
    candidates.setStore(&candidatesAux);
    addCandidate(candidates, sideA, 5U, currentBc, 25.F, 1U, 25U);
    addCandidate(candidates, sideA, 5U, currentBc, 50.F, 2U, 50U);
    addCandidate(candidates, sideA, 5U, currentBc, 40.F, 3U, 40U);
    addCandidate(candidates, sideA, 5U, currentBc, 30.F, 4U, 30U);
    addCandidate(candidates, sideA, 5U, currentBc, 20.F, 5U, 20U);
    addCandidate(candidates, sideA, 5U, currentBc, 60.F, 6U, 60U);
    addCandidate(candidates, sideA, 6U, currentBc, 35.F, 1U, 635U);
    addCandidate(candidates, sideA, 6U, currentBc, 15.F, 2U, 615U);
    addCandidate(candidates, sideC, 5U, currentBc, 45.F, 1U, 545U);
    addCandidate(candidates, sideC, 5U, currentBc, 100.F, 0U, 500U);
  }

  xAOD::L1TGCCandDataContainer candidates;
  SG::AuxStoreInternal candidatesAux;
  L1Muon::TgcL0MdtCandidateSelector selector;
};

}  // namespace

BOOST_AUTO_TEST_SUITE(TgcL0MdtCandidateSelectorTest)

BOOST_FIXTURE_TEST_CASE(SelectsThreeCandidatesPerSector, CandidateFixture) {
  std::unique_ptr<xAOD::L1TGCCandDataContainer> output =
      selector.select(candidates);

  BOOST_REQUIRE_EQUAL(output->size(), 6U);
  BOOST_TEST(countGroup(*output, sideA, 5U) == 3U);
  BOOST_TEST(countGroup(*output, sideA, 6U) == 2U);
  BOOST_TEST(countGroup(*output, sideC, 5U) == 1U);

  BOOST_TEST((*output)[0]->l1NswSegment() == 60U);
  BOOST_TEST((*output)[0]->tcId() == 6U);
  BOOST_TEST((*output)[0]->l1Pt() > (*output)[1]->l1Pt());
  BOOST_TEST((*output)[1]->l1Pt() > (*output)[2]->l1Pt());
  BOOST_TEST((*output)[1]->l1NswSegment() == 50U);
  BOOST_TEST((*output)[2]->l1NswSegment() == 40U);
  BOOST_TEST((*output)[3]->l1SectorId() == 6U);
  BOOST_TEST((*output)[3]->l1NswSegment() == 635U);
  BOOST_TEST((*output)[4]->l1NswSegment() == 615U);
  BOOST_TEST((*output)[5]->l1SubdetectorId() == sideC);
  BOOST_TEST((*output)[5]->l1NswSegment() == 545U);
}

BOOST_FIXTURE_TEST_CASE(RejectsZeroTcIdAndReturnsView, CandidateFixture) {
  std::unique_ptr<xAOD::L1TGCCandDataContainer> output =
      selector.select(candidates);

  BOOST_REQUIRE_EQUAL(output->size(), 6U);
  const auto invalid =
      std::find_if(output->begin(), output->end(),
                   [](const xAOD::L1TGCCandData *candidate) {
                     return candidate->l1NswSegment() == 500U;
                   });
  const bool invalidRejected = invalid == output->end();
  BOOST_TEST(invalidRejected);
  BOOST_TEST(static_cast<int>(output->ownPolicy()) ==
             static_cast<int>(SG::VIEW_ELEMENTS));
}

BOOST_AUTO_TEST_SUITE_END()
