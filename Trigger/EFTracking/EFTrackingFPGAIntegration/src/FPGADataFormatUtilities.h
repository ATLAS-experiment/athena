/*
 * Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef EFTRACKING_FPGA_INTEGRATION_FPGADATAFORMATUTILITIES_H
#define EFTRACKING_FPGA_INTEGRATION_FPGADATAFORMATUTILITIES_H

#include <cstdint>

// Provider of simple function for conversion of data into the FPGA dataformat
// Using FPGA Dataformat doc v0.6
namespace FPGADataFormatUtilities
{
    consteval uint64_t SELECTBITS(uint8_t len, uint8_t startbit) {
        return (len == 64 ? UINTMAX_MAX : (((1ULL << len) - 1ULL) << startbit));
    }

	// EVT_HDR defined flags
	const int EVT_HDR_FLAG = 0xab;

	// EVT_HDR_W1 word description
	const int EVT_HDR_W1_FLAG_bits = 8;
	const int EVT_HDR_W1_FLAG_lsb = 56;
	const float EVT_HDR_W1_FLAG_mf = 1.;

	const int EVT_HDR_W1_L0ID_bits = 40;
	const int EVT_HDR_W1_L0ID_lsb = 16;
	const float EVT_HDR_W1_L0ID_mf = 1.;

	const int EVT_HDR_W1_BCID_bits = 12;
	const int EVT_HDR_W1_BCID_lsb = 4;
	const float EVT_HDR_W1_BCID_mf = 1.;

	const int EVT_HDR_W1_SPARE_bits = 4;
	const int EVT_HDR_W1_SPARE_lsb = 0;
	const float EVT_HDR_W1_SPARE_mf = 1.;

	// EVT_HDR_W2 word description
	const int EVT_HDR_W2_RUNNUMBER_bits = 32;
	const int EVT_HDR_W2_RUNNUMBER_lsb = 32;
	const float EVT_HDR_W2_RUNNUMBER_mf = 1.;

	const int EVT_HDR_W2_TIME_bits = 32;
	const int EVT_HDR_W2_TIME_lsb = 0;
	const float EVT_HDR_W2_TIME_mf = 1.;

	// EVT_HDR_W3 word description
	const int EVT_HDR_W3_STATUS_bits = 32;
	const int EVT_HDR_W3_STATUS_lsb = 32;
	const float EVT_HDR_W3_STATUS_mf = 1.;

	const int EVT_HDR_W3_CRC_bits = 32;
	const int EVT_HDR_W3_CRC_lsb = 0;
	const float EVT_HDR_W3_CRC_mf = 1.;

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

	inline EVT_HDR_w1 get_bitfields_EVT_HDR_w1 (const uint64_t& in) {
		EVT_HDR_w1 temp;
		temp.flag = (in & SELECTBITS(EVT_HDR_W1_FLAG_bits, EVT_HDR_W1_FLAG_lsb)) >> EVT_HDR_W1_FLAG_lsb;
		temp.l0id = (in & SELECTBITS(EVT_HDR_W1_L0ID_bits, EVT_HDR_W1_L0ID_lsb)) >> EVT_HDR_W1_L0ID_lsb;
		temp.bcid = (in & SELECTBITS(EVT_HDR_W1_BCID_bits, EVT_HDR_W1_BCID_lsb)) >> EVT_HDR_W1_BCID_lsb;
		temp.spare = (in & SELECTBITS(EVT_HDR_W1_SPARE_bits, EVT_HDR_W1_SPARE_lsb)) >> EVT_HDR_W1_SPARE_lsb;
		return temp;
	}

	inline EVT_HDR_w2 get_bitfields_EVT_HDR_w2 (const uint64_t& in) {
		EVT_HDR_w2 temp;
		temp.runnumber = (in & SELECTBITS(EVT_HDR_W2_RUNNUMBER_bits, EVT_HDR_W2_RUNNUMBER_lsb)) >> EVT_HDR_W2_RUNNUMBER_lsb;
		temp.time = (in & SELECTBITS(EVT_HDR_W2_TIME_bits, EVT_HDR_W2_TIME_lsb)) >> EVT_HDR_W2_TIME_lsb;
		return temp;
	}

	inline EVT_HDR_w3 get_bitfields_EVT_HDR_w3 (const uint64_t& in) {
		EVT_HDR_w3 temp;
		temp.status = (in & SELECTBITS(EVT_HDR_W3_STATUS_bits, EVT_HDR_W3_STATUS_lsb)) >> EVT_HDR_W3_STATUS_lsb;
		temp.crc = (in & SELECTBITS(EVT_HDR_W3_CRC_bits, EVT_HDR_W3_CRC_lsb)) >> EVT_HDR_W3_CRC_lsb;
		return temp;
	}

	inline uint64_t get_dataformat_EVT_HDR_w1 (const EVT_HDR_w1& in) {
		uint64_t temp = 0;
		temp |= (static_cast<uint64_t>(in.flag) << EVT_HDR_W1_FLAG_lsb);
		temp |= (static_cast<uint64_t>(in.l0id) << EVT_HDR_W1_L0ID_lsb);
		temp |= (static_cast<uint64_t>(in.bcid) << EVT_HDR_W1_BCID_lsb);
		temp |= (static_cast<uint64_t>(in.spare) << EVT_HDR_W1_SPARE_lsb);
		return temp;
	}

	inline uint64_t get_dataformat_EVT_HDR_w2 (const EVT_HDR_w2& in) {
		uint64_t temp = 0;
		temp |= (static_cast<uint64_t>(in.runnumber) << EVT_HDR_W2_RUNNUMBER_lsb);
		temp |= (static_cast<uint64_t>(in.time) << EVT_HDR_W2_TIME_lsb);
		return temp;
	}

	inline uint64_t get_dataformat_EVT_HDR_w3 (const EVT_HDR_w3& in) {
		uint64_t temp = 0;
		temp |= (static_cast<uint64_t>(in.status) << EVT_HDR_W3_STATUS_lsb);
		temp |= (static_cast<uint64_t>(in.crc) << EVT_HDR_W3_CRC_lsb);
		return temp;
	}

	inline EVT_HDR_w1 fill_EVT_HDR_w1 (const uint64_t& flag, const uint64_t& l0id, const uint64_t& bcid, const uint64_t& spare) {
		EVT_HDR_w1 temp;
		temp.flag = flag;
		temp.l0id = l0id;
		temp.bcid = bcid;
		temp.spare = spare;
		return temp;
	}

	inline EVT_HDR_w2 fill_EVT_HDR_w2 (const uint64_t& runnumber, const uint64_t& time) {
		EVT_HDR_w2 temp;
		temp.runnumber = runnumber;
		temp.time = time;
		return temp;
	}

	inline EVT_HDR_w3 fill_EVT_HDR_w3 (const uint64_t& status, const uint64_t& crc) {
		EVT_HDR_w3 temp;
		temp.status = status;
		temp.crc = crc;
		return temp;
	}

	inline uint64_t to_real_EVT_HDR_w1_flag (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_EVT_HDR_w1_l0id (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_EVT_HDR_w1_bcid (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_EVT_HDR_w1_spare (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_EVT_HDR_w2_runnumber (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_EVT_HDR_w2_time (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_EVT_HDR_w3_status (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_EVT_HDR_w3_crc (const uint64_t& in) {
		return in;
	}

	// EVT_FTR defined flags
	const int EVT_FTR_FLAG = 0xcd;

	// EVT_FTR_W1 word description
	const int EVT_FTR_W1_FLAG_bits = 8;
	const int EVT_FTR_W1_FLAG_lsb = 56;
	const float EVT_FTR_W1_FLAG_mf = 1.;

	const int EVT_FTR_W1_SPARE_bits = 24;
	const int EVT_FTR_W1_SPARE_lsb = 32;
	const float EVT_FTR_W1_SPARE_mf = 1.;

	const int EVT_FTR_W1_HDR_CRC_bits = 32;
	const int EVT_FTR_W1_HDR_CRC_lsb = 0;
	const float EVT_FTR_W1_HDR_CRC_mf = 1.;

	// EVT_FTR_W2 word description
	const int EVT_FTR_W2_ERROR_FLAGS_bits = 64;
	const int EVT_FTR_W2_ERROR_FLAGS_lsb = 0;
	const float EVT_FTR_W2_ERROR_FLAGS_mf = 1.;

	// EVT_FTR_W3 word description
	const int EVT_FTR_W3_WORD_COUNT_bits = 32;
	const int EVT_FTR_W3_WORD_COUNT_lsb = 32;
	const float EVT_FTR_W3_WORD_COUNT_mf = 1.;

	const int EVT_FTR_W3_CRC_bits = 32;
	const int EVT_FTR_W3_CRC_lsb = 0;
	const float EVT_FTR_W3_CRC_mf = 1.;

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

	inline EVT_FTR_w1 get_bitfields_EVT_FTR_w1 (const uint64_t& in) {
		EVT_FTR_w1 temp;
		temp.flag = (in & SELECTBITS(EVT_FTR_W1_FLAG_bits, EVT_FTR_W1_FLAG_lsb)) >> EVT_FTR_W1_FLAG_lsb;
		temp.spare = (in & SELECTBITS(EVT_FTR_W1_SPARE_bits, EVT_FTR_W1_SPARE_lsb)) >> EVT_FTR_W1_SPARE_lsb;
		temp.hdr_crc = (in & SELECTBITS(EVT_FTR_W1_HDR_CRC_bits, EVT_FTR_W1_HDR_CRC_lsb)) >> EVT_FTR_W1_HDR_CRC_lsb;
		return temp;
	}

	inline EVT_FTR_w2 get_bitfields_EVT_FTR_w2 (const uint64_t& in) {
		EVT_FTR_w2 temp;
		temp.error_flags = (in & SELECTBITS(EVT_FTR_W2_ERROR_FLAGS_bits, EVT_FTR_W2_ERROR_FLAGS_lsb)) >> EVT_FTR_W2_ERROR_FLAGS_lsb;
		return temp;
	}

	inline EVT_FTR_w3 get_bitfields_EVT_FTR_w3 (const uint64_t& in) {
		EVT_FTR_w3 temp;
		temp.word_count = (in & SELECTBITS(EVT_FTR_W3_WORD_COUNT_bits, EVT_FTR_W3_WORD_COUNT_lsb)) >> EVT_FTR_W3_WORD_COUNT_lsb;
		temp.crc = (in & SELECTBITS(EVT_FTR_W3_CRC_bits, EVT_FTR_W3_CRC_lsb)) >> EVT_FTR_W3_CRC_lsb;
		return temp;
	}

	inline uint64_t get_dataformat_EVT_FTR_w1 (const EVT_FTR_w1& in) {
		uint64_t temp = 0;
		temp |= (static_cast<uint64_t>(in.flag) << EVT_FTR_W1_FLAG_lsb);
		temp |= (static_cast<uint64_t>(in.spare) << EVT_FTR_W1_SPARE_lsb);
		temp |= (static_cast<uint64_t>(in.hdr_crc) << EVT_FTR_W1_HDR_CRC_lsb);
		return temp;
	}

	inline uint64_t get_dataformat_EVT_FTR_w2 (const EVT_FTR_w2& in) {
		uint64_t temp = 0;
		temp |= (static_cast<uint64_t>(in.error_flags) << EVT_FTR_W2_ERROR_FLAGS_lsb);
		return temp;
	}

	inline uint64_t get_dataformat_EVT_FTR_w3 (const EVT_FTR_w3& in) {
		uint64_t temp = 0;
		temp |= (static_cast<uint64_t>(in.word_count) << EVT_FTR_W3_WORD_COUNT_lsb);
		temp |= (static_cast<uint64_t>(in.crc) << EVT_FTR_W3_CRC_lsb);
		return temp;
	}

	inline EVT_FTR_w1 fill_EVT_FTR_w1 (const uint64_t& flag, const uint64_t& spare, const uint64_t& hdr_crc) {
		EVT_FTR_w1 temp;
		temp.flag = flag;
		temp.spare = spare;
		temp.hdr_crc = hdr_crc;
		return temp;
	}

	inline EVT_FTR_w2 fill_EVT_FTR_w2 (const uint64_t& error_flags) {
		EVT_FTR_w2 temp;
		temp.error_flags = error_flags;
		return temp;
	}

	inline EVT_FTR_w3 fill_EVT_FTR_w3 (const uint64_t& word_count, const uint64_t& crc) {
		EVT_FTR_w3 temp;
		temp.word_count = word_count;
		temp.crc = crc;
		return temp;
	}

	inline uint64_t to_real_EVT_FTR_w1_flag (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_EVT_FTR_w1_spare (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_EVT_FTR_w1_hdr_crc (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_EVT_FTR_w2_error_flags (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_EVT_FTR_w3_word_count (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_EVT_FTR_w3_crc (const uint64_t& in) {
		return in;
	}

	// M_HDR defined flags
	const int M_HDR_FLAG = 0x55;

	// M_HDR_W1 word description
	const int M_HDR_W1_FLAG_bits = 8;
	const int M_HDR_W1_FLAG_lsb = 56;
	const float M_HDR_W1_FLAG_mf = 1.;

	const int M_HDR_W1_MODID_bits = 32;
	const int M_HDR_W1_MODID_lsb = 24;
	const float M_HDR_W1_MODID_mf = 1.;

	const int M_HDR_W1_MODHASH_bits = 16;
	const int M_HDR_W1_MODHASH_lsb = 8;
	const float M_HDR_W1_MODHASH_mf = 1.;

	const int M_HDR_W1_SPARE_bits = 8;
	const int M_HDR_W1_SPARE_lsb = 0;
	const float M_HDR_W1_SPARE_mf = 1.;

	typedef struct M_HDR_w1 {
		uint64_t flag : M_HDR_W1_FLAG_bits;
		uint64_t modid : M_HDR_W1_MODID_bits;
		uint64_t modhash : M_HDR_W1_MODHASH_bits;
		uint64_t spare : M_HDR_W1_SPARE_bits;
	} M_HDR_w1;

	inline M_HDR_w1 get_bitfields_M_HDR_w1 (const uint64_t& in) {
		M_HDR_w1 temp;
		temp.flag = (in & SELECTBITS(M_HDR_W1_FLAG_bits, M_HDR_W1_FLAG_lsb)) >> M_HDR_W1_FLAG_lsb;
		temp.modid = (in & SELECTBITS(M_HDR_W1_MODID_bits, M_HDR_W1_MODID_lsb)) >> M_HDR_W1_MODID_lsb;
		temp.modhash = (in & SELECTBITS(M_HDR_W1_MODHASH_bits, M_HDR_W1_MODHASH_lsb)) >> M_HDR_W1_MODHASH_lsb;
		temp.spare = (in & SELECTBITS(M_HDR_W1_SPARE_bits, M_HDR_W1_SPARE_lsb)) >> M_HDR_W1_SPARE_lsb;
		return temp;
	}

	inline uint64_t get_dataformat_M_HDR_w1 (const M_HDR_w1& in) {
		uint64_t temp = 0;
		temp |= (static_cast<uint64_t>(in.flag) << M_HDR_W1_FLAG_lsb);
		temp |= (static_cast<uint64_t>(in.modid) << M_HDR_W1_MODID_lsb);
		temp |= (static_cast<uint64_t>(in.modhash) << M_HDR_W1_MODHASH_lsb);
		temp |= (static_cast<uint64_t>(in.spare) << M_HDR_W1_SPARE_lsb);
		return temp;
	}

	inline M_HDR_w1 fill_M_HDR_w1 (const uint64_t& flag, const uint64_t& modid, const uint64_t& modhash, const uint64_t& spare) {
		M_HDR_w1 temp;
		temp.flag = flag;
		temp.modid = modid;
		temp.modhash = modhash;
		temp.spare = spare;
		return temp;
	}

	inline uint64_t to_real_M_HDR_w1_flag (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_M_HDR_w1_modid (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_M_HDR_w1_modhash (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_M_HDR_w1_spare (const uint64_t& in) {
		return in;
	}

	// SLICE_HDR defined flags
	const int SLICE_HDR_FLAG = 0x92;

	// SLICE_HDR_W1 word description
	const int SLICE_HDR_W1_FLAG_bits = 8;
	const int SLICE_HDR_W1_FLAG_lsb = 56;
	const float SLICE_HDR_W1_FLAG_mf = 1.;

	const int SLICE_HDR_W1_SLICEID_bits = 11;
	const int SLICE_HDR_W1_SLICEID_lsb = 45;
	const float SLICE_HDR_W1_SLICEID_mf = 1.;

	const int SLICE_HDR_W1_ETA_REGION_bits = 6;
	const int SLICE_HDR_W1_ETA_REGION_lsb = 39;
	const float SLICE_HDR_W1_ETA_REGION_mf = 1.;

	const int SLICE_HDR_W1_PHI_REGION_bits = 6;
	const int SLICE_HDR_W1_PHI_REGION_lsb = 33;
	const float SLICE_HDR_W1_PHI_REGION_mf = 1.;

	const int SLICE_HDR_W1_SPARE_bits = 33;
	const int SLICE_HDR_W1_SPARE_lsb = 0;
	const float SLICE_HDR_W1_SPARE_mf = 1.;

	typedef struct SLICE_HDR_w1 {
		uint64_t flag : SLICE_HDR_W1_FLAG_bits;
		uint64_t sliceid : SLICE_HDR_W1_SLICEID_bits;
		uint64_t eta_region : SLICE_HDR_W1_ETA_REGION_bits;
		uint64_t phi_region : SLICE_HDR_W1_PHI_REGION_bits;
		uint64_t spare : SLICE_HDR_W1_SPARE_bits;
	} SLICE_HDR_w1;

	inline SLICE_HDR_w1 get_bitfields_SLICE_HDR_w1 (const uint64_t& in) {
		SLICE_HDR_w1 temp;
		temp.flag = (in & SELECTBITS(SLICE_HDR_W1_FLAG_bits, SLICE_HDR_W1_FLAG_lsb)) >> SLICE_HDR_W1_FLAG_lsb;
		temp.sliceid = (in & SELECTBITS(SLICE_HDR_W1_SLICEID_bits, SLICE_HDR_W1_SLICEID_lsb)) >> SLICE_HDR_W1_SLICEID_lsb;
		temp.eta_region = (in & SELECTBITS(SLICE_HDR_W1_ETA_REGION_bits, SLICE_HDR_W1_ETA_REGION_lsb)) >> SLICE_HDR_W1_ETA_REGION_lsb;
		temp.phi_region = (in & SELECTBITS(SLICE_HDR_W1_PHI_REGION_bits, SLICE_HDR_W1_PHI_REGION_lsb)) >> SLICE_HDR_W1_PHI_REGION_lsb;
		temp.spare = (in & SELECTBITS(SLICE_HDR_W1_SPARE_bits, SLICE_HDR_W1_SPARE_lsb)) >> SLICE_HDR_W1_SPARE_lsb;
		return temp;
	}

	inline uint64_t get_dataformat_SLICE_HDR_w1 (const SLICE_HDR_w1& in) {
		uint64_t temp = 0;
		temp |= (static_cast<uint64_t>(in.flag) << SLICE_HDR_W1_FLAG_lsb);
		temp |= (static_cast<uint64_t>(in.sliceid) << SLICE_HDR_W1_SLICEID_lsb);
		temp |= (static_cast<uint64_t>(in.eta_region) << SLICE_HDR_W1_ETA_REGION_lsb);
		temp |= (static_cast<uint64_t>(in.phi_region) << SLICE_HDR_W1_PHI_REGION_lsb);
		temp |= (static_cast<uint64_t>(in.spare) << SLICE_HDR_W1_SPARE_lsb);
		return temp;
	}

	inline SLICE_HDR_w1 fill_SLICE_HDR_w1 (const uint64_t& flag, const uint64_t& sliceid, const uint64_t& eta_region, const uint64_t& phi_region, const uint64_t& spare) {
		SLICE_HDR_w1 temp;
		temp.flag = flag;
		temp.sliceid = sliceid;
		temp.eta_region = eta_region;
		temp.phi_region = phi_region;
		temp.spare = spare;
		return temp;
	}

	inline uint64_t to_real_SLICE_HDR_w1_flag (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_SLICE_HDR_w1_sliceid (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_SLICE_HDR_w1_eta_region (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_SLICE_HDR_w1_phi_region (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_SLICE_HDR_w1_spare (const uint64_t& in) {
		return in;
	}

	// RD_HDR defined flags
	const int RD_HDR_FLAG = 0xbb;

	// RD_HDR_W1 word description
	const int RD_HDR_W1_FLAG_bits = 8;
	const int RD_HDR_W1_FLAG_lsb = 56;
	const float RD_HDR_W1_FLAG_mf = 1.;

	const int RD_HDR_W1_TYPE_bits = 4;
	const int RD_HDR_W1_TYPE_lsb = 52;
	const float RD_HDR_W1_TYPE_mf = 1.;

	const int RD_HDR_W1_ETA_REGION_bits = 6;
	const int RD_HDR_W1_ETA_REGION_lsb = 46;
	const float RD_HDR_W1_ETA_REGION_mf = 1.;

	const int RD_HDR_W1_PHI_REGION_bits = 6;
	const int RD_HDR_W1_PHI_REGION_lsb = 40;
	const float RD_HDR_W1_PHI_REGION_mf = 1.;

	const int RD_HDR_W1_SLICE_bits = 5;
	const int RD_HDR_W1_SLICE_lsb = 35;
	const float RD_HDR_W1_SLICE_mf = 1.;

	const int RD_HDR_W1_HOUGH_X_BIN_bits = 8;
	const int RD_HDR_W1_HOUGH_X_BIN_lsb = 27;
	const float RD_HDR_W1_HOUGH_X_BIN_mf = 1.;

	const int RD_HDR_W1_HOUGH_Y_BIN_bits = 8;
	const int RD_HDR_W1_HOUGH_Y_BIN_lsb = 19;
	const float RD_HDR_W1_HOUGH_Y_BIN_mf = 1.;

	const int RD_HDR_W1_SECOND_STAGE_bits = 1;
	const int RD_HDR_W1_SECOND_STAGE_lsb = 18;
	const float RD_HDR_W1_SECOND_STAGE_mf = 1.;

	const int RD_HDR_W1_LAYER_BITMASK_bits = 13;
	const int RD_HDR_W1_LAYER_BITMASK_lsb = 5;
	const float RD_HDR_W1_LAYER_BITMASK_mf = 1.;

	const int RD_HDR_W1_SPARE_bits = 5;
	const int RD_HDR_W1_SPARE_lsb = 0;
	const float RD_HDR_W1_SPARE_mf = 1.;

	// RD_HDR_W2 word description
	const int RD_HDR_W2_GLOBAL_PHI_bits = 16;
	const int RD_HDR_W2_GLOBAL_PHI_lsb = 48;
	const float RD_HDR_W2_GLOBAL_PHI_mf = 1.;

	const int RD_HDR_W2_GLOBAL_ETA_bits = 16;
	const int RD_HDR_W2_GLOBAL_ETA_lsb = 32;
	const float RD_HDR_W2_GLOBAL_ETA_mf = 1.;

	const int RD_HDR_W2_SPARE_bits = 32;
	const int RD_HDR_W2_SPARE_lsb = 0;
	const float RD_HDR_W2_SPARE_mf = 1.;

	typedef struct RD_HDR_w1 {
		uint64_t flag : RD_HDR_W1_FLAG_bits;
		uint64_t type : RD_HDR_W1_TYPE_bits;
		uint64_t eta_region : RD_HDR_W1_ETA_REGION_bits;
		uint64_t phi_region : RD_HDR_W1_PHI_REGION_bits;
		uint64_t slice : RD_HDR_W1_SLICE_bits;
		uint64_t hough_x_bin : RD_HDR_W1_HOUGH_X_BIN_bits;
		uint64_t hough_y_bin : RD_HDR_W1_HOUGH_Y_BIN_bits;
		uint64_t second_stage : RD_HDR_W1_SECOND_STAGE_bits;
		uint64_t layer_bitmask : RD_HDR_W1_LAYER_BITMASK_bits;
		uint64_t spare : RD_HDR_W1_SPARE_bits;
	} RD_HDR_w1;

	typedef struct RD_HDR_w2 {
		uint64_t global_phi : RD_HDR_W2_GLOBAL_PHI_bits;
		uint64_t global_eta : RD_HDR_W2_GLOBAL_ETA_bits;
		uint64_t spare : RD_HDR_W2_SPARE_bits;
	} RD_HDR_w2;

	inline RD_HDR_w1 get_bitfields_RD_HDR_w1 (const uint64_t& in) {
		RD_HDR_w1 temp;
		temp.flag = (in & SELECTBITS(RD_HDR_W1_FLAG_bits, RD_HDR_W1_FLAG_lsb)) >> RD_HDR_W1_FLAG_lsb;
		temp.type = (in & SELECTBITS(RD_HDR_W1_TYPE_bits, RD_HDR_W1_TYPE_lsb)) >> RD_HDR_W1_TYPE_lsb;
		temp.eta_region = (in & SELECTBITS(RD_HDR_W1_ETA_REGION_bits, RD_HDR_W1_ETA_REGION_lsb)) >> RD_HDR_W1_ETA_REGION_lsb;
		temp.phi_region = (in & SELECTBITS(RD_HDR_W1_PHI_REGION_bits, RD_HDR_W1_PHI_REGION_lsb)) >> RD_HDR_W1_PHI_REGION_lsb;
		temp.slice = (in & SELECTBITS(RD_HDR_W1_SLICE_bits, RD_HDR_W1_SLICE_lsb)) >> RD_HDR_W1_SLICE_lsb;
		temp.hough_x_bin = (in & SELECTBITS(RD_HDR_W1_HOUGH_X_BIN_bits, RD_HDR_W1_HOUGH_X_BIN_lsb)) >> RD_HDR_W1_HOUGH_X_BIN_lsb;
		temp.hough_y_bin = (in & SELECTBITS(RD_HDR_W1_HOUGH_Y_BIN_bits, RD_HDR_W1_HOUGH_Y_BIN_lsb)) >> RD_HDR_W1_HOUGH_Y_BIN_lsb;
		temp.second_stage = (in & SELECTBITS(RD_HDR_W1_SECOND_STAGE_bits, RD_HDR_W1_SECOND_STAGE_lsb)) >> RD_HDR_W1_SECOND_STAGE_lsb;
		temp.layer_bitmask = (in & SELECTBITS(RD_HDR_W1_LAYER_BITMASK_bits, RD_HDR_W1_LAYER_BITMASK_lsb)) >> RD_HDR_W1_LAYER_BITMASK_lsb;
		temp.spare = (in & SELECTBITS(RD_HDR_W1_SPARE_bits, RD_HDR_W1_SPARE_lsb)) >> RD_HDR_W1_SPARE_lsb;
		return temp;
	}

	inline RD_HDR_w2 get_bitfields_RD_HDR_w2 (const uint64_t& in) {
		RD_HDR_w2 temp;
		temp.global_phi = (in & SELECTBITS(RD_HDR_W2_GLOBAL_PHI_bits, RD_HDR_W2_GLOBAL_PHI_lsb)) >> RD_HDR_W2_GLOBAL_PHI_lsb;
		temp.global_eta = (in & SELECTBITS(RD_HDR_W2_GLOBAL_ETA_bits, RD_HDR_W2_GLOBAL_ETA_lsb)) >> RD_HDR_W2_GLOBAL_ETA_lsb;
		temp.spare = (in & SELECTBITS(RD_HDR_W2_SPARE_bits, RD_HDR_W2_SPARE_lsb)) >> RD_HDR_W2_SPARE_lsb;
		return temp;
	}

	inline uint64_t get_dataformat_RD_HDR_w1 (const RD_HDR_w1& in) {
		uint64_t temp = 0;
		temp |= (static_cast<uint64_t>(in.flag) << RD_HDR_W1_FLAG_lsb);
		temp |= (static_cast<uint64_t>(in.type) << RD_HDR_W1_TYPE_lsb);
		temp |= (static_cast<uint64_t>(in.eta_region) << RD_HDR_W1_ETA_REGION_lsb);
		temp |= (static_cast<uint64_t>(in.phi_region) << RD_HDR_W1_PHI_REGION_lsb);
		temp |= (static_cast<uint64_t>(in.slice) << RD_HDR_W1_SLICE_lsb);
		temp |= (static_cast<uint64_t>(in.hough_x_bin) << RD_HDR_W1_HOUGH_X_BIN_lsb);
		temp |= (static_cast<uint64_t>(in.hough_y_bin) << RD_HDR_W1_HOUGH_Y_BIN_lsb);
		temp |= (static_cast<uint64_t>(in.second_stage) << RD_HDR_W1_SECOND_STAGE_lsb);
		temp |= (static_cast<uint64_t>(in.layer_bitmask) << RD_HDR_W1_LAYER_BITMASK_lsb);
		temp |= (static_cast<uint64_t>(in.spare) << RD_HDR_W1_SPARE_lsb);
		return temp;
	}

	inline uint64_t get_dataformat_RD_HDR_w2 (const RD_HDR_w2& in) {
		uint64_t temp = 0;
		temp |= (static_cast<uint64_t>(in.global_phi) << RD_HDR_W2_GLOBAL_PHI_lsb);
		temp |= (static_cast<uint64_t>(in.global_eta) << RD_HDR_W2_GLOBAL_ETA_lsb);
		temp |= (static_cast<uint64_t>(in.spare) << RD_HDR_W2_SPARE_lsb);
		return temp;
	}

	inline RD_HDR_w1 fill_RD_HDR_w1 (const uint64_t& flag, const uint64_t& type, const uint64_t& eta_region, const uint64_t& phi_region, const uint64_t& slice, const uint64_t& hough_x_bin, const uint64_t& hough_y_bin, const uint64_t& second_stage, const uint64_t& layer_bitmask, const uint64_t& spare) {
		RD_HDR_w1 temp;
		temp.flag = flag;
		temp.type = type;
		temp.eta_region = eta_region;
		temp.phi_region = phi_region;
		temp.slice = slice;
		temp.hough_x_bin = hough_x_bin;
		temp.hough_y_bin = hough_y_bin;
		temp.second_stage = second_stage;
		temp.layer_bitmask = layer_bitmask;
		temp.spare = spare;
		return temp;
	}

	inline RD_HDR_w2 fill_RD_HDR_w2 (const uint64_t& global_phi, const uint64_t& global_eta, const uint64_t& spare) {
		RD_HDR_w2 temp;
		temp.global_phi = global_phi;
		temp.global_eta = global_eta;
		temp.spare = spare;
		return temp;
	}

	inline uint64_t to_real_RD_HDR_w1_flag (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_RD_HDR_w1_type (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_RD_HDR_w1_eta_region (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_RD_HDR_w1_phi_region (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_RD_HDR_w1_slice (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_RD_HDR_w1_hough_x_bin (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_RD_HDR_w1_hough_y_bin (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_RD_HDR_w1_second_stage (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_RD_HDR_w1_layer_bitmask (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_RD_HDR_w1_spare (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_RD_HDR_w2_global_phi (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_RD_HDR_w2_global_eta (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_RD_HDR_w2_spare (const uint64_t& in) {
		return in;
	}

	// GTRACK_HDR defined flags
	const int GTRACK_HDR_FLAG = 0xee;

	// GTRACK_HDR_W1 word description
	const int GTRACK_HDR_W1_FLAG_bits = 8;
	const int GTRACK_HDR_W1_FLAG_lsb = 56;
	const float GTRACK_HDR_W1_FLAG_mf = 1.;

	const int GTRACK_HDR_W1_TYPE_bits = 4;
	const int GTRACK_HDR_W1_TYPE_lsb = 52;
	const float GTRACK_HDR_W1_TYPE_mf = 1.;

	const int GTRACK_HDR_W1_ETA_REGION_bits = 6;
	const int GTRACK_HDR_W1_ETA_REGION_lsb = 46;
	const float GTRACK_HDR_W1_ETA_REGION_mf = 1.;

	const int GTRACK_HDR_W1_PHI_REGION_bits = 6;
	const int GTRACK_HDR_W1_PHI_REGION_lsb = 40;
	const float GTRACK_HDR_W1_PHI_REGION_mf = 1.;

	const int GTRACK_HDR_W1_SLICE_bits = 5;
	const int GTRACK_HDR_W1_SLICE_lsb = 35;
	const float GTRACK_HDR_W1_SLICE_mf = 1.;

	const int GTRACK_HDR_W1_HOUGH_X_BIN_bits = 8;
	const int GTRACK_HDR_W1_HOUGH_X_BIN_lsb = 27;
	const float GTRACK_HDR_W1_HOUGH_X_BIN_mf = 1.;

	const int GTRACK_HDR_W1_HOUGH_Y_BIN_bits = 8;
	const int GTRACK_HDR_W1_HOUGH_Y_BIN_lsb = 19;
	const float GTRACK_HDR_W1_HOUGH_Y_BIN_mf = 1.;

	const int GTRACK_HDR_W1_SECOND_STAGE_bits = 1;
	const int GTRACK_HDR_W1_SECOND_STAGE_lsb = 18;
	const float GTRACK_HDR_W1_SECOND_STAGE_mf = 1.;

	const int GTRACK_HDR_W1_LAYER_BITMASK_bits = 13;
	const int GTRACK_HDR_W1_LAYER_BITMASK_lsb = 5;
	const float GTRACK_HDR_W1_LAYER_BITMASK_mf = 1.;

	const int GTRACK_HDR_W1_SPARE_bits = 5;
	const int GTRACK_HDR_W1_SPARE_lsb = 0;
	const float GTRACK_HDR_W1_SPARE_mf = 1.;

	// GTRACK_HDR_W2 word description
	const int GTRACK_HDR_W2_SCORE_bits = 16;
	const int GTRACK_HDR_W2_SCORE_lsb = 48;
	const float GTRACK_HDR_W2_SCORE_mf = 2048.;

	const int GTRACK_HDR_W2_D0_bits = 16;
	const int GTRACK_HDR_W2_D0_lsb = 32;
	const float GTRACK_HDR_W2_D0_mf = 4096.;

	const int GTRACK_HDR_W2_Z0_bits = 16;
	const int GTRACK_HDR_W2_Z0_lsb = 16;
	const float GTRACK_HDR_W2_Z0_mf = 32.;

	const int GTRACK_HDR_W2_SPARE_bits = 16;
	const int GTRACK_HDR_W2_SPARE_lsb = 0;
	const float GTRACK_HDR_W2_SPARE_mf = 1.;

	// GTRACK_HDR_W3 word description
	const int GTRACK_HDR_W3_QOVERPT_bits = 16;
	const int GTRACK_HDR_W3_QOVERPT_lsb = 48;
	const float GTRACK_HDR_W3_QOVERPT_mf = 32768.;

	const int GTRACK_HDR_W3_PHI_bits = 16;
	const int GTRACK_HDR_W3_PHI_lsb = 32;
	const float GTRACK_HDR_W3_PHI_mf = 8192.;

	const int GTRACK_HDR_W3_ETA_bits = 16;
	const int GTRACK_HDR_W3_ETA_lsb = 16;
	const float GTRACK_HDR_W3_ETA_mf = 8192.;

	const int GTRACK_HDR_W3_SPARE_bits = 16;
	const int GTRACK_HDR_W3_SPARE_lsb = 0;
	const float GTRACK_HDR_W3_SPARE_mf = 1.;

	typedef struct GTRACK_HDR_w1 {
		uint64_t flag : GTRACK_HDR_W1_FLAG_bits;
		uint64_t type : GTRACK_HDR_W1_TYPE_bits;
		uint64_t eta_region : GTRACK_HDR_W1_ETA_REGION_bits;
		uint64_t phi_region : GTRACK_HDR_W1_PHI_REGION_bits;
		uint64_t slice : GTRACK_HDR_W1_SLICE_bits;
		uint64_t hough_x_bin : GTRACK_HDR_W1_HOUGH_X_BIN_bits;
		uint64_t hough_y_bin : GTRACK_HDR_W1_HOUGH_Y_BIN_bits;
		uint64_t second_stage : GTRACK_HDR_W1_SECOND_STAGE_bits;
		uint64_t layer_bitmask : GTRACK_HDR_W1_LAYER_BITMASK_bits;
		uint64_t spare : GTRACK_HDR_W1_SPARE_bits;
	} GTRACK_HDR_w1;

	typedef struct GTRACK_HDR_w2 {
		uint64_t score : GTRACK_HDR_W2_SCORE_bits;
		int64_t d0 : GTRACK_HDR_W2_D0_bits;
		int64_t z0 : GTRACK_HDR_W2_Z0_bits;
		uint64_t spare : GTRACK_HDR_W2_SPARE_bits;
	} GTRACK_HDR_w2;

	typedef struct GTRACK_HDR_w3 {
		int64_t qoverpt : GTRACK_HDR_W3_QOVERPT_bits;
		int64_t phi : GTRACK_HDR_W3_PHI_bits;
		int64_t eta : GTRACK_HDR_W3_ETA_bits;
		uint64_t spare : GTRACK_HDR_W3_SPARE_bits;
	} GTRACK_HDR_w3;

	inline GTRACK_HDR_w1 get_bitfields_GTRACK_HDR_w1 (const uint64_t& in) {
		GTRACK_HDR_w1 temp;
		temp.flag = (in & SELECTBITS(GTRACK_HDR_W1_FLAG_bits, GTRACK_HDR_W1_FLAG_lsb)) >> GTRACK_HDR_W1_FLAG_lsb;
		temp.type = (in & SELECTBITS(GTRACK_HDR_W1_TYPE_bits, GTRACK_HDR_W1_TYPE_lsb)) >> GTRACK_HDR_W1_TYPE_lsb;
		temp.eta_region = (in & SELECTBITS(GTRACK_HDR_W1_ETA_REGION_bits, GTRACK_HDR_W1_ETA_REGION_lsb)) >> GTRACK_HDR_W1_ETA_REGION_lsb;
		temp.phi_region = (in & SELECTBITS(GTRACK_HDR_W1_PHI_REGION_bits, GTRACK_HDR_W1_PHI_REGION_lsb)) >> GTRACK_HDR_W1_PHI_REGION_lsb;
		temp.slice = (in & SELECTBITS(GTRACK_HDR_W1_SLICE_bits, GTRACK_HDR_W1_SLICE_lsb)) >> GTRACK_HDR_W1_SLICE_lsb;
		temp.hough_x_bin = (in & SELECTBITS(GTRACK_HDR_W1_HOUGH_X_BIN_bits, GTRACK_HDR_W1_HOUGH_X_BIN_lsb)) >> GTRACK_HDR_W1_HOUGH_X_BIN_lsb;
		temp.hough_y_bin = (in & SELECTBITS(GTRACK_HDR_W1_HOUGH_Y_BIN_bits, GTRACK_HDR_W1_HOUGH_Y_BIN_lsb)) >> GTRACK_HDR_W1_HOUGH_Y_BIN_lsb;
		temp.second_stage = (in & SELECTBITS(GTRACK_HDR_W1_SECOND_STAGE_bits, GTRACK_HDR_W1_SECOND_STAGE_lsb)) >> GTRACK_HDR_W1_SECOND_STAGE_lsb;
		temp.layer_bitmask = (in & SELECTBITS(GTRACK_HDR_W1_LAYER_BITMASK_bits, GTRACK_HDR_W1_LAYER_BITMASK_lsb)) >> GTRACK_HDR_W1_LAYER_BITMASK_lsb;
		temp.spare = (in & SELECTBITS(GTRACK_HDR_W1_SPARE_bits, GTRACK_HDR_W1_SPARE_lsb)) >> GTRACK_HDR_W1_SPARE_lsb;
		return temp;
	}

	inline GTRACK_HDR_w2 get_bitfields_GTRACK_HDR_w2 (const uint64_t& in) {
		GTRACK_HDR_w2 temp;
		temp.score = (in & SELECTBITS(GTRACK_HDR_W2_SCORE_bits, GTRACK_HDR_W2_SCORE_lsb)) >> GTRACK_HDR_W2_SCORE_lsb;
		temp.d0 = (in & SELECTBITS(GTRACK_HDR_W2_D0_bits, GTRACK_HDR_W2_D0_lsb)) >> GTRACK_HDR_W2_D0_lsb;
		temp.z0 = (in & SELECTBITS(GTRACK_HDR_W2_Z0_bits, GTRACK_HDR_W2_Z0_lsb)) >> GTRACK_HDR_W2_Z0_lsb;
		temp.spare = (in & SELECTBITS(GTRACK_HDR_W2_SPARE_bits, GTRACK_HDR_W2_SPARE_lsb)) >> GTRACK_HDR_W2_SPARE_lsb;
		return temp;
	}

	inline GTRACK_HDR_w3 get_bitfields_GTRACK_HDR_w3 (const uint64_t& in) {
		GTRACK_HDR_w3 temp;
		temp.qoverpt = (in & SELECTBITS(GTRACK_HDR_W3_QOVERPT_bits, GTRACK_HDR_W3_QOVERPT_lsb)) >> GTRACK_HDR_W3_QOVERPT_lsb;
		temp.phi = (in & SELECTBITS(GTRACK_HDR_W3_PHI_bits, GTRACK_HDR_W3_PHI_lsb)) >> GTRACK_HDR_W3_PHI_lsb;
		temp.eta = (in & SELECTBITS(GTRACK_HDR_W3_ETA_bits, GTRACK_HDR_W3_ETA_lsb)) >> GTRACK_HDR_W3_ETA_lsb;
		temp.spare = (in & SELECTBITS(GTRACK_HDR_W3_SPARE_bits, GTRACK_HDR_W3_SPARE_lsb)) >> GTRACK_HDR_W3_SPARE_lsb;
		return temp;
	}
	 
	inline uint64_t get_dataformat_GTRACK_HDR_w1 (const GTRACK_HDR_w1& in) {
		uint64_t temp = 0;
		temp |= (static_cast<uint64_t>(in.flag) << GTRACK_HDR_W1_FLAG_lsb);
		temp |= (static_cast<uint64_t>(in.type) << GTRACK_HDR_W1_TYPE_lsb);
		temp |= (static_cast<uint64_t>(in.eta_region) << GTRACK_HDR_W1_ETA_REGION_lsb);
		temp |= (static_cast<uint64_t>(in.phi_region) << GTRACK_HDR_W1_PHI_REGION_lsb);
		temp |= (static_cast<uint64_t>(in.slice) << GTRACK_HDR_W1_SLICE_lsb);
		temp |= (static_cast<uint64_t>(in.hough_x_bin) << GTRACK_HDR_W1_HOUGH_X_BIN_lsb);
		temp |= (static_cast<uint64_t>(in.hough_y_bin) << GTRACK_HDR_W1_HOUGH_Y_BIN_lsb);
		temp |= (static_cast<uint64_t>(in.second_stage) << GTRACK_HDR_W1_SECOND_STAGE_lsb);
		temp |= (static_cast<uint64_t>(in.layer_bitmask) << GTRACK_HDR_W1_LAYER_BITMASK_lsb);
		temp |= (static_cast<uint64_t>(in.spare) << GTRACK_HDR_W1_SPARE_lsb);
		return temp;
	}

	inline uint64_t get_dataformat_GTRACK_HDR_w2 (const GTRACK_HDR_w2& in) {
		uint64_t temp = 0;
		temp |= (static_cast<uint64_t>(in.score) << GTRACK_HDR_W2_SCORE_lsb);
		temp |= (static_cast<uint64_t>(in.d0) << GTRACK_HDR_W2_D0_lsb);
		temp |= (static_cast<uint64_t>(in.z0) << GTRACK_HDR_W2_Z0_lsb);
		temp |= (static_cast<uint64_t>(in.spare) << GTRACK_HDR_W2_SPARE_lsb);
		return temp;
	}

	inline uint64_t get_dataformat_GTRACK_HDR_w3 (const GTRACK_HDR_w3& in) {
		uint64_t temp = 0;
		temp |= (static_cast<uint64_t>(in.qoverpt) << GTRACK_HDR_W3_QOVERPT_lsb);
		temp |= (static_cast<uint64_t>(in.phi) << GTRACK_HDR_W3_PHI_lsb);
		temp |= (static_cast<uint64_t>(in.eta) << GTRACK_HDR_W3_ETA_lsb);
		temp |= (static_cast<uint64_t>(in.spare) << GTRACK_HDR_W3_SPARE_lsb);
		return temp;
	}

	inline GTRACK_HDR_w1 fill_GTRACK_HDR_w1 (const uint64_t& flag, const uint64_t& type, const uint64_t& eta_region, const uint64_t& phi_region, const uint64_t& slice, const uint64_t& hough_x_bin, const uint64_t& hough_y_bin, const uint64_t& second_stage, const uint64_t& layer_bitmask, const uint64_t& spare) {
		GTRACK_HDR_w1 temp;
		temp.flag = flag;
		temp.type = type;
		temp.eta_region = eta_region;
		temp.phi_region = phi_region;
		temp.slice = slice;
		temp.hough_x_bin = hough_x_bin;
		temp.hough_y_bin = hough_y_bin;
		temp.second_stage = second_stage;
		temp.layer_bitmask = layer_bitmask;
		temp.spare = spare;
		return temp;
	}

	inline GTRACK_HDR_w2 fill_GTRACK_HDR_w2 (const uint64_t& score, const int64_t& d0, const int64_t& z0, const uint64_t& spare) {
		GTRACK_HDR_w2 temp;
		temp.score = score;
		temp.d0 = d0;
		temp.z0 = z0;
		temp.spare = spare;
		return temp;
	}

	inline GTRACK_HDR_w3 fill_GTRACK_HDR_w3 (const int64_t& qoverpt, const int64_t& phi, const int64_t& eta, const uint64_t& spare) {
		GTRACK_HDR_w3 temp;
		temp.qoverpt = qoverpt;
		temp.phi = phi;
		temp.eta = eta;
		temp.spare = spare;
		return temp;
	}

	inline uint64_t to_real_GTRACK_HDR_w1_flag (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_GTRACK_HDR_w1_type (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_GTRACK_HDR_w1_eta_region (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_GTRACK_HDR_w1_phi_region (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_GTRACK_HDR_w1_slice (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_GTRACK_HDR_w1_hough_x_bin (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_GTRACK_HDR_w1_hough_y_bin (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_GTRACK_HDR_w1_second_stage (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_GTRACK_HDR_w1_layer_bitmask (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_GTRACK_HDR_w1_spare (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_GTRACK_HDR_w2_score (const uint64_t& in) {
		return in;
	}

	inline int64_t to_real_GTRACK_HDR_w2_d0 (const int64_t& in) {
		return in;
	}

	inline int64_t to_real_GTRACK_HDR_w2_z0 (const int64_t& in) {
		return in;
	}

	inline uint64_t to_real_GTRACK_HDR_w2_spare (const uint64_t& in) {
		return in;
	}

	inline int64_t to_real_GTRACK_HDR_w3_qoverpt (const int64_t& in) {
		return in;
	}

	inline int64_t to_real_GTRACK_HDR_w3_phi (const int64_t& in) {
		return in;
	}

	inline int64_t to_real_GTRACK_HDR_w3_eta (const int64_t& in) {
		return in;
	}

	inline uint64_t to_real_GTRACK_HDR_w3_spare (const uint64_t& in) {
		return in;
	}

	// PIXEL_CLUSTER word description
	const int PIXEL_CLUSTER_LAST_bits = 1;
	const int PIXEL_CLUSTER_LAST_lsb = 63;
	const float PIXEL_CLUSTER_LAST_mf = 1.;

	const int PIXEL_CLUSTER_COL_bits = 13;
	const int PIXEL_CLUSTER_COL_lsb = 50;
	const float PIXEL_CLUSTER_COL_mf = 1.;

	const int PIXEL_CLUSTER_ROW_bits = 13;
	const int PIXEL_CLUSTER_ROW_lsb = 37;
	const float PIXEL_CLUSTER_ROW_mf = 1.;

	const int PIXEL_CLUSTER_CLUSTERID_bits = 13;
	const int PIXEL_CLUSTER_CLUSTERID_lsb = 24;
	const float PIXEL_CLUSTER_CLUSTERID_mf = 1.;

	const int PIXEL_CLUSTER_CENTROID_COL_bits = 12;
	const int PIXEL_CLUSTER_CENTROID_COL_lsb = 12;
	const float PIXEL_CLUSTER_CENTROID_COL_mf = 16.;

	const int PIXEL_CLUSTER_CENTROID_ROW_bits = 12;
	const int PIXEL_CLUSTER_CENTROID_ROW_lsb = 0;
	const float PIXEL_CLUSTER_CENTROID_ROW_mf = 16.;

	typedef struct PIXEL_CLUSTER {
		uint64_t last : PIXEL_CLUSTER_LAST_bits;
		uint64_t col : PIXEL_CLUSTER_COL_bits;
		uint64_t row : PIXEL_CLUSTER_ROW_bits;
		uint64_t clusterid : PIXEL_CLUSTER_CLUSTERID_bits;
		uint64_t centroid_col : PIXEL_CLUSTER_CENTROID_COL_bits;
		uint64_t centroid_row : PIXEL_CLUSTER_CENTROID_ROW_bits;
	} PIXEL_CLUSTER;

	inline PIXEL_CLUSTER get_bitfields_PIXEL_CLUSTER (const uint64_t& in) {
		PIXEL_CLUSTER temp;
		temp.last = (in & SELECTBITS(PIXEL_CLUSTER_LAST_bits, PIXEL_CLUSTER_LAST_lsb)) >> PIXEL_CLUSTER_LAST_lsb;
		temp.col = (in & SELECTBITS(PIXEL_CLUSTER_COL_bits, PIXEL_CLUSTER_COL_lsb)) >> PIXEL_CLUSTER_COL_lsb;
		temp.row = (in & SELECTBITS(PIXEL_CLUSTER_ROW_bits, PIXEL_CLUSTER_ROW_lsb)) >> PIXEL_CLUSTER_ROW_lsb;
		temp.clusterid = (in & SELECTBITS(PIXEL_CLUSTER_CLUSTERID_bits, PIXEL_CLUSTER_CLUSTERID_lsb)) >> PIXEL_CLUSTER_CLUSTERID_lsb;
		temp.centroid_col = (in & SELECTBITS(PIXEL_CLUSTER_CENTROID_COL_bits, PIXEL_CLUSTER_CENTROID_COL_lsb)) >> PIXEL_CLUSTER_CENTROID_COL_lsb;
		temp.centroid_row = (in & SELECTBITS(PIXEL_CLUSTER_CENTROID_ROW_bits, PIXEL_CLUSTER_CENTROID_ROW_lsb)) >> PIXEL_CLUSTER_CENTROID_ROW_lsb;
		return temp;
	}

	inline uint64_t get_dataformat_PIXEL_CLUSTER (const PIXEL_CLUSTER& in) {
		uint64_t temp = 0;
		temp |= (static_cast<uint64_t>(in.last) << PIXEL_CLUSTER_LAST_lsb);
		temp |= (static_cast<uint64_t>(in.col) << PIXEL_CLUSTER_COL_lsb);
		temp |= (static_cast<uint64_t>(in.row) << PIXEL_CLUSTER_ROW_lsb);
		temp |= (static_cast<uint64_t>(in.clusterid) << PIXEL_CLUSTER_CLUSTERID_lsb);
		temp |= (static_cast<uint64_t>(in.centroid_col) << PIXEL_CLUSTER_CENTROID_COL_lsb);
		temp |= (static_cast<uint64_t>(in.centroid_row) << PIXEL_CLUSTER_CENTROID_ROW_lsb);
		return temp;
	}

	inline PIXEL_CLUSTER fill_PIXEL_CLUSTER (const uint64_t& last, const uint64_t& col, const uint64_t& row, const uint64_t& clusterid, const double& centroid_col, const double& centroid_row) {
		PIXEL_CLUSTER temp;
		temp.last = last;
		temp.col = col;
		temp.row = row;
		temp.clusterid = clusterid;
		temp.centroid_col = (uint64_t)(centroid_col * PIXEL_CLUSTER_CENTROID_COL_mf);
		temp.centroid_row = (uint64_t)(centroid_row * PIXEL_CLUSTER_CENTROID_ROW_mf);
		return temp;
	}

	inline uint64_t to_real_PIXEL_CLUSTER_last (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_PIXEL_CLUSTER_col (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_PIXEL_CLUSTER_row (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_PIXEL_CLUSTER_clusterid (const uint64_t& in) {
		return in;
	}

	inline double to_real_PIXEL_CLUSTER_centroid_col (const uint64_t& in) {
		return (double)in / PIXEL_CLUSTER_CENTROID_COL_mf;
	}

	inline double to_real_PIXEL_CLUSTER_centroid_row (const uint64_t& in) {
		return (double)in / PIXEL_CLUSTER_CENTROID_ROW_mf;
	}

	// STRIP_CLUSTER word description
	const int STRIP_CLUSTER_LAST_bits = 1;
	const int STRIP_CLUSTER_LAST_lsb = 31;
	const float STRIP_CLUSTER_LAST_mf = 1.;

	const int STRIP_CLUSTER_ROW_bits = 1;
	const int STRIP_CLUSTER_ROW_lsb = 30;
	const float STRIP_CLUSTER_ROW_mf = 1.;

	const int STRIP_CLUSTER_NSTRIPS_bits = 2;
	const int STRIP_CLUSTER_NSTRIPS_lsb = 28;
	const float STRIP_CLUSTER_NSTRIPS_mf = 1.;

	const int STRIP_CLUSTER_STRIP_INDEX_bits = 12;
	const int STRIP_CLUSTER_STRIP_INDEX_lsb = 16;
	const float STRIP_CLUSTER_STRIP_INDEX_mf = 1.;

	const int STRIP_CLUSTER_CLUSTERID_bits = 13;
	const int STRIP_CLUSTER_CLUSTERID_lsb = 3;
	const float STRIP_CLUSTER_CLUSTERID_mf = 1.;

	const int STRIP_CLUSTER_SPARE_bits = 3;
	const int STRIP_CLUSTER_SPARE_lsb = 0;
	const float STRIP_CLUSTER_SPARE_mf = 1.;

	typedef struct STRIP_CLUSTER {
		uint64_t last : STRIP_CLUSTER_LAST_bits;
		uint64_t row : STRIP_CLUSTER_ROW_bits;
		uint64_t nstrips : STRIP_CLUSTER_NSTRIPS_bits;
		uint64_t strip_index : STRIP_CLUSTER_STRIP_INDEX_bits;
		uint64_t clusterid : STRIP_CLUSTER_CLUSTERID_bits;
		uint64_t spare : STRIP_CLUSTER_SPARE_bits;
	} STRIP_CLUSTER;

	// Even though the input is 64 bits, this function really only operates on the lower 32 bits
	// Make sure to shift right by 32 bits to get the upper 32 bits
	inline STRIP_CLUSTER get_bitfields_STRIP_CLUSTER (const uint64_t& in) {
		STRIP_CLUSTER temp;
		temp.last = (in & SELECTBITS(STRIP_CLUSTER_LAST_bits, STRIP_CLUSTER_LAST_lsb)) >> STRIP_CLUSTER_LAST_lsb;
		temp.row = (in & SELECTBITS(STRIP_CLUSTER_ROW_bits, STRIP_CLUSTER_ROW_lsb)) >> STRIP_CLUSTER_ROW_lsb;
		temp.nstrips = (in & SELECTBITS(STRIP_CLUSTER_NSTRIPS_bits, STRIP_CLUSTER_NSTRIPS_lsb)) >> STRIP_CLUSTER_NSTRIPS_lsb;
		temp.strip_index = (in & SELECTBITS(STRIP_CLUSTER_STRIP_INDEX_bits, STRIP_CLUSTER_STRIP_INDEX_lsb)) >> STRIP_CLUSTER_STRIP_INDEX_lsb;
		temp.clusterid = (in & SELECTBITS(STRIP_CLUSTER_CLUSTERID_bits, STRIP_CLUSTER_CLUSTERID_lsb)) >> STRIP_CLUSTER_CLUSTERID_lsb;
		temp.spare = (in & SELECTBITS(STRIP_CLUSTER_SPARE_bits, STRIP_CLUSTER_SPARE_lsb)) >> STRIP_CLUSTER_SPARE_lsb;
		return temp;
	}

	inline uint64_t get_dataformat_STRIP_CLUSTER_up32 (const uint64_t& in) {
		return (in & SELECTBITS(32, 32)) >> 32;
}

	inline uint64_t get_dataformat_STRIP_CLUSTER_low32 (const uint64_t& in) {
		return (in & SELECTBITS(32, 0));
}

	inline uint64_t get_dataformat_STRIP_CLUSTER (const STRIP_CLUSTER& in) {
		uint64_t temp = 0;
		temp |= (static_cast<uint64_t>(in.last) << STRIP_CLUSTER_LAST_lsb);
		temp |= (static_cast<uint64_t>(in.row) << STRIP_CLUSTER_ROW_lsb);
		temp |= (static_cast<uint64_t>(in.nstrips) << STRIP_CLUSTER_NSTRIPS_lsb);
		temp |= (static_cast<uint64_t>(in.strip_index) << STRIP_CLUSTER_STRIP_INDEX_lsb);
		temp |= (static_cast<uint64_t>(in.clusterid) << STRIP_CLUSTER_CLUSTERID_lsb);
		temp |= (static_cast<uint64_t>(in.spare) << STRIP_CLUSTER_SPARE_lsb);
		return temp;
	}

	inline uint64_t get_dataformat_STRIP_CLUSTER_64 (const uint64_t& up, const uint64_t& low) {
		uint64_t temp = up << 32;
		return (temp | low);
}

	inline STRIP_CLUSTER fill_STRIP_CLUSTER (const uint64_t& last, const uint64_t& row, const uint64_t& nstrips, const uint64_t& strip_index, const uint64_t& clusterid, const uint64_t& spare) {
		STRIP_CLUSTER temp;
		temp.last = last;
		temp.row = row;
		temp.nstrips = nstrips;
		temp.strip_index = strip_index;
		temp.clusterid = clusterid;
		temp.spare = spare;
		return temp;
	}

	inline uint64_t to_real_STRIP_CLUSTER_last (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_STRIP_CLUSTER_row (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_STRIP_CLUSTER_nstrips (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_STRIP_CLUSTER_strip_index (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_STRIP_CLUSTER_clusterid (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_STRIP_CLUSTER_spare (const uint64_t& in) {
		return in;
	}

	// GHITZ_W1 word description
	const int GHITZ_W1_LAST_bits = 1;
	const int GHITZ_W1_LAST_lsb = 63;
	const float GHITZ_W1_LAST_mf = 1.;

	const int GHITZ_W1_LYR_bits = 4;
	const int GHITZ_W1_LYR_lsb = 59;
	const float GHITZ_W1_LYR_mf = 1.;

	const int GHITZ_W1_RAD_bits = 18;
	const int GHITZ_W1_RAD_lsb = 41;
	const float GHITZ_W1_RAD_mf = 256.;

	const int GHITZ_W1_PHI_bits = 16;
	const int GHITZ_W1_PHI_lsb = 25;
	const float GHITZ_W1_PHI_mf = 8192.;

	const int GHITZ_W1_Z_bits = 18;
	const int GHITZ_W1_Z_lsb = 7;
	const float GHITZ_W1_Z_mf = 128.;

	const int GHITZ_W1_ROW_bits = 6;
	const int GHITZ_W1_ROW_lsb = 1;
	const float GHITZ_W1_ROW_mf = 1.;

	const int GHITZ_W1_LASTOFSLICE_bits = 1;
	const int GHITZ_W1_LASTOFSLICE_lsb = 0;
	const float GHITZ_W1_LASTOFSLICE_mf = 1.;

	// GHITZ_W2 word description
	const int GHITZ_W2_CLUSTER1_bits = 13;
	const int GHITZ_W2_CLUSTER1_lsb = 51;
	const float GHITZ_W2_CLUSTER1_mf = 1.;

	const int GHITZ_W2_CLUSTER2_bits = 13;
	const int GHITZ_W2_CLUSTER2_lsb = 38;
	const float GHITZ_W2_CLUSTER2_mf = 1.;

	const int GHITZ_W2_SPARE_bits = 38;
	const int GHITZ_W2_SPARE_lsb = 0;
	const float GHITZ_W2_SPARE_mf = 1.;

	typedef struct GHITZ_w1 {
		uint64_t last : GHITZ_W1_LAST_bits;
		uint64_t lyr : GHITZ_W1_LYR_bits;
		uint64_t rad : GHITZ_W1_RAD_bits;
		int64_t phi : GHITZ_W1_PHI_bits;
		int64_t z : GHITZ_W1_Z_bits;
		uint64_t row : GHITZ_W1_ROW_bits;
		uint64_t lastofslice : GHITZ_W1_LASTOFSLICE_bits;
	} GHITZ_w1;

	typedef struct GHITZ_w2 {
		uint64_t cluster1 : GHITZ_W2_CLUSTER1_bits;
		uint64_t cluster2 : GHITZ_W2_CLUSTER2_bits;
		uint64_t spare : GHITZ_W2_SPARE_bits;
	} GHITZ_w2;

	inline GHITZ_w1 get_bitfields_GHITZ_w1 (const uint64_t& in) {
		GHITZ_w1 temp;
		temp.last = (in & SELECTBITS(GHITZ_W1_LAST_bits, GHITZ_W1_LAST_lsb)) >> GHITZ_W1_LAST_lsb;
		temp.lyr = (in & SELECTBITS(GHITZ_W1_LYR_bits, GHITZ_W1_LYR_lsb)) >> GHITZ_W1_LYR_lsb;
		temp.rad = (in & SELECTBITS(GHITZ_W1_RAD_bits, GHITZ_W1_RAD_lsb)) >> GHITZ_W1_RAD_lsb;
		temp.phi = (in & SELECTBITS(GHITZ_W1_PHI_bits, GHITZ_W1_PHI_lsb)) >> GHITZ_W1_PHI_lsb;
		temp.z = (in & SELECTBITS(GHITZ_W1_Z_bits, GHITZ_W1_Z_lsb)) >> GHITZ_W1_Z_lsb;
		temp.row = (in & SELECTBITS(GHITZ_W1_ROW_bits, GHITZ_W1_ROW_lsb)) >> GHITZ_W1_ROW_lsb;
		temp.lastofslice = (in & SELECTBITS(GHITZ_W1_LASTOFSLICE_bits, GHITZ_W1_LASTOFSLICE_lsb)) >> GHITZ_W1_LASTOFSLICE_lsb;
		return temp;
	}

	inline GHITZ_w2 get_bitfields_GHITZ_w2 (const uint64_t& in) {
		GHITZ_w2 temp;
		temp.cluster1 = (in & SELECTBITS(GHITZ_W2_CLUSTER1_bits, GHITZ_W2_CLUSTER1_lsb)) >> GHITZ_W2_CLUSTER1_lsb;
		temp.cluster2 = (in & SELECTBITS(GHITZ_W2_CLUSTER2_bits, GHITZ_W2_CLUSTER2_lsb)) >> GHITZ_W2_CLUSTER2_lsb;
		temp.spare = (in & SELECTBITS(GHITZ_W2_SPARE_bits, GHITZ_W2_SPARE_lsb)) >> GHITZ_W2_SPARE_lsb;
		return temp;
	}

	inline uint64_t get_dataformat_GHITZ_w1 (const GHITZ_w1& in) {
		uint64_t temp = 0;
		temp |= (static_cast<uint64_t>(in.last) << GHITZ_W1_LAST_lsb);
		temp |= (static_cast<uint64_t>(in.lyr) << GHITZ_W1_LYR_lsb);
		temp |= (static_cast<uint64_t>(in.rad) << GHITZ_W1_RAD_lsb);
		temp |= (static_cast<uint64_t>(in.phi) << GHITZ_W1_PHI_lsb);
		temp |= (static_cast<uint64_t>(in.z) << GHITZ_W1_Z_lsb);
		temp |= (static_cast<uint64_t>(in.row) << GHITZ_W1_ROW_lsb);
		temp |= (static_cast<uint64_t>(in.lastofslice) << GHITZ_W1_LASTOFSLICE_lsb);
		return temp;
	}

	inline uint64_t get_dataformat_GHITZ_w2 (const GHITZ_w2& in) {
		uint64_t temp = 0;
		temp |= (static_cast<uint64_t>(in.cluster1) << GHITZ_W2_CLUSTER1_lsb);
		temp |= (static_cast<uint64_t>(in.cluster2) << GHITZ_W2_CLUSTER2_lsb);
		temp |= (static_cast<uint64_t>(in.spare) << GHITZ_W2_SPARE_lsb);
		return temp;
	}

	inline GHITZ_w1 fill_GHITZ_w1 (const uint64_t& last, const uint64_t& lyr, const double& rad, const double& phi, const double& z, const uint64_t& row, const uint64_t& lastofslice) {
		GHITZ_w1 temp;
		temp.last = last;
		temp.lyr = lyr;
		temp.rad = (uint64_t)(rad * GHITZ_W1_RAD_mf);
		temp.phi = (int64_t)(phi * GHITZ_W1_PHI_mf);
		temp.z = (int64_t)(z * GHITZ_W1_Z_mf);
		temp.row = row;
		temp.lastofslice = lastofslice;
		return temp;
	}

	inline GHITZ_w2 fill_GHITZ_w2 (const uint64_t& cluster1, const uint64_t& cluster2, const uint64_t& spare) {
		GHITZ_w2 temp;
		temp.cluster1 = cluster1;
		temp.cluster2 = cluster2;
		temp.spare = spare;
		return temp;
	}

	inline uint64_t to_real_GHITZ_w1_last (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_GHITZ_w1_lyr (const uint64_t& in) {
		return in;
	}

	inline double to_real_GHITZ_w1_rad (const uint64_t& in) {
		return (double)in / GHITZ_W1_RAD_mf;
	}

	inline double to_real_GHITZ_w1_phi (const int64_t& in) {
		return (double)in / GHITZ_W1_PHI_mf;
	}

	inline double to_real_GHITZ_w1_z (const int64_t& in) {
		return (double)in / GHITZ_W1_Z_mf;
	}

	inline uint64_t to_real_GHITZ_w1_row (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_GHITZ_w1_lastofslice (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_GHITZ_w2_cluster1 (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_GHITZ_w2_cluster2 (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_GHITZ_w2_spare (const uint64_t& in) {
		return in;
	}

	// EDM_STRIPCLUSTER_W1 word description
	const int EDM_STRIPCLUSTER_W1_ID_HASH_bits = 32;
	const int EDM_STRIPCLUSTER_W1_ID_HASH_lsb = 32;
	const float EDM_STRIPCLUSTER_W1_ID_HASH_mf = 1.;

	const int EDM_STRIPCLUSTER_W1_IDENTIFIER_bits = 32;
	const int EDM_STRIPCLUSTER_W1_IDENTIFIER_lsb = 0;
	const float EDM_STRIPCLUSTER_W1_IDENTIFIER_mf = 1.;

	// EDM_STRIPCLUSTER_W2 word description
	const int EDM_STRIPCLUSTER_W2_RDO_LIST_W1_bits = 32;
	const int EDM_STRIPCLUSTER_W2_RDO_LIST_W1_lsb = 32;
	const float EDM_STRIPCLUSTER_W2_RDO_LIST_W1_mf = 1.;

	const int EDM_STRIPCLUSTER_W2_RDO_LIST_W2_bits = 32;
	const int EDM_STRIPCLUSTER_W2_RDO_LIST_W2_lsb = 0;
	const float EDM_STRIPCLUSTER_W2_RDO_LIST_W2_mf = 1.;

	// EDM_STRIPCLUSTER_W3 word description
	const int EDM_STRIPCLUSTER_W3_RDO_LIST_W3_bits = 32;
	const int EDM_STRIPCLUSTER_W3_RDO_LIST_W3_lsb = 32;
	const float EDM_STRIPCLUSTER_W3_RDO_LIST_W3_mf = 1.;

	const int EDM_STRIPCLUSTER_W3_RDO_LIST_W4_bits = 32;
	const int EDM_STRIPCLUSTER_W3_RDO_LIST_W4_lsb = 0;
	const float EDM_STRIPCLUSTER_W3_RDO_LIST_W4_mf = 1.;

	// EDM_STRIPCLUSTER_W4 word description
	const int EDM_STRIPCLUSTER_W4_LOCALPOSITION_X_bits = 21;
	const int EDM_STRIPCLUSTER_W4_LOCALPOSITION_X_lsb = 43;
	const float EDM_STRIPCLUSTER_W4_LOCALPOSITION_X_mf = 8192.;

	const int EDM_STRIPCLUSTER_W4_LOCALPOSITION_Y_bits = 21;
	const int EDM_STRIPCLUSTER_W4_LOCALPOSITION_Y_lsb = 22;
	const float EDM_STRIPCLUSTER_W4_LOCALPOSITION_Y_mf = 8192.;

	const int EDM_STRIPCLUSTER_W4_LOCALCOVARIANCE_XX_bits = 20;
	const int EDM_STRIPCLUSTER_W4_LOCALCOVARIANCE_XX_lsb = 2;
	const float EDM_STRIPCLUSTER_W4_LOCALCOVARIANCE_XX_mf = 524288.;

	const int EDM_STRIPCLUSTER_W4_SPARE_bits = 2;
	const int EDM_STRIPCLUSTER_W4_SPARE_lsb = 0;
	const float EDM_STRIPCLUSTER_W4_SPARE_mf = 1.;

	// EDM_STRIPCLUSTER_W5 word description
	const int EDM_STRIPCLUSTER_W5_GLOBALPOSITION_X_bits = 27;
	const int EDM_STRIPCLUSTER_W5_GLOBALPOSITION_X_lsb = 37;
	const float EDM_STRIPCLUSTER_W5_GLOBALPOSITION_X_mf = 65536.;

	const int EDM_STRIPCLUSTER_W5_GLOBALPOSITION_Y_bits = 27;
	const int EDM_STRIPCLUSTER_W5_GLOBALPOSITION_Y_lsb = 10;
	const float EDM_STRIPCLUSTER_W5_GLOBALPOSITION_Y_mf = 65536.;

	const int EDM_STRIPCLUSTER_W5_CHANNELS_IN_PHI_bits = 6;
	const int EDM_STRIPCLUSTER_W5_CHANNELS_IN_PHI_lsb = 4;
	const float EDM_STRIPCLUSTER_W5_CHANNELS_IN_PHI_mf = 1.;

	const int EDM_STRIPCLUSTER_W5_SPARE_bits = 4;
	const int EDM_STRIPCLUSTER_W5_SPARE_lsb = 0;
	const float EDM_STRIPCLUSTER_W5_SPARE_mf = 1.;

	// EDM_STRIPCLUSTER_W6 word description
	const int EDM_STRIPCLUSTER_W6_GLOBALPOSITION_Z_bits = 29;
	const int EDM_STRIPCLUSTER_W6_GLOBALPOSITION_Z_lsb = 35;
	const float EDM_STRIPCLUSTER_W6_GLOBALPOSITION_Z_mf = 65536.;

	const int EDM_STRIPCLUSTER_W6_LASTWORD_bits = 1;
	const int EDM_STRIPCLUSTER_W6_LASTWORD_lsb = 34;
	const float EDM_STRIPCLUSTER_W6_LASTWORD_mf = 1.;

	const int EDM_STRIPCLUSTER_W6_INDEX_bits = 32;
	const int EDM_STRIPCLUSTER_W6_INDEX_lsb = 2;
	const float EDM_STRIPCLUSTER_W6_INDEX_mf = 1.;

	const int EDM_STRIPCLUSTER_W6_SPARE_bits = 2;
	const int EDM_STRIPCLUSTER_W6_SPARE_lsb = 0;
	const float EDM_STRIPCLUSTER_W6_SPARE_mf = 1.;

	typedef struct EDM_STRIPCLUSTER_w1 {
		uint64_t id_hash : EDM_STRIPCLUSTER_W1_ID_HASH_bits;
		uint64_t identifier : EDM_STRIPCLUSTER_W1_IDENTIFIER_bits;
	} EDM_STRIPCLUSTER_w1;

	typedef struct EDM_STRIPCLUSTER_w2 {
		uint64_t rdo_list_w1 : EDM_STRIPCLUSTER_W2_RDO_LIST_W1_bits;
		uint64_t rdo_list_w2 : EDM_STRIPCLUSTER_W2_RDO_LIST_W2_bits;
	} EDM_STRIPCLUSTER_w2;

	typedef struct EDM_STRIPCLUSTER_w3 {
		uint64_t rdo_list_w3 : EDM_STRIPCLUSTER_W3_RDO_LIST_W3_bits;
		uint64_t rdo_list_w4 : EDM_STRIPCLUSTER_W3_RDO_LIST_W4_bits;
	} EDM_STRIPCLUSTER_w3;

	typedef struct EDM_STRIPCLUSTER_w4 {
		int64_t localposition_x : EDM_STRIPCLUSTER_W4_LOCALPOSITION_X_bits;
		int64_t localposition_y : EDM_STRIPCLUSTER_W4_LOCALPOSITION_Y_bits;
		uint64_t localcovariance_xx : EDM_STRIPCLUSTER_W4_LOCALCOVARIANCE_XX_bits;
		uint64_t spare : EDM_STRIPCLUSTER_W4_SPARE_bits;
	} EDM_STRIPCLUSTER_w4;

	typedef struct EDM_STRIPCLUSTER_w5 {
		int64_t globalposition_x : EDM_STRIPCLUSTER_W5_GLOBALPOSITION_X_bits;
		int64_t globalposition_y : EDM_STRIPCLUSTER_W5_GLOBALPOSITION_Y_bits;
		uint64_t channels_in_phi : EDM_STRIPCLUSTER_W5_CHANNELS_IN_PHI_bits;
		uint64_t spare : EDM_STRIPCLUSTER_W5_SPARE_bits;
	} EDM_STRIPCLUSTER_w5;

	typedef struct EDM_STRIPCLUSTER_w6 {
		int64_t globalposition_z : EDM_STRIPCLUSTER_W6_GLOBALPOSITION_Z_bits;
		uint64_t lastword : EDM_STRIPCLUSTER_W6_LASTWORD_bits;
		uint64_t index : EDM_STRIPCLUSTER_W6_INDEX_bits;
		uint64_t spare : EDM_STRIPCLUSTER_W6_SPARE_bits;
	} EDM_STRIPCLUSTER_w6;

	inline EDM_STRIPCLUSTER_w1 get_bitfields_EDM_STRIPCLUSTER_w1 (const uint64_t& in) {
		EDM_STRIPCLUSTER_w1 temp;
		temp.id_hash = (in & SELECTBITS(EDM_STRIPCLUSTER_W1_ID_HASH_bits, EDM_STRIPCLUSTER_W1_ID_HASH_lsb)) >> EDM_STRIPCLUSTER_W1_ID_HASH_lsb;
		temp.identifier = (in & SELECTBITS(EDM_STRIPCLUSTER_W1_IDENTIFIER_bits, EDM_STRIPCLUSTER_W1_IDENTIFIER_lsb)) >> EDM_STRIPCLUSTER_W1_IDENTIFIER_lsb;
		return temp;
	}

	inline EDM_STRIPCLUSTER_w2 get_bitfields_EDM_STRIPCLUSTER_w2 (const uint64_t& in) {
		EDM_STRIPCLUSTER_w2 temp;
		temp.rdo_list_w1 = (in & SELECTBITS(EDM_STRIPCLUSTER_W2_RDO_LIST_W1_bits, EDM_STRIPCLUSTER_W2_RDO_LIST_W1_lsb)) >> EDM_STRIPCLUSTER_W2_RDO_LIST_W1_lsb;
		temp.rdo_list_w2 = (in & SELECTBITS(EDM_STRIPCLUSTER_W2_RDO_LIST_W2_bits, EDM_STRIPCLUSTER_W2_RDO_LIST_W2_lsb)) >> EDM_STRIPCLUSTER_W2_RDO_LIST_W2_lsb;
		return temp;
	}

	inline EDM_STRIPCLUSTER_w3 get_bitfields_EDM_STRIPCLUSTER_w3 (const uint64_t& in) {
		EDM_STRIPCLUSTER_w3 temp;
		temp.rdo_list_w3 = (in & SELECTBITS(EDM_STRIPCLUSTER_W3_RDO_LIST_W3_bits, EDM_STRIPCLUSTER_W3_RDO_LIST_W3_lsb)) >> EDM_STRIPCLUSTER_W3_RDO_LIST_W3_lsb;
		temp.rdo_list_w4 = (in & SELECTBITS(EDM_STRIPCLUSTER_W3_RDO_LIST_W4_bits, EDM_STRIPCLUSTER_W3_RDO_LIST_W4_lsb)) >> EDM_STRIPCLUSTER_W3_RDO_LIST_W4_lsb;
		return temp;
	}

	inline EDM_STRIPCLUSTER_w4 get_bitfields_EDM_STRIPCLUSTER_w4 (const uint64_t& in) {
		EDM_STRIPCLUSTER_w4 temp;
		temp.localposition_x = (in & SELECTBITS(EDM_STRIPCLUSTER_W4_LOCALPOSITION_X_bits, EDM_STRIPCLUSTER_W4_LOCALPOSITION_X_lsb)) >> EDM_STRIPCLUSTER_W4_LOCALPOSITION_X_lsb;
		temp.localposition_y = (in & SELECTBITS(EDM_STRIPCLUSTER_W4_LOCALPOSITION_Y_bits, EDM_STRIPCLUSTER_W4_LOCALPOSITION_Y_lsb)) >> EDM_STRIPCLUSTER_W4_LOCALPOSITION_Y_lsb;
		temp.localcovariance_xx = (in & SELECTBITS(EDM_STRIPCLUSTER_W4_LOCALCOVARIANCE_XX_bits, EDM_STRIPCLUSTER_W4_LOCALCOVARIANCE_XX_lsb)) >> EDM_STRIPCLUSTER_W4_LOCALCOVARIANCE_XX_lsb;
		temp.spare = (in & SELECTBITS(EDM_STRIPCLUSTER_W4_SPARE_bits, EDM_STRIPCLUSTER_W4_SPARE_lsb)) >> EDM_STRIPCLUSTER_W4_SPARE_lsb;
		return temp;
	}

	inline EDM_STRIPCLUSTER_w5 get_bitfields_EDM_STRIPCLUSTER_w5 (const uint64_t& in) {
		EDM_STRIPCLUSTER_w5 temp;
		temp.globalposition_x = (in & SELECTBITS(EDM_STRIPCLUSTER_W5_GLOBALPOSITION_X_bits, EDM_STRIPCLUSTER_W5_GLOBALPOSITION_X_lsb)) >> EDM_STRIPCLUSTER_W5_GLOBALPOSITION_X_lsb;
		temp.globalposition_y = (in & SELECTBITS(EDM_STRIPCLUSTER_W5_GLOBALPOSITION_Y_bits, EDM_STRIPCLUSTER_W5_GLOBALPOSITION_Y_lsb)) >> EDM_STRIPCLUSTER_W5_GLOBALPOSITION_Y_lsb;
		temp.channels_in_phi = (in & SELECTBITS(EDM_STRIPCLUSTER_W5_CHANNELS_IN_PHI_bits, EDM_STRIPCLUSTER_W5_CHANNELS_IN_PHI_lsb)) >> EDM_STRIPCLUSTER_W5_CHANNELS_IN_PHI_lsb;
		temp.spare = (in & SELECTBITS(EDM_STRIPCLUSTER_W5_SPARE_bits, EDM_STRIPCLUSTER_W5_SPARE_lsb)) >> EDM_STRIPCLUSTER_W5_SPARE_lsb;
		return temp;
	}

	inline EDM_STRIPCLUSTER_w6 get_bitfields_EDM_STRIPCLUSTER_w6 (const uint64_t& in) {
		EDM_STRIPCLUSTER_w6 temp;
		temp.globalposition_z = (in & SELECTBITS(EDM_STRIPCLUSTER_W6_GLOBALPOSITION_Z_bits, EDM_STRIPCLUSTER_W6_GLOBALPOSITION_Z_lsb)) >> EDM_STRIPCLUSTER_W6_GLOBALPOSITION_Z_lsb;
		temp.lastword = (in & SELECTBITS(EDM_STRIPCLUSTER_W6_LASTWORD_bits, EDM_STRIPCLUSTER_W6_LASTWORD_lsb)) >> EDM_STRIPCLUSTER_W6_LASTWORD_lsb;
		temp.index = (in & SELECTBITS(EDM_STRIPCLUSTER_W6_INDEX_bits, EDM_STRIPCLUSTER_W6_INDEX_lsb)) >> EDM_STRIPCLUSTER_W6_INDEX_lsb;
		temp.spare = (in & SELECTBITS(EDM_STRIPCLUSTER_W6_SPARE_bits, EDM_STRIPCLUSTER_W6_SPARE_lsb)) >> EDM_STRIPCLUSTER_W6_SPARE_lsb;
		return temp;
	}

	inline uint64_t get_dataformat_EDM_STRIPCLUSTER_w1 (const EDM_STRIPCLUSTER_w1& in) {
		uint64_t temp = 0;
		temp |= (static_cast<uint64_t>(in.id_hash) << EDM_STRIPCLUSTER_W1_ID_HASH_lsb);
		temp |= (static_cast<uint64_t>(in.identifier) << EDM_STRIPCLUSTER_W1_IDENTIFIER_lsb);
		return temp;
	}

	inline uint64_t get_dataformat_EDM_STRIPCLUSTER_w2 (const EDM_STRIPCLUSTER_w2& in) {
		uint64_t temp = 0;
		temp |= (static_cast<uint64_t>(in.rdo_list_w1) << EDM_STRIPCLUSTER_W2_RDO_LIST_W1_lsb);
		temp |= (static_cast<uint64_t>(in.rdo_list_w2) << EDM_STRIPCLUSTER_W2_RDO_LIST_W2_lsb);
		return temp;
	}

	inline uint64_t get_dataformat_EDM_STRIPCLUSTER_w3 (const EDM_STRIPCLUSTER_w3& in) {
		uint64_t temp = 0;
		temp |= (static_cast<uint64_t>(in.rdo_list_w3) << EDM_STRIPCLUSTER_W3_RDO_LIST_W3_lsb);
		temp |= (static_cast<uint64_t>(in.rdo_list_w4) << EDM_STRIPCLUSTER_W3_RDO_LIST_W4_lsb);
		return temp;
	}

	inline uint64_t get_dataformat_EDM_STRIPCLUSTER_w4 (const EDM_STRIPCLUSTER_w4& in) {
		uint64_t temp = 0;
		temp |= (static_cast<uint64_t>(in.localposition_x) << EDM_STRIPCLUSTER_W4_LOCALPOSITION_X_lsb);
		temp |= (static_cast<uint64_t>(in.localposition_y) << EDM_STRIPCLUSTER_W4_LOCALPOSITION_Y_lsb);
		temp |= (static_cast<uint64_t>(in.localcovariance_xx) << EDM_STRIPCLUSTER_W4_LOCALCOVARIANCE_XX_lsb);
		temp |= (static_cast<uint64_t>(in.spare) << EDM_STRIPCLUSTER_W4_SPARE_lsb);
		return temp;
	}

	inline uint64_t get_dataformat_EDM_STRIPCLUSTER_w5 (const EDM_STRIPCLUSTER_w5& in) {
		uint64_t temp = 0;
		temp |= (static_cast<uint64_t>(in.globalposition_x) << EDM_STRIPCLUSTER_W5_GLOBALPOSITION_X_lsb);
		temp |= (static_cast<uint64_t>(in.globalposition_y) << EDM_STRIPCLUSTER_W5_GLOBALPOSITION_Y_lsb);
		temp |= (static_cast<uint64_t>(in.channels_in_phi) << EDM_STRIPCLUSTER_W5_CHANNELS_IN_PHI_lsb);
		temp |= (static_cast<uint64_t>(in.spare) << EDM_STRIPCLUSTER_W5_SPARE_lsb);
		return temp;
	}

	inline uint64_t get_dataformat_EDM_STRIPCLUSTER_w6 (const EDM_STRIPCLUSTER_w6& in) {
		uint64_t temp = 0;
		temp |= (static_cast<uint64_t>(in.globalposition_z) << EDM_STRIPCLUSTER_W6_GLOBALPOSITION_Z_lsb);
		temp |= (static_cast<uint64_t>(in.lastword) << EDM_STRIPCLUSTER_W6_LASTWORD_lsb);
		temp |= (static_cast<uint64_t>(in.index) << EDM_STRIPCLUSTER_W6_INDEX_lsb);
		temp |= (static_cast<uint64_t>(in.spare) << EDM_STRIPCLUSTER_W6_SPARE_lsb);
		return temp;
	}

	inline EDM_STRIPCLUSTER_w1 fill_EDM_STRIPCLUSTER_w1 (const uint64_t& id_hash, const uint64_t& identifier) {
		EDM_STRIPCLUSTER_w1 temp;
		temp.id_hash = id_hash;
		temp.identifier = identifier;
		return temp;
	}

	inline EDM_STRIPCLUSTER_w2 fill_EDM_STRIPCLUSTER_w2 (const uint64_t& rdo_list_w1, const uint64_t& rdo_list_w2) {
		EDM_STRIPCLUSTER_w2 temp;
		temp.rdo_list_w1 = rdo_list_w1;
		temp.rdo_list_w2 = rdo_list_w2;
		return temp;
	}

	inline EDM_STRIPCLUSTER_w3 fill_EDM_STRIPCLUSTER_w3 (const uint64_t& rdo_list_w3, const uint64_t& rdo_list_w4) {
		EDM_STRIPCLUSTER_w3 temp;
		temp.rdo_list_w3 = rdo_list_w3;
		temp.rdo_list_w4 = rdo_list_w4;
		return temp;
	}

	inline EDM_STRIPCLUSTER_w4 fill_EDM_STRIPCLUSTER_w4 (const double& localposition_x, const double& localposition_y, const double& localcovariance_xx, const uint64_t& spare) {
		EDM_STRIPCLUSTER_w4 temp;
		temp.localposition_x = (int64_t)(localposition_x * EDM_STRIPCLUSTER_W4_LOCALPOSITION_X_mf);
		temp.localposition_y = (int64_t)(localposition_y * EDM_STRIPCLUSTER_W4_LOCALPOSITION_Y_mf);
		temp.localcovariance_xx = (uint64_t)(localcovariance_xx * EDM_STRIPCLUSTER_W4_LOCALCOVARIANCE_XX_mf);
		temp.spare = spare;
		return temp;
	}

	inline EDM_STRIPCLUSTER_w5 fill_EDM_STRIPCLUSTER_w5 (const double& globalposition_x, const double& globalposition_y, const uint64_t& channels_in_phi, const uint64_t& spare) {
		EDM_STRIPCLUSTER_w5 temp;
		temp.globalposition_x = (int64_t)(globalposition_x * EDM_STRIPCLUSTER_W5_GLOBALPOSITION_X_mf);
		temp.globalposition_y = (int64_t)(globalposition_y * EDM_STRIPCLUSTER_W5_GLOBALPOSITION_Y_mf);
		temp.channels_in_phi = channels_in_phi;
		temp.spare = spare;
		return temp;
	}

	inline EDM_STRIPCLUSTER_w6 fill_EDM_STRIPCLUSTER_w6 (const double& globalposition_z, const uint64_t& lastword, const uint64_t& index, const uint64_t& spare) {
		EDM_STRIPCLUSTER_w6 temp;
		temp.globalposition_z = (int64_t)(globalposition_z * EDM_STRIPCLUSTER_W6_GLOBALPOSITION_Z_mf);
		temp.lastword = lastword;
		temp.index = index;
		temp.spare = spare;
		return temp;
	}

	inline uint64_t to_real_EDM_STRIPCLUSTER_w1_id_hash (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_EDM_STRIPCLUSTER_w1_identifier (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_EDM_STRIPCLUSTER_w2_rdo_list_w1 (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_EDM_STRIPCLUSTER_w2_rdo_list_w2 (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_EDM_STRIPCLUSTER_w3_rdo_list_w3 (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_EDM_STRIPCLUSTER_w3_rdo_list_w4 (const uint64_t& in) {
		return in;
	}

	inline double to_real_EDM_STRIPCLUSTER_w4_localposition_x (const int64_t& in) {
		return (double)in / EDM_STRIPCLUSTER_W4_LOCALPOSITION_X_mf;
	}

	inline double to_real_EDM_STRIPCLUSTER_w4_localposition_y (const int64_t& in) {
		return (double)in / EDM_STRIPCLUSTER_W4_LOCALPOSITION_Y_mf;
	}

	inline double to_real_EDM_STRIPCLUSTER_w4_localcovariance_xx (const uint64_t& in) {
		return (double)in / EDM_STRIPCLUSTER_W4_LOCALCOVARIANCE_XX_mf;
	}

	inline uint64_t to_real_EDM_STRIPCLUSTER_w4_spare (const uint64_t& in) {
		return in;
	}

	inline double to_real_EDM_STRIPCLUSTER_w5_globalposition_x (const int64_t& in) {
		return (double)in / EDM_STRIPCLUSTER_W5_GLOBALPOSITION_X_mf;
	}

	inline double to_real_EDM_STRIPCLUSTER_w5_globalposition_y (const int64_t& in) {
		return (double)in / EDM_STRIPCLUSTER_W5_GLOBALPOSITION_Y_mf;
	}

	inline uint64_t to_real_EDM_STRIPCLUSTER_w5_channels_in_phi (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_EDM_STRIPCLUSTER_w5_spare (const uint64_t& in) {
		return in;
	}

	inline double to_real_EDM_STRIPCLUSTER_w6_globalposition_z (const int64_t& in) {
		return (double)in / EDM_STRIPCLUSTER_W6_GLOBALPOSITION_Z_mf;
	}

	inline uint64_t to_real_EDM_STRIPCLUSTER_w6_lastword (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_EDM_STRIPCLUSTER_w6_index (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_EDM_STRIPCLUSTER_w6_spare (const uint64_t& in) {
		return in;
	}

	// EDM_PIXELCLUSTER_W1 word description
	const int EDM_PIXELCLUSTER_W1_ID_HASH_bits = 32;
	const int EDM_PIXELCLUSTER_W1_ID_HASH_lsb = 32;
	const float EDM_PIXELCLUSTER_W1_ID_HASH_mf = 1.;

	const int EDM_PIXELCLUSTER_W1_IDENTFIER_bits = 32;
	const int EDM_PIXELCLUSTER_W1_IDENTFIER_lsb = 0;
	const float EDM_PIXELCLUSTER_W1_IDENTFIER_mf = 1.;

	// EDM_PIXELCLUSTER_W2 word description
	const int EDM_PIXELCLUSTER_W2_RDO_LIST_W1_bits = 32;
	const int EDM_PIXELCLUSTER_W2_RDO_LIST_W1_lsb = 32;
	const float EDM_PIXELCLUSTER_W2_RDO_LIST_W1_mf = 1.;

	const int EDM_PIXELCLUSTER_W2_RDO_LIST_W2_bits = 32;
	const int EDM_PIXELCLUSTER_W2_RDO_LIST_W2_lsb = 0;
	const float EDM_PIXELCLUSTER_W2_RDO_LIST_W2_mf = 1.;

	// EDM_PIXELCLUSTER_W3 word description
	const int EDM_PIXELCLUSTER_W3_RDO_LIST_W3_bits = 32;
	const int EDM_PIXELCLUSTER_W3_RDO_LIST_W3_lsb = 32;
	const float EDM_PIXELCLUSTER_W3_RDO_LIST_W3_mf = 1.;

	const int EDM_PIXELCLUSTER_W3_RDO_LIST_W4_bits = 32;
	const int EDM_PIXELCLUSTER_W3_RDO_LIST_W4_lsb = 0;
	const float EDM_PIXELCLUSTER_W3_RDO_LIST_W4_mf = 1.;

	// EDM_PIXELCLUSTER_W4 word description
	const int EDM_PIXELCLUSTER_W4_LOCALPOSITION_X_bits = 20;
	const int EDM_PIXELCLUSTER_W4_LOCALPOSITION_X_lsb = 44;
	const float EDM_PIXELCLUSTER_W4_LOCALPOSITION_X_mf = 8192.;

	const int EDM_PIXELCLUSTER_W4_LOCALPOSITION_Y_bits = 20;
	const int EDM_PIXELCLUSTER_W4_LOCALPOSITION_Y_lsb = 24;
	const float EDM_PIXELCLUSTER_W4_LOCALPOSITION_Y_mf = 8192.;

	const int EDM_PIXELCLUSTER_W4_CHANNELS_IN_PHI_bits = 5;
	const int EDM_PIXELCLUSTER_W4_CHANNELS_IN_PHI_lsb = 19;
	const float EDM_PIXELCLUSTER_W4_CHANNELS_IN_PHI_mf = 1.;

	const int EDM_PIXELCLUSTER_W4_CHANNELS_IN_ETA_bits = 5;
	const int EDM_PIXELCLUSTER_W4_CHANNELS_IN_ETA_lsb = 14;
	const float EDM_PIXELCLUSTER_W4_CHANNELS_IN_ETA_mf = 1.;

	const int EDM_PIXELCLUSTER_W4_WIDTH_IN_ETA_bits = 6;
	const int EDM_PIXELCLUSTER_W4_WIDTH_IN_ETA_lsb = 8;
	const float EDM_PIXELCLUSTER_W4_WIDTH_IN_ETA_mf = 32.;

	const int EDM_PIXELCLUSTER_W4_SPARE_bits = 8;
	const int EDM_PIXELCLUSTER_W4_SPARE_lsb = 0;
	const float EDM_PIXELCLUSTER_W4_SPARE_mf = 1.;

	// EDM_PIXELCLUSTER_W5 word description
	const int EDM_PIXELCLUSTER_W5_LOCALCOVARIANCE_XX_bits = 20;
	const int EDM_PIXELCLUSTER_W5_LOCALCOVARIANCE_XX_lsb = 44;
	const float EDM_PIXELCLUSTER_W5_LOCALCOVARIANCE_XX_mf = 524288.;

	const int EDM_PIXELCLUSTER_W5_LOCALCOVARIANCE_YY_bits = 20;
	const int EDM_PIXELCLUSTER_W5_LOCALCOVARIANCE_YY_lsb = 24;
	const float EDM_PIXELCLUSTER_W5_LOCALCOVARIANCE_YY_mf = 524288.;

	const int EDM_PIXELCLUSTER_W5_OMEGA_X_bits = 10;
	const int EDM_PIXELCLUSTER_W5_OMEGA_X_lsb = 14;
	const float EDM_PIXELCLUSTER_W5_OMEGA_X_mf = 512.;

	const int EDM_PIXELCLUSTER_W5_OMEGA_Y_bits = 10;
	const int EDM_PIXELCLUSTER_W5_OMEGA_Y_lsb = 4;
	const float EDM_PIXELCLUSTER_W5_OMEGA_Y_mf = 512.;

	const int EDM_PIXELCLUSTER_W5_SPARE_bits = 4;
	const int EDM_PIXELCLUSTER_W5_SPARE_lsb = 0;
	const float EDM_PIXELCLUSTER_W5_SPARE_mf = 1.;

	// EDM_PIXELCLUSTER_W6 word description
	const int EDM_PIXELCLUSTER_W6_GLOBALPOSITION_X_bits = 26;
	const int EDM_PIXELCLUSTER_W6_GLOBALPOSITION_X_lsb = 38;
	const float EDM_PIXELCLUSTER_W6_GLOBALPOSITION_X_mf = 65536.;

	const int EDM_PIXELCLUSTER_W6_GLOBALPOSITION_Y_bits = 26;
	const int EDM_PIXELCLUSTER_W6_GLOBALPOSITION_Y_lsb = 12;
	const float EDM_PIXELCLUSTER_W6_GLOBALPOSITION_Y_mf = 65536.;

	const int EDM_PIXELCLUSTER_W6_SPARE_bits = 12;
	const int EDM_PIXELCLUSTER_W6_SPARE_lsb = 0;
	const float EDM_PIXELCLUSTER_W6_SPARE_mf = 1.;

	// EDM_PIXELCLUSTER_W7 word description
	const int EDM_PIXELCLUSTER_W7_GLOBALPOSITION_Z_bits = 27;
	const int EDM_PIXELCLUSTER_W7_GLOBALPOSITION_Z_lsb = 37;
	const float EDM_PIXELCLUSTER_W7_GLOBALPOSITION_Z_mf = 65536.;

	const int EDM_PIXELCLUSTER_W7_TOTAL_TOT_bits = 9;
	const int EDM_PIXELCLUSTER_W7_TOTAL_TOT_lsb = 28;
	const float EDM_PIXELCLUSTER_W7_TOTAL_TOT_mf = 1.;

	const int EDM_PIXELCLUSTER_W7_LASTWORD_bits = 1;
	const int EDM_PIXELCLUSTER_W7_LASTWORD_lsb = 27;
	const float EDM_PIXELCLUSTER_W7_LASTWORD_mf = 1.;

	const int EDM_PIXELCLUSTER_W7_SPARE_bits = 27;
	const int EDM_PIXELCLUSTER_W7_SPARE_lsb = 0;
	const float EDM_PIXELCLUSTER_W7_SPARE_mf = 1.;

	typedef struct EDM_PIXELCLUSTER_w1 {
		uint64_t id_hash : EDM_PIXELCLUSTER_W1_ID_HASH_bits;
		uint64_t identfier : EDM_PIXELCLUSTER_W1_IDENTFIER_bits;
	} EDM_PIXELCLUSTER_w1;

	typedef struct EDM_PIXELCLUSTER_w2 {
		uint64_t rdo_list_w1 : EDM_PIXELCLUSTER_W2_RDO_LIST_W1_bits;
		uint64_t rdo_list_w2 : EDM_PIXELCLUSTER_W2_RDO_LIST_W2_bits;
	} EDM_PIXELCLUSTER_w2;

	typedef struct EDM_PIXELCLUSTER_w3 {
		uint64_t rdo_list_w3 : EDM_PIXELCLUSTER_W3_RDO_LIST_W3_bits;
		uint64_t rdo_list_w4 : EDM_PIXELCLUSTER_W3_RDO_LIST_W4_bits;
	} EDM_PIXELCLUSTER_w3;

	typedef struct EDM_PIXELCLUSTER_w4 {
		int64_t localposition_x : EDM_PIXELCLUSTER_W4_LOCALPOSITION_X_bits;
		int64_t localposition_y : EDM_PIXELCLUSTER_W4_LOCALPOSITION_Y_bits;
		uint64_t channels_in_phi : EDM_PIXELCLUSTER_W4_CHANNELS_IN_PHI_bits;
		uint64_t channels_in_eta : EDM_PIXELCLUSTER_W4_CHANNELS_IN_ETA_bits;
		uint64_t width_in_eta : EDM_PIXELCLUSTER_W4_WIDTH_IN_ETA_bits;
		uint64_t spare : EDM_PIXELCLUSTER_W4_SPARE_bits;
	} EDM_PIXELCLUSTER_w4;

	typedef struct EDM_PIXELCLUSTER_w5 {
		uint64_t localcovariance_xx : EDM_PIXELCLUSTER_W5_LOCALCOVARIANCE_XX_bits;
		uint64_t localcovariance_yy : EDM_PIXELCLUSTER_W5_LOCALCOVARIANCE_YY_bits;
		uint64_t omega_x : EDM_PIXELCLUSTER_W5_OMEGA_X_bits;
		uint64_t omega_y : EDM_PIXELCLUSTER_W5_OMEGA_Y_bits;
		uint64_t spare : EDM_PIXELCLUSTER_W5_SPARE_bits;
	} EDM_PIXELCLUSTER_w5;

	typedef struct EDM_PIXELCLUSTER_w6 {
		int64_t globalposition_x : EDM_PIXELCLUSTER_W6_GLOBALPOSITION_X_bits;
		int64_t globalposition_y : EDM_PIXELCLUSTER_W6_GLOBALPOSITION_Y_bits;
		uint64_t spare : EDM_PIXELCLUSTER_W6_SPARE_bits;
	} EDM_PIXELCLUSTER_w6;

	typedef struct EDM_PIXELCLUSTER_w7 {
		int64_t globalposition_z : EDM_PIXELCLUSTER_W7_GLOBALPOSITION_Z_bits;
		uint64_t total_tot : EDM_PIXELCLUSTER_W7_TOTAL_TOT_bits;
		uint64_t lastword : EDM_PIXELCLUSTER_W7_LASTWORD_bits;
		uint64_t spare : EDM_PIXELCLUSTER_W7_SPARE_bits;
	} EDM_PIXELCLUSTER_w7;

	inline EDM_PIXELCLUSTER_w1 get_bitfields_EDM_PIXELCLUSTER_w1 (const uint64_t& in) {
		EDM_PIXELCLUSTER_w1 temp;
		temp.id_hash = (in & SELECTBITS(EDM_PIXELCLUSTER_W1_ID_HASH_bits, EDM_PIXELCLUSTER_W1_ID_HASH_lsb)) >> EDM_PIXELCLUSTER_W1_ID_HASH_lsb;
		temp.identfier = (in & SELECTBITS(EDM_PIXELCLUSTER_W1_IDENTFIER_bits, EDM_PIXELCLUSTER_W1_IDENTFIER_lsb)) >> EDM_PIXELCLUSTER_W1_IDENTFIER_lsb;
		return temp;
	}

	inline EDM_PIXELCLUSTER_w2 get_bitfields_EDM_PIXELCLUSTER_w2 (const uint64_t& in) {
		EDM_PIXELCLUSTER_w2 temp;
		temp.rdo_list_w1 = (in & SELECTBITS(EDM_PIXELCLUSTER_W2_RDO_LIST_W1_bits, EDM_PIXELCLUSTER_W2_RDO_LIST_W1_lsb)) >> EDM_PIXELCLUSTER_W2_RDO_LIST_W1_lsb;
		temp.rdo_list_w2 = (in & SELECTBITS(EDM_PIXELCLUSTER_W2_RDO_LIST_W2_bits, EDM_PIXELCLUSTER_W2_RDO_LIST_W2_lsb)) >> EDM_PIXELCLUSTER_W2_RDO_LIST_W2_lsb;
		return temp;
	}

	inline EDM_PIXELCLUSTER_w3 get_bitfields_EDM_PIXELCLUSTER_w3 (const uint64_t& in) {
		EDM_PIXELCLUSTER_w3 temp;
		temp.rdo_list_w3 = (in & SELECTBITS(EDM_PIXELCLUSTER_W3_RDO_LIST_W3_bits, EDM_PIXELCLUSTER_W3_RDO_LIST_W3_lsb)) >> EDM_PIXELCLUSTER_W3_RDO_LIST_W3_lsb;
		temp.rdo_list_w4 = (in & SELECTBITS(EDM_PIXELCLUSTER_W3_RDO_LIST_W4_bits, EDM_PIXELCLUSTER_W3_RDO_LIST_W4_lsb)) >> EDM_PIXELCLUSTER_W3_RDO_LIST_W4_lsb;
		return temp;
	}

	inline EDM_PIXELCLUSTER_w4 get_bitfields_EDM_PIXELCLUSTER_w4 (const uint64_t& in) {
		EDM_PIXELCLUSTER_w4 temp;
		temp.localposition_x = (in & SELECTBITS(EDM_PIXELCLUSTER_W4_LOCALPOSITION_X_bits, EDM_PIXELCLUSTER_W4_LOCALPOSITION_X_lsb)) >> EDM_PIXELCLUSTER_W4_LOCALPOSITION_X_lsb;
		temp.localposition_y = (in & SELECTBITS(EDM_PIXELCLUSTER_W4_LOCALPOSITION_Y_bits, EDM_PIXELCLUSTER_W4_LOCALPOSITION_Y_lsb)) >> EDM_PIXELCLUSTER_W4_LOCALPOSITION_Y_lsb;
		temp.channels_in_phi = (in & SELECTBITS(EDM_PIXELCLUSTER_W4_CHANNELS_IN_PHI_bits, EDM_PIXELCLUSTER_W4_CHANNELS_IN_PHI_lsb)) >> EDM_PIXELCLUSTER_W4_CHANNELS_IN_PHI_lsb;
		temp.channels_in_eta = (in & SELECTBITS(EDM_PIXELCLUSTER_W4_CHANNELS_IN_ETA_bits, EDM_PIXELCLUSTER_W4_CHANNELS_IN_ETA_lsb)) >> EDM_PIXELCLUSTER_W4_CHANNELS_IN_ETA_lsb;
		temp.width_in_eta = (in & SELECTBITS(EDM_PIXELCLUSTER_W4_WIDTH_IN_ETA_bits, EDM_PIXELCLUSTER_W4_WIDTH_IN_ETA_lsb)) >> EDM_PIXELCLUSTER_W4_WIDTH_IN_ETA_lsb;
		temp.spare = (in & SELECTBITS(EDM_PIXELCLUSTER_W4_SPARE_bits, EDM_PIXELCLUSTER_W4_SPARE_lsb)) >> EDM_PIXELCLUSTER_W4_SPARE_lsb;
		return temp;
	}

	inline EDM_PIXELCLUSTER_w5 get_bitfields_EDM_PIXELCLUSTER_w5 (const uint64_t& in) {
		EDM_PIXELCLUSTER_w5 temp;
		temp.localcovariance_xx = (in & SELECTBITS(EDM_PIXELCLUSTER_W5_LOCALCOVARIANCE_XX_bits, EDM_PIXELCLUSTER_W5_LOCALCOVARIANCE_XX_lsb)) >> EDM_PIXELCLUSTER_W5_LOCALCOVARIANCE_XX_lsb;
		temp.localcovariance_yy = (in & SELECTBITS(EDM_PIXELCLUSTER_W5_LOCALCOVARIANCE_YY_bits, EDM_PIXELCLUSTER_W5_LOCALCOVARIANCE_YY_lsb)) >> EDM_PIXELCLUSTER_W5_LOCALCOVARIANCE_YY_lsb;
		temp.omega_x = (in & SELECTBITS(EDM_PIXELCLUSTER_W5_OMEGA_X_bits, EDM_PIXELCLUSTER_W5_OMEGA_X_lsb)) >> EDM_PIXELCLUSTER_W5_OMEGA_X_lsb;
		temp.omega_y = (in & SELECTBITS(EDM_PIXELCLUSTER_W5_OMEGA_Y_bits, EDM_PIXELCLUSTER_W5_OMEGA_Y_lsb)) >> EDM_PIXELCLUSTER_W5_OMEGA_Y_lsb;
		temp.spare = (in & SELECTBITS(EDM_PIXELCLUSTER_W5_SPARE_bits, EDM_PIXELCLUSTER_W5_SPARE_lsb)) >> EDM_PIXELCLUSTER_W5_SPARE_lsb;
		return temp;
	}

	inline EDM_PIXELCLUSTER_w6 get_bitfields_EDM_PIXELCLUSTER_w6 (const uint64_t& in) {
		EDM_PIXELCLUSTER_w6 temp;
		temp.globalposition_x = (in & SELECTBITS(EDM_PIXELCLUSTER_W6_GLOBALPOSITION_X_bits, EDM_PIXELCLUSTER_W6_GLOBALPOSITION_X_lsb)) >> EDM_PIXELCLUSTER_W6_GLOBALPOSITION_X_lsb;
		temp.globalposition_y = (in & SELECTBITS(EDM_PIXELCLUSTER_W6_GLOBALPOSITION_Y_bits, EDM_PIXELCLUSTER_W6_GLOBALPOSITION_Y_lsb)) >> EDM_PIXELCLUSTER_W6_GLOBALPOSITION_Y_lsb;
		temp.spare = (in & SELECTBITS(EDM_PIXELCLUSTER_W6_SPARE_bits, EDM_PIXELCLUSTER_W6_SPARE_lsb)) >> EDM_PIXELCLUSTER_W6_SPARE_lsb;
		return temp;
	}

	inline EDM_PIXELCLUSTER_w7 get_bitfields_EDM_PIXELCLUSTER_w7 (const uint64_t& in) {
		EDM_PIXELCLUSTER_w7 temp;
		temp.globalposition_z = (in & SELECTBITS(EDM_PIXELCLUSTER_W7_GLOBALPOSITION_Z_bits, EDM_PIXELCLUSTER_W7_GLOBALPOSITION_Z_lsb)) >> EDM_PIXELCLUSTER_W7_GLOBALPOSITION_Z_lsb;
		temp.total_tot = (in & SELECTBITS(EDM_PIXELCLUSTER_W7_TOTAL_TOT_bits, EDM_PIXELCLUSTER_W7_TOTAL_TOT_lsb)) >> EDM_PIXELCLUSTER_W7_TOTAL_TOT_lsb;
		temp.lastword = (in & SELECTBITS(EDM_PIXELCLUSTER_W7_LASTWORD_bits, EDM_PIXELCLUSTER_W7_LASTWORD_lsb)) >> EDM_PIXELCLUSTER_W7_LASTWORD_lsb;
		temp.spare = (in & SELECTBITS(EDM_PIXELCLUSTER_W7_SPARE_bits, EDM_PIXELCLUSTER_W7_SPARE_lsb)) >> EDM_PIXELCLUSTER_W7_SPARE_lsb;
		return temp;
	}

	inline uint64_t get_dataformat_EDM_PIXELCLUSTER_w1 (const EDM_PIXELCLUSTER_w1& in) {
		uint64_t temp = 0;
		temp |= (static_cast<uint64_t>(in.id_hash) << EDM_PIXELCLUSTER_W1_ID_HASH_lsb);
		temp |= (static_cast<uint64_t>(in.identfier) << EDM_PIXELCLUSTER_W1_IDENTFIER_lsb);
		return temp;
	}

	inline uint64_t get_dataformat_EDM_PIXELCLUSTER_w2 (const EDM_PIXELCLUSTER_w2& in) {
		uint64_t temp = 0;
		temp |= (static_cast<uint64_t>(in.rdo_list_w1) << EDM_PIXELCLUSTER_W2_RDO_LIST_W1_lsb);
		temp |= (static_cast<uint64_t>(in.rdo_list_w2) << EDM_PIXELCLUSTER_W2_RDO_LIST_W2_lsb);
		return temp;
	}

	inline uint64_t get_dataformat_EDM_PIXELCLUSTER_w3 (const EDM_PIXELCLUSTER_w3& in) {
		uint64_t temp = 0;
		temp |= (static_cast<uint64_t>(in.rdo_list_w3) << EDM_PIXELCLUSTER_W3_RDO_LIST_W3_lsb);
		temp |= (static_cast<uint64_t>(in.rdo_list_w4) << EDM_PIXELCLUSTER_W3_RDO_LIST_W4_lsb);
		return temp;
	}

	inline uint64_t get_dataformat_EDM_PIXELCLUSTER_w4 (const EDM_PIXELCLUSTER_w4& in) {
		uint64_t temp = 0;
		temp |= (static_cast<uint64_t>(in.localposition_x) << EDM_PIXELCLUSTER_W4_LOCALPOSITION_X_lsb);
		temp |= (static_cast<uint64_t>(in.localposition_y) << EDM_PIXELCLUSTER_W4_LOCALPOSITION_Y_lsb);
		temp |= (static_cast<uint64_t>(in.channels_in_phi) << EDM_PIXELCLUSTER_W4_CHANNELS_IN_PHI_lsb);
		temp |= (static_cast<uint64_t>(in.channels_in_eta) << EDM_PIXELCLUSTER_W4_CHANNELS_IN_ETA_lsb);
		temp |= (static_cast<uint64_t>(in.width_in_eta) << EDM_PIXELCLUSTER_W4_WIDTH_IN_ETA_lsb);
		temp |= (static_cast<uint64_t>(in.spare) << EDM_PIXELCLUSTER_W4_SPARE_lsb);
		return temp;
	}

	inline uint64_t get_dataformat_EDM_PIXELCLUSTER_w5 (const EDM_PIXELCLUSTER_w5& in) {
		uint64_t temp = 0;
		temp |= (static_cast<uint64_t>(in.localcovariance_xx) << EDM_PIXELCLUSTER_W5_LOCALCOVARIANCE_XX_lsb);
		temp |= (static_cast<uint64_t>(in.localcovariance_yy) << EDM_PIXELCLUSTER_W5_LOCALCOVARIANCE_YY_lsb);
		temp |= (static_cast<uint64_t>(in.omega_x) << EDM_PIXELCLUSTER_W5_OMEGA_X_lsb);
		temp |= (static_cast<uint64_t>(in.omega_y) << EDM_PIXELCLUSTER_W5_OMEGA_Y_lsb);
		temp |= (static_cast<uint64_t>(in.spare) << EDM_PIXELCLUSTER_W5_SPARE_lsb);
		return temp;
	}

	inline uint64_t get_dataformat_EDM_PIXELCLUSTER_w6 (const EDM_PIXELCLUSTER_w6& in) {
		uint64_t temp = 0;
		temp |= (static_cast<uint64_t>(in.globalposition_x) << EDM_PIXELCLUSTER_W6_GLOBALPOSITION_X_lsb);
		temp |= (static_cast<uint64_t>(in.globalposition_y) << EDM_PIXELCLUSTER_W6_GLOBALPOSITION_Y_lsb);
		temp |= (static_cast<uint64_t>(in.spare) << EDM_PIXELCLUSTER_W6_SPARE_lsb);
		return temp;
	}

	inline uint64_t get_dataformat_EDM_PIXELCLUSTER_w7 (const EDM_PIXELCLUSTER_w7& in) {
		uint64_t temp = 0;
		temp |= (static_cast<uint64_t>(in.globalposition_z) << EDM_PIXELCLUSTER_W7_GLOBALPOSITION_Z_lsb);
		temp |= (static_cast<uint64_t>(in.total_tot) << EDM_PIXELCLUSTER_W7_TOTAL_TOT_lsb);
		temp |= (static_cast<uint64_t>(in.lastword) << EDM_PIXELCLUSTER_W7_LASTWORD_lsb);
		temp |= (static_cast<uint64_t>(in.spare) << EDM_PIXELCLUSTER_W7_SPARE_lsb);
		return temp;
	}

	inline EDM_PIXELCLUSTER_w1 fill_EDM_PIXELCLUSTER_w1 (const uint64_t& id_hash, const uint64_t& identfier) {
		EDM_PIXELCLUSTER_w1 temp;
		temp.id_hash = id_hash;
		temp.identfier = identfier;
		return temp;
	}

	inline EDM_PIXELCLUSTER_w2 fill_EDM_PIXELCLUSTER_w2 (const uint64_t& rdo_list_w1, const uint64_t& rdo_list_w2) {
		EDM_PIXELCLUSTER_w2 temp;
		temp.rdo_list_w1 = rdo_list_w1;
		temp.rdo_list_w2 = rdo_list_w2;
		return temp;
	}

	inline EDM_PIXELCLUSTER_w3 fill_EDM_PIXELCLUSTER_w3 (const uint64_t& rdo_list_w3, const uint64_t& rdo_list_w4) {
		EDM_PIXELCLUSTER_w3 temp;
		temp.rdo_list_w3 = rdo_list_w3;
		temp.rdo_list_w4 = rdo_list_w4;
		return temp;
	}

	inline EDM_PIXELCLUSTER_w4 fill_EDM_PIXELCLUSTER_w4 (const double& localposition_x, const double& localposition_y, const uint64_t& channels_in_phi, const uint64_t& channels_in_eta, const double& width_in_eta, const uint64_t& spare) {
		EDM_PIXELCLUSTER_w4 temp;
		temp.localposition_x = (int64_t)(localposition_x * EDM_PIXELCLUSTER_W4_LOCALPOSITION_X_mf);
		temp.localposition_y = (int64_t)(localposition_y * EDM_PIXELCLUSTER_W4_LOCALPOSITION_Y_mf);
		temp.channels_in_phi = channels_in_phi;
		temp.channels_in_eta = channels_in_eta;
		temp.width_in_eta = (uint64_t)(width_in_eta * EDM_PIXELCLUSTER_W4_WIDTH_IN_ETA_mf);
		temp.spare = spare;
		return temp;
	}

	inline EDM_PIXELCLUSTER_w5 fill_EDM_PIXELCLUSTER_w5 (const double& localcovariance_xx, const double& localcovariance_yy, const double& omega_x, const double& omega_y, const uint64_t& spare) {
		EDM_PIXELCLUSTER_w5 temp;
		temp.localcovariance_xx = (uint64_t)(localcovariance_xx * EDM_PIXELCLUSTER_W5_LOCALCOVARIANCE_XX_mf);
		temp.localcovariance_yy = (uint64_t)(localcovariance_yy * EDM_PIXELCLUSTER_W5_LOCALCOVARIANCE_YY_mf);
		temp.omega_x = (uint64_t)(omega_x * EDM_PIXELCLUSTER_W5_OMEGA_X_mf);
		temp.omega_y = (uint64_t)(omega_y * EDM_PIXELCLUSTER_W5_OMEGA_Y_mf);
		temp.spare = spare;
		return temp;
	}

	inline EDM_PIXELCLUSTER_w6 fill_EDM_PIXELCLUSTER_w6 (const double& globalposition_x, const double& globalposition_y, const uint64_t& spare) {
		EDM_PIXELCLUSTER_w6 temp;
		temp.globalposition_x = (int64_t)(globalposition_x * EDM_PIXELCLUSTER_W6_GLOBALPOSITION_X_mf);
		temp.globalposition_y = (int64_t)(globalposition_y * EDM_PIXELCLUSTER_W6_GLOBALPOSITION_Y_mf);
		temp.spare = spare;
		return temp;
	}

	inline EDM_PIXELCLUSTER_w7 fill_EDM_PIXELCLUSTER_w7 (const double& globalposition_z, const uint64_t& total_tot, const uint64_t& lastword, const uint64_t& spare) {
		EDM_PIXELCLUSTER_w7 temp;
		temp.globalposition_z = (int64_t)(globalposition_z * EDM_PIXELCLUSTER_W7_GLOBALPOSITION_Z_mf);
		temp.total_tot = total_tot;
		temp.lastword = lastword;
		temp.spare = spare;
		return temp;
	}

	inline uint64_t to_real_EDM_PIXELCLUSTER_w1_id_hash (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_EDM_PIXELCLUSTER_w1_identfier (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_EDM_PIXELCLUSTER_w2_rdo_list_w1 (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_EDM_PIXELCLUSTER_w2_rdo_list_w2 (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_EDM_PIXELCLUSTER_w3_rdo_list_w3 (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_EDM_PIXELCLUSTER_w3_rdo_list_w4 (const uint64_t& in) {
		return in;
	}

	inline double to_real_EDM_PIXELCLUSTER_w4_localposition_x (const int64_t& in) {
		return (double)in / EDM_PIXELCLUSTER_W4_LOCALPOSITION_X_mf;
	}

	inline double to_real_EDM_PIXELCLUSTER_w4_localposition_y (const int64_t& in) {
		return (double)in / EDM_PIXELCLUSTER_W4_LOCALPOSITION_Y_mf;
	}

	inline uint64_t to_real_EDM_PIXELCLUSTER_w4_channels_in_phi (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_EDM_PIXELCLUSTER_w4_channels_in_eta (const uint64_t& in) {
		return in;
	}

	inline double to_real_EDM_PIXELCLUSTER_w4_width_in_eta (const uint64_t& in) {
		return (double)in / EDM_PIXELCLUSTER_W4_WIDTH_IN_ETA_mf;
	}

	inline uint64_t to_real_EDM_PIXELCLUSTER_w4_spare (const uint64_t& in) {
		return in;
	}

	inline double to_real_EDM_PIXELCLUSTER_w5_localcovariance_xx (const uint64_t& in) {
		return (double)in / EDM_PIXELCLUSTER_W5_LOCALCOVARIANCE_XX_mf;
	}

	inline double to_real_EDM_PIXELCLUSTER_w5_localcovariance_yy (const uint64_t& in) {
		return (double)in / EDM_PIXELCLUSTER_W5_LOCALCOVARIANCE_YY_mf;
	}

	inline double to_real_EDM_PIXELCLUSTER_w5_omega_x (const uint64_t& in) {
		return (double)in / EDM_PIXELCLUSTER_W5_OMEGA_X_mf;
	}

	inline double to_real_EDM_PIXELCLUSTER_w5_omega_y (const uint64_t& in) {
		return (double)in / EDM_PIXELCLUSTER_W5_OMEGA_Y_mf;
	}

	inline uint64_t to_real_EDM_PIXELCLUSTER_w5_spare (const uint64_t& in) {
		return in;
	}

	inline double to_real_EDM_PIXELCLUSTER_w6_globalposition_x (const int64_t& in) {
		return (double)in / EDM_PIXELCLUSTER_W6_GLOBALPOSITION_X_mf;
	}

	inline double to_real_EDM_PIXELCLUSTER_w6_globalposition_y (const int64_t& in) {
		return (double)in / EDM_PIXELCLUSTER_W6_GLOBALPOSITION_Y_mf;
	}

	inline uint64_t to_real_EDM_PIXELCLUSTER_w6_spare (const uint64_t& in) {
		return in;
	}

	inline double to_real_EDM_PIXELCLUSTER_w7_globalposition_z (const int64_t& in) {
		return (double)in / EDM_PIXELCLUSTER_W7_GLOBALPOSITION_Z_mf;
	}

	inline uint64_t to_real_EDM_PIXELCLUSTER_w7_total_tot (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_EDM_PIXELCLUSTER_w7_lastword (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_EDM_PIXELCLUSTER_w7_spare (const uint64_t& in) {
		return in;
	}

	// PIXEL_EF_RDO word description
	const int PIXEL_EF_RDO_LAST_bits = 1;
	const int PIXEL_EF_RDO_LAST_lsb = 63;
	const float PIXEL_EF_RDO_LAST_mf = 1.;

	const int PIXEL_EF_RDO_ROW_bits = 10;
	const int PIXEL_EF_RDO_ROW_lsb = 53;
	const float PIXEL_EF_RDO_ROW_mf = 1.;

	const int PIXEL_EF_RDO_COL_bits = 10;
	const int PIXEL_EF_RDO_COL_lsb = 43;
	const float PIXEL_EF_RDO_COL_mf = 1.;

	const int PIXEL_EF_RDO_TOT_bits = 4;
	const int PIXEL_EF_RDO_TOT_lsb = 39;
	const float PIXEL_EF_RDO_TOT_mf = 1.;

	const int PIXEL_EF_RDO_LVL1_bits = 1;
	const int PIXEL_EF_RDO_LVL1_lsb = 38;
	const float PIXEL_EF_RDO_LVL1_mf = 1.;

	const int PIXEL_EF_RDO_ID_bits = 13;
	const int PIXEL_EF_RDO_ID_lsb = 25;
	const float PIXEL_EF_RDO_ID_mf = 1.;

	const int PIXEL_EF_RDO_SPARE_bits = 25;
	const int PIXEL_EF_RDO_SPARE_lsb = 0;
	const float PIXEL_EF_RDO_SPARE_mf = 1.;

	typedef struct PIXEL_EF_RDO {
		uint64_t last : PIXEL_EF_RDO_LAST_bits;
		uint64_t row : PIXEL_EF_RDO_ROW_bits;
		uint64_t col : PIXEL_EF_RDO_COL_bits;
		uint64_t tot : PIXEL_EF_RDO_TOT_bits;
		uint64_t lvl1 : PIXEL_EF_RDO_LVL1_bits;
		uint64_t id : PIXEL_EF_RDO_ID_bits;
		uint64_t spare : PIXEL_EF_RDO_SPARE_bits;
	} PIXEL_EF_RDO;

	inline PIXEL_EF_RDO get_bitfields_PIXEL_EF_RDO (const uint64_t& in) {
		PIXEL_EF_RDO temp;
		temp.last = (in & SELECTBITS(PIXEL_EF_RDO_LAST_bits, PIXEL_EF_RDO_LAST_lsb)) >> PIXEL_EF_RDO_LAST_lsb;
		temp.row = (in & SELECTBITS(PIXEL_EF_RDO_ROW_bits, PIXEL_EF_RDO_ROW_lsb)) >> PIXEL_EF_RDO_ROW_lsb;
		temp.col = (in & SELECTBITS(PIXEL_EF_RDO_COL_bits, PIXEL_EF_RDO_COL_lsb)) >> PIXEL_EF_RDO_COL_lsb;
		temp.tot = (in & SELECTBITS(PIXEL_EF_RDO_TOT_bits, PIXEL_EF_RDO_TOT_lsb)) >> PIXEL_EF_RDO_TOT_lsb;
		temp.lvl1 = (in & SELECTBITS(PIXEL_EF_RDO_LVL1_bits, PIXEL_EF_RDO_LVL1_lsb)) >> PIXEL_EF_RDO_LVL1_lsb;
		temp.id = (in & SELECTBITS(PIXEL_EF_RDO_ID_bits, PIXEL_EF_RDO_ID_lsb)) >> PIXEL_EF_RDO_ID_lsb;
		temp.spare = (in & SELECTBITS(PIXEL_EF_RDO_SPARE_bits, PIXEL_EF_RDO_SPARE_lsb)) >> PIXEL_EF_RDO_SPARE_lsb;
		return temp;
	}

	inline uint64_t get_dataformat_PIXEL_EF_RDO (const PIXEL_EF_RDO& in) {
		uint64_t temp = 0;
		temp |= (static_cast<uint64_t>(in.last) << PIXEL_EF_RDO_LAST_lsb);
		temp |= (static_cast<uint64_t>(in.row) << PIXEL_EF_RDO_ROW_lsb);
		temp |= (static_cast<uint64_t>(in.col) << PIXEL_EF_RDO_COL_lsb);
		temp |= (static_cast<uint64_t>(in.tot) << PIXEL_EF_RDO_TOT_lsb);
		temp |= (static_cast<uint64_t>(in.lvl1) << PIXEL_EF_RDO_LVL1_lsb);
		temp |= (static_cast<uint64_t>(in.id) << PIXEL_EF_RDO_ID_lsb);
		temp |= (static_cast<uint64_t>(in.spare) << PIXEL_EF_RDO_SPARE_lsb);
		return temp;
	}

	inline PIXEL_EF_RDO fill_PIXEL_EF_RDO (const uint64_t& last, const uint64_t& row, const uint64_t& col, const uint64_t& tot, const uint64_t& lvl1, const uint64_t& id, const uint64_t& spare) {
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

	inline uint64_t to_real_PIXEL_EF_RDO_last (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_PIXEL_EF_RDO_row (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_PIXEL_EF_RDO_col (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_PIXEL_EF_RDO_tot (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_PIXEL_EF_RDO_lvl1 (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_PIXEL_EF_RDO_id (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_PIXEL_EF_RDO_spare (const uint64_t& in) {
		return in;
	}

	// STRIP_EF_RDO word description
	const int STRIP_EF_RDO_LAST_bits = 1;
	const int STRIP_EF_RDO_LAST_lsb = 31;
	const float STRIP_EF_RDO_LAST_mf = 1.;

	const int STRIP_EF_RDO_CHIPID_bits = 4;
	const int STRIP_EF_RDO_CHIPID_lsb = 27;
	const float STRIP_EF_RDO_CHIPID_mf = 1.;

	const int STRIP_EF_RDO_STRIP_NUM_bits = 8;
	const int STRIP_EF_RDO_STRIP_NUM_lsb = 19;
	const float STRIP_EF_RDO_STRIP_NUM_mf = 1.;

	const int STRIP_EF_RDO_CLUSTER_MAP_bits = 3;
	const int STRIP_EF_RDO_CLUSTER_MAP_lsb = 16;
	const float STRIP_EF_RDO_CLUSTER_MAP_mf = 1.;

	const int STRIP_EF_RDO_ID_bits = 13;
	const int STRIP_EF_RDO_ID_lsb = 3;
	const float STRIP_EF_RDO_ID_mf = 1.;

	const int STRIP_EF_RDO_SPARE_bits = 3;
	const int STRIP_EF_RDO_SPARE_lsb = 0;
	const float STRIP_EF_RDO_SPARE_mf = 1.;

	typedef struct STRIP_EF_RDO {
		uint64_t last : STRIP_EF_RDO_LAST_bits;
		uint64_t chipid : STRIP_EF_RDO_CHIPID_bits;
		uint64_t strip_num : STRIP_EF_RDO_STRIP_NUM_bits;
		uint64_t cluster_map : STRIP_EF_RDO_CLUSTER_MAP_bits;
		uint64_t id : STRIP_EF_RDO_ID_bits;
		uint64_t spare : STRIP_EF_RDO_SPARE_bits;
	} STRIP_EF_RDO;

	inline STRIP_EF_RDO get_bitfields_STRIP_EF_RDO (const uint64_t& in) {
		STRIP_EF_RDO temp;
		temp.last = (in & SELECTBITS(STRIP_EF_RDO_LAST_bits, STRIP_EF_RDO_LAST_lsb)) >> STRIP_EF_RDO_LAST_lsb;
		temp.chipid = (in & SELECTBITS(STRIP_EF_RDO_CHIPID_bits, STRIP_EF_RDO_CHIPID_lsb)) >> STRIP_EF_RDO_CHIPID_lsb;
		temp.strip_num = (in & SELECTBITS(STRIP_EF_RDO_STRIP_NUM_bits, STRIP_EF_RDO_STRIP_NUM_lsb)) >> STRIP_EF_RDO_STRIP_NUM_lsb;
		temp.cluster_map = (in & SELECTBITS(STRIP_EF_RDO_CLUSTER_MAP_bits, STRIP_EF_RDO_CLUSTER_MAP_lsb)) >> STRIP_EF_RDO_CLUSTER_MAP_lsb;
		temp.id = (in & SELECTBITS(STRIP_EF_RDO_ID_bits, STRIP_EF_RDO_ID_lsb)) >> STRIP_EF_RDO_ID_lsb;
		temp.spare = (in & SELECTBITS(STRIP_EF_RDO_SPARE_bits, STRIP_EF_RDO_SPARE_lsb)) >> STRIP_EF_RDO_SPARE_lsb;
		return temp;
	}

	inline uint64_t get_dataformat_STRIP_EF_RDO_up32 (const uint64_t& in) {
		return (in & SELECTBITS(32, 32)) >> 32;
}

	inline uint64_t get_dataformat_STRIP_EF_RDO_low32 (const uint64_t& in) {
		return (in & SELECTBITS(32, 0));
}

	inline uint64_t get_dataformat_STRIP_EF_RDO (const STRIP_EF_RDO& in) {
		uint64_t temp = 0;
		temp |= (static_cast<uint64_t>(in.last) << STRIP_EF_RDO_LAST_lsb);
		temp |= (static_cast<uint64_t>(in.chipid) << STRIP_EF_RDO_CHIPID_lsb);
		temp |= (static_cast<uint64_t>(in.strip_num) << STRIP_EF_RDO_STRIP_NUM_lsb);
		temp |= (static_cast<uint64_t>(in.cluster_map) << STRIP_EF_RDO_CLUSTER_MAP_lsb);
		temp |= (static_cast<uint64_t>(in.id) << STRIP_EF_RDO_ID_lsb);
		temp |= (static_cast<uint64_t>(in.spare) << STRIP_EF_RDO_SPARE_lsb);
		return temp;
	}

	inline uint64_t get_dataformat_STRIP_EF_RDO_64 (const uint64_t& up, const uint64_t& low) {
		uint64_t temp = up << 32;
		return (temp | low);
}

	inline STRIP_EF_RDO fill_STRIP_EF_RDO (const uint64_t& last, const uint64_t& chipid, const uint64_t& strip_num, const uint64_t& cluster_map, const uint64_t& id, const uint64_t& spare) {
		STRIP_EF_RDO temp;
		temp.last = last;
		temp.chipid = chipid;
		temp.strip_num = strip_num;
		temp.cluster_map = cluster_map;
		temp.id = id;
		temp.spare = spare;
		return temp;
	}

	inline uint64_t to_real_STRIP_EF_RDO_last (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_STRIP_EF_RDO_chipid (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_STRIP_EF_RDO_strip_num (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_STRIP_EF_RDO_cluster_map (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_STRIP_EF_RDO_id (const uint64_t& in) {
		return in;
	}

	inline uint64_t to_real_STRIP_EF_RDO_spare (const uint64_t& in) {
		return in;
	}


};

#undef SELECTBITS

#endif // EFTRACKING_FPGA_INTEGRATION_FPGADATAFORMATUTILITIES_H
