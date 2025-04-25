/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MuonPRD_MultiTruthMaker_H
#define MuonPRD_MultiTruthMaker_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonPrepRawData/MMPrepDataContainer.h"
#include "MuonPrepRawData/MuonPrepDataCollection.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonPrepRawData/sTgcPrepDataContainer.h"
#include "MuonSimData/CscSimDataCollection.h"
#include "MuonSimData/MuonSimData.h"
#include "MuonSimData/MuonSimDataCollection.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "TrkTruthData/PRD_MultiTruthCollection.h"

class MuonPRD_MultiTruthMaker : public AthReentrantAlgorithm {
public:
    using AthReentrantAlgorithm::AthReentrantAlgorithm;
    virtual StatusCode initialize() override final;
    virtual StatusCode execute(const EventContext& ctx) const  override final;

private:
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    SG::ReadHandleKey<Muon::MdtPrepDataContainer> m_MdtPrdKey{this, "MdtPrdKey",  "MDT_DriftCircles" };
    SG::ReadHandleKey<Muon::CscPrepDataContainer> m_CscPrdKey{this, "CscPrdKey", "CSC_Clusters" };
    SG::ReadHandleKey<Muon::RpcPrepDataContainer> m_RpcPrdKey{this, "RpcPrdKey", "RPC_Measurements"};
    SG::ReadHandleKey<Muon::TgcPrepDataContainer> m_TgcPrdKey{this, "TgcPrdKey", "TGC_MeasurementsAllBCs" };
    SG::ReadHandleKey<Muon::sTgcPrepDataContainer> m_sTgcPrdKey{this, "sTgcPrdKey", "STGC_Measurements" };
    SG::ReadHandleKey<Muon::MMPrepDataContainer> m_MmPrdKey{this, "MmPrdKey", "MM_Measurements"};

    SG::ReadHandleKey<MuonSimDataCollection> m_MdtSDOKey{this, "MdtSdoKey", "MDT_SDO"};
    SG::ReadHandleKey<CscSimDataCollection>  m_CscSDOKey{this, "CscSdoKey", "CSC_SDO"};
    SG::ReadHandleKey<MuonSimDataCollection> m_RpcSDOKey{this, "RpdSdoKey", "RPC_SDO"};
    SG::ReadHandleKey<MuonSimDataCollection> m_TgcSDOKey{this, "TgcSdoKey", "TGC_SDO"};
    SG::ReadHandleKey<MuonSimDataCollection> m_sTgcSDOKey{this, "sTgcSdoKey", "sTGC_SDO"};
    SG::ReadHandleKey<MuonSimDataCollection> m_MmSDOKey{this, "MmSdoKey", "MM_SDO"};

    SG::WriteHandleKey<PRD_MultiTruthCollection> m_MdtTruthMapKey{this, "MdtTruthMapKey", "MDT_TruthMap"};
    SG::WriteHandleKey<PRD_MultiTruthCollection> m_RpcTruthMapKey{this, "RpcTruthMapKey", "RPC_TruthMap"};
    SG::WriteHandleKey<PRD_MultiTruthCollection> m_CscTruthMapKey{this, "CscTruthMapKey", "CSC_TruthMap"};
    SG::WriteHandleKey<PRD_MultiTruthCollection> m_TgcTruthMapKey{this, "TgcTruthMapKey", "TGC_TruthMap"};
    SG::WriteHandleKey<PRD_MultiTruthCollection> m_sTgcTruthMapKey{this, "sTgcTruthMapKey", "STGC_TruthMap"};
    SG::WriteHandleKey<PRD_MultiTruthCollection> m_MmTruthMapKey{this, "MmTruthMapKey", "MM_TruthMap" };

    //----------------------------------------------------------------
    template <class PrdType, class SimCollection>
    StatusCode buildPRD_Truth(const EventContext& ctx,
                              const SG::ReadHandleKey<Muon::MuonPrepDataContainerT<PrdType>>& prepDataKey, 
                              const SG::ReadHandleKey<SimCollection>& sdoKey,
                              const SG::WriteHandleKey<PRD_MultiTruthCollection>& outputKey) const;
};

#endif  // PRD_MULTITRUTHMAKER_H
