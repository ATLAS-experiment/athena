/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TgcCondDbTestAlg.h"

// Constructor
TgcCondDbTestAlg::TgcCondDbTestAlg(const std::string& name, ISvcLocator* pSvcLocator) : AthAlgorithm(name, pSvcLocator) {}

// Destructor
TgcCondDbTestAlg::~TgcCondDbTestAlg() = default;

// Initialize
StatusCode TgcCondDbTestAlg::initialize() {
    ATH_MSG_INFO("Calling initialize");
    ATH_CHECK(m_readKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    return StatusCode::SUCCESS;
}

// Execute
StatusCode TgcCondDbTestAlg::execute(const EventContext& ctx) {  

    ATH_MSG_INFO("Calling execute");   
    SG::ReadCondHandle<TgcCondDbData> readHandle{m_readKey, ctx};
    if (!readHandle.isValid()) {
        ATH_MSG_ERROR("Null pointer to the read conditions object");
        return StatusCode::FAILURE;
    } 
    
    ATH_MSG_INFO("Loaded successfully the dead channel data");    
    const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};
    for (MuonIdHelper::const_id_iterator ch_itr = idHelper.module_begin(); 
                                         ch_itr !=idHelper.module_end(); ++ch_itr) {
        const Identifier& chambId{*ch_itr};
        ATH_MSG_DEBUG("Test dead gas gaps in chamber "<<m_idHelperSvc->toString(chambId));
        for (int gasGap = idHelper.gasGapMin(chambId); gasGap <= idHelper.gasGapMax(chambId); ++gasGap) {
            const Identifier gasGapId = idHelper.channelID(chambId, gasGap, false, 1);
            if (!readHandle->isGood(gasGapId)) {
                ATH_MSG_ALWAYS("Dead gas gap detected "<<m_idHelperSvc->toStringGasGap(gasGapId));
            }
        }
    }    
    return StatusCode::SUCCESS;
}
