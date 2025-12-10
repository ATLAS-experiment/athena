/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigT1RPChardware_RPCRXRODDecode_H
#define TrigT1RPChardware_RPCRXRODDecode_H

#include "MuonCablingTools/BaseObject.h"
#include "TrigT1RPChardware/Lvl1Def.h"
#include "TrigT1RPChardware/RPCRODStructure.h"
#include "TrigT1RPChardware/SectorLogicRXReadOut.h"

class RPCRXRODDecode : public BaseObject {
public:
    RPCRXRODDecode();
    sbit32 gimeRODDataAddress() { return m_RODDataAddress; };
    void RODAddresses(const RODword *RODData, const sbit32 numberOfStatusElements, const sbit32 statusBlockPosition);
    void RODHeader(const RODword *ROBData);
    int pushWord(const ubit16 inword, uint NOBXS);
    int pushWord(const RODword inword, ubit16 j, uint NOBXS);
    MatrixReadOut *CMFragment() { return &CMRO; };
    SectorLogicRXReadOut *SLFragment() { return &SLRO; };
    void gimeCMROData();
    void RODHeaderDisplay();
    //
    /// ROD structure control flags
    ubit16 gimeCMFlag() { return m_CMFlag; };
    ubit16 gimePADFlag() { return m_PADFlag; };
    ubit16 gimeRXFlag() { return m_RXFlag; };
    void enablePrintOut();
    void disablePrintOut();
    //
    // Special for sector logic
    void setSLFragmentFound(bool slFound) { m_slFound = slFound; };

    //
    /// ROD HEADER
    // 0xdeadcafe = no record content for 32bit words
    RODword headerMarker{0xdeadcafe};
    RODword headerSize{0xdeadcafe};
    RODword formatVersion{0xdeadcafe};
    RODword sourceIdentifier{0xdeadcafe};
    RODword Level1ID{0xdeadcafe};
    RODword BunchXingID{0xdeadcafe};
    RODword Level1Type{0xdeadcafe};
    RODword DetectorEventType{0xdeadcafe};
    //
    // 9999 = no record content for 16bit words
    ubit16 SourceReserved{9999};
    ubit16 SourceModuleType{9999};
    ubit16 SourceSubDetectorID{9999};
    ubit16 SourceModuleID{9999};
    //
    /// Sector, Pad and Matrix identifiers
    ubit16 SectorID{9999};
    ubit16 PadID{9999};
    ubit16 CMID{9999};
    ubit16 CMFragCheck{9999};
    //
    /// service objects
    RXReadOutStructure RXROS{};
    PadReadOutStructure PDROS{};
    MatrixReadOutStructure CMROS{};
    MatrixReadOut CMRO{};
    SectorLogicRXReadOutStructure SLROS{};
    SectorLogicRXReadOut SLRO{};

private:
    bool m_isSLBody{};
    //
    /// ROD Data address
    sbit32 m_RODDataAddress{};
    //
    /// define "previous" type of 16-bit data record
    enum recType { Empty, CMHead, CMSub, CMBod, CMFoot, PadHead, PadSub, PadPre, PadFoot, SLHead, SLFoot, RXHead, RXFoot };
    recType m_previousRecord{recType::Empty};
    //
    /// RPC data markers
    ubit16 m_field{0xf000}; //!< field map of word identifier
    ubit16 m_noRecord16{9999};
    RODword m_noRecord32{0xdeadcafe};

    ubit16 m_reserved4{0xe000};
    //
    /// data structure control flags
    ubit16 m_CMFlag{};
    ubit16 m_PADFlag{};
    ubit16 m_RXFlag{};
    //
    /// enable printouts
    bool m_enablePrintOut{};
    //
    /// the SL fragment was found
    bool m_slFound{};
};

#endif
