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


size_t Muon::nsw::STGTPMMPacket::Size(const int ver){
  size_t packet_size_w = 0;
  size_t word_size = 32;
  switch (ver) {
    case 1:
      packet_size_w =  Muon::nsw::STGTPMMData::size_v1 / word_size;
      break;
    case 2:
      packet_size_w = Muon::nsw::STGTPMMData::size_v2 / word_size;
      break;
    case 3:
      packet_size_w = Muon::nsw::STGTPMMData::size_v3 / word_size;
      break;
    default:
      packet_size_w = 0;
      break;
  }
  return packet_size_w;
}
Muon::nsw::STGTPMMPacket::STGTPMMPacket(const std::vector<uint32_t>& payload, const int ver=1) {

  if (ver < 3){
     // This section did not exist before v3
     return;
  }

  size_t packet_size_w = Size(ver);

  if (std::size(payload) != packet_size_w) {
    throw std::runtime_error(
                             Muon::nsw::format("Packet vector has size {} instead of expected size {}", std::size(payload), packet_size_w));
  }

  const auto packets = std::span{payload.data(), std::size(payload)};
  auto readPointer = std::size_t{0};
  auto decode = [&packets](std::size_t& readPointer, const std::size_t size) {
    return decode_and_advance<std::uint64_t, std::uint32_t>(packets, readPointer, size);
  };


  decode(readPointer, Muon::nsw::STGTPMMData::size_v3_padding);
  m_valids = decode(readPointer, Muon::nsw::STGTPMMData::size_output_mm_valids);
  for (std::size_t i = Muon::nsw::STGTPMMData::num_mm; i > 0; --i) {
    const auto index = i - 1;
    uint32_t segment_bit = 1 << index;
    uint32_t valid_segment = (m_valids & segment_bit);
    m_segmentData.at(index).monitor = decode(readPointer, Muon::nsw::STGTPMMData::size_output_mm_monitor);
    m_segmentData.at(index).spare = decode(readPointer, Muon::nsw::STGTPMMData::size_output_mm_spare);
    m_segmentData.at(index).lowRes = decode(readPointer, Muon::nsw::STGTPMMData::size_output_mm_lowRes);
    m_segmentData.at(index).phiRes = decode(readPointer, Muon::nsw::STGTPMMData::size_output_mm_phiRes);
    m_segmentData.at(index).dTheta = decode(readPointer, Muon::nsw::STGTPMMData::size_output_mm_dTheta);
    m_segmentData.at(index).phiID = decode(readPointer, Muon::nsw::STGTPMMData::size_output_mm_phiID);
    m_segmentData.at(index).rIndex = decode(readPointer, Muon::nsw::STGTPMMData::size_output_mm_rIndex);
    if (!valid_segment)
    {
	m_segmentData.at(index).monitor = 0;
	m_segmentData.at(index).spare = 0;
	m_segmentData.at(index).lowRes = 0;
	m_segmentData.at(index).phiRes = 0;
	m_segmentData.at(index).dTheta = Muon::nsw::STGTPMMData::mm_stream_invalid_dTheta; // this is the invalid flag
	m_segmentData.at(index).phiID = 0;
	m_segmentData.at(index).rIndex = 0;
    }
 }
  m_BCID = decode(readPointer, Muon::nsw::STGTPMMData::size_bcid);

}

const Muon::nsw::STGTPMMPacket::MMSegmentData& Muon::nsw::STGTPMMPacket::Segment(
    const std::size_t segment) const {
  if (segment >= STGTPMMData::num_mm) {
    throw std::out_of_range(
                            Muon::nsw::format("Requested segment {} which does not exist (max {})", segment, STGTPMMData::num_mm - 1));
  }
  return m_segmentData.at(segment);
}

size_t Muon::nsw::STGTPPadPacket::Size(const int ver){
  size_t packet_size_w = 0;
  size_t word_size = 32;
  switch (ver) {
    case 1:
      packet_size_w =  Muon::nsw::STGTPPad::size_v1 / word_size;
      break;
    case 2:
      packet_size_w = Muon::nsw::STGTPPad::size_v2 / word_size;
      break;
    case 3:
      packet_size_w = Muon::nsw::STGTPPad::size_v3 / word_size;
      break;
    default:
      packet_size_w = 0;
      break;
  }
  return packet_size_w;
}
Muon::nsw::STGTPPadPacket::STGTPPadPacket(const std::vector<uint32_t>& payload, const int ver=1) {
  size_t packet_size_w = Size(ver);
  if (std::size(payload) != packet_size_w) {
    throw std::runtime_error(
			     Muon::nsw::format("Packet vector has size {} instead of expected size {}", std::size(payload), packet_size_w));
  }

  const auto packets = std::span{payload.data(), std::size(payload)};
  auto readPointer = std::size_t{0};
  auto decode = [&packets](std::size_t& readPointer, const std::size_t size) {
    return decode_and_advance<std::uint64_t, std::uint32_t>(packets, readPointer, size);
  };

  if (ver >= 3)
  {
       // in versions 3+ the padding appears first
       decode(readPointer, Muon::nsw::STGTPPad::size_v3_padding);
  }
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

size_t Muon::nsw::STGTPSegmentPacket::Size(const int ver){
  size_t packet_size_w = 0;
  size_t word_size = 32;
  switch (ver) {
    case 1:
      packet_size_w =  Muon::nsw::STGTPSegments::size_v1 / word_size;;
      break;
    case 2:
      packet_size_w = Muon::nsw::STGTPSegments::size_v2 / word_size;;
      break;
    case 3:
      packet_size_w = Muon::nsw::STGTPSegments::size_v3 / word_size;
      break;
    default:
      packet_size_w = 0;
      break;
  }
  return packet_size_w;
}

Muon::nsw::STGTPSegmentPacket::STGTPSegmentPacket(const std::vector<uint32_t>& payload, const int ver=1) {
  size_t packet_size_w = Size(ver);

  if (std::size(payload) != packet_size_w) {
    throw std::runtime_error(
			     Muon::nsw::format("Packet vector has size {} instead of expected size {}", std::size(payload), packet_size_w));
  }
  auto readPointer = std::size_t{0};
  const auto packets = std::span{payload.data(), std::size(payload)};
  auto decode = [&packets](std::size_t& readPointer, const std::size_t size) {
    return decode_and_advance<std::uint64_t, std::uint32_t>(packets, readPointer, size);
  };

  if (ver < 3)
  {
     // In the versions prior to version 3, there was a LUT used to merge segments
     // the versions following 3 use an algorithm that simplifies the merge, and hence the 
     // structure of these data changed
     m_lut_choice = decode(readPointer, Muon::nsw::STGTPSegments::size_lut_choice_selection);
     m_nsw_segment_selector = decode(readPointer, Muon::nsw::STGTPSegments::size_nsw_segment_selector);
     m_valid_segment_selector = decode(readPointer, Muon::nsw::STGTPSegments::size_valid_segment_selector);
  }
  else
  {
     decode(readPointer, Muon::nsw::STGTPSegments::size_v3_padding);
     m_nsw_segment_selector = decode(readPointer, Muon::nsw::STGTPSegments::size_valid_segment_v3);
     m_valid_segment_selector = decode(readPointer, Muon::nsw::STGTPSegments::size_valid_segment_selector);
  }

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
