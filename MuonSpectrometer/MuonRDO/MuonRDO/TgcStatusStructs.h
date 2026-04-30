/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONRDO_TGCSTATUSSTRUCTS_H
#define MUONRDO_TGCSTATUSSTRUCTS_H

#include <cstdint>

namespace MuonRDO{
  struct Errors{
    unsigned badBcID:1;
    unsigned badL1Id:1;
    unsigned timedout:1;
    unsigned badData:1;
    unsigned overflow:1;
  };
  
  struct RodStatus{
    unsigned EC_RXsend:1; // Error in request to send an event via RXlink
    unsigned EC_FELdown:1; // A Front End link has gone down - abandoned
    unsigned EC_frame:1; // Invalid FE link framing words
    unsigned EC_Glnk:1; // G-link error
    unsigned EC_xor:1; // Invalid XOR event checksum
    unsigned EC_ovfl:1; // Input FE event is too long or FE FIFO overflow
    unsigned EC_timeout:1; // Timeout expired for at least one FE link
    unsigned EC_xormezz:1; // Bad XOR checksum from mezz board
    unsigned EC_wc0:1; // Event has WC=0
    unsigned EC_L1ID:1; // L1ID mismatch (TTC EVID FIFO vs local).
    unsigned EC_nohdr:1; // First word is not header
    unsigned EC_rectype:1; // Unrecognized record type
    unsigned EC_null:1; // Unexpected nulls in FE input
    unsigned EC_order:1; // Word is out of order
    unsigned EC_LDB:1; // Invalid or unexpected Local Data Block ID
    unsigned EC_RXovfl:1; // RXfifo has overflowed
    unsigned EC_SSWerr:1; // SSW reports T1C, NRC, T2C, or GlinkNoLock error
    unsigned EC_sbid:1; // Illegal SB ID
    unsigned EC_unxsbid:1; // Unexpected SB ID received
    unsigned EC_dupsb:1; // SB ID is duplicated in the event
    unsigned EC_ec4:1; // Unexpected SB L1 Event ID(lo 4)
    unsigned EC_bc:1; // Unexpected SB BCID
    unsigned EC_celladr:1; // Invalid cell address
    unsigned EC_hitovfl:1; // Too many hits in event
    unsigned EC_trgbit:1; // Unexpected trigger bits
    unsigned EC_badEoE:1; // Bad End-of-event marker received, not 0xFCA
    unsigned EC_endWCnot0:1; // WC not 0 after EoE marker
    unsigned EC_noEoE:1; // No End-of-event marker received
  };
  
  struct LocalStatus{
    unsigned mergedHitBCs:1;
    unsigned mergedTrackletBCs:1;
    unsigned sortedHits:1;
    unsigned sortedTracklets:1;
    unsigned hasRoI:1;
    unsigned fakeSsw:1;
    unsigned fill1:10;
  };
  
  template <unsigned int N, typename IntType>
  constexpr unsigned int
  bitValue(IntType value){
    static_assert(N < sizeof(IntType) * 8U);
    return (value >> N) & IntType{1};
  }
  
  inline RodStatus
  setRodStatus(uint32_t data){
    RodStatus status{};
    status.EC_RXsend    = bitValue<0>(data);
    status.EC_FELdown   = bitValue<1>(data);
    status.EC_frame     = bitValue<2>(data);
    status.EC_Glnk      = bitValue<3>(data);
    status.EC_xor       = bitValue<4>(data);
    status.EC_ovfl      = bitValue<5>(data);
    status.EC_timeout   = bitValue<6>(data);
    status.EC_xormezz   = bitValue<7>(data);
    status.EC_wc0       = bitValue<8>(data);
    status.EC_L1ID      = bitValue<9>(data);
    status.EC_nohdr     = bitValue<10>(data);
    status.EC_rectype   = bitValue<11>(data);
    status.EC_null      = bitValue<12>(data);
    status.EC_order     = bitValue<13>(data);
    status.EC_LDB       = bitValue<14>(data);
    status.EC_RXovfl    = bitValue<15>(data);
    status.EC_SSWerr    = bitValue<16>(data);
    status.EC_sbid      = bitValue<17>(data);
    status.EC_unxsbid   = bitValue<18>(data);
    status.EC_dupsb     = bitValue<19>(data);
    status.EC_ec4       = bitValue<20>(data);
    status.EC_bc        = bitValue<21>(data);
    status.EC_celladr   = bitValue<22>(data);
    status.EC_hitovfl   = bitValue<23>(data);
    status.EC_trgbit    = bitValue<24>(data);
    status.EC_badEoE    = bitValue<25>(data);
    status.EC_endWCnot0 = bitValue<26>(data);
    status.EC_noEoE     = bitValue<27>(data);
    return status;
  }
  
  inline Errors
  setErrors(uint16_t data){
    Errors errors{};
    errors.badBcID  = bitValue<0>(data);
    errors.badL1Id  = bitValue<1>(data);
    errors.timedout = bitValue<2>(data);
    errors.badData  = bitValue<3>(data);
    errors.overflow = bitValue<4>(data);
    return errors;
  }
  
  inline LocalStatus
  setLocalStatus(uint32_t data){
    LocalStatus status{};
    status.mergedHitBCs      = bitValue<0>(data);
    status.mergedTrackletBCs = bitValue<1>(data);
    status.sortedHits        = bitValue<2>(data);
    status.sortedTracklets   = bitValue<3>(data);
    status.hasRoI            = bitValue<4>(data);
    status.fakeSsw           = bitValue<5>(data);
    status.fill1             = (data >> 6) & 0x3ffU; //10-bit field
    return status;
  }
}

#endif