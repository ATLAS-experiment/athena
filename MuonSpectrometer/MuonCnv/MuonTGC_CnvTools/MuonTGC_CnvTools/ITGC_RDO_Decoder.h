/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONTGC_CNVTOOL_ITGC_RDO_Decoder_H
#define MUONTGC_CNVTOOL_ITGC_RDO_Decoder_H

#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/IAlgTool.h"
class TgcDigit;
class TgcRawData;
class Identifier;

namespace Muon {

/** @class ITGC_RDO_Decoder
 *  The interface for AlgTool which provides conversion from RDO data to
 *  TGC digits and offline ID.
 *  @author Susumu Oda <Susumu.Oda@cern.ch>
 */

class ITGC_RDO_Decoder : virtual public IAlgTool {

   public:
    /** Provide interface ID of ITGC_RDO_Decoder */
    DeclareInterfaceID(ITGC_RDO_Decoder, 1, 0);

    /** Set a flag for application of patch.
     *  Need to describe when patch is required. */
    virtual void applyPatch(bool patch) = 0;
    /** Get TGC Digit from TGC RDO */
    virtual std::unique_ptr<TgcDigit> getDigit(const EventContext& ctx,
                                               const TgcRawData& rawData,
                                               bool orFlag) const = 0;
    /** Get offline ID and bcTag from TGC RDO */
    virtual Identifier getOfflineData(const EventContext& ctx,
                                      const TgcRawData& rawData, bool orFlag,
                                      uint16_t& bctag) const = 0;
};

}  // namespace Muon

#endif  // MUONTGC_CNVTOOL_ITGC_RDO_Decoder_H
