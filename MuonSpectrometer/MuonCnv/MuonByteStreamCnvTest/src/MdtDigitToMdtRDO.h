/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MDTDIGITTOMDTRDO_H
#define MDTDIGITTOMDTRDO_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "MuonCablingData/MuonMDT_CablingMap.h"
#include "MuonCondData/MdtCondDbData.h"
#include "MuonDigitContainer/MdtDigitContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRDO/MdtCsmContainer.h"
#include "StoreGate/ReadCondHandleKey.h"
/////////////////////////////////////////////////////////////////////////////

class MdtDigitToMdtRDO : public AthReentrantAlgorithm {
public:
    MdtDigitToMdtRDO(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~MdtDigitToMdtRDO() = default;
    virtual StatusCode initialize() override final;
    virtual StatusCode execute(const EventContext& ctx) const override final;

private:
    // NOTE: although this function has no clients in release 22, currently the Run2 trigger simulation is still run in
    //       release 21 on RDOs produced in release 22. Since release 21 accesses the TagInfo, it needs to be written to the
    //       RDOs produced in release 22. The fillTagInfo() function thus needs to stay in release 22 until the workflow changes
    StatusCode fillTagInfo() const;

protected:
    ///
    bool m_BMGpresent{false};
    int m_BMG_station_name{-1};
    int m_BIS_station_name{-1};
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    SG::WriteHandleKey<MdtCsmContainer> m_csmContainerKey{this, "OutputObjectName", "MDTCSM", "WriteHandleKey for Output MdtCsmContainer"};
    SG::ReadHandleKey<MdtDigitContainer> m_digitContainerKey{this, "InputObjectName", "MDT_DIGITS",
                                                             "ReadHandleKey for Input MdtDigitContainer"};
    SG::ReadCondHandleKey<MuonMDT_CablingMap> m_cablingKey{this, "CablingKey", "MuonMDT_CablingMap", "Key of MuonMDT_CablingMap"};
    SG::ReadCondHandleKey<MdtCondDbData> m_condKey{this, "ConditionsKey", "MdtCondDbData", "Key of MDT condition data"};

    Gaudi::Property<bool> m_isPhaseII{this, "isPhaseII", false, "Switch to set the phase II geometry. Allows for cabling failures"};
};

#endif
