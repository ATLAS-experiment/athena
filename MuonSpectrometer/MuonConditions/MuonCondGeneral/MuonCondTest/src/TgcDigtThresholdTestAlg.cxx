/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TgcDigtThresholdTestAlg.h"

// Constructor
TgcDigtThresholdTestAlg::TgcDigtThresholdTestAlg(const std::string& name, ISvcLocator* pSvcLocator) : AthAlgorithm(name, pSvcLocator) {}

// Destructor
TgcDigtThresholdTestAlg::~TgcDigtThresholdTestAlg() = default;

// Initialize
StatusCode TgcDigtThresholdTestAlg::initialize() {
    ATH_MSG_INFO("Calling initialize");
    ATH_CHECK(m_readKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    return StatusCode::SUCCESS;
}

// Execute
StatusCode TgcDigtThresholdTestAlg::execute(const EventContext& ctx) {  

    ATH_MSG_INFO("Calling execute");   
    SG::ReadCondHandle<TgcDigitThresholdData> readHandle{m_readKey, ctx};
    if (!readHandle.isValid()) {
        ATH_MSG_ERROR("Null pointer to the read conditions object");
        return StatusCode::FAILURE;
    } 
    
    ATH_MSG_INFO("Loaded successfully the dead channel data");    
    const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};
    std::unordered_set<Identifier> uniqueThreshData{};
    for (MuonIdHelper::const_id_iterator ch_itr = idHelper.module_begin(); 
                                         ch_itr !=idHelper.module_end(); ++ch_itr) {
        const Identifier& chambId{*ch_itr};
        ATH_MSG_DEBUG("Test dead gas gaps in chamber "<<m_idHelperSvc->toString(chambId));
        for (int gasGap = idHelper.gasGapMin(chambId); gasGap <= idHelper.gasGapMax(chambId); ++gasGap) {
            for (bool isStrip : {false, true}) {
                const Identifier gasGapId = idHelper.channelID(chambId, gasGap, isStrip, 1);
                const double threshold = readHandle->getThreshold(gasGapId);
                if (threshold < 0) {
                    ATH_MSG_WARNING("No energy threshold given for "<<m_idHelperSvc->toString(gasGapId));
                }
                if (!uniqueThreshData.insert(idHelper.channelID(idHelper.stationName(chambId),
                                                                std::abs(idHelper.stationEta(chambId)),
                                                                1, gasGap, isStrip,1)).second) continue;
                ATH_MSG_ALWAYS("Chamber "<<m_idHelperSvc->toString(gasGapId)<<" has an energy thresold of "<<threshold);
            }
        }
    }    
    return StatusCode::SUCCESS;
}
