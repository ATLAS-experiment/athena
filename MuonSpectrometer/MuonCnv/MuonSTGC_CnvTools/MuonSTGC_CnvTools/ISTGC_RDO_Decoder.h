/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONSTGC_CNVTOOLS_ISTGC_RDO_DECODER_H
#define MUONSTGC_CNVTOOLS_ISTGC_RDO_DECODER_H

#include "GaudiKernel/IAlgTool.h"

class sTgcDigit;
class sTgcIdHelper;
class Identifier;


namespace Muon {
  class STGC_RawData;
  
  class ISTGC_RDO_Decoder : virtual public IAlgTool {
    
  public:
    virtual ~ISTGC_RDO_Decoder() = default;
    
    /** AlgTool InterfaceID
     */
    DeclareInterfaceID(Muon::ISTGC_RDO_Decoder, 1, 0);

    virtual sTgcDigit * getDigit(const EventContext& ctx, const Muon::STGC_RawData * Rawdata) const = 0;
    
  };
  
}

#endif



