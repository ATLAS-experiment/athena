/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONLAYERHOUGHALG_H
#define MUONLAYERHOUGHALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonRecHelperTools/MuonEDMPrinterTool.h"
#include "MuonRecToolInterfaces/HoughDataPerSec.h"
#include "MuonRecToolInterfaces/IMuonHoughPatternFinderTool.h"

class MuonLayerHoughAlg : public AthReentrantAlgorithm {
public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    virtual ~MuonLayerHoughAlg() = default;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

private:
    SG::ReadHandleKey<Muon::TgcPrepDataContainer> m_keyTgc{this, "TgcPrepDataContainer", "TGC_Measurements"};
    SG::ReadHandleKey<Muon::RpcPrepDataContainer> m_keyRpc{this, "RpcPrepDataContainer", "RPC_Measurements"};
    SG::ReadHandleKey<Muon::CscPrepDataContainer> m_keyCsc{this, "CscPrepDataContainer", "CSC_Clusters"};
    SG::ReadHandleKey<Muon::MdtPrepDataContainer> m_keyMdt{this, "MdtPrepDataContainer", "MDT_DriftCircles"};
    SG::ReadHandleKey<Muon::sTgcPrepDataContainer> m_keysTgc{this, "sTgcPrepDataContainer", "STGC_Measurements"};
    SG::ReadHandleKey<Muon::MMPrepDataContainer> m_keyMM{this, "MMPrepDataContainer", "MM_Measurements"};

    SG::WriteHandleKey<MuonPatternCombinationCollection> m_combis{this, "MuonPatternCombinationCollection", "MuonLayerHoughCombis"};
    SG::WriteHandleKey<Muon::HoughDataPerSectorVec> m_houghDataPerSectorVecKey{this, "Key_MuonLayerHoughToolHoughDataPerSectorVec",
                                                                               "HoughDataPerSectorVec", "HoughDataPerSectorVec key"};
    PublicToolHandle<Muon::MuonEDMPrinterTool> m_printer{this, "printerTool", "Muon::MuonEDMPrinterTool/MuonEDMPrinterTool"};
    ToolHandle<Muon::IMuonHoughPatternFinderTool> m_layerTool{this, "MuonLayerScanTool", "Muon::MuonLayerHoughTool/MuonLayerHoughTool"};
    Gaudi::Property<bool> m_printSummary{this, "PrintSummary", false};
};



#endif
