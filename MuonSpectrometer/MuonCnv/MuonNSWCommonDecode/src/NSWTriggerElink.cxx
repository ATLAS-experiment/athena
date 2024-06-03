/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <vector>
#include <sstream>

#include "ers/ers.h"
#include <MuonNSWCommonDecode/NSWDecodeHelper.h>

#include "MuonNSWCommonDecode/NSWTriggerElink.h"

Muon::nsw::NSWTriggerElink::NSWTriggerElink (const uint32_t *bs, const uint32_t remaining) {

  // Felix header (2 words) - remember that main decoder interface is already checking that remaining>2 (so preventing a crash)
  // redoing it here since this class is supposed to be independent

  static const uint32_t min_packet_size = 2;
  if ( remaining < min_packet_size ) {
    Muon::nsw::NSWTriggerException e ( Muon::nsw::format("Bytestream shorter than minimum size: provided {}, expected {}", remaining, min_packet_size) , 1);
    throw e;
  }

  std::span<const std::uint32_t> data{bs, 2};
  m_packet_status = Muon::nsw::bit_slice<uint64_t>(data,0,15);
  m_wordCountFlx = Muon::nsw::bit_slice<uint64_t>(data,16,31);

  if ( m_packet_status != 0 ) {
    //later in commissioning, will decide if one can also accept statuses !=0 and can delegate to user checks (or maybe to the decoder upper level)
    Muon::nsw::NSWTriggerException e ( Muon::nsw::format("Packet status in FELIX header {}", m_packet_status), 2);
    throw e;
  }

  if ( remaining < m_wordCountFlx ) {
    Muon::nsw::NSWTriggerException e ( Muon::nsw::format("Packet length in FELIX header ({}) is larger than available data ({})", m_wordCountFlx, remaining), 3);
    throw e;
  }

  m_elinkWord = data[1];
  m_elinkId.reset(new Muon::nsw::NSWResourceId (m_elinkWord));

  m_wordCount=m_wordCountFlx; 
  //just trusting felix/swrod (TP packets have variables size in general; there's no expected size or prediction that can be done)

  ERS_DEBUG(2, Muon::nsw::format("\n FELIX header: \n"
				 "  status {}\n"
				 "  size {}\n"
				 "  id {}", 
				 m_packet_status, m_wordCountFlx, m_elinkWord));

}
