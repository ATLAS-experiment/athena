/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <vector>
#include <exception>
#include <sstream>
#include <string>

#include "MuonNSWCommonDecode/NSWMMTPDecodeBitmaps.h"

#include "MuonNSWCommonDecode/NSWTriggerElink.h"
#include "MuonNSWCommonDecode/NSWTriggerMML1AElink.h"
#include "MuonNSWCommonDecode/NSWResourceId.h"
#include "MuonNSWCommonDecode/MMARTPacket.h"
#include "MuonNSWCommonDecode/MMTrigPacket.h"
#include "ers/ers.h"

Muon::nsw::NSWTriggerMML1AElink::NSWTriggerMML1AElink (const uint32_t *bs, const uint32_t remaining):
  NSWTriggerElink (bs, remaining)
{

  std::size_t size_word{sizeof(uint32_t) * 8};
  // 2 felix header 32b words already decoded
  std::size_t readPointer{2 * 32};
  std::span<const std::uint32_t> data{bs, remaining};
  //once format finalized, checking a minimum size or at least the structure

  m_head_fragID = Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPL1A::size_head_fragID);
  m_head_sectID = Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPL1A::size_head_sectID);
  m_head_EC =     Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPL1A::size_head_EC);
  m_head_flags =  Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPL1A::size_head_flags);
  m_head_BCID =   Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPL1A::size_head_BCID);
  m_head_orbit =  Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPL1A::size_head_orbit);
  m_head_spare =  Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPL1A::size_head_spare);
  m_L1ID =        Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPL1A::size_L1ID);

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

  m_l1a_versionID =         Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPL1A::size_l1a_versionID);
  m_l1a_local_req_BCID =    Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPL1A::size_l1a_local_req_BCID);
  m_l1a_local_rel_BCID =    Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPL1A::size_l1a_local_rel_BCID);
  m_l1a_open_BCID =         Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPL1A::size_l1a_open_BCID);
  m_l1a_req_BCID =          Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPL1A::size_l1a_req_BCID);
  m_l1a_close_BCID =        Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPL1A::size_l1a_close_BCID);
  m_l1a_timeout =           Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPL1A::size_l1a_timeout);
  m_l1a_open_BCID_offset =  Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPL1A::size_l1a_open_BCID_offset);
  m_l1a_req_BCID_offset =   Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPL1A::size_l1a_req_BCID_offset);
  m_l1a_close_BCID_offset = Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPL1A::size_l1a_close_BCID_offset);
  m_l1a_timeout_config =    Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPL1A::size_l1a_timeout_config);
  m_l1a_busy_thr =          Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPL1A::size_l1a_busy_thr);
  m_l1a_engine_snapshot =   Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPL1A::size_l1a_engine_snapshot);
  m_l1a_link_const =        Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPL1A::size_l1a_link_const);
  m_l1a_padding =           Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPL1A::size_l1a_padding);
  //m_l1a_link_const is important! It identifies the elink! It should be ABCD1230/ABCD1231/ABCD1232
  //not checked during decoding but must be checked at some point

  //already checked in NSWTriggerElink that remaining >= m_wordCountFlx 
  while ( readPointer < (m_wordCountFlx-1) * size_word ) {
    //later during commissioning, need to change to ( readPointer < (m_wordCountFlx-1-stream_block_size) * size_word )
    //later during commissioning, checking if there's at least enough space for the stream header and stream content 
    //(protected for now by a generic expression in bit_slice, just want to be more specific)
    
    uint32_t current_stream_head_nbits =     Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPL1A::size_stream_head_nbits);
    uint32_t current_stream_head_nwords =    Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPL1A::size_stream_head_nwords);
    uint32_t current_stream_head_fifo_size = Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPL1A::size_stream_head_fifo_size);
    uint32_t current_stream_head_streamID =  Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPL1A::size_stream_head_streamID);

    ERS_DEBUG(2, Muon::nsw::format("\n Stream header: \n"
				   "  nbits:       {}\n"
				   "  nwords:      {}\n"
				   "  fifo_size:   {}\n"
				   "  streamID:    {}",
				   current_stream_head_nbits, current_stream_head_nwords, current_stream_head_fifo_size, current_stream_head_streamID));


    //zero padding to multiples of 16bits - TP logic - this is the real number of bits to read
    current_stream_head_nbits = current_stream_head_nbits%16? ((current_stream_head_nbits+15)/16)*16 : current_stream_head_nbits;

    m_stream_head_nbits.push_back ( current_stream_head_nbits );
    m_stream_head_nwords.push_back ( current_stream_head_nwords );
    m_stream_head_fifo_size.push_back ( current_stream_head_fifo_size );
    m_stream_head_streamID.push_back ( current_stream_head_streamID );

    int current_stream_head_n32b_per_word = (current_stream_head_nbits-1)/32 +1; //how many 32b words needed for a art/trigger packet
    std::vector<std::vector<uint32_t>> current_stream_data;

    //this block is very generic and agnostic of MMTP stream data format!
    //only assumption: art/trigger packet size is multiple of 32b --- SF FIX BEFORE MR!
    for (uint i = 0; i<current_stream_head_nwords; i++){
      std::vector<uint32_t> current_stream_word;
      for (int j = 0; j < current_stream_head_n32b_per_word; j++){
	current_stream_word.push_back( Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, size_word) );
      }
      current_stream_data.push_back(current_stream_word);
    }
    m_stream_data.push_back(current_stream_data);

  }

  for (uint i=0; i<m_stream_data.size(); i++){
    
    switch (m_stream_head_streamID[i]) {
    case 0xAAA0:
    case 0xAAA1:
    case 0xAAA2:
    case 0xAAA3:
      for (uint j = 0; j < m_stream_data[i].size(); j++){
        if ( m_stream_data[i][j].size()!=3 ){
	  Muon::nsw::NSWTriggerException e ( Muon::nsw::format( "Stream ID 0xAAA[0-3] packet length not correct: expected 3, got ({})", m_stream_data[i][j].size() ), 6);
          throw e;
        }
        m_art_packets.push_back( std::make_shared<Muon::nsw::MMARTPacket>(m_stream_data[i][j]) ); //arg will be a vector<uint32_t> of size 3
      }
      break;
    case 0xAAA4:
      for (uint j = 0; j < m_stream_data[i].size(); j++){
        if ( m_stream_data[i][j].size()!=2 ){
	  Muon::nsw::NSWTriggerException e ( Muon::nsw::format( "Stream ID 0xAAA4 packet length not correct: expected 2, got ({})", m_stream_data[i][j].size() ), 7);
          throw e;
        }
        m_trig_packets.push_back( std::make_shared<Muon::nsw::MMTrigPacket>(m_stream_data[i][j]) ); //arg will be a vector<uint32_t> of size 2
      }
      break;
    default:
      Muon::nsw::NSWTriggerException e ( Muon::nsw::format("Stream ID in MMTP L1A not recognized: {}", m_stream_head_streamID[i]), 5);
      throw e;
    }
    
  }

  // warning: swROD is zero padding the last word to 32b
  // this means that there can be a leftover 16b 0x0000 after the CRC
  // m_wordCountFlx is anyway "aware" of this
  m_trailer_CRC = Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPL1A::size_trailer_CRC);

  // calculate CRC by hand now - need to exclude the FEC header (2 words)
  m_trailer_CRC_calc = Muon::nsw::get_16bxor_crc(bs+2, m_wordCountFlx-2);

}
