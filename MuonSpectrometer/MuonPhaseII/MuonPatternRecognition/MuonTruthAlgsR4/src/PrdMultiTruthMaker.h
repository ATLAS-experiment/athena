/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRUTHALGSR4_PRDMULTITRUTHMAKER_H
#define MUONTRUTHALGSR4_PRDMULTITRUTHMAKER_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/ReadDecorHandleKeyArray.h"
#include "StoreGate/WriteHandleKey.h"

#include "TrkTruthData/PRD_MultiTruthCollection.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

namespace MuonR4{
    class PrdMultiTruthMaker : public AthReentrantAlgorithm{
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;

            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext& ctx) const override final;
        private:
            
            using xAODPrdCont_t = xAOD::UncalibratedMeasurementContainer;
            using xAODPrdKey_t = SG::ReadHandleKey<xAODPrdCont_t>;
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

            
            template<typename PrdType>
                StatusCode buildMultiCollection(const EventContext& ctx,
                                                const SG::ReadHandleKey<Muon::MuonPrepDataContainerT<PrdType>>& prdKey,
                                                const SG::WriteHandleKey<PRD_MultiTruthCollection>& writeKey,
                                                const std::vector<const xAODPrdCont_t*>& measContainers) const;
            
            Gaudi::Property<std::string> m_simLinkDecor{this, "SimHitLinkDecoration", "simHitLink"};

            SG::ReadHandleKeyArray<xAODPrdCont_t> m_xAODPrdKeys{this, "PrdContainer", {}};
            SG::ReadDecorHandleKeyArray<xAODPrdCont_t> m_simHitDecorKeys{this, "TruthSimDecorKeys", {}};
            
            SG::ReadHandleKey<Muon::MdtPrepDataContainer> m_MdtPrdKey{this, "MdtPrdKey",  "MDT_DriftCircles" };
            SG::ReadHandleKey<Muon::RpcPrepDataContainer> m_RpcPrdKey{this, "RpcPrdKey", "RPC_Measurements"};
            SG::ReadHandleKey<Muon::TgcPrepDataContainer> m_TgcPrdKey{this, "TgcPrdKey", "TGC_Measurements" };
            SG::ReadHandleKey<Muon::sTgcPrepDataContainer> m_sTgcPrdKey{this, "sTgcPrdKey", "STGC_Measurements" };
            SG::ReadHandleKey<Muon::MMPrepDataContainer> m_MmPrdKey{this, "MmPrdKey", "MM_Measurements"};

            SG::WriteHandleKey<PRD_MultiTruthCollection> m_MdtTruthMapKey{this, "MdtTruthMapKey", "MDT_TruthMap"};
            SG::WriteHandleKey<PRD_MultiTruthCollection> m_RpcTruthMapKey{this, "RpcTruthMapKey", "RPC_TruthMap"};
            SG::WriteHandleKey<PRD_MultiTruthCollection> m_TgcTruthMapKey{this, "TgcTruthMapKey", "TGC_TruthMap"};
            SG::WriteHandleKey<PRD_MultiTruthCollection> m_sTgcTruthMapKey{this, "sTgcTruthMapKey", "STGC_TruthMap"};
            SG::WriteHandleKey<PRD_MultiTruthCollection> m_MmTruthMapKey{this, "MmTruthMapKey", "MM_TruthMap" };

    };
}
#endif