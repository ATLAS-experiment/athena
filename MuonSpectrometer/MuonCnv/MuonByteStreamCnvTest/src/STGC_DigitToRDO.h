/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef STGCDIGITTORDO_H
#define STGCDIGITTORDO_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "MuonDigitContainer/sTgcDigitContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRDO/STGC_RawDataContainer.h"
#include "NSWCalibTools/INSWCalibTool.h"
#include "StoreGate/DataHandle.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "MuonCablingData/Nsw_CablingMap.h"

/////////////////////////////////////////////////////////////////////////////

class STGC_DigitToRDO : public AthReentrantAlgorithm {
public:
    STGC_DigitToRDO(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~STGC_DigitToRDO() = default;
    virtual StatusCode initialize() override final;
    virtual StatusCode execute(const EventContext& ctx) const override final;

private:
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    SG::WriteHandleKey<Muon::STGC_RawDataContainer> m_rdoContainer{this, "OutputObjectName", "sTGCRDO",
                                                                   "WriteHandleKey for Output STGC_RawDataContainer"};
    SG::ReadHandleKey<sTgcDigitContainer> m_digitContainer{this, "InputObjectName", "sTGC_DIGITS",
                                                           "ReadHandleKey for Input sTgcDigitContainer"};
    ToolHandle<Muon::INSWCalibTool> m_calibTool{this, "CalibrationTool", ""};

    //The cabling map is only needed for studies of the mm connector misalignmen, but not it regular jobs. Therefore the key is left empty here.
    SG::ReadCondHandleKey<Nsw_CablingMap> m_cablingKey{this, "CablingMap", "","Key of Nsw_CablingMap"};

};

#endif
