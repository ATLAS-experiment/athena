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

class STGTPPadPacketTest : public ::testing::Test {
 protected:
  void SetUp() override {
    const auto data = std::vector(std::cbegin(s_DATA), std::cend(s_DATA));
    m_decoder = std::make_unique<Muon::nsw::STGTPPadPacket>(data,1);
  }

  std::unique_ptr<Muon::nsw::STGTPPadPacket> m_decoder{nullptr};
  static constexpr std::array<std::uint32_t, 3> s_DATA{4027526130, 3206217498, 4283322368};
  static constexpr auto s_MAX_SIZE_IDS = std::size_t{4};
};

TEST_F(STGTPPadPacketTest, BCID) {
  EXPECT_EQ(m_decoder->BCID(), 1253);
}

TEST_F(STGTPPadPacketTest, BandIds) {
  static constexpr auto EXPECTATION = std::array<std::uint32_t, s_MAX_SIZE_IDS>{255, 26, 255, 26};
  for (std::size_t index = 0; index < s_MAX_SIZE_IDS; ++index) {
    EXPECT_EQ(m_decoder->BandID(index), EXPECTATION.at(index));
  }
}

TEST_F(STGTPPadPacketTest, BandIdThrows) {
  EXPECT_THROW(static_cast<void>(m_decoder->BandID(s_MAX_SIZE_IDS + 1)), std::out_of_range);
}

TEST_F(STGTPPadPacketTest, PhiIds) {
  static constexpr auto EXPECTATION = std::array<std::uint32_t, s_MAX_SIZE_IDS>{63, 10, 63, 10};
  for (std::size_t index = 0; index < s_MAX_SIZE_IDS; ++index) {
    EXPECT_EQ(m_decoder->PhiID(index), EXPECTATION.at(index));
  }
}

TEST_F(STGTPPadPacketTest, PhiIdThrows) {
  EXPECT_THROW(static_cast<void>(m_decoder->PhiID(s_MAX_SIZE_IDS + 1)), std::out_of_range);
}

TEST_F(STGTPPadPacketTest, PadIdleFlag) {
  EXPECT_EQ(m_decoder->PadIdleFlag(), 0);
}

TEST_F(STGTPPadPacketTest, CoincidenceWedge) {
  EXPECT_EQ(m_decoder->CoincidenceWedge(), 61455);
}

ATLAS_GOOGLE_TEST_MAIN
