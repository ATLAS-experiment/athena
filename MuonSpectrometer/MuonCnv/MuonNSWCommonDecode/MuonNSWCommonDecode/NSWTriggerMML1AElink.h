/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef _MUON_NSW_TRIGGER_MML1A_ELINK_H_
#define _MUON_NSW_TRIGGER_MML1A_ELINK_H_

#include "MuonNSWCommonDecode/NSWTriggerElink.h"
#include <stdint.h>
#include <vector>
#include <memory>

namespace Muon
{
  namespace nsw
  {
    class NSWResourceId;

    class NSWTriggerElinkException;

    class MMARTPacket;

    class MMTrigPacket;

    class NSWTriggerMML1AElink : public NSWTriggerElink
    {
     public:

      NSWTriggerMML1AElink (const uint32_t *bs, uint32_t remaining);
      virtual ~NSWTriggerMML1AElink () = default;


      uint32_t head_fragID () const {return m_head_fragID;};
      uint32_t head_sectID () const {return m_head_sectID;};
      uint32_t head_EC () const {return m_head_EC;};
      uint32_t head_flags () const {return m_head_flags;};
      uint32_t head_BCID () const {return m_head_BCID;};
      uint32_t head_orbit () const {return m_head_orbit;};
      uint32_t head_spare () const {return m_head_spare;};
      uint32_t L1ID () const {return m_L1ID;};

      uint32_t l1a_versionID () const {return m_l1a_versionID;};
      uint32_t l1a_local_req_BCID () const {return m_l1a_local_req_BCID;};
      uint32_t l1a_local_rel_BCID () const {return m_l1a_local_rel_BCID;};
      uint32_t l1a_open_BCID () const {return m_l1a_open_BCID;};
      uint32_t l1a_req_BCID () const {return m_l1a_req_BCID;};
      uint32_t l1a_close_BCID () const {return m_l1a_close_BCID;};
      uint32_t l1a_timeout () const {return m_l1a_timeout;};
      uint32_t l1a_open_BCID_offset () const {return m_l1a_open_BCID_offset;};
      uint32_t l1a_req_BCID_offset () const {return m_l1a_req_BCID_offset;};
      uint32_t l1a_close_BCID_offset () const {return m_l1a_close_BCID_offset;};
      uint32_t l1a_timeout_config () const {return m_l1a_timeout_config;};
      uint32_t l1a_busy_thr () const {return m_l1a_busy_thr;};
      uint32_t l1a_engine_snapshot () const {return m_l1a_engine_snapshot;};
      uint32_t l1a_link_const () const {return m_l1a_link_const;};
      uint32_t l1a_padding () const {return m_l1a_padding;};

      const std::vector<uint32_t>& stream_head_nbits () const {return m_stream_head_nbits;};
      const std::vector<uint32_t>& stream_head_nwords () const {return m_stream_head_nwords;};
      const std::vector<uint32_t>& stream_head_fifo_size () const {return m_stream_head_fifo_size;};
      const std::vector<uint32_t>& stream_head_streamID () const {return m_stream_head_streamID;};
      const std::vector<std::vector<std::vector<uint32_t>>>& stream_data () const {return m_stream_data;};

      uint32_t trailer_CRC () const {return m_trailer_CRC;};

      uint32_t trailer_CRC_calc () const {return m_trailer_CRC_calc;};
      bool is_crc_ok () const {return m_trailer_CRC == m_trailer_CRC_calc;};

      const std::vector<std::shared_ptr<Muon::nsw::MMARTPacket>>& art_packets () const {return m_art_packets;};
      const std::vector<std::shared_ptr<Muon::nsw::MMTrigPacket>>& trig_packets () const {return m_trig_packets;};

     private:

      uint32_t m_head_fragID;
      uint32_t m_head_sectID;
      uint32_t m_head_EC;
      uint32_t m_head_flags;
      uint32_t m_head_BCID;
      uint32_t m_head_orbit;
      uint32_t m_head_spare;
      uint32_t m_L1ID;

      uint32_t m_l1a_versionID;
      uint32_t m_l1a_local_req_BCID;
      uint32_t m_l1a_local_rel_BCID;
      uint32_t m_l1a_open_BCID;
      uint32_t m_l1a_req_BCID;
      uint32_t m_l1a_close_BCID;
      uint32_t m_l1a_timeout;
      uint32_t m_l1a_open_BCID_offset;
      uint32_t m_l1a_req_BCID_offset;
      uint32_t m_l1a_close_BCID_offset;
      uint32_t m_l1a_timeout_config;
      uint32_t m_l1a_busy_thr;
      uint32_t m_l1a_engine_snapshot;
      uint32_t m_l1a_link_const;
      uint32_t m_l1a_padding;

      std::vector<uint32_t> m_stream_head_nbits;
      std::vector<uint32_t> m_stream_head_nwords;
      std::vector<uint32_t> m_stream_head_fifo_size;
      std::vector<uint32_t> m_stream_head_streamID;
      std::vector<std::vector<std::vector<uint32_t>>> m_stream_data; //size is potentially not known a priori...
      //first vector had stream index
      //second vector contains stream data words - length defined by m_stream_head_nwords
      //third vector used because stream data size (m_stream_head_nwords) can exceed maximum compiler size (uint64_t)
      //CRC are actually 16b for now but keeping 32b for future proofing
      uint32_t m_trailer_CRC;
      uint32_t m_trailer_CRC_calc; //calculated CRC

      std::vector<std::shared_ptr<Muon::nsw::MMARTPacket>> m_art_packets;
      std::vector<std::shared_ptr<Muon::nsw::MMTrigPacket>> m_trig_packets;

    };
  }
}


#endif // _MUON_NSW_TRIGGER_MML1A_ELINK_H_
