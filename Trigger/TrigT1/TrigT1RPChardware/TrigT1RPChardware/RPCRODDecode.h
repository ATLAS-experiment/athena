/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigT1RPChardware_RPCRODDecode_H
#define TrigT1RPChardware_RPCRODDecode_H

#include "MuonCablingTools/BaseObject.h"
#include "TrigT1RPChardware/Lvl1Def.h"
#include "TrigT1RPChardware/RPCRODStructure.h"

class RPCRODDecode : public BaseObject {
public:
    RPCRODDecode();
    ~RPCRODDecode();
    sbit32 gimeRODDataAddress() { return m_RODDataAddress; };
    void RODAddresses(const RODword *RODData, const sbit32 numberOfStatusElements, const sbit32 statusBlockPosition);
    void RODHeader(const RODword *ROBData);
    int pushWord(const ubit16 inword, uint NOBXS);
    int pushWord(const RODword inword, ubit16 j, uint NOBXS);
    MatrixReadOut *CMFragment() { return &CMRO; };
    SectorLogicReadOut *SLFragment() { return &SLRO; };
    void gimeCMROData();
    void RODHeaderDisplay();
    //
    // ROD structure control flags
    ubit16 gimeCMFlag() { return m_CMFlag; };
    ubit16 gimePADFlag() { return m_PADFlag; };
    ubit16 gimeRXFlag() { return m_RXFlag; };
    void enablePrintOut();
    void disablePrintOut();
    //
    // ROD HEADER
    RODword headerMarker;
    RODword headerSize;
    RODword formatVersion;
    RODword sourceIdentifier;
    RODword Level1ID;
    RODword BunchXingID;
    RODword Level1Type;
    RODword DetectorEventType;
    ubit16 SourceReserved;
    ubit16 SourceModuleType;
    ubit16 SourceSubDetectorID;
    ubit16 SourceModuleID;
    //
    // Sector, PAd and Matrix identifiers
    ubit16 SectorID;
    ubit16 PadID;
    ubit16 CMID;
    ubit16 CMFragCheck;
    //
    // service objects
    RXReadOutStructure RXROS;
    PadReadOutStructure PDROS;
    MatrixReadOutStructure CMROS;
    MatrixReadOut CMRO;
    SectorLogicReadOutStructure SLROS;
    SectorLogicReadOut SLRO;

private:
    bool m_isSLBody{};
    //
    // ROD Data address
    sbit32 m_RODDataAddress{sbit32(0xdeadcafe)};// ROD Data Address
    //
    // define "previous" type of 16-bit data record
    enum recType { Empty, CMHead, CMSub, CMBod, CMFoot, PadHead, PadSub, PadPre, PadFoot, SLHead, SLFoot, RXHead, RXFoot };
    recType m_previousRecord{recType::Empty};
    //
    // RPC data markers
    ubit16 m_field{0xf000}; // field map of word identifier
    ubit16 m_noRecord16{9999}; // no record content for 16bit words
    RODword m_noRecord32{0xdeadcafe}; // no record content for 32bit words
    // ubit16 PADSubHeader;
    // ubit16 PADPreFooter;
    // ubit16 reserved3{0xd000};
    ubit16 m_reserved4{0xe000}; // reserved
    // ubit16 reserved5{0xf000};
    //
    // data structure control flags
    ubit16 m_CMFlag{};
    ubit16 m_PADFlag{};
    ubit16 m_RXFlag{};
    //
    // enable printouts
    bool m_enablePrintOut{};
};

#endif
