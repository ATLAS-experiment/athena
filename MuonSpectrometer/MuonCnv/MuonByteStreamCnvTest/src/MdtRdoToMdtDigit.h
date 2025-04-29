/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONBYTESTREAMCNVTEST_MDTRDOTOMDTDIGIT_H
#define MUONBYTESTREAMCNVTEST_MDTRDOTOMDTDIGIT_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonDigitContainer/MdtDigitContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonMDT_CnvTools/IMDT_RDO_Decoder.h"
#include "MuonRDO/MdtCsmContainer.h"

class MdtRdoToMdtDigit : public AthReentrantAlgorithm {
public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;
    virtual ~MdtRdoToMdtDigit() = default;
    virtual StatusCode initialize() override final;
    virtual StatusCode execute(const EventContext& ctx) const override final;

private:
    using DigitCollection = std::vector<std::unique_ptr<MdtDigitCollection>>;
    StatusCode decodeMdt(const EventContext& ctx, const MdtCsm& rdoCollection, DigitCollection& digitContainer) const;

    ToolHandle<Muon::IMDT_RDO_Decoder> m_mdtRdoDecoderTool{this, "mdtRdoDecoderTool", "Muon::MdtRDO_Decoder", ""};
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    SG::ReadHandleKey<MdtCsmContainer> m_mdtRdoKey{this, "MdtRdoContainer", "MDTCSM", "Mdt RDO Input"};
    SG::WriteHandleKey<MdtDigitContainer> m_mdtDigitKey{this, "MdtDigitContainer", "MDT_DIGITS", "Mdt Digit Output"};
};

#endif
