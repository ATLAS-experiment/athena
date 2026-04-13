/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonPRDCacheCreator.h"

#include "AthViews/View.h"
#include "MuonDigitContainer/TgcDigit.h"


StatusCode MuonPRDCacheCreator::initialize() {
    ATH_CHECK(m_CscCacheKey.initialize(m_idHelperSvc->hasCSC()));
    ATH_CHECK(m_CscStripCacheKey.initialize(m_idHelperSvc->hasCSC()));
    ATH_CHECK(m_MdtCacheKey.initialize(m_idHelperSvc->hasMDT()));
    ATH_CHECK(m_RpcCacheKey.initialize(m_idHelperSvc->hasRPC()));
    ATH_CHECK(m_sTgcCacheKey.initialize(m_idHelperSvc->hasSTGC()));
    ATH_CHECK(m_MmCacheKey.initialize(m_idHelperSvc->hasMM()));
    ATH_CHECK(m_RpcCoinCacheKey.initialize(SG::AllowEmpty));
    
    const bool doTgcCoinCache = not m_tgcCoinCacheKeyStr.empty();
    if (doTgcCoinCache) {
        m_TgcCoinCacheKeys.resize(TgcDigit::BC_NEXTNEXT);
        for (int ibc = 0; ibc < TgcDigit::BC_NEXTNEXT; ibc++) {
            const int bcTag = ibc + 1;
            std::ostringstream location;
            location << m_tgcCoinCacheKeyStr.value() << (bcTag == TgcDigit::BC_PREVIOUS ? "PriorBC" : "")
                     << (bcTag == TgcDigit::BC_NEXT ? "NextBC" : "") << (bcTag == TgcDigit::BC_NEXTNEXT ? "NextNextBC" : "");
            m_TgcCoinCacheKeys.at(ibc) = location.str();
            ATH_MSG_INFO("Setting next TGC Coin Cache to " << location.str());
        }  // BC loop
    }
    ATH_CHECK(m_TgcCacheKey.initialize(SG::AllowEmpty));
    ATH_CHECK(m_TgcCoinCacheKeys.initialize(doTgcCoinCache));

    ATH_CHECK(m_idHelperSvc.retrieve());
    if (m_disableWarning) m_disableWarningCheck.test_and_set(std::memory_order_relaxed);
    return StatusCode::SUCCESS;
}

StatusCode MuonPRDCacheCreator::execute(const EventContext& ctx) const {
    ATH_CHECK(checkInsideViewOnce(ctx));

    // Create all the cache containers (if the tools are available)
    // CSC
    if (m_idHelperSvc->hasCSC()) {
        ATH_CHECK(createContainer(m_CscCacheKey, m_idHelperSvc->cscIdHelper().module_hash_max(), ctx));
        ATH_CHECK(createContainer(m_CscStripCacheKey, m_idHelperSvc->cscIdHelper().module_hash_max(), ctx));
    }

    // MDT
    if (m_idHelperSvc->hasMDT()) {
        ATH_CHECK(createContainer(m_MdtCacheKey, m_idHelperSvc->mdtIdHelper().detectorElement_hash_max(), ctx));
    }

    // RPC
    if (m_idHelperSvc->hasRPC()) {
        ATH_CHECK(createContainer(m_RpcCacheKey, m_idHelperSvc->rpcIdHelper().module_hash_max(), ctx));
        ATH_CHECK(createContainer(m_RpcCoinCacheKey, m_idHelperSvc->rpcIdHelper().module_hash_max(), ctx));
    }

    // TGC
    if (m_idHelperSvc->hasTGC()) {

        ATH_CHECK(createContainer(m_TgcCacheKey, m_idHelperSvc->tgcIdHelper().module_hash_max(), ctx));
        for (const auto& tgcCoinCacheKey : m_TgcCoinCacheKeys) {
            ATH_CHECK(createContainer(tgcCoinCacheKey, m_idHelperSvc->tgcIdHelper().module_hash_max(), ctx));
        }
    }

    // NSW STGC
    if (m_idHelperSvc->hasSTGC()) { 
        ATH_CHECK(createContainer(m_sTgcCacheKey, m_idHelperSvc->stgcIdHelper().module_hash_max(), ctx)); 
    }

    // NSW MM
    if (m_idHelperSvc->hasMM()) { 
        ATH_CHECK(createContainer(m_MmCacheKey, m_idHelperSvc->mmIdHelper().module_hash_max(), ctx)); 
    }

    return StatusCode::SUCCESS;
}
