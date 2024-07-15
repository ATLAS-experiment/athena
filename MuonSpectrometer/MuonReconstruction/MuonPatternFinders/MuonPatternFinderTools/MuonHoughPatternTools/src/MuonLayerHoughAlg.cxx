/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonLayerHoughAlg.h"

#include "StoreGate/ReadHandle.h"
#include "MuonPrepRawData/CscPrepDataCollection.h"
#include "MuonPrepRawData/MMPrepDataCollection.h"
#include "MuonPrepRawData/MdtPrepDataCollection.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonPrepRawData/RpcPrepDataCollection.h"
#include "MuonPrepRawData/TgcPrepDataCollection.h"
#include "MuonPrepRawData/sTgcPrepDataCollection.h"

MuonLayerHoughAlg::MuonLayerHoughAlg(const std::string& name, ISvcLocator* pSvcLocator) : 
        AthReentrantAlgorithm(name, pSvcLocator) {}


template <class T> StatusCode MuonLayerHoughAlg::retrieveContainer(const EventContext& ctx,
                                                                   const SG::ReadHandleKey<T>& key,
                                                                   const T*& contPtr) const {
    if (key.empty()) {
        ATH_MSG_DEBUG("No key of type "<<typeid(T).name()<<" has been set. Set to nullptr");
        contPtr = nullptr;
        return StatusCode::SUCCESS;
    }
    SG::ReadHandle<T> handle(key, ctx);
    ATH_CHECK(handle.isValid());
    contPtr = handle.cptr();
    return StatusCode::SUCCESS;
}

StatusCode MuonLayerHoughAlg::initialize() {
    if (m_layerTool.empty()) {
        ATH_MSG_ERROR("MuonLayerScanTool property is empty");
        return StatusCode::FAILURE;
    }
    ATH_CHECK(m_layerTool.retrieve());
    ATH_CHECK(m_printer.retrieve());
    ATH_CHECK(m_keyRpc.initialize());
    ATH_CHECK(m_keyMdt.initialize());
    ATH_CHECK(m_keyTgc.initialize());
    ATH_CHECK(m_keyCsc.initialize(!m_keyCsc.empty()));
    ATH_CHECK(m_keysTgc.initialize(!m_keysTgc.empty()));
    ATH_CHECK(m_keyMM.initialize(!m_keyMM.empty()));
    ATH_CHECK(m_combis.initialize());
    ATH_CHECK(m_houghDataPerSectorVecKey.initialize());

    return StatusCode::SUCCESS;
}

StatusCode MuonLayerHoughAlg::execute(const EventContext& ctx) const {
    const Muon::RpcPrepDataContainer* rpcPrds{nullptr};
    const Muon::MdtPrepDataContainer* mdtPrds{nullptr};
    const Muon::TgcPrepDataContainer* tgcPrds{nullptr};
    const Muon::CscPrepDataContainer* cscPrds{nullptr};
    const Muon::sTgcPrepDataContainer* stgcPrds{nullptr};
    const Muon::MMPrepDataContainer* mmPrds{nullptr};
    ATH_CHECK(retrieveContainer(ctx, m_keyMdt, mdtPrds));
    ATH_CHECK(retrieveContainer(ctx, m_keyRpc, rpcPrds));
    ATH_CHECK(retrieveContainer(ctx, m_keyTgc, tgcPrds));
    ATH_CHECK(retrieveContainer(ctx, m_keyCsc, cscPrds));
    ATH_CHECK(retrieveContainer(ctx, m_keysTgc, stgcPrds));
    ATH_CHECK(retrieveContainer(ctx, m_keyMM, mmPrds));
    
    
 
    ATH_MSG_VERBOSE("calling layer tool ");
    auto [combis, houghDataPerSectorVec] = m_layerTool->find(mdtPrds, cscPrds, tgcPrds, rpcPrds, stgcPrds, mmPrds, ctx);
    SG::WriteHandle<MuonPatternCombinationCollection> Handle(m_combis, ctx);
    if (combis) {
        if (Handle.record(std::move(combis)).isFailure()) {
            ATH_MSG_WARNING("Failed to record MuonPatternCombinationCollection at MuonLayerHoughCombis");
        } else {
            ATH_MSG_DEBUG("Recorded MuonPatternCombinationCollection at MuonLayerHoughCombis: size " << Handle->size());
            if (m_printSummary || msgLvl(MSG::DEBUG)) {
                ATH_MSG_INFO("Number of MuonPatternCombinations  " << Handle->size() << std::endl << m_printer->print(*Handle));
            }
        }
    } else {
        ATH_MSG_VERBOSE("CombinationCollection " << m_combis << " is empty, recording");
        ATH_CHECK(Handle.record(std::make_unique<MuonPatternCombinationCollection>()));
    }

    // write hough data to SG
    SG::WriteHandle<Muon::HoughDataPerSectorVec> handle{m_houghDataPerSectorVecKey, ctx};
    if (houghDataPerSectorVec) {
        ATH_CHECK(handle.record(std::move(houghDataPerSectorVec)));
    } else {
        ATH_MSG_VERBOSE("HoughDataPerSectorVec " << m_houghDataPerSectorVecKey << " is empty, recording");
        ATH_CHECK(handle.record(std::make_unique<Muon::HoughDataPerSectorVec>()));
    }
    return StatusCode::SUCCESS;
}  // execute
