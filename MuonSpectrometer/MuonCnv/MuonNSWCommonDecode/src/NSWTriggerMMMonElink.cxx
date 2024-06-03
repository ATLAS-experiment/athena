/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <vector>
#include <exception>
#include <sstream>
#include <string>

#include "MuonNSWCommonDecode/NSWMMTPDecodeBitmaps.h"

#include "MuonNSWCommonDecode/NSWTriggerElink.h"
#include "MuonNSWCommonDecode/NSWTriggerMMMonElink.h"
#include "MuonNSWCommonDecode/NSWResourceId.h"

Muon::nsw::NSWTriggerMMMonElink::NSWTriggerMMMonElink (const uint32_t *bs, const uint32_t remaining):
  NSWTriggerElink (bs, remaining)
{
 
  std::size_t size_word{sizeof(uint32_t) * 8};
  // 2 felix header 32b words already decoded;
  std::size_t readPointer{2 * 32};
  std::span<const std::uint32_t> data{bs, remaining};
  //once format finalized, checking a minimum size or at least the structure

  m_head_fragID =         Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPMON::size_head_fragID	     );
  m_head_sectID =         Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPMON::size_head_sectID	     );
  m_head_EC =             Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPMON::size_head_EC	     );
  m_head_flags =          Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPMON::size_head_flags	     );
  m_head_BCID =           Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPMON::size_head_BCID	     );
  m_head_orbit =          Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPMON::size_head_orbit	     );
  m_head_spare =          Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPMON::size_head_spare	     );
  m_L1ID =                Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPMON::size_L1ID    	     );
  m_head_coincBCID =      Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPMON::size_head_coincBCID   );
  m_head_regionCount =    Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPMON::size_head_regionCount );
  m_head_coincRegion =    Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPMON::size_head_coincRegion );
  m_head_reserved =       Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPMON::size_head_reserved    );

  //already checked in NSWTriggerElink that remaining >= m_wordCountFlx
  while ( readPointer < (m_wordCountFlx-1) * size_word ) {
    //later during commissioning, need to change to ( readPointer < (m_wordCountFlx-1-stream_block_size) * size_word )

    uint32_t current_streamID =     Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPMON::size_finder_streamID    ); 
    uint32_t current_regionCount =  Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPMON::size_finder_regionCount );
    uint32_t current_triggerID =    Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPMON::size_finder_triggerID   );
    
    if (current_streamID == 0b10110001){
      //finder
      m_finder_streamID.push_back(current_streamID);
      m_finder_regionCount.push_back(current_regionCount);
      m_finder_triggerID.push_back(current_triggerID);
      m_finder_V1.push_back(Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPMON::size_finder_V1));
      m_finder_V0.push_back(Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPMON::size_finder_V0));
      m_finder_U1.push_back(Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPMON::size_finder_U1));
      m_finder_U0.push_back(Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPMON::size_finder_U0));
      m_finder_X3.push_back(Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPMON::size_finder_X3));
      m_finder_X2.push_back(Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPMON::size_finder_X2));
      m_finder_X1.push_back(Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPMON::size_finder_X1));
      m_finder_X0.push_back(Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPMON::size_finder_X0));

    } else if (current_streamID == 0b10110010) {
      //fitter
      m_fitter_streamID.push_back(current_streamID);
      m_fitter_regionCount.push_back(current_regionCount);
      m_fitter_triggerID.push_back(current_triggerID);
      m_fitter_filler.push_back(Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPMON::size_fitter_filler));
      m_fitter_mxG.push_back(Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPMON::size_fitter_mxG));
      m_fitter_muG.push_back(Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPMON::size_fitter_muG));
      m_fitter_mvG.push_back(Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPMON::size_fitter_mvG));
      m_fitter_mxL.push_back(Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPMON::size_fitter_mxL));
      m_fitter_mx_ROI.push_back(Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPMON::size_fitter_mx_ROI));
      m_fitter_dTheta.push_back(Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPMON::size_fitter_dTheta));
      m_fitter_zero.push_back(Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPMON::size_fitter_zero));
      m_fitter_phiSign.push_back(Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPMON::size_fitter_phiSign));
      m_fitter_phiBin.push_back(Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPMON::size_fitter_phiBin));
      m_fitter_rBin.push_back(Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPMON::size_fitter_rBin));


    } else { 
      Muon::nsw::NSWTriggerException e ( Muon::nsw::format("Stream ID in MMTP Monitoring packet now recognized: {}", current_streamID), 4);
      throw e;
    }

  }

  //warning: how the swROD is behaving if the last work is a uint16 only? Just 0-padding
  m_trailer_CRC = Muon::nsw::decode_and_advance<uint64_t>(data, readPointer, Muon::nsw::MMTPMON::size_trailer_CRC);


}
