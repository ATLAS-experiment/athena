/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TGCDIGITTOTGCRDO_H
#define TGCDIGITTOTGCRDO_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "MuonDigitContainer/TgcDigitContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRDO/TgcRdo.h"
#include "MuonRDO/TgcRdoContainer.h"
#include "StoreGate/DataHandle.h"
#include "TGCcablingInterface/ITGCcablingServerSvc.h"
#include "TGCcablingInterface/ITGCcablingSvc.h"

/////////////////////////////////////////////////////////////////////////////

class TgcDigitToTgcRDO : public AthReentrantAlgorithm {
public:
    TgcDigitToTgcRDO(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~TgcDigitToTgcRDO() = default;
    virtual StatusCode initialize() override final;
    virtual StatusCode execute(const EventContext& ctx) const override final;

private:
    StatusCode getCabling();

    // NOTE: although this function has no clients in release 22, currently the Run2 trigger simulation is still run in
    //       release 21 on RDOs produced in release 22. Since release 21 accesses the TagInfo, it needs to be written to the
    //       RDOs produced in release 22. The fillTagInfo() function thus needs to stay in release 22 until the workflow changes
    StatusCode fillTagInfo() const;

    static uint16_t getRdoId(const TgcRawData& rawData);

    BooleanProperty m_isNewTgcDigit{this, "isNewTgcDigit",  true};  // to select new TgcDigit (bcTag added)

protected:
    ServiceHandle<ITGCcablingServerSvc> m_tgc_cabling_server{this, "CablingServer", "Muon::TGCCablingServerSvc"};
    const ITGCcablingSvc* m_cabling{nullptr};

    SG::WriteHandleKey<TgcRdoContainer> m_rdoContainerKey{this, "OutputObjectName", "TGCRDO", "WriteHandleKey for Output TgcRdoContainer"};
    SG::ReadHandleKey<TgcDigitContainer> m_digitContainerKey{this, "InputObjectName", "TGC_DIGITS",
                                                             "ReadHandleKey for Input TgcDigitContainer"};
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    std::string m_cablingType;
};

#endif
