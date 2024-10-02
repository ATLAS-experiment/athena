/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
   */

#ifndef EFTRACKING_FPGA_INTEGRATION_FPGADATAFORMATUTILITIES_H
#define EFTRACKING_FPGA_INTEGRATION_FPGADATAFORMATUTILITIES_H

#include <cinttypes>

// Provides simple functions to convert data into FPGA format
namespace FPGADataFormatUtilites
{
  const int EVT_HDR_LWORDS                 = 3;

  // EVT_HDR defined flags
  const int EVT_HDR_FLAG                   = 0xab;

  // EVT_HDR_W1 word description
  const int EVT_HDR_W1_FLAG_bits           = 8 ;
  const int EVT_HDR_W1_FLAG_lsb            = 56;

  const int EVT_HDR_W1_L0ID_bits           = 40;
  const int EVT_HDR_W1_L0ID_lsb            = 16;

  const int EVT_HDR_W1_BCID_bits           = 12;
  const int EVT_HDR_W1_BCID_lsb            = 4;

  const int EVT_HDR_W1_SPARE_bits          = 4;
  const int EVT_HDR_W1_SPARE_lsb           = 0;

  // EVT_HDR_W2 word description
  const int EVT_HDR_W2_RUNNUMBER_bits      = 32;
  const int EVT_HDR_W2_RUNNUMBER_lsb       = 32;

  const int EVT_HDR_W2_TIME_bits           = 32;
  const int EVT_HDR_W2_TIME_lsb            = 0;


  // EVT_HDR_W3 word description
  const int EVT_HDR_W3_STATUS_bits         = 32;
  const int EVT_HDR_W3_STATUS_lsb          = 32;

  const int EVT_HDR_W3_CRC_bits            = 32;
  const int EVT_HDR_W3_CRC_lsb             = 0;


  // Event header format
  typedef struct EVT_HDR_w1 {
    uint64_t flag : EVT_HDR_W1_FLAG_bits;
    uint64_t l0id : EVT_HDR_W1_L0ID_bits;
    uint64_t bcid : EVT_HDR_W1_BCID_bits;
    uint64_t spare : EVT_HDR_W1_SPARE_bits;
  } EVT_HDR_w1;

  typedef struct EVT_HDR_w2 {
    uint64_t runnumber : EVT_HDR_W2_RUNNUMBER_bits;
    uint64_t time : EVT_HDR_W2_TIME_bits;
  } EVT_HDR_w2;

  typedef struct EVT_HDR_w3 {
    uint64_t status : EVT_HDR_W3_STATUS_bits;
    uint64_t crc : EVT_HDR_W3_CRC_bits;
  } EVT_HDR_w3;


  uint64_t get_dataformat_EVT_HDR_w1 (const EVT_HDR_w1& in) {
    uint64_t temp = 0;
    temp |= (static_cast<uint64_t>(in.flag) << EVT_HDR_W1_FLAG_lsb);
    temp |= (static_cast<uint64_t>(in.l0id) << EVT_HDR_W1_L0ID_lsb);
    temp |= (static_cast<uint64_t>(in.bcid) << EVT_HDR_W1_BCID_lsb);
    temp |= (static_cast<uint64_t>(in.spare) << EVT_HDR_W1_SPARE_lsb);
    return temp;
  }

  uint64_t get_dataformat_EVT_HDR_w2 (const EVT_HDR_w2& in) {
    uint64_t temp = 0;
    temp |= (static_cast<uint64_t>(in.runnumber) << EVT_HDR_W2_RUNNUMBER_lsb);
    temp |= (static_cast<uint64_t>(in.time) << EVT_HDR_W2_TIME_lsb);
    return temp;
  }

  uint64_t get_dataformat_EVT_HDR_w3 (const EVT_HDR_w3& in) {
    uint64_t temp = 0;
    temp |= (static_cast<uint64_t>(in.status) << EVT_HDR_W3_STATUS_lsb);
    temp |= (static_cast<uint64_t>(in.crc) << EVT_HDR_W3_CRC_lsb);
    return temp;
  }

  EVT_HDR_w1 fill_EVT_HDR_w1 (const uint64_t& flag, const uint64_t& l0id, const uint64_t& bcid, const uint64_t& spare) {
    EVT_HDR_w1 temp;
    temp.flag = flag;
    temp.l0id = l0id;
    temp.bcid = bcid;
    temp.spare = spare;
    return temp;
  }

  EVT_HDR_w2 fill_EVT_HDR_w2 (const uint64_t& runnumber, const uint64_t& time) {
    EVT_HDR_w2 temp;
    temp.runnumber = runnumber;
    temp.time = time;
    return temp;
  }

  EVT_HDR_w3 fill_EVT_HDR_w3 (const uint64_t& status, const uint64_t& crc) {
    EVT_HDR_w3 temp;
    temp.status = status;
    temp.crc = crc;
    return temp;
  }

  // Footer
  // EVT_FTR_W1 word description
  const int  EVT_FTR_FLAG                  = 0xcd;
  const int  EVT_FTR_W1_FLAG_bits          = 8 ;
  const int  EVT_FTR_W1_FLAG_lsb           = 56;

  const int  EVT_FTR_W1_SPARE_bits         = 24;
  const int  EVT_FTR_W1_SPARE_lsb          = 32;

  const int  EVT_FTR_W1_HDR_CRC_bits       = 32;
  const int  EVT_FTR_W1_HDR_CRC_lsb        = 0 ;

  // EVT_FTR_W2 word description               
  const int  EVT_FTR_W2_ERROR_FLAGS_bits   = 64;
  const int  EVT_FTR_W2_ERROR_FLAGS_lsb    = 0 ;

  // EVT_FTR_W3 word description               
  const int  EVT_FTR_W3_WORD_COUNT_bits    = 32;
  const int  EVT_FTR_W3_WORD_COUNT_lsb     = 32;

  const int  EVT_FTR_W3_CRC_bits           = 32;
  const int  EVT_FTR_W3_CRC_lsb            = 0 ;


  typedef struct EVT_FTR_w1 {
    uint64_t flag : EVT_FTR_W1_FLAG_bits;
    uint64_t spare : EVT_FTR_W1_SPARE_bits;
    uint64_t hdr_crc : EVT_FTR_W1_HDR_CRC_bits;
  } EVT_FTR_w1;

  typedef struct EVT_FTR_w2 {
    uint64_t error_flags : EVT_FTR_W2_ERROR_FLAGS_bits;
  } EVT_FTR_w2;

  typedef struct EVT_FTR_w3 {
    uint64_t word_count : EVT_FTR_W3_WORD_COUNT_bits;
    uint64_t crc : EVT_FTR_W3_CRC_bits;
  } EVT_FTR_w3;



  uint64_t get_dataformat_EVT_FTR_w1 (const EVT_FTR_w1 in) {
    uint64_t temp = 0;
    temp |= static_cast<uint64_t>(in.flag) << EVT_FTR_W1_FLAG_lsb;
    temp |= static_cast<uint64_t>(in.spare) << EVT_FTR_W1_SPARE_lsb;
    temp |= static_cast<uint64_t>(in.hdr_crc) << EVT_FTR_W1_HDR_CRC_lsb;
    return temp;
  }

  uint64_t get_dataformat_EVT_FTR_w2 (const EVT_FTR_w2 in) {
    uint64_t temp = 0;
    temp |= static_cast<uint64_t>(in.error_flags) << EVT_FTR_W2_ERROR_FLAGS_lsb;
    return temp;
  }

  uint64_t get_dataformat_EVT_FTR_w3 (const EVT_FTR_w3 in) {
    uint64_t temp = 0;
    temp |= static_cast<uint64_t>(in.word_count) << EVT_FTR_W3_WORD_COUNT_lsb;
    temp |= static_cast<uint64_t>(in.crc) << EVT_FTR_W3_CRC_lsb;
    return temp;
  }

  EVT_FTR_w1 fill_EVT_FTR_w1 (const uint64_t flag, const uint64_t spare, const uint64_t hdr_crc) {
    EVT_FTR_w1 temp;
    temp.flag = flag;
    temp.spare = spare;
    temp.hdr_crc = hdr_crc;
    return temp;
  }

  EVT_FTR_w2 fill_EVT_FTR_w2 (const uint64_t error_flags) {
    EVT_FTR_w2 temp;
    temp.error_flags = error_flags;
    return temp;
  }

  EVT_FTR_w3 fill_EVT_FTR_w3 (const uint64_t word_count, const uint64_t crc) {
    EVT_FTR_w3 temp;
    temp.word_count = word_count;
    temp.crc = crc;
    return temp;
  }

  // Module
  // M_HDR defined flags
  const int M_HDR_FLAG                    = 0x55;

  // M_HDR_W1 word description
  const int M_HDR_W1_FLAG_bits            = 8;
  const int M_HDR_W1_FLAG_lsb             = 56;

  const int M_HDR_W1_MODID_bits           = 32;
  const int M_HDR_W1_MODID_lsb            = 24;

  const int M_HDR_W1_SPARE_bits           = 24;
  const int M_HDR_W1_SPARE_lsb            = 0;

  typedef struct M_HDR_w3 {
    uint64_t flag : M_HDR_W1_FLAG_bits;
    uint64_t modid : M_HDR_W1_MODID_bits;
    uint64_t spare : M_HDR_W1_SPARE_bits;
  } M_HDR_w3;


  uint64_t get_dataformat_M_HDR_w3 (const M_HDR_w3 in) {
    uint64_t temp = 0;
    temp |= static_cast<uint64_t>(in.flag) << M_HDR_W1_FLAG_lsb;
    temp |= static_cast<uint64_t>(in.modid) << M_HDR_W1_MODID_lsb;
    temp |= static_cast<uint64_t>(in.spare) << M_HDR_W1_SPARE_lsb;
    return temp;
  }

  M_HDR_w3 fill_M_HDR_w3 (const uint64_t flag, const uint64_t modid, const uint64_t spare) {
    M_HDR_w3 temp;
    temp.flag = flag;
    temp.modid = modid;
    temp.spare = spare;
    return temp;
  }

  // PIXEL_EF_RDO word description
  const int  PIXEL_EF_RDO_LAST_bits         = 1 ;
  const int  PIXEL_EF_RDO_LAST_lsb          = 63;
                                              
  const int  PIXEL_EF_RDO_ROW_bits          = 10;
  const int  PIXEL_EF_RDO_ROW_lsb           = 53;
                                              
  const int  PIXEL_EF_RDO_COL_bits          = 10;
  const int  PIXEL_EF_RDO_COL_lsb           = 43;
                                              
  const int  PIXEL_EF_RDO_TOT_bits          = 4 ;
  const int  PIXEL_EF_RDO_TOT_lsb           = 39;
                                              
  const int  PIXEL_EF_RDO_LVL1_bits         = 1 ;
  const int  PIXEL_EF_RDO_LVL1_lsb          = 38;
                                              
  const int  PIXEL_EF_RDO_ID_bits           = 13;
  const int  PIXEL_EF_RDO_ID_lsb            = 25;
                                              
  const int  PIXEL_EF_RDO_SPARE_bits        = 25;
  const int  PIXEL_EF_RDO_SPARE_lsb         = 0;
                                              
  // STRIP_EF_RDO word description            
  const int  STRIP_EF_RDO_LAST_bits         = 1 ;
  const int  STRIP_EF_RDO_LAST_lsb          = 31;
                                              
  const int  STRIP_EF_RDO_CHIPID_bits       = 4 ;
  const int  STRIP_EF_RDO_CHIPID_lsb        = 27;
                                              
  const int  STRIP_EF_RDO_STRIP_NUM_bits    = 8 ;
  const int  STRIP_EF_RDO_STRIP_NUM_lsb     = 19;
                                              
  const int  STRIP_EF_RDO_CLUSTER_MAP_bits  = 3 ;
  const int  STRIP_EF_RDO_CLUSTER_MAP_lsb   = 16;
                                              
  const int  STRIP_EF_RDO_ID_bits           = 13;
  const int  STRIP_EF_RDO_ID_lsb            = 3 ;
                                              
  const int  STRIP_EF_RDO_SPARE_bits        = 3 ;
  const int  STRIP_EF_RDO_SPARE_lsb         = 0 ;

  typedef struct PIXEL_EF_RDO {
    unsigned int last : PIXEL_EF_RDO_LAST_bits;
    unsigned int row : PIXEL_EF_RDO_ROW_bits;
    unsigned int col : PIXEL_EF_RDO_COL_bits;
    unsigned int tot : PIXEL_EF_RDO_TOT_bits;
    unsigned int lvl1 : PIXEL_EF_RDO_LVL1_bits;
    unsigned int id : PIXEL_EF_RDO_ID_bits;
    unsigned int spare : PIXEL_EF_RDO_SPARE_bits;
  } PIXEL_EF_RDO;

  uint64_t get_dataformat_PIXEL_EF_RDO (const PIXEL_EF_RDO in) {
    uint64_t temp = 0;
    temp |= static_cast<uint64_t>(in.last) << PIXEL_EF_RDO_LAST_lsb;
    temp |= static_cast<uint64_t>(in.row) << PIXEL_EF_RDO_ROW_lsb;
    temp |= static_cast<uint64_t>(in.col) << PIXEL_EF_RDO_COL_lsb;
    temp |= static_cast<uint64_t>(in.tot) << PIXEL_EF_RDO_TOT_lsb;
    temp |= static_cast<uint64_t>(in.lvl1) << PIXEL_EF_RDO_LVL1_lsb;
    temp |= static_cast<uint64_t>(in.id) << PIXEL_EF_RDO_ID_lsb;
    temp |= static_cast<uint64_t>(in.spare) << PIXEL_EF_RDO_SPARE_lsb;
    return temp;
  }

  PIXEL_EF_RDO fill_PIXEL_EF_RDO (const uint64_t last, const uint64_t row, const uint64_t col, const uint64_t tot, const uint64_t lvl1, const uint64_t id, const uint64_t spare) {
    PIXEL_EF_RDO temp;
    temp.last = last;
    temp.row = row;
    temp.col = col;
    temp.tot = tot;
    temp.lvl1 = lvl1;
    temp.id = id;
    temp.spare = spare;
    return temp;
  }

  typedef struct STRIP_EF_RDO {
    unsigned int last : STRIP_EF_RDO_LAST_bits;
    unsigned int chipid : STRIP_EF_RDO_CHIPID_bits;
    unsigned int strip_num : STRIP_EF_RDO_STRIP_NUM_bits;
    unsigned int cluster_map : STRIP_EF_RDO_CLUSTER_MAP_bits;
    unsigned int id : STRIP_EF_RDO_ID_bits;
    unsigned int spare : STRIP_EF_RDO_SPARE_bits;
  } STRIP_EF_RDO;

  uint64_t get_dataformat_STRIP_EF_RDO (const STRIP_EF_RDO in) {
    uint64_t temp = 0;
    temp |= static_cast<uint64_t>(in.last) << STRIP_EF_RDO_LAST_lsb;
    temp |= static_cast<uint64_t>(in.chipid) << STRIP_EF_RDO_CHIPID_lsb;
    temp |= static_cast<uint64_t>(in.strip_num) << STRIP_EF_RDO_STRIP_NUM_lsb;
    temp |= static_cast<uint64_t>(in.cluster_map) << STRIP_EF_RDO_CLUSTER_MAP_lsb;
    temp |= static_cast<uint64_t>(in.id) << STRIP_EF_RDO_ID_lsb;
    temp |= static_cast<uint64_t>(in.spare) << STRIP_EF_RDO_SPARE_lsb;
    return temp;
  }
  
  STRIP_EF_RDO fill_STRIP_EF_RDO (const uint64_t last, const uint64_t chipid, const uint64_t strip_num, const uint64_t cluster_map, const uint64_t id, const uint64_t spare) {
    STRIP_EF_RDO temp;
    temp.last = last;
    temp.chipid = chipid;
    temp.strip_num = strip_num;
    temp.cluster_map = cluster_map;
    temp.id = id;
    temp.spare = spare;
    return temp;
  }


};

#endif  // EFTRACKING_FPGA_INTEGRATION_FPGADATAFORMATUTILITIES_H
