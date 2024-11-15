/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
   MuonNRPC_CablingAlg reads raw condition data and writes derived condition
   data to the condition store
*/

#ifndef MUONNRPC_CABLING_MUONNRPC_CABLINGALG_H
#define MUONNRPC_CABLING_MUONNRPC_CABLINGALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "MuonCablingData/MuonNRPC_CablingMap.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"

#include "nlohmann/json.hpp"


class MuonNRPC_CablingAlg : public AthReentrantAlgorithm {
   public:
    MuonNRPC_CablingAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~MuonNRPC_CablingAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

    virtual bool isReEntrant() const override final { return false; }

    using CablingData = NrpcCablingCoolData;

private:
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    SG::WriteCondHandleKey<MuonNRPC_CablingMap> m_writeKey{this, "WriteKey", "MuonNRPC_CablingMap", "Key of output NRPC cabling map"};

    SG::ReadCondHandleKey<CondAttrListCollection> m_readKeyMap{
        this, "MapFolders", "/RPC/NCABLING/JSON", "Database folder for the RPC cabling"};
   
    Gaudi::Property<std::string> m_extJSONFile{
        this, "JSONFile", "",
        "Specify an external JSON file containing the cabling information."};

    StatusCode payLoadJSON(MuonNRPC_CablingMap& cabling_map,
                           const nlohmann::json& payload) const;
};

#endif
