/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONNSWCOMMONDECODE_NSWTRIGGERSTGL1AELINK_H
#define MUONNSWCOMMONDECODE_NSWTRIGGERSTGL1AELINK_H

#include <cstdint>

#include <exception>
#include <vector>

#include "MuonNSWCommonDecode/NSWMMTPDecodeBitmaps.h"
#include "MuonNSWCommonDecode/NSWSTGTPDecodeBitmaps.h"
#include "MuonNSWCommonDecode/NSWTriggerElink.h"
#include "MuonNSWCommonDecode/STGTPPackets.h"

namespace Muon::nsw {
class NSWResourceId;

class NSWTriggerElinkException;

class NSWTriggerSTGL1AElink : public NSWTriggerElink {
 public:
  NSWTriggerSTGL1AElink(const std::uint32_t* bs, std::uint32_t remaining);
  virtual ~NSWTriggerSTGL1AElink() = default;

  std::uint32_t head_fragID() const { return m_head_fragID; };
  std::uint32_t head_sectID() const { return m_head_sectID; };
  std::uint32_t head_EC() const { return m_head_EC; };
  std::uint32_t head_flags() const { return m_head_flags; };
  std::uint32_t head_BCID() const { return m_head_BCID; };
  std::uint32_t head_orbit() const { return m_head_orbit; };
  std::uint32_t head_spare() const { return m_head_spare; };
  std::uint32_t L1ID() const { return m_L1ID; };
  
  std::uint32_t l1a_versionID () const {return m_l1a_versionID;};
  std::uint32_t l1a_local_req_BCID () const {return m_l1a_local_req_BCID;};
  std::uint32_t l1a_local_rel_BCID () const {return m_l1a_local_rel_BCID;};
  std::uint32_t l1a_open_BCID () const {return m_l1a_open_BCID;};
  std::uint32_t l1a_req_BCID () const {return m_l1a_req_BCID;};
  std::uint32_t l1a_close_BCID () const {return m_l1a_close_BCID;};
  std::uint32_t l1a_timeout () const {return m_l1a_timeout;};
  std::uint32_t head_overflowCount () const {return m_l1a_timeout;}; // compatibility version 1
  std::uint32_t l1a_open_BCID_offset () const {return m_l1a_open_BCID_offset;};
  std::uint32_t l1a_req_BCID_offset () const {return m_l1a_req_BCID_offset;};
  std::uint32_t l1a_close_BCID_offset () const {return m_l1a_close_BCID_offset;};
  std::uint32_t l1a_timeout_config () const {return m_l1a_timeout_config;};
  std::uint32_t l1a_busy_thr () const {return m_l1a_busy_thr;};
  std::uint32_t l1a_engine_snapshot () const {return m_l1a_engine_snapshot;};
  std::uint32_t l1a_link_const () const {return m_l1a_link_const;};
  std::uint32_t l1a_padding () const {return m_l1a_padding;};

  std::uint32_t head_wdw_matching_engines_usage () const {return m_l1a_wdw_matching_engines_usage; };
  
  const std::vector<std::uint32_t>& stream_head_nbits() const { return m_stream_head_nbits; };
  const std::vector<std::uint32_t>& stream_head_nwords() const { return m_stream_head_nwords; };
  const std::vector<std::uint32_t>& stream_head_fifo_size() const { return m_stream_head_fifo_size; };
  const std::vector<std::uint32_t>& stream_head_streamID() const { return m_stream_head_streamID; };
  const std::vector<std::vector<std::vector<std::uint32_t>>> stream_data() const { return m_stream_data; };
  std::uint32_t trailer_CRC() const { return m_trailer_CRC; };

  const std::vector<STGTPPadPacket>& pad_packets() const { return m_pad_packets; };
  const std::vector<STGTPSegmentPacket>& segment_packet() const { return m_segment_packets; };
  const std::vector<STGTPMMPacket>& mm_packet() const { return m_mm_packets; };
  const std::vector<STGTPStripPacket>& strip_packet() const { return m_strip_packets; };
  

 private:
  /**
   * @brief Helper struct to hold information of the header for each data chunk
   *
   * Contains information about type, number of words and bits per word
   */
  struct DataHeader {
    std::uint64_t nbits{};
    std::uint64_t nwords{};
    std::uint64_t fifo_size{};
    std::uint64_t streamID{};
    std::size_t total_expected_size{};
    std::size_t data_size{};
  };

  /**
  * @brief parse version workaround
  * @param std::size_t& readPointer
  * @return version
  *
  */
  int parse_version_workaround(std::size_t& readPointer);

  /**
   * @brief Decode the header
   *
   * @param readPointer Current read pointer position (updated by function)
   */
  void decode_header(std::size_t& readPointer, int& version);
  void decode_header_v3(std::size_t& readPointer, int& version);
  /**
   * @brief Decode the pad and segment data
   *
   * @param readPointer Current read pointer position (updated by function)
   */
  void decode_data(std::size_t& readPointer, int version);
  void decode_data_v3(std::size_t& readPointer, int version);

  /**
   * @brief Decode the header of each data segment
   *
   * Contains information about type, number of words and bits per word
   *
   * @param readPointer Current read pointer position (updated by function)
   * @return DataHeader Information from header
   */
  DataHeader decode_data_header(std::size_t& readPointer, int version);
  DataHeader decode_data_header_v3(std::size_t& readPointer, int version);
  /**
   * @brief Decode the payload of each data segment
   *
   * Contains actual data from pad or segments
   *
   * @param readPointer Current read pointer position (updated by function)
   * @param header Information from data header
   * @return DataHeader Information from header
   */
  [[nodiscard]] std::vector<std::vector<std::uint32_t>> decode_data_payload(std::size_t& readPointer,
                                                                            const DataHeader& header, int version) const;
  [[nodiscard]] std::vector<std::vector<std::uint32_t>> decode_data_payload_v3(std::size_t& readPointer,
                                                                            const DataHeader& header, int version) const;
  /**
   * @brief Analyze data chunks and create decoded objects
   */
  void analyze_data(int version);

  void analyze_data_v3(int version);
  /**
   * @brief Decode the trailer
   *
   * @param readPointer Current read pointer position (updated by function)
   */
  void decode_trailer(std::size_t& readPointer);

  /**
   * @brief Decode a value
   *
   * @param readPointer Current read pointer position (updated by function)
   * @param size Size of the value to be decoded
   * @return std::uint64_t Decoded value
   */
  [[nodiscard]] std::uint64_t decode(std::size_t& readPointer, std::size_t size) const;
  [[nodiscard]] std::uint64_t decode_v3(std::size_t& readPointer, std::size_t size) const;
  [[nodiscard]] static std::uint64_t correct_size_for_padding(std::uint64_t initial);

  std::span<const std::uint32_t> m_data;
  static constexpr auto WORD_SIZE = sizeof(decltype(m_data)::element_type) * 8;
  static constexpr auto WORD_SIZE_DOUBLE = static_cast<double>(WORD_SIZE);

  std::uint32_t m_head_fragID{};
  std::uint32_t m_head_sectID{};
  std::uint32_t m_head_EC{};
  std::uint32_t m_head_flags{};
  std::uint32_t m_head_BCID{};
  std::uint32_t m_head_orbit{};
  std::uint32_t m_head_spare{};
  
  std::uint32_t m_L1ID{};


  std::uint32_t m_l1a_versionID{};
  std::uint32_t m_l1a_local_req_BCID{};
  std::uint32_t m_l1a_local_rel_BCID{};
  std::uint32_t m_l1a_open_BCID{};
  std::uint32_t m_l1a_req_BCID{};
  std::uint32_t m_l1a_close_BCID{};
  std::uint32_t m_l1a_timeout{};
  std::uint32_t m_l1a_open_BCID_offset{};
  std::uint32_t m_l1a_req_BCID_offset{};
  std::uint32_t m_l1a_close_BCID_offset{};
  std::uint32_t m_l1a_timeout_config{};
  std::uint32_t m_l1a_busy_thr{};
  std::uint32_t m_l1a_engine_snapshot{};
  std::uint32_t m_l1a_link_const{};
  std::uint32_t m_l1a_padding{};

  std::uint32_t m_l1a_wdw_matching_engines_usage{};

  std::vector<std::uint32_t> m_stream_head_nbits;
  std::vector<std::uint32_t> m_stream_head_nwords;
  std::vector<std::uint32_t> m_stream_head_fifo_size;
  std::vector<std::uint32_t> m_stream_head_streamID;
  std::vector<std::vector<std::vector<std::uint32_t>>> m_stream_data;  // size is potentially not known a priori...
  // first vector had stream index
  // second vector contains stream data words - length defined by m_stream_head_nwords
  // third vector used because stream data size (m_stream_head_nwords) can exceed maximum compiler size (uint64_t)
  std::uint32_t m_trailer_CRC{};

  std::vector<STGTPPadPacket> m_pad_packets;
  std::vector<STGTPSegmentPacket> m_segment_packets;
  std::vector<STGTPMMPacket> m_mm_packets;
  std::vector<STGTPStripPacket> m_strip_packets;

  int m_packet_version;
};
}  // namespace Muon::nsw

#endif  // MUONNSWCOMMONDECODE_NSWTRIGGERSTGL1AELINK_H
