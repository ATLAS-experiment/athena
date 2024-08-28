/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONBYTESTREAMMDTRDODECODER_H
#define MUONBYTESTREAMMDTRDODECODER_H

#include <string>
#include <vector>

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "MuonCablingData/MuonMDT_CablingMap.h"
#include "MuonDigitContainer/MdtDigit.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonMDT_CnvTools/IMDT_RDO_Decoder.h"
#include "MuonRDO/MdtAmtHit.h"
#include "StoreGate/ReadCondHandleKey.h"

namespace Muon {
    // Decoder class for conversion from MDT RDOs to MDT digits
    // Stefano Rosati
    // CERN Jan 2004

    class MdtRDO_Decoder : public AthAlgTool, virtual public IMDT_RDO_Decoder {
    public:
        MdtRDO_Decoder(const std::string& type, const std::string& name, const IInterface* parent);

        StatusCode initialize() override final;

        std::unique_ptr<MdtDigit> getDigit(const EventContext& ctx,
                                           const MdtAmtHit& amtHit, 
                                           uint16_t subdetId, 
                                           uint16_t mrodId, 
                                           uint16_t csmId) const override final;

    private:
        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
        SG::ReadCondHandleKey<MuonMDT_CablingMap> m_readKey{this, "ReadKey", "MuonMDT_CablingMap", "Key of MuonMDT_CablingMap"};
    };

}  // namespace Muon


#endif
