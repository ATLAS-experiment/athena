// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGT1MUCTPIBITS_MUCTPI_BITS_H
#define TRIGT1MUCTPIBITS_MUCTPI_BITS_H

#include <cstdint>

namespace LVL1::MuCTPIBits {
  /// Binary 111 representing the maximal multiplicity value for a given threshold
  static constexpr uint32_t MULT_VAL = 7;
  /// Number of multiplicity bits reserved per threshold
  static constexpr uint32_t MULT_BITS = 3;
  /// Defining the number of p<sub>T</sub> thresholds in the system
  static constexpr uint32_t RUN3_MULT_THRESH_NUM = 32;
  static constexpr uint32_t MULT_THRESH_NUM = 6;
  /// Telling that the 3-bit BCID comes at "position 7" in the multiplicity word
  static constexpr uint32_t RUN3_MULT_BCID_POS = 26;
  static constexpr uint32_t MULT_BCID_POS = 7;

  /// Mask for the bit showing if more than two muon candidates were in the trigger sector
  static constexpr uint32_t CAND_OVERFLOW_MASK = 0x1;
  /// Position of the candidate overflow mask
  static constexpr uint32_t RUN3_CAND_OVERFLOW_SHIFT = 17;
  static constexpr uint32_t CAND_OVERFLOW_SHIFT = 0;

  /// Mask for the bit showing if more than one muon candidates were in the sector RoI
  static constexpr uint32_t ROI_OVERFLOW_MASK = 0x1;
  /// Position of the RoI overflow mask
  static constexpr uint32_t RUN3_ROI_OVERFLOW_SHIFT = 12;
  static constexpr uint32_t ROI_OVERFLOW_SHIFT = 1;

  /// Mask for the full potential ROI word from the data words
  static constexpr uint32_t ROI_MASK = 0xff; /// new in v2
  /// Mask for extracting the RoI for barrel candidates from the data words
  static constexpr uint32_t BARREL_ROI_MASK = 0x1f;
  /// Mask for extracting the RoI for endcap candidates from the data words
  static constexpr uint32_t ENDCAP_ROI_MASK = 0xff;
  /// Mask for extracting the RoI for forward candidates from the data words
  static constexpr uint32_t FORWARD_ROI_MASK = 0x3f;
  /// Position of the RoI bits in the data word
  static constexpr uint32_t RUN3_ROI_SHIFT = 0;
  static constexpr uint32_t ROI_SHIFT = 2;

  //===== Need to double check that the run3 positions are correct for the OL flags=====
  /// Mask for extracting the overlap bits for barrel candidates from the data words
  static constexpr uint32_t BARREL_OL_MASK = 0x3;
  static constexpr uint32_t BARREL_PHI_OL_MASK = 0x1;
  static constexpr uint32_t BARREL_ETA_OL_MASK = 0x2;
  /// Position of the overlap bits in barrel data words
  static constexpr uint32_t BARREL_OL_SHIFT = 9;
  /// Mask for extracting the overlap bits for endcap candidates from the data words
  static constexpr uint32_t ENDCAP_OL_MASK = 0x1;
  /// Position of the overlap bits in endcap data words
  //static constexpr uint32_t RUN3_ENDCAP_OL_SHIFT = 8; // 10-2 = 8 // not used in run3 format
  static constexpr uint32_t ENDCAP_OL_SHIFT = 10;

  /// Mask for extracting the p<sub>T</sub> threshold passed by the candidate from the data word
  static constexpr uint32_t RUN3_CAND_PT_MASK = 0xf;
  static constexpr uint32_t CAND_PT_MASK = 0x7;
  /// Position of the p<sub>T</sub> threshold bits in the data words
  static constexpr uint32_t RUN3_CAND_PT_SHIFT = 8;
  static constexpr uint32_t CAND_PT_SHIFT = 11;

  /// gone in v2
  //These aren't part of the LVL2 word in run 2, but part of the DAQ word
  /// Mask for extracting the last 3 bits of the BCID of the muon candidate from the data word
  static constexpr uint32_t CAND_BCID_MASK = 0x7;
  /// Position of the BCID bits in the data words
  static constexpr uint32_t CAND_BCID_SHIFT = 14;

  /// Mask for extracting the address of the muon candidate from the data word
  /// This is the mask and shift for the full sector address, including the hemisphere as the least significant bit
  static constexpr uint32_t RUN3_CAND_SECTOR_ADDRESS_MASK = 0xff;
  static constexpr uint32_t CAND_SECTOR_ADDRESS_MASK = 0xff;
  /// Mask for the bit showing which hemisphere the candidate came from.(1: positive; 0: negative)
  static constexpr uint32_t SECTOR_HEMISPHERE_MASK = 0x1;
  /// Position of the muon candidate's address in the data word
  static constexpr uint32_t RUN3_CAND_SECTOR_ADDRESS_SHIFT = 21;
  static constexpr uint32_t CAND_SECTOR_ADDRESS_SHIFT = 14;//17; 17 is for the DAQ word, not LVL2

  //====The shift doesn't correspond to the mask here (the mask corresponds to the SECTOR_ADDRESS_SHIFT above) === 
  /// Bit in the candidate's address turned on for endcap candidates
  static constexpr uint32_t ENDCAP_ADDRESS_MASK = 0x80;
  /// Bit in the candidate's address turned on for forward candidates
  static constexpr uint32_t FORWARD_ADDRESS_MASK = 0x40;
  /// Mask for both forward and endcap bits
  static constexpr uint32_t SUBSYS_ADDRESS_MASK = 0xc0;
  /// Position in the data word of the subsystem bits
  static constexpr uint32_t RUN3_SUBSYS_ADDRESS_SHIFT = 27;
  static constexpr uint32_t SUBSYS_ADDRESS_SHIFT = 20;
  /// Run-3 subsystem identifier bits location and values
  static constexpr uint32_t RUN3_SUBSYS_ADDRESS_BAFW_SHIFT = 27;
  static constexpr uint32_t RUN3_SUBSYS_ADDRESS_BAFW_MASK = 0b11;
  static constexpr uint32_t RUN3_SUBSYS_ADDRESS_BA_VAL = 0b00;
  static constexpr uint32_t RUN3_SUBSYS_ADDRESS_FW_VAL = 0b01;
  static constexpr uint32_t RUN3_SUBSYS_ADDRESS_EC_SHIFT = 28;
  static constexpr uint32_t RUN3_SUBSYS_ADDRESS_EC_MASK = 0b1;
  static constexpr uint32_t RUN3_SUBSYS_ADDRESS_EC_VAL = 0b1;
  static constexpr uint32_t RUN3_SUBSYS_HEMISPHERE_SHIFT = 21;
  static constexpr uint32_t RUN3_SUBSYS_HEMISPHERE_MASK = 0b1;

  /// Mask for extracting the sector ID for endcap candidates from the data word
  static constexpr uint32_t ENDCAP_SECTORID_MASK = 0x3f;
  /// Mask for extracting the sector ID for forward candidates from the data word
  static constexpr uint32_t FORWARD_SECTORID_MASK = 0x1f;
  /// Mask for extracting the sector ID for barrel candidates from the data word
  static constexpr uint32_t BARREL_SECTORID_MASK = 0x1f;
  /// Position of the sector ID itself
  static constexpr uint32_t RUN3_CAND_SECTORID_MASK = 0x7f;
  static constexpr uint32_t RUN3_CAND_SECTORID_SHIFT = 22;
  static constexpr uint32_t CAND_SECTORID_SHIFT = 15;

  /// gone in v2
  /// Mask for extracting the bit from the data word showing whether the candidate had the highest p<sub>T</sub> in the sector
  static constexpr uint32_t CAND_HIGHEST_PT_MASK = 0x1;
  /// Position of the "highest p<sub>T</sub>" bit
  static constexpr uint32_t CAND_HIGHEST_PT_SHIFT = 22;

  /// gone in v2
  //These aren't part of the LVL2 word in run 2, but part of the DAQ word
  /// Mask for extracting the bit from the data word showing if the muon candidate was sent to the RoIB
  static constexpr uint32_t CAND_SENT_ROI_MASK = 0x1;
  /// Position of the "candidate sent to RoIB" bit.
  static constexpr uint32_t CAND_SENT_ROI_SHIFT = 26;

  /// Position of the bit turned on for the multiplicity words that distinguishes them from the data words
  static constexpr uint32_t MULT_WORD_FLAG_SHIFT = 29;

  /// Position of the bit specifying the candidate's sign
  static constexpr uint32_t RUN3_CAND_TGC_CHARGE_SIGN_SHIFT = 12;
  static constexpr uint32_t CAND_TGC_CHARGE_SIGN_SHIFT = 27;
  /// Position of the bit specifying 3-station coincidence from the big wheel
  static constexpr uint32_t RUN3_CAND_TGC_BW2OR3_SHIFT = 13;
  /// Position of the bit specifying coincidence with inner detectors
  static constexpr uint32_t RUN3_CAND_TGC_INNERCOIN_SHIFT = 14;
  /// Position of the bit specifying if RoI is in a good b-field region (1=good, 0=bad)
  static constexpr uint32_t RUN3_CAND_TGC_GOODMF_SHIFT = 15;

  /// Position of the bit specifying if a candidate was vetoed in the multiplicity sum
  static constexpr uint32_t RUN3_CAND_VETO_SHIFT = 16;
  static constexpr uint32_t CAND_VETO_SHIFT = 28;

  // Timeslice and multiplicity words
  static constexpr uint32_t RUN3_TIMESLICE_MULT_WORD_ID_SHIFT = 30;
  static constexpr uint32_t RUN3_TIMESLICE_MULT_WORD_ID_MASK = 0b11;
  static constexpr uint32_t RUN3_TIMESLICE_MULT_WORD_ID_VAL = 0b10;
  static constexpr uint32_t RUN3_TIMESLICE_MULT_WORD_NUM_SHIFT = 28;
  static constexpr uint32_t RUN3_TIMESLICE_MULT_WORD_NUM_MASK = 0b11;
  static constexpr uint32_t RUN3_TIMESLICE_WORD_NUM_VAL = 0b00; // Timeslice word is 0, multiplicity words are 1, 2, 3
  static constexpr uint32_t RUN3_TIMESLICE_BCID_SHIFT = 16;
  static constexpr uint32_t RUN3_TIMESLICE_BCID_MASK = 0xfff;
  static constexpr uint32_t RUN3_TIMESLICE_NTOB_SHIFT = 10;
  static constexpr uint32_t RUN3_TIMESLICE_NTOB_MASK = 0x3f;
  static constexpr uint32_t RUN3_TIMESLICE_NCAND_SHIFT = 0;
  static constexpr uint32_t RUN3_TIMESLICE_NCAND_MASK = 0x3ff;
  static constexpr uint32_t RUN3_NSW_MONITORING_TRIGGER_SHIFT = 0x8;
  static constexpr uint32_t RUN3_NSW_MONITORING_TRIGGER_MASK = 0b1;
  static constexpr uint32_t RUN3_MULTIPLICITY_OVERFLOW_SHIFT = 0x9;
  static constexpr uint32_t RUN3_MULTIPLICITY_OVERFLOW_MASK = 0b1;
  static constexpr uint32_t RUN3_MULTIPLICITY_TRIGBITS_SHIFT = 0xa;
  static constexpr uint32_t RUN3_MULTIPLICITY_TRIGBITS_MASK = 0x3ffff;

  // Multiplicity word encoding (split 64-bit word into 28+28+8)
  static constexpr uint32_t RUN3_MULTIPLICITY_ENC_PART1_SHIFT = 0;
  static constexpr uint32_t RUN3_MULTIPLICITY_ENC_PART1_MASK = 0xffffff;
  static constexpr uint32_t RUN3_MULTIPLICITY_ENC_PART2_SHIFT = 28;
  static constexpr uint32_t RUN3_MULTIPLICITY_ENC_PART2_MASK = 0xffffff;
  static constexpr uint32_t RUN3_MULTIPLICITY_ENC_PART3_SHIFT = 56;
  static constexpr uint32_t RUN3_MULTIPLICITY_ENC_PART3_MASK = 0xff;
  // Multiplicity word decoding (28+28+8 into 64-bit word)
  static constexpr uint32_t RUN3_MULTIPLICITY_PART1_SHIFT = 0;
  static constexpr uint32_t RUN3_MULTIPLICITY_PART1_MASK = 0xfffffff;
  static constexpr uint32_t RUN3_MULTIPLICITY_PART2_SHIFT = 0;
  static constexpr uint32_t RUN3_MULTIPLICITY_PART2_MASK = 0xfffffff;
  static constexpr uint32_t RUN3_MULTIPLICITY_PART3_SHIFT = 0;
  static constexpr uint32_t RUN3_MULTIPLICITY_PART3_MASK = 0xff;

  // Candidate words
  static constexpr uint32_t RUN3_CAND_WORD_ID_SHIFT = 31;
  static constexpr uint32_t RUN3_CAND_WORD_ID_MASK = 0b1;
  static constexpr uint32_t RUN3_CAND_WORD_ID_VAL = 0b0;
  static constexpr uint32_t RUN3_CAND_WORD_SECTORERRORFLAG_SHIFT = 29;
  static constexpr uint32_t RUN3_CAND_WORD_SECTORERRORFLAG_MASK = 0x1;
  static constexpr uint32_t RUN3_CAND_WORD_SLID_SHIFT = 21;
  static constexpr uint32_t RUN3_CAND_WORD_SLID_MASK = 0xff;
  static constexpr uint32_t RUN3_CAND_WORD_SECTORFLAGS_GTN_SHIFT = 17;
  static constexpr uint32_t RUN3_CAND_WORD_SECTORFLAGS_GTN_MASK = 0x1;
  static constexpr uint32_t RUN3_CAND_WORD_VETO_SHIFT = 16;
  static constexpr uint32_t RUN3_CAND_WORD_VETO_MASK = 0x1;
  static constexpr uint32_t RUN3_CAND_WORD_CANDFLAGS_SHIFT = 12;
  static constexpr uint32_t RUN3_CAND_WORD_CANDFLAGS_MASK = 0xf;
  static constexpr uint32_t RUN3_CAND_WORD_CANDFLAGS_ECFW_R_SHIFT = 2;
  static constexpr uint32_t RUN3_CAND_WORD_CANDFLAGS_EC_R_MASK = 0x3f;
  static constexpr uint32_t RUN3_CAND_WORD_CANDFLAGS_FW_R_MASK = 0xf;
  static constexpr uint32_t RUN3_CAND_WORD_CANDFLAGS_ECFW_PHI_SHIFT = 0;
  static constexpr uint32_t RUN3_CAND_WORD_CANDFLAGS_ECFW_PHI_MASK = 0x3;
  static constexpr uint32_t RUN3_CAND_WORD_CANDFLAGS_BA_PHIOVERLAP_SHIFT = 13;
  static constexpr uint32_t RUN3_CAND_WORD_CANDFLAGS_BA_PHIOVERLAP_MASK = 0x1;
  static constexpr uint32_t RUN3_CAND_WORD_CANDFLAGS_NSWMON_SHIFT = 18;
  static constexpr uint32_t RUN3_CAND_WORD_CANDFLAGS_NSWMON_MASK = 0x1;
  static constexpr uint32_t RUN3_CAND_WORD_CANDFLAGS_BA_GT1ROI_SHIFT = 12;
  static constexpr uint32_t RUN3_CAND_WORD_CANDFLAGS_BA_GT1ROI_MASK = 0x1;
  static constexpr uint32_t RUN3_CAND_WORD_CANDFLAGS_ECFW_GOODMF_SHIFT = 15;
  static constexpr uint32_t RUN3_CAND_WORD_CANDFLAGS_ECFW_GOODMF_MASK = 0x1;
  static constexpr uint32_t RUN3_CAND_WORD_CANDFLAGS_ECFW_INNERCOIN_SHIFT = 14;
  static constexpr uint32_t RUN3_CAND_WORD_CANDFLAGS_ECFW_INNERCOIN_MASK = 0x1;
  static constexpr uint32_t RUN3_CAND_WORD_CANDFLAGS_ECFW_BW23_SHIFT = 13;
  static constexpr uint32_t RUN3_CAND_WORD_CANDFLAGS_ECFW_BW23_MASK = 0x1;
  static constexpr uint32_t RUN3_CAND_WORD_CANDFLAGS_ECFW_CHARGE_SHIFT = 12;
  static constexpr uint32_t RUN3_CAND_WORD_CANDFLAGS_ECFW_CHARGE_MASK = 0x1;
  static constexpr uint32_t RUN3_CAND_WORD_PT_SHIFT = 8;
  static constexpr uint32_t RUN3_CAND_WORD_PT_MASK = 0xf;
  static constexpr uint32_t RUN3_CAND_WORD_ROI_SHIFT = 0;
  static constexpr uint32_t RUN3_CAND_WORD_ROI_MASK = 0xff;

  // Topo TOB words
  static constexpr uint32_t RUN3_TOPO_WORD_ID_SHIFT = 29;
  static constexpr uint32_t RUN3_TOPO_WORD_ID_MASK = 0b111;
  static constexpr uint32_t RUN3_TOPO_WORD_ID_VAL = 0b110;
  static constexpr uint32_t RUN3_TOPO_WORD_HEMI_SHIFT = 24;
  static constexpr uint32_t RUN3_TOPO_WORD_HEMI_MASK = 0x1;
  static constexpr uint32_t RUN3_TOPO_WORD_FLAGS_SHIFT = 20;
  static constexpr uint32_t RUN3_TOPO_WORD_FLAGS_MASK = 0xf;
  static constexpr uint32_t RUN3_TOPO_WORD_PT_SHIFT = 16;
  static constexpr uint32_t RUN3_TOPO_WORD_PT_MASK = 0xf;
  static constexpr uint32_t RUN3_TOPO_WORD_ETA_SHIFT = 8;
  static constexpr uint32_t RUN3_TOPO_WORD_ETA_MASK = 0xff;
  static constexpr uint32_t RUN3_TOPO_WORD_PHI_SHIFT = 0;
  static constexpr uint32_t RUN3_TOPO_WORD_PHI_MASK = 0xff;
  static constexpr uint32_t RUN3_TOPO_WORD_DET_SHIFT = 13;
  static constexpr uint32_t RUN3_TOPO_WORD_DET_MASK  = 0x3;
  static constexpr uint32_t RUN3_TOPO_WORD_CANDFLAGS_ECFW_CHARGE_SHIFT = 20;
  static constexpr uint32_t RUN3_TOPO_WORD_CANDFLAGS_ECFW_BW23_SHIFT = 21;
  static constexpr uint32_t RUN3_TOPO_WORD_CANDFLAGS_ECFW_INNERCOIN_SHIFT = 22;
  static constexpr uint32_t RUN3_TOPO_WORD_CANDFLAGS_ECFW_GOODMF_SHIFT = 23;
  static constexpr uint32_t RUN3_TOPO_WORD_CANDFLAGS_ECFW_CHARGE_MASK = 0x1;
  static constexpr uint32_t RUN3_TOPO_WORD_CANDFLAGS_ECFW_BW23_MASK = 0x1;
  static constexpr uint32_t RUN3_TOPO_WORD_CANDFLAGS_ECFW_INNERCOIN_MASK = 0x1;
  static constexpr uint32_t RUN3_TOPO_WORD_CANDFLAGS_ECFW_GOODMF_MASK = 0x1;

  // ROD status word
  static constexpr uint32_t RUN3_STATUS_WORD_ID_SHIFT = 29;
  static constexpr uint32_t RUN3_STATUS_WORD_ID_MASK = 0b111;
  static constexpr uint32_t RUN3_STATUS_WORD_ID_VAL = 0b111;
  static constexpr uint32_t RUN3_STATUS_WORD_SHIFT = 0;
  static constexpr uint32_t RUN3_STATUS_WORD_MASK = 0xffff;

} // namespace LVL1::MuCTPIBits

namespace L1Muon::MuCTPIBits {
  // Shifts and masks for RPC & TGC SL to MUCTPI data format for Run 4
  static constexpr uint32_t RUN4_SL2MUCTPI_BOARD_ID_SHIFT = 20;
  static constexpr uint32_t RUN4_SL2MUCTPI_BOARD_ID_MASK = 0x7F;
  static constexpr uint32_t RUN4_SL2MUCTPI_FIBER_ID_SHIFT = 16;
  static constexpr uint32_t RUN4_SL2MUCTPI_FIBER_ID_MASK = 0b1111;
  static constexpr uint32_t RUN4_SL2MUCTPI_BCID_SHIFT = 0;
  static constexpr uint32_t RUN4_SL2MUCTPI_BCID_MASK = 0xFFF;
  static constexpr uint32_t RUN4_SL2MUCTPI_PT_VAL_SHIFT = 24;
  static constexpr uint32_t RUN4_SL2MUCTPI_PT_VAL_MASK = 0xFF;
  static constexpr uint32_t RUN4_SL2MUCTPI_CHARGE_SHIFT = 23;
  static constexpr uint32_t RUN4_SL2MUCTPI_CHARGE_MASK = 0x1;
  static constexpr uint32_t RUN4_SL2MUCTPI_PHI_SHIFT = 14;
  static constexpr uint32_t RUN4_SL2MUCTPI_PHI_MASK = 0x1FF;
  static constexpr uint32_t RUN4_SL2MUCTPI_ETA_SHIFT = 0;
  static constexpr uint32_t RUN4_SL2MUCTPI_ETA_MASK = 0x3FFF;
  static constexpr uint32_t RUN4_SL2MUCTPI_PTTHRESHOLD_SHIFT = 0;
  static constexpr uint32_t RUN4_SL2MUCTPI_PTTHRESHOLD_MASK = 0xF;
  static constexpr uint32_t RUN4_SL2MUCTPI_TCID_SHIFT = 4;
  static constexpr uint32_t RUN4_SL2MUCTPI_TCID_MASK = 0x7;
  static constexpr uint32_t RUN4_SL2MUCTPI_MDT_SHIFT = 7;
  static constexpr uint32_t RUN4_SL2MUCTPI_MDT_MASK = 0x1;
  static constexpr uint32_t RUN4_SL2MUCTPI_COINTYPE_SHIFT = 8;
  static constexpr uint32_t RUN4_SL2MUCTPI_COINTYPE_MASK = 0x7;
  static constexpr uint32_t RUN4_SL2MUCTPI_TC_SHIFT = 11;
  static constexpr uint32_t RUN4_SL2MUCTPI_TC_MASK = 0x1;
  static constexpr uint32_t RUN4_SL2MUCTPI_ET_SHIFT = 12;
  static constexpr uint32_t RUN4_SL2MUCTPI_ET_MASK = 0x3;
  static constexpr uint32_t RUN4_SL2MUCTPI_MDTFLAG_SHIFT = 14;
  static constexpr uint32_t RUN4_SL2MUCTPI_MDTFLAG_MASK = 0xF;
  static constexpr uint32_t RUN4_SL2MUCTPI_SEGNUM_SHIFT = 18;
  static constexpr uint32_t RUN4_SL2MUCTPI_SEGNUM_MASK = 0x3;
  static constexpr uint32_t RUN4_SL2MUCTPI_SEGQUAL_SHIFT = 20;
  static constexpr uint32_t RUN4_SL2MUCTPI_SEGQUAL_MASK = 0x7;
}


#endif // TRIGT1MUCTPIBITS_MUCTPI_BITS_H
