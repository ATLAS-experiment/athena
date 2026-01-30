/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigT1Interfaces/ITrigT1MuonRecRoiTool.h"
#include "TrigT1MuctpiBits/MuCTPI_Bits.h"

namespace LVL1{

  unsigned int ITrigT1MuonRecRoiTool::getBitMaskValue( const unsigned int * uintValue,
						       const unsigned int mask ) const {
    unsigned int maskcopy = mask;
    unsigned int result = *uintValue & mask;
    if ( mask != 0 ) {
      while ( ( maskcopy & 0x00000001 ) == 0 ) {
	maskcopy = maskcopy >> 1;
	result = result >> 1;
      }
    }
    return result;
  }

  ITrigT1MuonRecRoiTool::MuonTriggerSystem ITrigT1MuonRecRoiTool::getSystem( const unsigned int& roiWord ) const
  {
    unsigned int result = getBitMaskValue(&roiWord,SysIDMask());
    if( result == 0x0 ) return Barrel;
    if( result == 0x1 ) return Forward;
    //at this point, result is an unsigned int > 1 
    return Endcap;
  }
  
  ITrigT1MuonRecRoiTool::ITrigT1MuonRecRoiTool()
  {
    updateBitMask( Run2 );
  }
  void ITrigT1MuonRecRoiTool::updateBitMask( const ITrigT1MuonRecRoiTool::RoiWordFormat format ){
    if( format == Run2 ){
      m_IsRun3Mask            = 0x80000000;
      m_IsVetoedMask          = 0x10000000;
      m_ChargeMask            = 0x08000000;
      m_IsFirstCandMask       = 0x00400000;
      m_SectorAddressMask     = 0x003fc000;
      m_BarrelSectorIDMask    = 0x000f8000;
      m_EndcapSectorIDMask    = 0x001f8000;
      m_ForwardSectorIDMask   = 0x000f8000;
      m_SysIDMask             = 0x00300000;
      m_SubSysIDMask          = 0x00004000;
      m_ThresholdMask         = 0x00003800;
      m_BarrelRoIMask         = 0x0000007c;
      m_EndcapRoIMask         = 0x000003fc;
      m_ForwardRoIMask        = 0x000000fc;
      m_EndcapRMask           = 0x000003f0;
      m_EndcapPhiMask         = 0x0000000c;
      m_ForwardRMask          = 0x000000f0;
      m_ForwardPhiMask        = 0x0000000c;
      m_OverflowPerRoIMask    = 0x00000002;
      m_OverflowPerSectorMask = 0x00000001;
      m_BW2Or3Mask            = 0x00000000; // undef
      m_InnerCoinMask         = 0x00000000; // undef
      m_GoodMFMask            = 0x00000000; // undef
    }else if( format == Run3 ){
      m_IsRun3Mask            = 0x80000000;
      m_IsVetoedMask          = LVL1::MuCTPIBits::RUN3_CAND_WORD_VETO_MASK                     << LVL1::MuCTPIBits::RUN3_CAND_WORD_VETO_SHIFT;
      m_ChargeMask            = LVL1::MuCTPIBits::RUN3_CAND_WORD_CANDFLAGS_ECFW_CHARGE_MASK    << LVL1::MuCTPIBits::RUN3_CAND_WORD_CANDFLAGS_ECFW_CHARGE_SHIFT;
      m_IsFirstCandMask       = 0x00000000; // undef
      m_SectorAddressMask     = LVL1::MuCTPIBits::RUN3_CAND_SECTOR_ADDRESS_MASK                << LVL1::MuCTPIBits::RUN3_CAND_SECTOR_ADDRESS_SHIFT;
      m_BarrelSectorIDMask    = LVL1::MuCTPIBits::BARREL_SECTORID_MASK                         << LVL1::MuCTPIBits::RUN3_CAND_SECTORID_SHIFT;
      m_EndcapSectorIDMask    = LVL1::MuCTPIBits::ENDCAP_SECTORID_MASK                         << LVL1::MuCTPIBits::RUN3_CAND_SECTORID_SHIFT;
      m_ForwardSectorIDMask   = LVL1::MuCTPIBits::FORWARD_SECTORID_MASK                        << LVL1::MuCTPIBits::RUN3_CAND_SECTORID_SHIFT;
      m_SysIDMask             = LVL1::MuCTPIBits::RUN3_SUBSYS_ADDRESS_BAFW_MASK                << LVL1::MuCTPIBits::RUN3_SUBSYS_ADDRESS_SHIFT;
      m_SubSysIDMask          = LVL1::MuCTPIBits::RUN3_SUBSYS_HEMISPHERE_MASK                  << LVL1::MuCTPIBits::RUN3_SUBSYS_HEMISPHERE_SHIFT;
      m_ThresholdMask         = LVL1::MuCTPIBits::RUN3_CAND_PT_MASK                            << LVL1::MuCTPIBits::RUN3_CAND_PT_SHIFT;
      m_BarrelRoIMask         = LVL1::MuCTPIBits::BARREL_ROI_MASK                              << LVL1::MuCTPIBits::RUN3_ROI_SHIFT;
      m_EndcapRoIMask         = LVL1::MuCTPIBits::ENDCAP_ROI_MASK                              << LVL1::MuCTPIBits::RUN3_ROI_SHIFT;
      m_ForwardRoIMask        = LVL1::MuCTPIBits::FORWARD_ROI_MASK                             << LVL1::MuCTPIBits::RUN3_ROI_SHIFT;
      m_EndcapRMask           = LVL1::MuCTPIBits::RUN3_CAND_WORD_CANDFLAGS_EC_R_MASK           << LVL1::MuCTPIBits::RUN3_CAND_WORD_CANDFLAGS_ECFW_R_SHIFT;
      m_EndcapPhiMask         = LVL1::MuCTPIBits::RUN3_CAND_WORD_CANDFLAGS_ECFW_PHI_MASK       << LVL1::MuCTPIBits::RUN3_CAND_WORD_CANDFLAGS_ECFW_PHI_SHIFT;
      m_ForwardRMask          = LVL1::MuCTPIBits::RUN3_CAND_WORD_CANDFLAGS_FW_R_MASK           << LVL1::MuCTPIBits::RUN3_CAND_WORD_CANDFLAGS_ECFW_R_SHIFT;
      m_ForwardPhiMask        = LVL1::MuCTPIBits::RUN3_CAND_WORD_CANDFLAGS_ECFW_PHI_MASK       << LVL1::MuCTPIBits::RUN3_CAND_WORD_CANDFLAGS_ECFW_PHI_SHIFT;
      m_OverflowPerRoIMask    = LVL1::MuCTPIBits::ROI_OVERFLOW_MASK                            << LVL1::MuCTPIBits::RUN3_ROI_OVERFLOW_SHIFT;
      m_OverflowPerSectorMask = LVL1::MuCTPIBits::CAND_OVERFLOW_MASK                           << LVL1::MuCTPIBits::RUN3_CAND_OVERFLOW_SHIFT;
      m_BW2Or3Mask            = LVL1::MuCTPIBits::RUN3_CAND_WORD_CANDFLAGS_ECFW_BW23_MASK      << LVL1::MuCTPIBits::RUN3_CAND_WORD_CANDFLAGS_ECFW_BW23_SHIFT;
      m_InnerCoinMask         = LVL1::MuCTPIBits::RUN3_CAND_WORD_CANDFLAGS_ECFW_INNERCOIN_MASK << LVL1::MuCTPIBits::RUN3_CAND_WORD_CANDFLAGS_ECFW_INNERCOIN_SHIFT;
      m_GoodMFMask            = LVL1::MuCTPIBits::RUN3_CAND_WORD_CANDFLAGS_ECFW_GOODMF_MASK    << LVL1::MuCTPIBits::RUN3_CAND_WORD_CANDFLAGS_ECFW_GOODMF_SHIFT;
    }else{
      // no update
    }
  }
  
}
