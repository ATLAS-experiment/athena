/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDALG_RPCCONDDBALG_H
#define MUONCONDALG_RPCCONDDBALG_H

// STL includes
#include <zlib.h>

#include <sstream>
#include <string>
#include <vector>

// Gaudi includes
#include "GaudiKernel/ServiceHandle.h"

// Athena includes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "MuonCondData/RpcCondDbData.h"
#include "CxxUtils/StringUtils.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"

class RpcCondDbAlg : public AthReentrantAlgorithm {
public:
    RpcCondDbAlg(const std::string &name, ISvcLocator *svc);
    virtual ~RpcCondDbAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext &) const override;
    virtual bool isReEntrant() const override { return false; }

private:
    template <class WriteCont>
    StatusCode addDependency(const EventContext& ctx,
                             const SG::ReadCondHandleKey<CondAttrListCollection>& key,
                             SG::WriteCondHandle<WriteCont>& writeHandle) const;

    StatusCode loadMcElementStatus(const EventContext & ctx, RpcCondDbData& condData) const;


    Gaudi::Property<bool> m_isData{this, "isData", false};

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    SG::WriteCondHandleKey<RpcCondDbData> m_writeKey{this, "WriteKey", "RpcCondDbData", "Key of output RPC condition data"};

    SG::ReadCondHandleKey<CondAttrListCollection> m_readKey_folder_mc_deadElements{this, "ReadKey_MC_DE", "/RPC/DQMF/ELEMENT_STATUS",
                                                                                   "Key of input RPC condition data for MC dead elements"};

};

#endif
