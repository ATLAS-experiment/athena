/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef NrpcDigitToNrpcRDO_H
#define NrpcDigitToNrpcRDO_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "MuonDigitContainer/RpcDigitContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

#include "xAODMuonRDO/NRPCRDO.h"
#include "xAODMuonRDO/NRPCRDOContainer.h"

#include "MuonCablingData/MuonNRPC_CablingMap.h"
#include "StoreGate/ReadCondHandleKey.h"

/////////////////////////////////////////////////////////////////////////////

class NrpcDigitToNrpcRDO : public AthReentrantAlgorithm {
public:
    NrpcDigitToNrpcRDO(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~NrpcDigitToNrpcRDO() = default;
    virtual StatusCode initialize() override final;
    virtual StatusCode execute(const EventContext& ctx) const override final;
    
private:
    Gaudi::Property<std::vector<std::string>> m_convStat{this, "ConvertHitsFromStations",
                                                            {"BIS"}, "Only hits from these RPC stations are converted to RDOs" };
    
    SG::ReadCondHandleKey<MuonNRPC_CablingMap> m_cablingKey{this, "CablingKey", "MuonNRPC_CablingMap", "Key of MuonNRPC_CablingMap"};

    SG::ReadHandleKey<RpcDigitContainer> m_digitContainerKey{this, "InputObjectName", "RPC_DIGITS",
                                                             "ReadHandleKey for Input RpcDigitContainer"};

    SG::WriteHandleKey<xAOD::NRPCRDOContainer> m_NrpcContainerKey{this, "NrpcRdoKey", "NRPCRDO", "WriteHandleKey for Output AOD::NRPCRDOContainer"};
    
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    
    std::set<int> m_selectedStations{};
};

#endif
