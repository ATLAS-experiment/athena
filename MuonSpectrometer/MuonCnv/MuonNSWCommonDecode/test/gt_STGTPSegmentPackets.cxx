/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @author Jonas Roemer
 * @date April 2023
 * @brief Tests for STG TP packet decoder
 */

#include <memory>
#include <stdexcept>

#include <gtest/gtest.h>
#include <AsgTesting/UnitTest.h>
#include "MuonNSWCommonDecode/STGTPPackets.h"

class STGTPSegmentPacketTest : public ::testing::Test {
 protected:
  void SetUp() override {
    const auto data = std::vector(std::cbegin(s_DATA), std::cend(s_DATA));
    m_decoder = std::make_unique<Muon::nsw::STGTPSegmentPacket>(data,1);
  }

  std::unique_ptr<Muon::nsw::STGTPSegmentPacket> m_decoder{nullptr};
  static constexpr std::array<std::uint32_t, 8> s_DATA{262144, 3316187135, 4278190080, 67109888, 262144, 67109888, 262144, 3317235711};
};

TEST_F(STGTPSegmentPacketTest, LUT_ChoiceSelection) {
  EXPECT_EQ(m_decoder->LUT_ChoiceSelection(), 1024);
}

TEST_F(STGTPSegmentPacketTest, NSW_SegmentSelector) {
  EXPECT_EQ(m_decoder->NSW_SegmentSelector(), 12);
}

TEST_F(STGTPSegmentPacketTest, ValidSegmentSelector) {
  EXPECT_EQ(m_decoder->ValidSegmentSelector(), 1448);
}

TEST_F(STGTPSegmentPacketTest, SegmentMonitor) {
  static constexpr std::array<std::uint32_t, Muon::nsw::STGTPSegments::num_segments> EXPECTATION{0, 0, 0, 0, 0, 0, 0, 1};
  for (std::size_t i=0; i<Muon::nsw::STGTPSegments::num_segments; ++i) {
    EXPECT_EQ(m_decoder->Segment(i).monitor, EXPECTATION.at(i));
  }
}

TEST_F(STGTPSegmentPacketTest, SegmentSpare) {
  static constexpr std::array<std::uint32_t, Muon::nsw::STGTPSegments::num_segments> EXPECTATION{0, 0, 0, 0, 0, 0, 0, 3};
  for (std::size_t i=0; i<Muon::nsw::STGTPSegments::num_segments; ++i) {
    EXPECT_EQ(m_decoder->Segment(i).spare, EXPECTATION.at(i));
  }
}

TEST_F(STGTPSegmentPacketTest, SegmentLowRes) {
  static constexpr std::array<std::uint32_t, Muon::nsw::STGTPSegments::num_segments> EXPECTATION{0, 0, 0, 0, 0, 0, 0, 1};
  for (std::size_t i=0; i<Muon::nsw::STGTPSegments::num_segments; ++i) {
    EXPECT_EQ(m_decoder->Segment(i).lowRes, EXPECTATION.at(i));
  }
}

TEST_F(STGTPSegmentPacketTest, SegmentPhiRes) {
  static constexpr std::array<std::uint32_t, Muon::nsw::STGTPSegments::num_segments> EXPECTATION{0, 0, 0, 0, 0, 0, 0, 1};
  for (std::size_t i=0; i<Muon::nsw::STGTPSegments::num_segments; ++i) {
    EXPECT_EQ(m_decoder->Segment(i).phiRes, EXPECTATION.at(i));
  }
}

TEST_F(STGTPSegmentPacketTest, SegmentDTheta) {
  static constexpr std::array<std::uint32_t, Muon::nsw::STGTPSegments::num_segments> EXPECTATION{3, 0, 0, 0, 0, 16, 0, 31};
  for (std::size_t i=0; i<Muon::nsw::STGTPSegments::num_segments; ++i) {
    std::cout << i << " " << m_decoder->Segment(i).dTheta << " " << EXPECTATION.at(i) << std::endl;
    EXPECT_EQ(m_decoder->Segment(i).dTheta, EXPECTATION.at(i));
  }
}

TEST_F(STGTPSegmentPacketTest, SegmentPhiId) {
  static constexpr std::array<std::uint32_t, Muon::nsw::STGTPSegments::num_segments> EXPECTATION{5, 4, 4, 0, 0, 0, 0, 63};
  for (std::size_t i=0; i<Muon::nsw::STGTPSegments::num_segments; ++i) {
    EXPECT_EQ(m_decoder->Segment(i).phiID, EXPECTATION.at(i));
  }
}

TEST_F(STGTPSegmentPacketTest, SegmentRIndex) {
  static constexpr std::array<std::uint32_t, Muon::nsw::STGTPSegments::num_segments> EXPECTATION{184, 0, 0, 4, 4, 4, 0, 255};
  for (std::size_t i=0; i<Muon::nsw::STGTPSegments::num_segments; ++i) {
    EXPECT_EQ(m_decoder->Segment(i).rIndex, EXPECTATION.at(i));
  }
}

TEST_F(STGTPSegmentPacketTest, BCID) {
  EXPECT_EQ(m_decoder->BCID(), 4095);
}

TEST_F(STGTPSegmentPacketTest, SectorID) {
  EXPECT_EQ(m_decoder->SectorID(), 15);
}

ATLAS_GOOGLE_TEST_MAIN
