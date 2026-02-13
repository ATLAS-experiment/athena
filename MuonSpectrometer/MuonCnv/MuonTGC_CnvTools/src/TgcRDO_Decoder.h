/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CNVTOOLS_TGCRDO_DECODER_H
#define MUONTGC_CNVTOOLS_TGCRDO_DECODER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "CxxUtils/checker_macros.h"
#include "MuonTGC_Cabling/TgcCablingMap.h"
#include "MuonTGC_CnvTools/ITGC_RDO_Decoder.h"

class TgcDigit;
class TgcRawData;
class Identifier;

namespace Muon {

/** @class TgcRDO_Decoder
 *  This class provides conversion from TGC RDO data to
 *  TGC Digits and offline ID.
 *
 *  @author Susumu Oda <Susumu.Oda@cern.ch>
 *
 *  This class was developed by Tadashi Maeno based on
 *  CscRDO_Decoder written by Ketevi A. Assamagan.
 */

class TgcRDO_Decoder : public extends<AthAlgTool, ITGC_RDO_Decoder> {
   public:
    using base_class::base_class;
    ~TgcRDO_Decoder() = default;

    virtual StatusCode initialize() override;

    /** Set a flag for application of patch.
     *  Need to describe when patch is required. */
    void applyPatch(bool patch);
    /** Get TGC Digit from TGC RDO */
    std::unique_ptr<TgcDigit> getDigit(const EventContext& ctx,
                                       const TgcRawData& rawData,
                                       bool orFlag) const override;
    /** Get offline ID and bcTag from TGC RDO */
    Identifier getOfflineData(const EventContext& ctx,
                              const TgcRawData& rawData, bool orFlag,
                              uint16_t& bctag) const override;

   private:
    SG::ReadCondHandleKey<Muon::TgcCablingMap> m_cablingKey{
        this, "CablingKey", "MuonTgc_CablingMap"};

    bool m_applyPatch{false};
};

}  // namespace Muon

#endif  // MUONTGC_CNVTOOLS_TGCRDO_DECODER_H
