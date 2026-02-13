/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TGCDIGITTOTGCRDO_H
#define TGCDIGITTOTGCRDO_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "MuonDigitContainer/TgcDigitContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRDO/TgcRdo.h"
#include "MuonRDO/TgcRdoContainer.h"
#include "StoreGate/DataHandle.h"
#include "MuonTGC_Cabling/TgcCablingMap.h"

/////////////////////////////////////////////////////////////////////////////

class TgcDigitToTgcRDO : public AthReentrantAlgorithm {
public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;
    virtual ~TgcDigitToTgcRDO() = default;
    virtual StatusCode initialize() override final;
    virtual StatusCode execute(const EventContext& ctx) const override final;

private:
    BooleanProperty m_isNewTgcDigit{this, "isNewTgcDigit",  true};  // to select new TgcDigit (bcTag added)

    SG::ReadCondHandleKey<Muon::TgcCablingMap> m_cablingKey{this, "CablingKey", "MuonTgc_CablingMap"};
    SG::WriteHandleKey<TgcRdoContainer> m_rdoContainerKey{this, "OutputObjectName", "TGCRDO", "WriteHandleKey for Output TgcRdoContainer"};
    SG::ReadHandleKey<TgcDigitContainer> m_digitContainerKey{this, "InputObjectName", "TGC_DIGITS",
                                                             "ReadHandleKey for Input TgcDigitContainer"};
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

};

#endif
