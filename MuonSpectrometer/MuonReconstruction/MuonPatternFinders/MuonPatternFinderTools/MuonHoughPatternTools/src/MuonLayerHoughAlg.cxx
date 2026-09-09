/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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

StatusCode MuonLayerHoughAlg::initialize() {
    if (m_layerTool.empty()) {
        ATH_MSG_ERROR("MuonLayerScanTool property is empty");
        return StatusCode::FAILURE;
    }
    ATH_CHECK(m_layerTool.retrieve());
    ATH_CHECK(m_printer.retrieve());
    ATH_CHECK(m_keyRpc.initialize(!m_keyRpc.empty()));
    ATH_CHECK(m_keyMdt.initialize(!m_keyMdt.empty()));
    ATH_CHECK(m_keyTgc.initialize(!m_keyTgc.empty()));
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
    ATH_CHECK(SG::get( mdtPrds, m_keyMdt, ctx));
    ATH_CHECK(SG::get( rpcPrds, m_keyRpc, ctx));
    ATH_CHECK(SG::get( tgcPrds, m_keyTgc, ctx));
    ATH_CHECK(SG::get( cscPrds, m_keyCsc, ctx));
    ATH_CHECK(SG::get( stgcPrds, m_keysTgc, ctx));
    ATH_CHECK(SG::get( mmPrds, m_keyMM, ctx));
    
    
 
    ATH_MSG_VERBOSE("calling layer tool ");
    auto [combis, houghDataPerSectorVec] = m_layerTool->find(mdtPrds, cscPrds, tgcPrds, rpcPrds, stgcPrds, mmPrds, ctx);
    SG::WriteHandle Handle(m_combis, ctx);
    if (combis) {
        ATH_CHECK(Handle.record(std::move(combis)));
    } else {
        ATH_MSG_VERBOSE("CombinationCollection " << m_combis << " is empty, recording");
        ATH_CHECK(Handle.record(std::make_unique<MuonPatternCombinationCollection>()));
    }

    // write hough data to SG
    SG::WriteHandle handle{m_houghDataPerSectorVecKey, ctx};
    if (houghDataPerSectorVec) {
        ATH_CHECK(handle.record(std::move(houghDataPerSectorVec)));
    } else {
        ATH_MSG_VERBOSE("HoughDataPerSectorVec " << m_houghDataPerSectorVecKey << " is empty, recording");
        ATH_CHECK(handle.record(std::make_unique<Muon::HoughDataPerSectorVec>()));
    }
    return StatusCode::SUCCESS;
}  // execute
