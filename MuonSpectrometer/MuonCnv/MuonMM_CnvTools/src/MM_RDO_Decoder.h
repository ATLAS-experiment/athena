/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONBYTESTREAMMMRDODECODER_H
#define MUONBYTESTREAMMMRDODECODER_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "MuonRDO/MM_RawData.h"
#include "MuonDigitContainer/MmDigit.h"
#include "MuonIdHelpers/MmIdHelper.h"

#include "MuonMM_CnvTools/IMM_RDO_Decoder.h"
#include "NSWCalibTools/INSWCalibTool.h"

#include <inttypes.h>
#include <vector>

class NswCalibDbTimeChargeData;

namespace Muon {
  // Decoder class for conversion from MM RDOs to MM digits
  // Adapted for MMs from Alexandre Laurier - June 2019
  
  class MM_RDO_Decoder: public AthAlgTool, virtual public IMM_RDO_Decoder {
    
  public:
    
    MM_RDO_Decoder( const std::string& type, const std::string& name,
        const IInterface* parent ) ;

    virtual StatusCode initialize() override final;
    
    std::unique_ptr<MmDigit> getDigit(const EventContext& ctx,
                                      const Muon::MM_RawData * data) const override final;
    
  private:
    
    const MmIdHelper* m_mmIdHelper{nullptr};
    ToolHandle<Muon::INSWCalibTool> m_calibTool{this,"CalibrationTool", ""};
    
  };
  
}



#endif
