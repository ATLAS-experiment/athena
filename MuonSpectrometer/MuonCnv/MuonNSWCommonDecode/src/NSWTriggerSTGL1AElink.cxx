/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonNSWCommonDecode/NSWTriggerSTGL1AElink.h"

#include "MuonNSWCommonDecode/NSWResourceId.h"
#include "MuonNSWCommonDecode/NSWSTGTPDecodeBitmaps.h"
#include "ers/ers.h"
#include <cmath>
#include <cstddef>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>

Muon::nsw::NSWTriggerSTGL1AElink::NSWTriggerSTGL1AElink(const uint32_t* bs, const uint32_t remaining)
    : NSWTriggerElink(bs, remaining), m_data{bs, remaining} {
  // TODO: once format finalized, checking a minimum size

  // 2 felix header 32b words already decoded;
  constexpr static auto START_DATA = std::size_t{2 * 32};
  auto readPointer = std::size_t{START_DATA};

  m_packet_version = parse_version_workaround(readPointer);
  decode_header(readPointer, m_packet_version);
  decode_data(readPointer, m_packet_version);
  decode_trailer(readPointer);
}

void Muon::nsw::NSWTriggerSTGL1AElink::decode_header(std::size_t& readPointer, int& version) {

  // This part is constant for all versions
  m_head_fragID = Muon::nsw::decode_and_advance<uint64_t>(m_data, readPointer, Muon::nsw::STGTPL1A::size_head_fragID);
  m_head_sectID = Muon::nsw::decode_and_advance<uint64_t>(m_data, readPointer, Muon::nsw::STGTPL1A::size_head_sectID);
  m_head_EC =     Muon::nsw::decode_and_advance<uint64_t>(m_data, readPointer, Muon::nsw::STGTPL1A::size_head_EC);
  m_head_flags =  Muon::nsw::decode_and_advance<uint64_t>(m_data, readPointer, Muon::nsw::STGTPL1A::size_head_flags);
  m_head_BCID =   Muon::nsw::decode_and_advance<uint64_t>(m_data, readPointer, Muon::nsw::STGTPL1A::size_head_BCID);
  m_head_orbit =  Muon::nsw::decode_and_advance<uint64_t>(m_data, readPointer, Muon::nsw::STGTPL1A::size_head_orbit);
  m_head_spare =  Muon::nsw::decode_and_advance<uint64_t>(m_data, readPointer, Muon::nsw::STGTPL1A::size_head_spare);
  m_L1ID =        Muon::nsw::decode_and_advance<uint64_t>(m_data, readPointer, Muon::nsw::STGTPL1A::size_L1ID);

  ERS_DEBUG(2, Muon::nsw::format("\n TP header: \n"
                                 "  fradID: {}\n"
                                 "  sectID: {}\n"
                                 "  EC:     {}\n"
                                 "  flags:  {}\n"
                                 "  BCID:   {}\n"
                                 "  orbit:  {}\n"
                                 "  spare:  {}\n"
                                 "  L1ID:   {}",
                                 m_head_fragID, m_head_sectID, m_head_EC, m_head_flags, m_head_BCID, m_head_orbit, m_head_spare, m_L1ID));

  if (version == 1)
  {
      m_l1a_open_BCID =         Muon::nsw::decode_and_advance<uint64_t>(m_data, readPointer, Muon::nsw::STGTPL1A::size_l1a_open_BCID);
      m_l1a_req_BCID =          Muon::nsw::decode_and_advance<uint64_t>(m_data, readPointer, Muon::nsw::STGTPL1A::size_l1a_req_BCID);
      m_l1a_close_BCID =        Muon::nsw::decode_and_advance<uint64_t>(m_data, readPointer, Muon::nsw::STGTPL1A::size_l1a_close_BCID);
      m_l1a_timeout =           Muon::nsw::decode_and_advance<uint64_t>(m_data, readPointer, Muon::nsw::STGTPL1A::size_l1a_timeout); // overflow count
      m_l1a_wdw_matching_engines_usage = 
                                Muon::nsw::decode_and_advance<uint64_t>(m_data, readPointer, Muon::nsw::STGTPL1A::size_l1a_wdw_matching_engines_usage);
      m_l1a_open_BCID_offset =  Muon::nsw::decode_and_advance<uint64_t>(m_data, readPointer, Muon::nsw::STGTPL1A::size_l1a_open_BCID_offset);
      m_l1a_req_BCID_offset =   Muon::nsw::decode_and_advance<uint64_t>(m_data, readPointer, Muon::nsw::STGTPL1A::size_l1a_req_BCID_offset);
      m_l1a_close_BCID_offset = Muon::nsw::decode_and_advance<uint64_t>(m_data, readPointer, Muon::nsw::STGTPL1A::size_l1a_close_BCID_offset);
      m_l1a_timeout_config =    Muon::nsw::decode_and_advance<uint64_t>(m_data, readPointer, Muon::nsw::STGTPL1A::size_l1a_timeout_config);
      
      m_l1a_link_const =        Muon::nsw::decode_and_advance<uint64_t>(m_data, readPointer, Muon::nsw::STGTPL1A::size_l1a_link_const); 
  }
  else
  {
      m_l1a_versionID =         Muon::nsw::decode_and_advance<uint64_t>(m_data, readPointer, Muon::nsw::STGTPL1A::size_l1a_versionID);
      if (m_l1a_versionID == 1)
      {
          version  = 2;
      }
      else if (m_l1a_versionID == 3)
      {
          version = m_l1a_versionID;
      }
      else
      {
           Muon::nsw::NSWTriggerException e ( Muon::nsw::format( "Stgc packet version not existent (corrupted message) expected 3, got ({})", m_l1a_versionID));
          throw e;

      }
      m_l1a_local_req_BCID =    Muon::nsw::decode_and_advance<uint64_t>(m_data, readPointer, Muon::nsw::STGTPL1A::size_l1a_local_req_BCID);
      m_l1a_local_rel_BCID =    Muon::nsw::decode_and_advance<uint64_t>(m_data, readPointer, Muon::nsw::STGTPL1A::size_l1a_local_rel_BCID);
      m_l1a_open_BCID =         Muon::nsw::decode_and_advance<uint64_t>(m_data, readPointer, Muon::nsw::STGTPL1A::size_l1a_open_BCID);
      m_l1a_req_BCID =          Muon::nsw::decode_and_advance<uint64_t>(m_data, readPointer, Muon::nsw::STGTPL1A::size_l1a_req_BCID);
      m_l1a_close_BCID =        Muon::nsw::decode_and_advance<uint64_t>(m_data, readPointer, Muon::nsw::STGTPL1A::size_l1a_close_BCID);
      m_l1a_timeout =           Muon::nsw::decode_and_advance<uint64_t>(m_data, readPointer, Muon::nsw::STGTPL1A::size_l1a_timeout);
      m_l1a_open_BCID_offset =  Muon::nsw::decode_and_advance<uint64_t>(m_data, readPointer, Muon::nsw::STGTPL1A::size_l1a_open_BCID_offset);
      m_l1a_req_BCID_offset =   Muon::nsw::decode_and_advance<uint64_t>(m_data, readPointer, Muon::nsw::STGTPL1A::size_l1a_req_BCID_offset);
      m_l1a_close_BCID_offset = Muon::nsw::decode_and_advance<uint64_t>(m_data, readPointer, Muon::nsw::STGTPL1A::size_l1a_close_BCID_offset);
      m_l1a_timeout_config =    Muon::nsw::decode_and_advance<uint64_t>(m_data, readPointer, Muon::nsw::STGTPL1A::size_l1a_timeout_config);
      m_l1a_busy_thr =          Muon::nsw::decode_and_advance<uint64_t>(m_data, readPointer, Muon::nsw::STGTPL1A::size_l1a_busy_thr);
      m_l1a_engine_snapshot =   Muon::nsw::decode_and_advance<uint64_t>(m_data, readPointer, Muon::nsw::STGTPL1A::size_l1a_engine_snapshot);
      m_l1a_link_const =        Muon::nsw::decode_and_advance<uint64_t>(m_data, readPointer, Muon::nsw::STGTPL1A::size_l1a_link_const);
      m_l1a_padding =           Muon::nsw::decode_and_advance<uint64_t>(m_data, readPointer, Muon::nsw::STGTPL1A::size_l1a_padding);
  }

}

int Muon::nsw::NSWTriggerSTGL1AElink::parse_version_workaround(std::size_t& readPointer)
{
   auto anchor_value = Muon::nsw::STGTPL1A::version1_anchor_value;
   auto anchor = Muon::nsw::decode_at_loc<uint64_t>(m_data, readPointer, Muon::nsw::STGTPL1A::loc_version1_anchor, Muon::nsw::STGTPL1A::size_l1a_link_const);
   
   if (anchor == anchor_value)
   {
      return 1;
   }

   return -1;
}

void Muon::nsw::NSWTriggerSTGL1AElink::decode_data_v3(std::size_t& readPointer, int version) {
  
  auto PADDING_BITS_END = std::size_t{16};

  
  const auto endOfData = m_wordCountFlx * WORD_SIZE - Muon::nsw::STGTPL1A::size_trailer_CRC - PADDING_BITS_END;
  
  while (readPointer < endOfData) {
    static constexpr auto SIZE_DATA_HEADER = STGTPL1A::size_stream_head_nbits + STGTPL1A::size_stream_head_nwords +
                                             STGTPL1A::size_stream_head_fifo_size + STGTPL1A::size_stream_head_streamID;
    if (readPointer + SIZE_DATA_HEADER > endOfData) {
      throw std::length_error(
                              Muon::nsw::format("Read pointer ({}) would excede memory dedicated to data chunks ({}) while parsing the header (size: {})",
                                                readPointer, endOfData, SIZE_DATA_HEADER));
    }
    const auto header_data = decode_data_header_v3(readPointer, version);
    if (header_data.total_expected_size > m_wordCountFlx * WORD_SIZE    - readPointer + 1) {
      throw std::length_error(Muon::nsw::format("STG TP stream size {} larger than expected packet size {}",
                                          header_data.total_expected_size,
                                          m_wordCountFlx * WORD_SIZE - readPointer + 1));
    }
    if (header_data.nwords * header_data.data_size + readPointer > endOfData) {
      throw std::length_error(Muon::nsw::format("Requested to decode {} bits but only {} bits are remaining",
                                          header_data.nwords * header_data.data_size, endOfData - readPointer));
    }
    m_stream_data.push_back(decode_data_payload_v3(readPointer, header_data, version));
  }
  analyze_data_v3(version);
}

void Muon::nsw::NSWTriggerSTGL1AElink::decode_data(std::size_t& readPointer, int version) {
  if (version == 3)
  {
     return decode_data_v3(readPointer, version);
  }
  auto PADDING_BITS_END = std::size_t{16};

  const auto endOfData = m_wordCountFlx * WORD_SIZE - Muon::nsw::STGTPL1A::size_trailer_CRC - PADDING_BITS_END;
  while (readPointer < endOfData) {
    static constexpr auto SIZE_DATA_HEADER = STGTPL1A::size_stream_head_nbits + STGTPL1A::size_stream_head_nwords +
                                             STGTPL1A::size_stream_head_fifo_size + STGTPL1A::size_stream_head_streamID;
    if (readPointer + SIZE_DATA_HEADER > endOfData) {
      throw std::length_error(
			      Muon::nsw::format("Read pointer ({}) would excede memory dedicated to data chunks ({}) while parsing the header (size: {})",
						readPointer, endOfData, SIZE_DATA_HEADER));
    }
    const auto header_data = decode_data_header(readPointer, version);

    if (header_data.total_expected_size > m_wordCountFlx    - ceil(readPointer / WORD_SIZE ) + 1) {
      throw std::length_error(Muon::nsw::format("STG TP stream size {} larger than expected packet size {}",
                                          header_data.total_expected_size,
                                          m_wordCountFlx  - ceil(readPointer / WORD_SIZE ) + 1));
    }
    if (header_data.nwords * header_data.data_size + readPointer > endOfData) {
      throw std::length_error(Muon::nsw::format("Requested to decode {} bits but only {} bits are remaining",
                                          header_data.nwords * header_data.data_size, endOfData - readPointer));
    }

    m_stream_data.push_back(decode_data_payload(readPointer, header_data, version));
  }
  analyze_data(version);
}

Muon::nsw::NSWTriggerSTGL1AElink::DataHeader Muon::nsw::NSWTriggerSTGL1AElink::decode_data_header_v3(
    std::size_t& readPointer, int version) {
  

  if (version != 3){
  	throw std::invalid_argument("decode_data_header_v3 version should be exactly 3"); 
  }

  size_t felix_word_size =  WORD_SIZE_DOUBLE;
  auto PADDING_BITS_END = std::size_t{16};
  const auto stream_head_nbits = decode(readPointer, STGTPL1A::size_stream_head_nbits);
  const auto stream_head_nwords = decode(readPointer, STGTPL1A::size_stream_head_nwords);
  const auto current_stream_head_fifo_size = decode(readPointer, STGTPL1A::size_stream_head_fifo_size);
  const auto current_stream_head_streamID = decode(readPointer, STGTPL1A::size_stream_head_streamID);
  const auto total_expected_size = stream_head_nbits * stream_head_nwords;
  auto endOfData = m_wordCountFlx * felix_word_size - Muon::nsw::STGTPL1A::size_trailer_CRC - PADDING_BITS_END;
  size_t current_stream_head_nbits = 0;
  size_t current_stream_head_nwords = 0;

  switch (current_stream_head_streamID)
  {
        case STGTPPad::pad_stream_header:
           current_stream_head_nbits = STGTPPad::size_v3;
           break;
        case STGTPSegments::merge_stream_header:
           current_stream_head_nbits = STGTPSegments::size_v3;
           break;
        case STGTPMMData::mm_stream_header:
           current_stream_head_nbits = STGTPMMData::size_v3;
           break;
        default:
           break;
  }

  if (current_stream_head_nbits == 0 )
  {
      throw std::runtime_error( "Corrupted message - sTGC stream has an unrecognized stream header");
  } 
  
  if (readPointer +  total_expected_size > endOfData)
  {
     throw std::runtime_error("Corrupted message - sTGC expected size goes beyond the end of data");
  }

  current_stream_head_nwords = (stream_head_nwords == 0) ? 0 : total_expected_size / current_stream_head_nbits;
  size_t data_size = std::ceil(total_expected_size / felix_word_size);

  // in version 3 nbits correspond to either 16 or 32 bits. There is a stable packet size that we use and 
  // we extract it from the code. The 16 or 32 bits is a consequence of an optimization in the firmware that 
  // results in big gains wrt routing and resources
  m_stream_head_nbits.push_back(current_stream_head_nbits); 
  m_stream_head_nwords.push_back(current_stream_head_nwords);
  m_stream_head_fifo_size.push_back(current_stream_head_fifo_size);
  m_stream_head_streamID.push_back(current_stream_head_streamID);

   

  ERS_DEBUG(2, Muon::nsw::format("stream_head_nbits: {}", current_stream_head_nbits));
  ERS_DEBUG(2, Muon::nsw::format("stream_head_nwords: {}", current_stream_head_nwords));
  ERS_DEBUG(2, Muon::nsw::format("stream_head_fifo_size: {}", current_stream_head_fifo_size));
  ERS_DEBUG(2, Muon::nsw::format("stream_head_streamID: {}", current_stream_head_streamID));
  ERS_DEBUG(2, Muon::nsw::format("total_expected_size: {}", total_expected_size));
  ERS_DEBUG(2, Muon::nsw::format("m_wordCountFlx: {}, ceil(readPointer/{}): {}", m_wordCountFlx,felix_word_size,
                           ceil(readPointer / felix_word_size)));


  return {current_stream_head_nbits, current_stream_head_nwords, current_stream_head_fifo_size,
          current_stream_head_streamID,        total_expected_size,        data_size};
} 

Muon::nsw::NSWTriggerSTGL1AElink::DataHeader Muon::nsw::NSWTriggerSTGL1AElink::decode_data_header(
    std::size_t& readPointer, int version) {
  if (version >= 3){
      return decode_data_header_v3(readPointer, version);
  }
  const auto current_stream_head_nbits = decode(readPointer, STGTPL1A::size_stream_head_nbits);
  const auto current_stream_head_nwords = decode(readPointer, STGTPL1A::size_stream_head_nwords);
  const auto current_stream_head_fifo_size = decode(readPointer, STGTPL1A::size_stream_head_fifo_size);
  const auto current_stream_head_streamID = decode(readPointer, STGTPL1A::size_stream_head_streamID);

  // zero padding to multiples of 16bits - TP logic - this is the real number
  // of bits to read
  const auto corrected_current_stream_head_nbits = correct_size_for_padding(current_stream_head_nbits);

  m_stream_head_nbits.push_back(corrected_current_stream_head_nbits);
  m_stream_head_nwords.push_back(current_stream_head_nwords);
  m_stream_head_fifo_size.push_back(current_stream_head_fifo_size);
  m_stream_head_streamID.push_back(current_stream_head_streamID);

  size_t word_size =  WORD_SIZE_DOUBLE;
  const auto data_size = static_cast<std::size_t>(std::ceil(corrected_current_stream_head_nbits / word_size));
  const auto total_expected_size = data_size * current_stream_head_nwords;
  
  ERS_DEBUG(2, Muon::nsw::format("stream_head_nbits: {}", corrected_current_stream_head_nbits));
  ERS_DEBUG(2, Muon::nsw::format("stream_head_nwords: {}", current_stream_head_nwords));
  ERS_DEBUG(2, Muon::nsw::format("stream_head_fifo_size: {}", current_stream_head_fifo_size));
  ERS_DEBUG(2, Muon::nsw::format("stream_head_streamID: {}", current_stream_head_streamID));
  ERS_DEBUG(2, Muon::nsw::format("total_expected_size: {}", data_size * current_stream_head_nwords));
  ERS_DEBUG(2, Muon::nsw::format("m_wordCountFlx: {}, ceil(readPointer/{}): {}", m_wordCountFlx, word_size,
                           ceil(readPointer / word_size)));

  return {corrected_current_stream_head_nbits, current_stream_head_nwords, current_stream_head_fifo_size,
          current_stream_head_streamID,        total_expected_size,        data_size};
}

std::vector<std::vector<std::uint32_t>> Muon::nsw::NSWTriggerSTGL1AElink::decode_data_payload_v3(
    std::size_t& readPointer, const DataHeader& header, int version) const {
    std::vector<std::vector<std::uint32_t>> current_stream_data{};

  if (version < 3){
        throw std::invalid_argument("decode_data_header_v3 version should be at least 3");
  }

  size_t word_size = WORD_SIZE;
  size_t felix_n_words = (header.nwords == 0 ) ? 0 : std::ceil(header.data_size / header.nwords); // the number of felix words per stream word
  for (std::size_t i = 0; i < header.nwords; ++i) {
    std::vector<std::uint32_t> data{};
    for (std::size_t j = 0; j < felix_n_words; ++j) {
      data.push_back(decode(readPointer, word_size));
    }
    current_stream_data.push_back(std::move(data));
  }
  return current_stream_data;
}
std::vector<std::vector<std::uint32_t>> Muon::nsw::NSWTriggerSTGL1AElink::decode_data_payload(
    std::size_t& readPointer, const DataHeader& header, int version) const {
  std::vector<std::vector<std::uint32_t>> current_stream_data{};

  if (version >= 3)
     return decode_data_payload_v3( readPointer,  header,  version);
  size_t word_size = WORD_SIZE; 
  for (std::size_t i = 0; i < header.nwords; ++i) {
    std::vector<std::uint32_t> data{};
    for (std::size_t j = 0; j < header.data_size; ++j) {
      data.push_back(decode(readPointer, word_size));
    }
    current_stream_data.push_back(std::move(data));
  }
  return current_stream_data;
 
}

void Muon::nsw::NSWTriggerSTGL1AElink::decode_trailer(std::size_t& readPointer) {
  // TODO: warning: how the swROD is behaving if the last work is a uint16 only? Just 0-padding?
  m_trailer_CRC = decode(readPointer, Muon::nsw::STGTPL1A::size_trailer_CRC);
}

std::uint64_t Muon::nsw::NSWTriggerSTGL1AElink::decode(std::size_t& readPointer, const std::size_t size) const {
  return Muon::nsw::decode_and_advance<std::uint64_t, std::uint32_t>(m_data, readPointer, size);
}

void Muon::nsw::NSWTriggerSTGL1AElink::analyze_data_v3(int version) { 

  size_t word_size = WORD_SIZE_DOUBLE;
  auto counterChunk = std::size_t{0};
  for (const auto& dataChunk : m_stream_data) {
    for (const auto& dataWord : dataChunk) {
      const auto expectedSize =
          static_cast<std::size_t>(std::ceil(m_stream_head_nbits.at(counterChunk) / word_size));
      if (std::size(dataWord) != expectedSize) {
        throw std::length_error(Muon::nsw::format("Stream data size {} does not match expected number of messages {}",
                                            std::size(dataWord), expectedSize));
      }
      switch (m_stream_head_streamID.at(counterChunk)) {
        case STGTPPad::pad_stream_header:
          m_pad_packets.emplace_back(dataWord,version);
          break;
        case STGTPSegments::merge_stream_header:
          m_segment_packets.emplace_back(dataWord,version);
          break;
        case STGTPMMData::mm_stream_header:
          m_mm_packets.emplace_back(dataWord,version);
          break;
        case STGTPStrips::strip_stream_header:
          m_strip_packets.emplace_back(dataWord,version);
          break;
        default:
          throw std::runtime_error(Muon::nsw::format("Invalid stream type {}", m_stream_head_streamID.at(counterChunk)));
      }
    }
    ++counterChunk;
  }

}

void Muon::nsw::NSWTriggerSTGL1AElink::analyze_data(int version) {
  if (version >= 3){
     return analyze_data_v3(version);
  }

  size_t word_size = WORD_SIZE_DOUBLE;
  auto counterChunk = std::size_t{0};
  for (const auto& dataChunk : m_stream_data) {
    for (const auto& dataWord : dataChunk) {
      const auto expectedSize =
          static_cast<std::size_t>(std::ceil(m_stream_head_nbits.at(counterChunk) / word_size));
      if (std::size(dataWord) != expectedSize) {
        throw std::length_error(Muon::nsw::format("Stream data size {} does not match expected number of messages {}",
                                            std::size(dataWord), expectedSize));
      }
      switch (m_stream_head_streamID.at(counterChunk)) {
        case STGTPPad::pad_stream_header:
          m_pad_packets.emplace_back(dataWord,version);
          break;
        case STGTPSegments::merge_stream_header:
          m_segment_packets.emplace_back(dataWord,version);
          break;
        case STGTPStrips::strip_stream_header:
          m_strip_packets.emplace_back(dataWord,version);
          break;
        default:
          throw std::runtime_error(Muon::nsw::format("Invalid stream type {}", m_stream_head_streamID.at(counterChunk)));
      }
    }
    ++counterChunk;
  }
}

std::uint64_t Muon::nsw::NSWTriggerSTGL1AElink::correct_size_for_padding(const std::uint64_t initial) {
  static constexpr auto PADDING = 16;
  if (initial % PADDING) {
    return ((initial + PADDING - 1) / PADDING) * PADDING;
  } else {
    return initial;
  }
}
