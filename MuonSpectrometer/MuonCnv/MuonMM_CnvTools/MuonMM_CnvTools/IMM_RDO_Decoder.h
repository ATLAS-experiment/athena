/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONMM_CNVTOOLS_IMM_RDO_DECODER_H
#define MUONMM_CNVTOOLS_IMM_RDO_DECODER_H

#include "GaudiKernel/IAlgTool.h"

class MmDigit;
class MmIdHelper;
class Identifier;


namespace Muon {
  class MM_RawData;
  
  class IMM_RDO_Decoder : virtual public IAlgTool {
    
  public:
    virtual ~IMM_RDO_Decoder() = default;
    /** AlgTool InterfaceID
     */
    DeclareInterfaceID(Muon::IMM_RDO_Decoder, 1, 0);
    
    virtual std::unique_ptr<MmDigit> getDigit(const EventContext& ctx,
                                              const Muon::MM_RawData* Rawdata) const = 0;
    
  };
  
}
#endif



