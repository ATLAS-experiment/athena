/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CNVTOOLS_TGC_RODDECODERRAWDATA_H
#define MUONTGC_CNVTOOLS_TGC_RODDECODERRAWDATA_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "MuonTGC_Cabling/TgcCablingMap.h"
#include "MuonTGC_CnvTools/ITGC_RodDecoder.h"
#include "TgcRODReadOut.h"
class TgcRdo;

namespace Muon {

/** @class TGC_RodDecoderRawdata
 *  A tool to decode a TGC ROB fragment written in the raw data format into TGC
 * RDO.
 *  @author Susumu Oda <Susumu.Oda@cern.ch>
 *  @author Hisaya Kurashige
 *
 *  This class was originally developed by Tadashi Maeno based on RpcROD_Decoder
 *  written by Ketevi A. Assamagan. Hisaya Kurashige removed TGC ROD Encoder
 * from this and updated decodeRodToRdo in January 2008. Takashi Kubota migrated
 * to MuonTGC_CnvTools package in July 2008. The previous class name was
 * TgcROD_Decoder.
 */

class TGC_RodDecoderRawdata : public extends<AthAlgTool, ITGC_RodDecoder> {
   public:
    /** Default constructor */
    using base_class::base_class;
    /** Default destructor */
    virtual ~TGC_RodDecoderRawdata();

    /** Standard AlgTool method */
    virtual StatusCode initialize() override;
    /** Convert ROBFragment to RDO */
    virtual StatusCode fillCollection(
        const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment& robFrag,
        TgcRdoContainer& rdoIdc, const EventContext& ctx) const override;

   private:
    /** Retrieve header of ROBFragment */
    TgcRdo* getCollection(
        const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment& robFrag,
        TgcRdoContainer& rdoIdc) const;
    /** Convert data contents of ROBFragment to RDO
     *  The same byteStream2Rdo method in TGC_RodDecoderReadout is used for
     * checking conversion validation */
    void byteStream2Rdo(OFFLINE_FRAGMENTS_NAMESPACE::PointerType bs,
                        TgcRdo& rdo, uint32_t source_id) const;
    /** Show status words */
    void showStatusWords(const uint32_t source_id, const uint16_t rdoId,
                         const int idHash, const uint32_t nstatus,
                         const uint32_t* status) const;
    /** Switch for reading IDs in SLB Header or ROD Header */
    Gaudi::Property<bool> m_readSlbHeaderId{this, "ReadSlbHeaderId", false};
    /** Switch for checking rawdata format with readout format */
    Gaudi::Property<bool> m_checkRawData{this, "CheckRawData", false};
    /** RawData format converter */
    std::unique_ptr<TgcRODReadOut> m_tgcRODReadOut{};
    /** Flag for showStatusWords */
    Gaudi::Property<bool> m_showStatusWords{this, "ShowStatusWords", false};

    SG::ReadCondHandleKey<Muon::TgcCablingMap> m_cablingKey{
        this, "CablingKey", "MuonTgc_CablingMap"};
};

}  // namespace Muon

#endif  // MUONTGC_CNVTOOLS_TGC_RODDECODERRAWDATA_H
