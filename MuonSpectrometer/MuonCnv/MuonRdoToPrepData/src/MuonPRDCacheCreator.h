/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#pragma once

#include "GaudiKernel/ServiceHandle.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonPrepRawData/MuonPrepDataCollection_Cache.h"
#include "MuonTrigCoinData/MuonTrigCoinData_Cache.h"
#include "ViewAlgs/IDCCacheCreatorBase.h"

// Class for setting up PRD cache containers
class MuonPRDCacheCreator : public IDCCacheCreatorBase {
public:
    /// Constructor
    using IDCCacheCreatorBase::IDCCacheCreatorBase;
    /// Destructor
    virtual ~MuonPRDCacheCreator() = default;

    /// Initialize the algorithm
    virtual StatusCode initialize() override;

    /// Execture the algorithm
    virtual StatusCode execute(const EventContext &ctx) const override;

protected:
    /// Write handle keys for the PRD caches
    SG::WriteHandleKey<CscPrepDataCollection_Cache> m_CscCacheKey{this, "CscCacheKey", ""};
    SG::WriteHandleKey<CscStripPrepDataCollection_Cache> m_CscStripCacheKey{this, "CscStripCacheKey", ""};
    SG::WriteHandleKey<MdtPrepDataCollection_Cache> m_MdtCacheKey{this, "MdtCacheKey", ""};
    SG::WriteHandleKey<RpcPrepDataCollection_Cache> m_RpcCacheKey{this, "RpcCacheKey", ""};
    SG::WriteHandleKey<TgcPrepDataCollection_Cache> m_TgcCacheKey{this, "TgcCacheKey", ""};
    SG::WriteHandleKey<sTgcPrepDataCollection_Cache> m_sTgcCacheKey{this, "sTgcCacheKey", ""};
    SG::WriteHandleKey<MMPrepDataCollection_Cache> m_MmCacheKey{this, "MmCacheKey", ""};
    SG::WriteHandleKey<RpcCoinDataCollection_Cache> m_RpcCoinCacheKey{this, "RpcCoinCacheKey", ""};
    SG::WriteHandleKeyArray<TgcCoinDataCollection_Cache> m_TgcCoinCacheKeys{this, "TgcCoinCacheKeys", {}};

    /// Name for the TGC Coin cache containers
    Gaudi::Property<std::string> m_tgcCoinCacheKeyStr{this, "TgcCoinCacheStr", "", "Prefix for names of TGC Coin Cache collections"};

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    Gaudi::Property<bool> m_disableWarning{this, "disableWarning", false};

};  // class MuonPRDCacheCreator
