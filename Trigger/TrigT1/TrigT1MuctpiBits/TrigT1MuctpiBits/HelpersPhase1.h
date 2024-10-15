/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGT1MUCTPIBITS_HELPERSPHASE1_H
#define TRIGT1MUCTPIBITS_HELPERSPHASE1_H

#include "MuCTPI_Bits.h"
#include <array>
#include <vector>
#include <string_view>
#include <iostream>

namespace LVL1::MuCTPIBits {
  // Helper types
  enum class WordType : uint8_t {Undefined=0, Timeslice, Multiplicity, Candidate, Topo, Status, MAX};
  enum class SubsysID : uint8_t {Undefined=0, Barrel, Forward, Endcap, MAX};

  // Mapping of the six RPC indexes into the 15 TGC indexes
  const uint32_t RPCtoTGC_pt_map[7] = {0, 2, 4, 6, 8, 10, 12};

  // Status data word error definitions
  static constexpr std::array<std::string_view,16> DataStatusWordErrors = {
    "Event number mismatch between MSPA and TRP in the central time slice",
    "Event number mismatch between MSPC and TRP in the central time slice",
    "Event number mismatch between MSPA and MSPC in any time slice",
    "BCID mismatch between TRP and MSPA in the central time slice",
    "BCID mismatch between TRP and MSPC in the central time slice",
    "BCID mismatch between MSPA and MSPC in any time slice",
    "MSPA multiplicity LVDS link CRC error in any time slice",
    "MSPC multiplicity LVDS link CRC error in any time slice",
    "Sector logic error flag set on any of the 104 MSPA sectors",
    "Sector logic error flag set on any of the 104 MSPC sectors",
    "Error flag set in any of the muon candidates in the event after zero-supression",
    "CRC error on the MSPA DAQ link (in any time slice)",
    "CRC error on the MSPC DAQ link (in any time slice)",
    "TriggerType reception timeout error",
    "MSPA DAQ link input FIFO full flag (cleared at EOF)",
    "MSPC DAQ link input FIFO full flag (cleared at EOF)"
  };

  // Helper functions
  /// Extract sub-word from 32-bit word by applying a shift and a mask
  inline constexpr uint32_t maskedWord(uint32_t word, uint32_t shift, uint32_t mask) {
    return ((word >> shift) & mask);
  }

  /// Extract sub-word from 64-bit word by applying a shift and a mask
  inline constexpr uint32_t maskedWord(uint64_t word, uint32_t shift, uint32_t mask) {
    return ((word >> shift) & mask);
  }

  /// Extract sub-word from 32-bit word by applying a shift and a mask
  inline constexpr uint32_t buildWord(uint32_t value, uint32_t shift, uint32_t mask) {
    return ((value & mask) << shift);
  }

  /// Compare a sub-word of a 32-bit word to an expected value
  inline constexpr bool wordEquals(uint32_t word, uint32_t shift, uint32_t mask, uint32_t value) {
    return maskedWord(word, shift, mask) == value;
  }

  /// Determine the type of a MUCTPI ROD word
  inline constexpr WordType getWordType(uint32_t word) {
    if (wordEquals(word, RUN3_TIMESLICE_MULT_WORD_ID_SHIFT, RUN3_TIMESLICE_MULT_WORD_ID_MASK, RUN3_TIMESLICE_MULT_WORD_ID_VAL)) {
      if (wordEquals(word, RUN3_TIMESLICE_MULT_WORD_NUM_SHIFT, RUN3_TIMESLICE_MULT_WORD_NUM_MASK, RUN3_TIMESLICE_WORD_NUM_VAL)) {
	return WordType::Timeslice;
      }
      return WordType::Multiplicity;
    } else if (wordEquals(word, RUN3_CAND_WORD_ID_SHIFT, RUN3_CAND_WORD_ID_MASK, RUN3_CAND_WORD_ID_VAL)) {
      return WordType::Candidate;
    } else if (wordEquals(word, RUN3_TOPO_WORD_ID_SHIFT, RUN3_TOPO_WORD_ID_MASK, RUN3_TOPO_WORD_ID_VAL)) {
      return WordType::Topo;
    } else if (wordEquals(word, RUN3_STATUS_WORD_ID_SHIFT, RUN3_STATUS_WORD_ID_MASK, RUN3_STATUS_WORD_ID_VAL)) {
      return WordType::Status;
    }
	return WordType::Undefined;
  }

  /// Decode timeslice word
  inline constexpr auto timesliceHeader(uint32_t word) {
    struct {
      uint32_t bcid{0};
      uint32_t tobCount{0};
      uint32_t candCount{0};
    } header;
    header.bcid = maskedWord(word, RUN3_TIMESLICE_BCID_SHIFT, RUN3_TIMESLICE_BCID_MASK);
    header.tobCount = maskedWord(word, RUN3_TIMESLICE_NTOB_SHIFT, RUN3_TIMESLICE_NTOB_MASK);
    header.candCount = maskedWord(word, RUN3_TIMESLICE_NCAND_SHIFT, RUN3_TIMESLICE_NCAND_MASK);
    return header;
  }

  /// Encode timeslice word
  inline constexpr uint32_t timesliceHeader(uint32_t bcid, uint32_t tobCount, uint32_t candCount) {
    uint32_t word{0};
    word |= buildWord(RUN3_TIMESLICE_MULT_WORD_ID_VAL, RUN3_TIMESLICE_MULT_WORD_ID_SHIFT, RUN3_TIMESLICE_MULT_WORD_ID_MASK);
    word |= buildWord(RUN3_TIMESLICE_WORD_NUM_VAL, RUN3_TIMESLICE_MULT_WORD_NUM_SHIFT, RUN3_TIMESLICE_MULT_WORD_NUM_MASK);
    word |= buildWord(bcid, RUN3_TIMESLICE_BCID_SHIFT, RUN3_TIMESLICE_BCID_MASK);
    word |= buildWord(tobCount, RUN3_TIMESLICE_NTOB_SHIFT, RUN3_TIMESLICE_NTOB_MASK);
    word |= buildWord(candCount, RUN3_TIMESLICE_NCAND_SHIFT, RUN3_TIMESLICE_NCAND_MASK);
    return word;
  }

  /// Decode topo word :
  inline constexpr auto topoHeader(uint32_t word) {
    struct {
      bool     flag0{0};
      bool     flag1{0};
      bool     flag2{0};
      bool     flag3{0};
      uint32_t pt{0};
      uint32_t etacode{0};
      uint32_t phicode{0};
      uint32_t barrel_eta_lookup{0};
      uint32_t barrel_phi_lookup{0};
      uint32_t hemi{0};
      uint32_t det{0};
      uint32_t sec{0};
      uint32_t roi{0};
    } header;
    header.flag0   = maskedWord(word, RUN3_TOPO_WORD_FLAGS_SHIFT,   0x1); //20
    header.flag1   = maskedWord(word, RUN3_TOPO_WORD_FLAGS_SHIFT+1, 0x1); //21
    header.flag2   = maskedWord(word, RUN3_TOPO_WORD_FLAGS_SHIFT+2, 0x1); //22
    header.flag3   = maskedWord(word, RUN3_TOPO_WORD_FLAGS_SHIFT+3, 0x1); //23
    header.pt      = maskedWord(word, RUN3_TOPO_WORD_PT_SHIFT,      RUN3_TOPO_WORD_PT_MASK);
    header.etacode = maskedWord(word, RUN3_TOPO_WORD_ETA_SHIFT,     RUN3_TOPO_WORD_ETA_MASK);
    header.phicode = maskedWord(word, RUN3_TOPO_WORD_PHI_SHIFT,     RUN3_TOPO_WORD_PHI_MASK);
    // HEMISPHERE 0: C-side (-) / 1: A-side (-)
    header.hemi    = maskedWord(word, RUN3_TOPO_WORD_HEMI_SHIFT,    RUN3_TOPO_WORD_HEMI_MASK);
    // Barrel: 00 - EC: 1X - FW: 01 - see: https://indico.cern.ch/event/864390/contributions/3642129/attachments/1945776/3234220/ctp_topo_encoding.pdf
    header.det     = maskedWord(word, RUN3_TOPO_WORD_DET_SHIFT,     RUN3_TOPO_WORD_DET_MASK);
    // set EC to 2 instead of sometimes 3 - see above why
    if (header.det > 2) header.det = 2;
    // Decode Barrel:
    if (header.det == 0){
      header.sec = header.phicode >> 3;
      header.barrel_eta_lookup = (header.etacode >> 1) & 0xf;
      header.barrel_phi_lookup = header.phicode & 0x7;
    }
    // FWD
    else if (header.det == 1){
      header.sec = header.phicode >> 3 ;
      header.roi = ((header.etacode & 0x1f) << 2) | ((header.phicode >> 1) & 0x3) ;
    }
    // EC
    else if (header.det == 2){
      header.sec = header.phicode >> 2 ;
      header.roi = ((header.etacode & 0x3f) << 2) | (header.phicode & 0x3) ;
    }
    return header;
  }

  /// Decode the index of the multitpicity word, which is 1, 2, or 3
  inline constexpr uint32_t multiplicityWordNumber(uint32_t word) {
    return maskedWord(word, RUN3_TIMESLICE_MULT_WORD_NUM_SHIFT, RUN3_TIMESLICE_MULT_WORD_NUM_MASK);
  }

  /// Encode the multiplicity words
  inline constexpr std::array<uint32_t,3> multiplicityWords(uint64_t multiplicity, uint32_t triggerBits, bool overflow) {
    std::array<uint32_t,3> words{}; // zero-initialised
    for (uint32_t iWord=0; iWord<words.size(); ++iWord) {
      words[iWord] |= buildWord(RUN3_TIMESLICE_MULT_WORD_ID_VAL, RUN3_TIMESLICE_MULT_WORD_ID_SHIFT, RUN3_TIMESLICE_MULT_WORD_ID_MASK);
      words[iWord] |= buildWord(iWord+1, RUN3_TIMESLICE_MULT_WORD_NUM_SHIFT, RUN3_TIMESLICE_MULT_WORD_NUM_MASK);
    }
    words[0] |= maskedWord(multiplicity, RUN3_MULTIPLICITY_PART1_SHIFT, RUN3_MULTIPLICITY_PART1_MASK);
    words[1] |= maskedWord(multiplicity, RUN3_MULTIPLICITY_PART2_SHIFT, RUN3_MULTIPLICITY_PART2_MASK);
    words[2] |= maskedWord(multiplicity, RUN3_MULTIPLICITY_PART3_SHIFT, RUN3_MULTIPLICITY_PART3_MASK);
    words[2] |= maskedWord(triggerBits, RUN3_MULTIPLICITY_TRIGBITS_SHIFT, RUN3_MULTIPLICITY_TRIGBITS_MASK);
    words[2] |= maskedWord(static_cast<uint32_t>(overflow), RUN3_MULTIPLICITY_OVERFLOW_SHIFT, RUN3_MULTIPLICITY_OVERFLOW_MASK);
    return words;
  }

  /// Decode the subsys ID from RoI candidate word
  inline constexpr SubsysID getSubsysID(uint32_t word) {
    if (wordEquals(word, RUN3_SUBSYS_ADDRESS_EC_SHIFT, RUN3_SUBSYS_ADDRESS_EC_MASK, RUN3_SUBSYS_ADDRESS_EC_VAL)) {
      return SubsysID::Endcap;
    } else if (wordEquals(word, RUN3_SUBSYS_ADDRESS_BAFW_SHIFT, RUN3_SUBSYS_ADDRESS_BAFW_MASK, RUN3_SUBSYS_ADDRESS_FW_VAL)) {
      return SubsysID::Forward;
    } else if (wordEquals(word, RUN3_SUBSYS_ADDRESS_BAFW_SHIFT, RUN3_SUBSYS_ADDRESS_BAFW_MASK, RUN3_SUBSYS_ADDRESS_BA_VAL)) {
      return SubsysID::Barrel;
    }
    return SubsysID::Undefined;
  }

  /// Decode the data status word (returns a vector of bit indices for the errors set - empty if no errors)
  inline std::vector<size_t> getDataStatusWordErrors(uint32_t word) {
    uint16_t status = maskedWord(word, RUN3_STATUS_WORD_SHIFT, RUN3_STATUS_WORD_MASK);
    if (status==0) return {};
    std::vector<size_t> errors;
    for (size_t bit=0; bit<DataStatusWordErrors.size(); ++bit) {
      if (wordEquals(status, bit, 1u, 1u)) {
		  errors.push_back(bit);
      }
    }
    return errors;
  }

  /// Encode the data status word
  inline constexpr uint32_t dataStatusWord(uint16_t status) {
    uint32_t word = buildWord(RUN3_STATUS_WORD_ID_VAL, RUN3_STATUS_WORD_ID_SHIFT, RUN3_STATUS_WORD_ID_MASK);
    word |= status;
    return word;
  }
  struct TimesliceHeader {
      uint16_t bcid{0};
      uint16_t tobCount{0};
      uint16_t candCount{0};
  };
  struct Multiplicity {
        bool nswMon               = false;
        bool candOverflow         = false;
        std::vector<uint32_t> cnt = {};
        uint64_t bits = 0;
  };
  struct Candidate {
      bool     side = false;//C=0 A=1
      SubsysID type = SubsysID::Undefined;
      uint32_t num{0};
      uint32_t pt{0}; //1-15
      // RPC pT in the cand word is a number between 1 and 6. Needs to be mapped between 1 and 15
	  uint32_t mappedPt{0};
      uint32_t roi{0};
	  uint32_t subsystem{0};
	  float eta{0.};
	  float phi{0.};
      bool errorFlag           = false;
      bool vetoFlag            = false;
      bool sectorFlag_gtN      = false;//BA: gt2, EC/FW: gt4
      bool sectorFlag_nswMon   = false;//EC/FW only
      bool candFlag_phiOverlap = false;//BA only
      bool candFlag_gt1CandRoi = false;//BA only
      bool candFlag_GoodMF     = false;//EC/FW only
      bool candFlag_InnerCoin  = false;//EC/FW only
      bool candFlag_BW23       = false;//EC/FW only
      bool candFlag_Charge     = false;//EC/FW only
      Candidate(uint32_t word)
      {
          errorFlag = maskedWord(word, RUN3_CAND_WORD_SECTORERRORFLAG_SHIFT, RUN3_CAND_WORD_SECTORERRORFLAG_MASK);
          type = getSubsysID(word);
          side = maskedWord(word, RUN3_SUBSYS_HEMISPHERE_SHIFT, RUN3_SUBSYS_HEMISPHERE_MASK);
          vetoFlag = maskedWord(word, RUN3_CAND_WORD_VETO_SHIFT, RUN3_CAND_WORD_VETO_MASK);
	  sectorFlag_gtN = maskedWord(word, RUN3_CAND_WORD_SECTORFLAGS_GTN_SHIFT, RUN3_CAND_WORD_SECTORFLAGS_GTN_MASK);
	  pt = maskedWord(word, RUN3_CAND_WORD_PT_SHIFT, RUN3_CAND_WORD_PT_MASK);
          mappedPt = maskedWord(word, RUN3_CAND_WORD_PT_SHIFT, RUN3_CAND_WORD_PT_MASK);
	  roi = maskedWord(word, RUN3_CAND_WORD_ROI_SHIFT, RUN3_CAND_WORD_ROI_MASK);
	  sectorFlag_nswMon  = maskedWord(word, RUN3_CAND_WORD_CANDFLAGS_NSWMON_SHIFT, RUN3_CAND_WORD_CANDFLAGS_NSWMON_MASK); // for BA is zero
          if(type==SubsysID::Endcap) {
              candFlag_GoodMF    = maskedWord(word, RUN3_CAND_WORD_CANDFLAGS_ECFW_GOODMF_SHIFT, RUN3_CAND_WORD_CANDFLAGS_ECFW_GOODMF_MASK);
              candFlag_InnerCoin = maskedWord(word, RUN3_CAND_WORD_CANDFLAGS_ECFW_INNERCOIN_SHIFT, RUN3_CAND_WORD_CANDFLAGS_ECFW_INNERCOIN_MASK);
              candFlag_BW23      = maskedWord(word, RUN3_CAND_WORD_CANDFLAGS_ECFW_BW23_SHIFT, RUN3_CAND_WORD_CANDFLAGS_ECFW_BW23_MASK);
              candFlag_Charge    = maskedWord(word, RUN3_CAND_WORD_CANDFLAGS_ECFW_CHARGE_SHIFT, RUN3_CAND_WORD_CANDFLAGS_ECFW_CHARGE_MASK);
              num = maskedWord(word, RUN3_CAND_SECTORID_SHIFT, ENDCAP_SECTORID_MASK);
	      subsystem = 1;			  
		  }
	  else if(type==SubsysID::Barrel)
          {
              candFlag_phiOverlap = maskedWord(word, RUN3_CAND_WORD_CANDFLAGS_BA_PHIOVERLAP_SHIFT, RUN3_CAND_WORD_CANDFLAGS_BA_PHIOVERLAP_MASK);
              candFlag_gt1CandRoi = maskedWord(word, RUN3_CAND_WORD_CANDFLAGS_BA_GT1ROI_SHIFT, RUN3_CAND_WORD_CANDFLAGS_BA_GT1ROI_MASK);
	      subsystem = 0;
              num = maskedWord(word, RUN3_CAND_SECTORID_SHIFT, BARREL_SECTORID_MASK);//same as FW
          }
          else
          {
              candFlag_GoodMF    = maskedWord(word, RUN3_CAND_WORD_CANDFLAGS_ECFW_GOODMF_SHIFT, RUN3_CAND_WORD_CANDFLAGS_ECFW_GOODMF_MASK);
              candFlag_InnerCoin = maskedWord(word, RUN3_CAND_WORD_CANDFLAGS_ECFW_INNERCOIN_SHIFT, RUN3_CAND_WORD_CANDFLAGS_ECFW_INNERCOIN_MASK);
              candFlag_BW23      = maskedWord(word, RUN3_CAND_WORD_CANDFLAGS_ECFW_BW23_SHIFT, RUN3_CAND_WORD_CANDFLAGS_ECFW_BW23_MASK);
              candFlag_Charge    = maskedWord(word, RUN3_CAND_WORD_CANDFLAGS_ECFW_CHARGE_SHIFT, RUN3_CAND_WORD_CANDFLAGS_ECFW_CHARGE_MASK);
              num = maskedWord(word, RUN3_CAND_SECTORID_SHIFT, BARREL_SECTORID_MASK);//same as FW
	      subsystem = 2;
          }
      }
      void print() const//this function has only debug purposes
	  {
		  std::cout << "Muon word content (cand): ";
		  std::cout << (side?"Side A, ":"Side C, ");
		  if(type == SubsysID::Barrel)
			  std::cout << "BA" << num << " ";
		  else
			  std::cout << (type==SubsysID::Forward?"FW":"EC") << num << " ";
		  std::cout << "Eta = " << eta << " ";
		  std::cout << "Phi = " << phi << " ";
		  std::cout << "pT = " << pt << " "; //Remember the internal mapping. RPC has 6 thresholds and TGC has 15.
		  std::cout << "mappedPt = " << mappedPt << " ";
		  std::cout << "CF: ";
		  if(type != SubsysID::Barrel) {
			  std::cout << " GMF: " << (candFlag_GoodMF?"1":"0");     
			  std::cout << " InC: " << (candFlag_InnerCoin?"1":"0");  
			  std::cout << " -BW: " << (candFlag_BW23?"1":"0");       
			  std::cout << " Chg: " << (candFlag_Charge?"1":"0");     
		  }
		  else {
			  std::cout << " PhO: " << (candFlag_phiOverlap?"1":"0");
			  std::cout << " 1Ro: " << (candFlag_gt1CandRoi?"1":"0");
		  }		  
		  std::cout << " SF: " << std::endl;
		  if(type != SubsysID::Barrel) {
			  std::cout << " NSM: " << (sectorFlag_nswMon?"1":"0");
			  std::cout << " 4SL: " << (sectorFlag_gtN?"1":"0");
		  }
		  else {
			  std::cout << " 2SL: " << (sectorFlag_gtN?"1":"0");
		  }
		  std::cout << " Veto = " << (vetoFlag?"1":"0") << " ";
		  std::cout << std::endl;
	  }
  };
  struct TopoTOB {
	  bool     side = false;//C=0 A=1
      uint32_t pt{0};//1-15
      uint32_t etaRaw{0};
      uint32_t phiRaw{0};
	  uint32_t roi{0};
	  uint32_t barrel_eta_lookup{0};
      uint32_t barrel_phi_lookup{0};
      uint32_t det{0};
      uint32_t sec{0};
	  uint32_t subsystem{0};
	  float    etaDecoded{0.};
	  float    phiDecoded{0.};
      bool candFlag_GoodMF     = false;//EC/FW only
      bool candFlag_InnerCoin  = false;//EC/FW only
      bool candFlag_BW23       = false;//EC/FW only
      bool candFlag_Charge     = false;//EC/FW only
	  void setTopoRoI()
	  {
		// BA
		if (det == 0){
		  sec = phiRaw >> 3;
		  barrel_eta_lookup = (etaRaw >> 1) & 0xf;
		  barrel_phi_lookup = phiRaw & 0x7;
		}
		// FWD
		else if (det == 1){
		  sec = phiRaw >> 3 ;
		  roi = ((etaRaw & 0x1f) << 2) | ((phiRaw >> 1) & 0x3) ;
		}
		// EC
		else if (det == 2){
		  sec = phiRaw >> 2 ;
		  roi = ((etaRaw & 0x3f) << 2) | (phiRaw & 0x3) ;
		}
	  }
	  TopoTOB(uint32_t word)
      {
          side = maskedWord(word, RUN3_TOPO_WORD_HEMI_SHIFT, RUN3_TOPO_WORD_HEMI_MASK);
          det  = maskedWord(word, RUN3_TOPO_WORD_DET_SHIFT, RUN3_TOPO_WORD_DET_MASK);
          pt   = maskedWord(word, RUN3_TOPO_WORD_PT_SHIFT, RUN3_TOPO_WORD_PT_MASK);
		  // Barrel:00 - EC:1X - FW:01 - see: https://indico.cern.ch/event/864390/contributions/3642129/attachments/1945776/3234220/ctp_topo_encoding.pdf
		  // set EC to 2 instead of sometimes 3 - see above why
		  if(det > 2) det = 2;
		  if(det == 0) subsystem = 0;
		  else if(det == 1) subsystem = 2;
		  else if(det == 2) subsystem = 1;
		  candFlag_GoodMF    = maskedWord(word, RUN3_TOPO_WORD_CANDFLAGS_ECFW_GOODMF_SHIFT, RUN3_TOPO_WORD_CANDFLAGS_ECFW_GOODMF_MASK);
          candFlag_InnerCoin = maskedWord(word, RUN3_TOPO_WORD_CANDFLAGS_ECFW_INNERCOIN_SHIFT, RUN3_TOPO_WORD_CANDFLAGS_ECFW_INNERCOIN_MASK);
          candFlag_BW23      = maskedWord(word, RUN3_TOPO_WORD_CANDFLAGS_ECFW_BW23_SHIFT, RUN3_TOPO_WORD_CANDFLAGS_ECFW_BW23_MASK);
          candFlag_Charge    = maskedWord(word, RUN3_TOPO_WORD_CANDFLAGS_ECFW_CHARGE_SHIFT, RUN3_TOPO_WORD_CANDFLAGS_ECFW_CHARGE_MASK);
          etaRaw = maskedWord(word, RUN3_TOPO_WORD_ETA_SHIFT, RUN3_TOPO_WORD_ETA_MASK);
          phiRaw = maskedWord(word, RUN3_TOPO_WORD_PHI_SHIFT, RUN3_TOPO_WORD_PHI_MASK);
		  setTopoRoI();
      }
	  void print() const //this function is just for debug purposes
	  {
		  std::cout << "Muon word content (TOB) : ";
		  std::cout << (side?"Side A, ":"Side C, ");
		  if(det == 0)
			  std::cout << "BA" << sec << " ";
		  else
			  std::cout << (det==1?"FW":"EC") << sec << " ";
		  std::cout << "Eta = " << etaDecoded << " ";
		  std::cout << "Phi = " << phiDecoded << " ";
		  std::cout << "pT = " << pt << std::endl;
		  std::cout << "CF:";
		  if(det == 1 || det == 2) {
			  std::cout << " GMF: " << (candFlag_GoodMF?"1":"0");
			  std::cout << " InC: " << (candFlag_InnerCoin?"1":"0");
			  std::cout << " -BW: " << (candFlag_BW23?"1":"0");
			  std::cout << " Chg: " << (candFlag_Charge?"1":"0");
		  }
		  std::cout << std::endl;
	  }
  };

  struct Slice {
      uint32_t bcid{0},nTOB{0},nCand{0};
      Multiplicity           mlt;
      std::vector<Candidate> cand       = {};
      std::vector<TopoTOB>   tob        = {};
  };

} // namespace LVL1::MuCTPIBits

#endif // TRIGT1MUCTPIBITS_HELPERSPHASE1_H
