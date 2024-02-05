/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MMDIGITTORDO_H
#define MMDIGITTORDO_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "MuonDigitContainer/MmDigitContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRDO/MM_RawDataContainer.h"
#include "NSWCalibTools/INSWCalibTool.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "MuonCablingData/Nsw_CablingMap.h"

/////////////////////////////////////////////////////////////////////////////

class MM_DigitToRDO : public AthReentrantAlgorithm {
public:
    MM_DigitToRDO(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~MM_DigitToRDO() = default;
    virtual StatusCode initialize() override final;
    virtual StatusCode execute(const EventContext& ctx) const override final;

private:
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    SG::WriteHandleKey<Muon::MM_RawDataContainer> m_rdoContainer{this, "OutputObjectName", "MMRDO",
                                                                 "WriteHandleKey for Output MM_RawDataContainer"};
    SG::ReadHandleKey<MmDigitContainer> m_digitContainer{this, "InputObjectName", "MM_DIGITS", "ReadHAndleKey for Input MmDigitContainer"};
    ToolHandle<Muon::INSWCalibTool> m_calibTool{this, "CalibrationTool", ""};
    //The cabling map is only needed for studies of the mm connector misalignment, but not it regular jobs. Therefore the key is left empty here.
    SG::ReadCondHandleKey<Nsw_CablingMap> m_cablingKey{this, "CablingMap", "","Key of Nsw_CablingMap"};
};

#endif
