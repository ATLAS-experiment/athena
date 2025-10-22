/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef _MUON_NSW_STGTP_DECODE_BITMAPS_H_
#define _MUON_NSW_STGTP_DECODE_BITMAPS_H_

#include "MuonNSWCommonDecode/NSWDecodeHelper.h"
#include <stdexcept>
#include <cstddef>

namespace Muon
{
  namespace nsw
  {
    namespace STGTPL1A {

      constexpr int size_head_fragID =                        4;
      constexpr int size_head_sectID =                        4;
      constexpr int size_head_EC =                            1;
      constexpr int size_head_flags =                         7;
      constexpr int size_head_BCID =                         12;
      constexpr int size_head_orbit =                         2;
      constexpr int size_head_spare =                         2;
      constexpr int size_L1ID =                              32;

      constexpr int size_l1a_versionID =          8; // version 2+
      constexpr int size_l1a_local_req_BCID =    12;
      constexpr int size_l1a_local_rel_BCID =    12;
      constexpr int size_l1a_open_BCID =         12;
      constexpr int size_l1a_req_BCID =          12;
      constexpr int size_l1a_close_BCID =        12;
      constexpr int size_l1a_timeout =           12;
      constexpr int size_l1a_open_BCID_offset =  12;
      constexpr int size_l1a_req_BCID_offset =   12;
      constexpr int size_l1a_close_BCID_offset = 12;
      constexpr int size_l1a_timeout_config =    12;
      constexpr int size_l1a_busy_thr =           8; // version 2+
      constexpr int size_l1a_engine_snapshot =   16; // version 2+
      constexpr int size_l1a_link_const =        32;
      constexpr int size_l1a_padding =            8; // version 2+


      constexpr int size_l1a_wdw_matching_engines_usage =   32; // this is only present in version 1
      constexpr int loc_version1_anchor =        192;
      constexpr uint32_t version1_anchor_value =      0xABC0ABC1;   


 
      constexpr int size_stream_head_nbits =                 16;
      constexpr int size_stream_head_nwords =                16;
      constexpr int size_stream_head_fifo_size =             16;
      constexpr int size_stream_head_streamID =              16;
      constexpr int size_trailer_CRC =                       16;
      

    };

    namespace STGTPPad {
      constexpr std::size_t num_pads =                                4;
      constexpr std::size_t pad_stream_header =                      0xAAD0;
      constexpr std::size_t n_words =                                 3; // size in 32 bit words
      constexpr std::size_t size_coincidence_wedge =                 16;
      constexpr std::size_t size_phiID =                              6;
      constexpr std::size_t size_bandID =                             8;
      constexpr std::size_t size_BCID =                              12;
      constexpr std::size_t size_spare =                              3;
      constexpr std::size_t size_idleFlag =                           1;
      constexpr std::size_t size_padding =                            8;
      constexpr std::size_t size_v2_padding =                         40;
      constexpr std::size_t size_v3_padding =                         40;
      constexpr std::size_t size_v1         =                         96;
      constexpr std::size_t size_v2         =                         96;
      constexpr std::size_t size_v3         =                         128;
      constexpr int         pad_stream_invalid_band_id =              0xFF; 
    };

    namespace STGTPMMData {
       constexpr std::size_t num_mm =                    8;
       constexpr int mm_stream_header  =              0xC0CA;
       constexpr int size_output_mm_valids =              8;
       constexpr int size_output_mm_monitor =             1;
       constexpr int size_output_mm_spare =               2;
       constexpr int size_output_mm_lowRes =              1;
       constexpr int size_output_mm_phiRes =              1;
       constexpr int size_output_mm_dTheta =              5;
       constexpr int size_output_mm_phiID =               6;
       constexpr int size_output_mm_rIndex =              8;

       constexpr int size_bcid             =                   12;
       constexpr int size_v3_padding       =                   44; // now we have the valid bits

       constexpr std::size_t size_v1         =                   0;
       constexpr std::size_t size_v2         =                   0;
       constexpr std::size_t size_v3         =                   256;
       constexpr int         mm_stream_invalid_dTheta =          16;

    };    
 
    namespace STGTPSegments {
       constexpr std::size_t num_segments =                    8;

       constexpr int merge_stream_header  =              0xAEE0;
       constexpr int n_words =                                32; 
       constexpr int size_lut_choice_selection =              24;
       constexpr int size_nsw_segment_selector =              12;
       constexpr int size_valid_segment_selector =            12;
       
       constexpr int size_output_segment_monitor =             1;
       constexpr int size_output_segment_spare =               2;
       constexpr int size_output_segment_lowRes =              1;
       constexpr int size_output_segment_phiRes =              1;
       constexpr int size_output_segment_dTheta =              5;
       constexpr int size_output_segment_phiID =               6;
       constexpr int size_output_segment_rIndex =              8;

       constexpr int size_bcid             =                   12;
       constexpr int size_sectorID         =                   4;
       constexpr int size_v3_padding       =                   28;
       constexpr std::size_t size_v1       =                   256;
       constexpr std::size_t size_v2       =                   256;
       constexpr std::size_t size_v3       =                   256;
       constexpr std::size_t size_valid_segment_v3 =             8;
       constexpr int         merge_stream_invalid_dTheta =       16;

      enum class MergedSegmentProperty {
          Monitor = fill_bitmask<uint32_t>(0, size_output_segment_monitor),
          Spare = fill_bitmask<uint32_t>(max_bit(Monitor) + 1, size_output_segment_spare),
          lowRes = fill_bitmask<uint32_t>(max_bit(Spare) + 1, size_output_segment_lowRes),
          phiRes = fill_bitmask<uint32_t>(max_bit(lowRes) + 1 , size_output_segment_phiRes),
          dTheta = fill_bitmask<uint32_t>(max_bit(phiRes)+ 1, size_output_segment_dTheta),
          phiID = fill_bitmask<uint32_t>(max_bit(dTheta)+1 , size_output_segment_phiID),
          rIndex = fill_bitmask<uint32_t>(max_bit(phiID)+1, size_output_segment_rIndex)
      };
      
      constexpr uint32_t getSegmentProperty(const uint32_t mask, const MergedSegmentProperty prop) {
          const auto shift = min_bit(static_cast<uint32_t>(prop));
          if (shift < 0) {
            throw std::runtime_error("bitshift is negative in NSWSTGTPDecodeBitmaps getSegmentProperty");
          }
          return (mask & static_cast<uint32_t>(prop) ) >> shift;
      }
      
      constexpr void encodeSegmentProperty(const MergedSegmentProperty prop, const uint32_t word, uint32_t& buffer) {
          const auto shift = min_bit(static_cast<uint32_t>(prop));
          if (shift < 0) {
            throw std::runtime_error("bitshift is negative in NSWSTGTPDecodeBitmaps encodeSegmentProperty");
          }
          uint32_t shifted_word = (word << shift);
          buffer = (buffer) | (shifted_word & static_cast<uint32_t>(prop));
      }
      
      namespace moduleIDBits{
          /// Large or Small wedge
          constexpr uint8_t stationID = 1;
          /// Side 0 for A / 1 for C
          constexpr uint8_t detectorSite = 1;
          /// 1 to 3 
          constexpr uint8_t stationEta = 4;
          /// station Phi 1 to 8
          constexpr uint8_t stationPhi = 8;
      }
      enum class ModuleIDProperty{
          stationID = fill_bitmask<uint32_t>(0, moduleIDBits::stationID),
          detectorSite = fill_bitmask<uint32_t>(max_bit(stationID) +1, moduleIDBits::detectorSite),
          stationEta = fill_bitmask<uint32_t>(max_bit(detectorSite) + 1, moduleIDBits::stationEta),
          stationPhi = fill_bitmask<uint32_t>(max_bit(stationEta) + 1, moduleIDBits::stationPhi),
      };
      constexpr uint32_t getIdentifierProperty(const uint32_t mask, const ModuleIDProperty prop) {
        const auto shift = min_bit(static_cast<uint32_t>(prop));
        if (shift < 0) {
          throw std::runtime_error("bitshift is negative in NSWSTGTPDecodeBitmaps getIdentifierProperty");
        }
        return (mask &  static_cast<uint32_t>(prop)) >> shift;
        
      }
      constexpr void encodeIdentifierProperty(const ModuleIDProperty prop, const uint32_t word, uint32_t& buffer) {
          const auto shift = min_bit(static_cast<uint32_t>(prop));
          if (shift < 0) {
            throw std::runtime_error("bitshift is negative in NSWSTGTPDecodeBitmaps encodeIdentifierProperty");
          }
          uint32_t shifted_word = (word << shift);
          buffer = (buffer) | (shifted_word & static_cast<uint32_t>(prop));
      }
    }

    namespace STGTPStrips {
       constexpr std::size_t num_strips =                     112;
       constexpr std::size_t num_offsets =                      8;

       constexpr int strip_stream_header  =              0xC01A;
       constexpr int size_padding =                           70;
       constexpr int size_strip_adc =                          6;
       constexpr int size_offset =                            16;
       constexpr int size_phi_id_value =                       5;
       constexpr int size_phi_id_sign =                        1;
       constexpr int size_band_id =                            8;
       constexpr int size_bcid =                              12;

       constexpr std::size_t size_v1 =                       896;
       constexpr std::size_t size_v2 =                       896;
       constexpr std::size_t size_v3 =                       896;
    }
  }
}

#endif // _MUON_NSW_STGTP_DECODE_BITMAPS_H_
