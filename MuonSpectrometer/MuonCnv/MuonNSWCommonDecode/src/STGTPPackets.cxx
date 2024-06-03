/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonNSWCommonDecode/STGTPPackets.h"

#include <algorithm>
#include <exception>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include "MuonNSWCommonDecode/NSWDecodeHelper.h"
#include "MuonNSWCommonDecode/NSWMMTPDecodeBitmaps.h"
#include "MuonNSWCommonDecode/NSWSTGTPDecodeBitmaps.h"

Muon::nsw::STGTPPadPacket::STGTPPadPacket(const std::vector<uint32_t>& payload) {
  static constexpr auto PACKETS_SIZE = std::size_t{3};
  if (std::size(payload) != PACKETS_SIZE) {
    throw std::runtime_error(
			     Muon::nsw::format("Packet vector has size {} instead of expected size {}", std::size(payload), PACKETS_SIZE));
  }

  const auto packets = std::span{payload.data(), std::size(payload)};
  auto readPointer = std::size_t{0};
  auto decode = [&packets](std::size_t& readPointer, const std::size_t size) {
    return decode_and_advance<std::uint64_t, std::uint32_t>(packets, readPointer, size);
  };

  m_coincWedge = decode(readPointer, Muon::nsw::STGTPPad::size_coincidence_wedge);
  for (std::size_t i = Muon::nsw::STGTPPad::num_pads; i > 0; --i) {
    const auto index = i - 1;
    m_phiIDs.at(index) = decode(readPointer, Muon::nsw::STGTPPad::size_phiID);
  }

  for (std::size_t i = Muon::nsw::STGTPPad::num_pads; i > 0; --i) {
    const auto index = i - 1;
    m_bandIDs.at(index) = decode(readPointer, Muon::nsw::STGTPPad::size_bandID);
  }

  m_BCID = decode(readPointer, Muon::nsw::STGTPPad::size_BCID);

  readPointer += Muon::nsw::STGTPPad::size_spare;
  m_idleFlag = decode(readPointer, Muon::nsw::STGTPPad::size_idleFlag);
}

Muon::nsw::STGTPSegmentPacket::STGTPSegmentPacket(const std::vector<uint32_t>& payload) {
  static constexpr auto PACKETS_SIZE = std::size_t{8};
  if (std::size(payload) != PACKETS_SIZE) {
    throw std::runtime_error(
			     Muon::nsw::format("Packet vector has size {} instead of expected size {}", std::size(payload), PACKETS_SIZE));
  }
  auto readPointer = std::size_t{0};
  const auto packets = std::span{payload.data(), std::size(payload)};
  auto decode = [&packets](std::size_t& readPointer, const std::size_t size) {
    return decode_and_advance<std::uint64_t, std::uint32_t>(packets, readPointer, size);
  };

  m_lut_choice = decode(readPointer, Muon::nsw::STGTPSegments::size_lut_choice_selection);
  m_nsw_segment_selector = decode(readPointer, Muon::nsw::STGTPSegments::size_nsw_segment_selector);
  m_valid_segment_selector = decode(readPointer, Muon::nsw::STGTPSegments::size_valid_segment_selector);

  for (std::size_t i = Muon::nsw::STGTPSegments::num_segments; i > 0; --i) {
    const auto index = i - 1;
    m_segmentData.at(index).monitor = decode(readPointer, Muon::nsw::STGTPSegments::size_output_segment_monitor);
    m_segmentData.at(index).spare = decode(readPointer, Muon::nsw::STGTPSegments::size_output_segment_spare);
    m_segmentData.at(index).lowRes = decode(readPointer, Muon::nsw::STGTPSegments::size_output_segment_lowRes);
    m_segmentData.at(index).phiRes = decode(readPointer, Muon::nsw::STGTPSegments::size_output_segment_phiRes);
    m_segmentData.at(index).dTheta = decode(readPointer, Muon::nsw::STGTPSegments::size_output_segment_dTheta);
    m_segmentData.at(index).phiID = decode(readPointer, Muon::nsw::STGTPSegments::size_output_segment_phiID);
    m_segmentData.at(index).rIndex = decode(readPointer, Muon::nsw::STGTPSegments::size_output_segment_rIndex);
  }

  m_BCID = decode(readPointer, Muon::nsw::STGTPSegments::size_bcid);
  m_sectorID = decode(readPointer, Muon::nsw::STGTPSegments::size_sectorID);
}

const Muon::nsw::STGTPSegmentPacket::SegmentData& Muon::nsw::STGTPSegmentPacket::Segment(
    const std::size_t segment) const {
  if (segment >= STGTPSegments::num_segments) {
    throw std::out_of_range(
			    Muon::nsw::format("Requested segment {} which does not exist (max {})", segment, STGTPSegments::num_segments - 1));
  }
  return m_segmentData.at(segment);
}
