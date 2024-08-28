/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONMDT_CNVTOOLS_IMDT_RDO_DECODER_H
#define MUONMDT_CNVTOOLS_IMDT_RDO_DECODER_H

#include "GaudiKernel/IAlgTool.h"

class MdtDigit;
class MdtAmtHit;
class Identifier;


namespace Muon {

    class IMDT_RDO_Decoder : virtual public IAlgTool {
    public:
        /** AlgTool InterfaceID
         */
        DeclareInterfaceID(Muon::IMDT_RDO_Decoder , 1, 0);
        
        virtual std::unique_ptr<MdtDigit> getDigit(const EventContext& ctx,
                                                   const MdtAmtHit& amtHit, 
                                                   uint16_t subdetId, 
                                                   uint16_t mrodId, 
                                                   uint16_t csmId) const = 0;

    };

}  // namespace Muon

#endif
