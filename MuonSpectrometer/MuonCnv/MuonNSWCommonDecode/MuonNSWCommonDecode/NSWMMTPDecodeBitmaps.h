/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef _MUON_NSW_MMTP_DECODE_BITMAPS_H_
#define _MUON_NSW_MMTP_DECODE_BITMAPS_H_
#include <cstdio>
#include <cinttypes>
#include <MuonNSWCommonDecode/NSWDecodeHelper.h>

namespace Muon
{
  namespace nsw
  {
    namespace MMTPL1A {
      constexpr int size_head_fragID =            4;
      constexpr int size_head_sectID =            4;
      constexpr int size_head_EC =                1;
      constexpr int size_head_flags =             7;
      constexpr int size_head_BCID =             12;
      constexpr int size_head_orbit =             2;
      constexpr int size_head_spare =             2;
      constexpr int size_L1ID =                  32;
      constexpr int size_l1a_versionID =          8;
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
      constexpr int size_l1a_busy_thr =           8;
      constexpr int size_l1a_engine_snapshot =   16;
      constexpr int size_l1a_link_const =        32;
      constexpr int size_l1a_padding =            8;
      constexpr int size_stream_head_nbits =     16;
      constexpr int size_stream_head_nwords =    16;
      constexpr int size_stream_head_fifo_size = 16;
      constexpr int size_stream_head_streamID =  16;
      constexpr int size_trailer_CRC =           16;
    };

    namespace MMTPMON {
      constexpr int size_head_fragID =         4;
      constexpr int size_head_sectID =         4;
      constexpr int size_head_EC =             1;
      constexpr int size_head_flags =          7;
      constexpr int size_head_BCID =          12;
      constexpr int size_head_orbit =          2;
      constexpr int size_head_spare =          2;
      constexpr int size_L1ID =               32;
      constexpr int size_head_coincBCID =     12;
      constexpr int size_head_regionCount =    4;
      constexpr int size_head_coincRegion =    4;
      constexpr int size_head_reserved =      44;
      constexpr int size_finder_streamID =     8;
      constexpr int size_finder_regionCount =  4;
      constexpr int size_finder_triggerID =    4;
      constexpr int size_finder_V1 =          16;
      constexpr int size_finder_V0 =          16;
      constexpr int size_finder_U1 =          16;
      constexpr int size_finder_U0 =          16;
      constexpr int size_finder_X3 =          16;
      constexpr int size_finder_X2 =          16;
      constexpr int size_finder_X1 =          16;
      constexpr int size_finder_X0 =          16;
      constexpr int size_fitter_streamID =     8;
      constexpr int size_fitter_regionCount =  4;
      constexpr int size_fitter_triggerID =    4;
      constexpr int size_fitter_filler =      16;
      constexpr int size_fitter_mxG =         16;
      constexpr int size_fitter_muG =         16;
      constexpr int size_fitter_mvG =         16;
      constexpr int size_fitter_mxL =         16;
      constexpr int size_fitter_mx_ROI =      16;
      constexpr int size_fitter_dTheta =      16;
      constexpr int size_fitter_zero =         2;
      constexpr int size_fitter_phiSign =      1;
      constexpr int size_fitter_phiBin =       5;
      constexpr int size_fitter_rBin =         8;
      constexpr int size_trailer_CRC =        16;
    };

    namespace MMART {
      constexpr int size_art_BCID =             11;
      constexpr int size_art_pipeID =            2;
      constexpr int size_art_fiberID =           3;
      constexpr int size_art_VMMmap =           32;
      constexpr int size_art_ARTs =              6;
    };

    namespace MMTRIG {
      constexpr int size_trig_globalX =         16;
      constexpr int size_trig_globalU =         16;
      constexpr int size_trig_BCID =            12;
      constexpr int size_trig_reserved =         1;
      constexpr int size_trig_dTheta =           5;
      constexpr int size_trig_phiBin =           6;
      constexpr int size_trig_rBin =             8;
    };

  }
}

#endif // _MUON_NSW_MMTP_DECODE_BITMAPS_H_
