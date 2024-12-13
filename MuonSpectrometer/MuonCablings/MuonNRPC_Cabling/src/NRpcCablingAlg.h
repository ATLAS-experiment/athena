/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
   NRpcCablingAlg reads raw condition data and writes derived condition
   data to the condition store
*/

#ifndef MUONNRPC_CABLING_MUONNRPC_CABLINGALG_H
#define MUONNRPC_CABLING_MUONNRPC_CABLINGALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"
#include "MuonCablingData/RpcCablingMap.h"

#include "nlohmann/json.hpp"

namespace Muon{
  class NRpcCablingAlg : public AthReentrantAlgorithm {
  public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    virtual ~NRpcCablingAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext &ctx) const override;
    virtual bool isReEntrant() const override final { return false; }

  private:
    ServiceHandle<IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    SG::WriteCondHandleKey<RpcCablingMap> m_writeKey{this, "WriteKey", "MuonNRPC_CablingMap", "Key of output NRPC cabling map"};

    SG::ReadCondHandleKey<CondAttrListCollection> m_readKeyMap{this, "MapFolders", "/RPC/NCABLING/JSON",
                                                               "Database folder for the RPC cabling"};

    Gaudi::Property<std::string> m_extJSONFile{this, "JSONFile", "",
                                               "Specify an external JSON file containing the cabling information."};

    /** @brief Load the payload of the actual cabling map from offline <-> online  */
    StatusCode parsePayload(RpcCablingMap &cablingMap,
                            const nlohmann::json &payload) const;
  };

}
#endif
