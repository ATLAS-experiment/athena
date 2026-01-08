/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDALG_RPCCONDDBALG_H
#define MUONCONDALG_RPCCONDDBALG_H

// Athena includes
#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "MuonCondData/RpcCondDbData.h"
#include "CxxUtils/StringUtils.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"

namespace Muon{
class RpcCondDbAlg : public AthCondAlgorithm {
public:
    using AthCondAlgorithm::AthCondAlgorithm;
    virtual ~RpcCondDbAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext &) const override;

private:
    template <class WriteCont>
    StatusCode addCondDependency(const EventContext& ctx,
                                 const SG::ReadCondHandleKey<CondAttrListCollection>& key,
                                 SG::WriteCondHandle<WriteCont>& writeHandle) const;

    StatusCode loadMcElementStatus(const EventContext & ctx, RpcCondDbData& condData) const;


    Gaudi::Property<bool> m_isData{this, "isData", false};

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    SG::WriteCondHandleKey<RpcCondDbData> m_writeKey{this, "WriteKey", "RpcCondDbData", "Key of output RPC condition data"};

    SG::ReadCondHandleKey<CondAttrListCollection> m_readKey_folder_mc_deadElements{this, "ReadKey_MC_DE", "/RPC/DQMF/ELEMENT_STATUS",
                                                                                   "Key of input RPC condition data for MC dead elements"};

};
}
#endif
