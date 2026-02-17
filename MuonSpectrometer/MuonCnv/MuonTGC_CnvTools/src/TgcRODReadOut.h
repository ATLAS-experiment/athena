/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CNVTOOLS_TGCRODREADOUT_H
#define MUONTGC_CNVTOOLS_TGCRODREADOUT_H


#include "ByteStreamData/RawEvent.h"
#include "MuonRDO/TgcRdo.h"
#include "MuonTGC_Cabling/TgcCablingMap.h"
#include "TgcSlbDataHelper.h"

#include <inttypes.h>
#include <vector>
#include <memory>
#include <array>
#include <atomic>

class MsgStream;
namespace Muon {
class TgcSlbData;
class TgcSlbDataHelper;

/** @class TgcRODReadOut
 *
 *  TGC ROD decoder for conversion from ROB fragment to TgcRDO
 *
 *  @author Susumu Oda <Susumu.Oda@cern.ch>
 *  @author Hisaya Kurashige
 *
 *  This class was developed by Tadashi Maeno based on
 *  MdtRODReadOut written by S. Rosati. Hisaya Kurashige
 *  removed TGC ROD Encoder and updated decodeRodToRdo
 *  on January 2008. Takashi Kubota migrated to
 *  MuonTGC_CnvTools.
 */

class TgcRODReadOut {
   private:
    typedef OFFLINE_FRAGMENTS_NAMESPACE::PointerType ByteStream;

   public:
    /** Constructor */
    TgcRODReadOut() = default;
    /** Destructor */
    virtual ~TgcRODReadOut();

    /** Convert BS (ROB fragment) to RDO */
    StatusCode byteStream2Rdo(const ByteStream& bs, TgcRdo& tgcRdo,
                              uint32_t source_id, const TgcCablingMap& cabling,
                              MsgStream& log) const;
    /** Convert BS (ROB fragment) to RDO and compare decoded RDO container
     *  and another RDO container decoded by other converter */
    bool check(const ByteStream& bs, const TgcRdo& tgcRdo, uint32_t source_id,
               const TgcCablingMap& cabling,

               MsgStream& log) const;
    /** Compare two RDO containers */
    void compare(const TgcRdo& rdo, const TgcRdo& newRdo, MsgStream& msg) const;
    /** Compare two RDOs */
    static bool isMatched(const TgcRawData& rdo1, const TgcRawData& rdo2);
    /** Decode BS to RDO container */
    StatusCode decodeRodToRdo(TgcRdo& tgcRdo, const ByteStream& vData,
                              uint16_t subDetectorId, uint16_t rodId,
                              uint32_t l1Id, uint16_t bcId,
                              const TgcCablingMap& cabling,
                              MsgStream& log) const;

   protected:
    enum {
        RawDataFragMask = 0xFF000000,    // RawData Fragment HEADER  Mask
        FragmentIdMask = 0xFF000000,     // Fragmen ID Mask
        FragmentCountMask = 0x00FFFFFF,  // Fragmen ID Mask
        HeaderMask = 0xE0000000,         //
        HeaderEvent = 0x00000000,        // Event Header
        HeaderError = 0x20000000,        // Error Report
        HeaderSLB10 = 0x40000000,        // SLB Header 10
        HeaderSLB11 = 0x60000000,        // SLB Header 11
        HeaderSLBC = 0x80000000,         // central bunch
        HeaderSLBP = 0xA0000000,         // previous bunch
        HeaderSLBN = 0xC0000000,         // next bunch
        HeaderTrailer = 0x70000000,      // SLB Header 11

        ROD_START = 0xEE1234EE,
        ROD_HEADER_SIZE = 0x09,
        ROD_STATUS_SIZE = 0x05
    };

    /** Set sbLoc */
    bool setSbLoc(uint16_t subDetectorId, uint16_t rodId, TgcSlbData* slb,
                  int rxId, const TgcCablingMap& cabling, MsgStream& log) const;

   private:
    /** The number of RODs (1-24 for 12-fold) */
    enum NROD_SIDE {
        NROD = 24 + 1,
        NSROD = 6 + 1,
        ASIDE = 0x67,  // 103
        CSIDE = 0x68   // 104
    };
    /** The number of failures on decodeRodToRdo */
    mutable std::array<std::atomic<unsigned int>, NROD + 1>
        m_failedDecodeRodToRdo ATLAS_THREAD_SAFE{};
    /** The number of strange header and SizeRawData */
    mutable std::array<std::atomic<unsigned int>, NROD + 1>
        m_failedHeaderSizeRawData ATLAS_THREAD_SAFE{};
    /** The number of failures on setSbLoc */
    mutable std::array<std::atomic<unsigned int>, NROD + 1> m_failedSetSbLoc
        ATLAS_THREAD_SAFE{};
    /** The number of failures on setType */
    mutable std::array<std::atomic<unsigned int>, NROD + 1> m_failedSetType
        ATLAS_THREAD_SAFE{};
    /** The number of failures on getSLBIDfromRxID */
    mutable std::array<std::atomic<unsigned int>, NROD + 1>
        m_failedGetSLBIDfromRxID ATLAS_THREAD_SAFE{};
    /** The number of failures on getReadoutIDfromSLBID */
    mutable std::array<std::atomic<unsigned int>, NROD + 1>
        m_failedGetReadoutIDfromSLBID ATLAS_THREAD_SAFE{};

    /** TGC SLB data helper */
    std::unique_ptr<TgcSlbDataHelper> m_tgcSlbDataHelper{
        std::make_unique<TgcSlbDataHelper>()};
};

}  // namespace Muon

#endif  // MUONTGC_CNVTOOLS_TGCRODREADOUT_H
