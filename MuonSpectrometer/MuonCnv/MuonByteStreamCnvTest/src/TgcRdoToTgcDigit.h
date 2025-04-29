/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONBYTESTREAMCNVTEST_TGCRDOTOTGCDIGIT_H
#define MUONBYTESTREAMCNVTEST_TGCRDOTOTGCDIGIT_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonDigitContainer/TgcDigitContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRDO/TgcRdoContainer.h"
#include "MuonTGC_CnvTools/ITGC_RDO_Decoder.h"
#include "MuonTGC_Cabling/MuonTGC_CablingSvc.h"

class TgcRdoToTgcDigit : public AthReentrantAlgorithm {
public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm; 
    virtual ~TgcRdoToTgcDigit() = default;

    virtual StatusCode initialize() override final;
    virtual StatusCode execute(const EventContext& ctx) const override final;

private:
    StatusCode decodeTgc(const TgcRdo*, TgcDigitContainer*, Identifier&) const;

    ToolHandle<Muon::ITGC_RDO_Decoder> m_tgcRdoDecoderTool{this, "tgcRdoDecoderTool", "Muon::TgcRDO_Decoder", ""};
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    Gaudi::Property<bool> m_show_warning_level_invalid_TGC_A09_SSW6_hit{this, "show_warning_level_invalid_TGC_A09_SSW6_hit", false, ""};

    ServiceHandle<MuonTGC_CablingSvc> m_tgcCabling{this, "TGCCablingSvc", "MuonTGC_CablingSvc", ""};

    SG::ReadHandleKey<TgcRdoContainer> m_tgcRdoKey{this, "TgcRdoContainer", "TGCRDO", "Tgc RDO Input"};
    SG::WriteHandleKey<TgcDigitContainer> m_tgcDigitKey{this, "TgcDigitContainer", "TGC_DIGITS", "Tgc Digit Output"};
};

#endif
