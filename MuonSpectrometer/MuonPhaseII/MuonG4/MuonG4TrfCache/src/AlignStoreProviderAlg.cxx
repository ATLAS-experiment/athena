/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "AlignStoreProviderAlg.h"
#include "StoreGate/WriteHandle.h"

namespace MuonG4{
    StatusCode AlignStoreProviderAlg::initialize() {
        ATH_CHECK(m_readKey.initialize());
        ATH_CHECK(m_writeKey.initialize());
        return StatusCode::SUCCESS;
    }
    StatusCode AlignStoreProviderAlg::execute(const EventContext& ctx) const {
        const ActsTrk::DetectorAlignStore* condStore{nullptr};
        ATH_CHECK(SG::get(condStore, m_readKey, ctx));
        SG::WriteHandle writeHandle{m_writeKey, ctx};
        ATH_CHECK(writeHandle.record(std::make_unique<ActsTrk::DetectorAlignStore>(*condStore)));
        return StatusCode::SUCCESS;
    }
}
