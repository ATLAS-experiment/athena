/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef NrpcDigitToNrpcRDO_H
#define NrpcDigitToNrpcRDO_H

#include <unordered_set>
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "MuonDigitContainer/RpcDigitContainer.h"
#include "xAODMuonRDO/NRPCRDOContainer.h"
#include "MuonCablingData/RpcCablingMap.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"



/////////////////////////////////////////////////////////////////////////////
namespace Muon{
    class NrpcDigitToNrpcRDO : public AthReentrantAlgorithm {
      public:
        using AthReentrantAlgorithm::AthReentrantAlgorithm;
        virtual ~NrpcDigitToNrpcRDO() = default;
        virtual StatusCode initialize() override final;
        virtual StatusCode execute(const EventContext& ctx) const override final;
    
      private:
        Gaudi::Property<std::vector<std::string>> m_convStat{this, "ConvertHitsFromStations",
                                                            {"BIS"}, "Only hits from these RPC stations are converted to RDOs" };
    
        SG::ReadCondHandleKey<RpcCablingMap> m_cablingKey{this, "CablingKey", "MuonNRPC_CablingMap", "Key of MuonNRPC_CablingMap"};

        SG::ReadHandleKey<RpcDigitContainer> m_digitContainerKey{this, "RpcDigitContainer", "RPC_DIGITS",
                                                                 "ReadHandleKey for Input RpcDigitContainer"};

        SG::WriteHandleKey<xAOD::NRPCRDOContainer> m_NrpcContainerKey{this, "NrpcRdoKey", "NRPCRDO", "WriteHandleKey for Output AOD::NRPCRDOContainer"};
    
        ServiceHandle<IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    
        std::unordered_set<int> m_selectedStations{};
    };
}
#endif
