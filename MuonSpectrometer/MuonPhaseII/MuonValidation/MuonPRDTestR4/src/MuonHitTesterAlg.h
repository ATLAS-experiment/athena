/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONPRDTESTR4_MUONHITTESTERALH_H
#define MUONPRDTESTR4_MUONHITTESTERALH_H

#include "AthenaBaseComps/AthHistogramAlgorithm.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonTesterTree/MuonTesterTree.h"
#include "xAODEventInfo/EventInfo.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"

namespace MuonPRDTest {
    class ParticleVariables;
    class SegmentVariables;
}

namespace MuonValR4 {
    class MuonHitTesterAlg: public AthHistogramAlgorithm{
        public:
            using AthHistogramAlgorithm::AthHistogramAlgorithm;
            virtual ~MuonHitTesterAlg();
            StatusCode initialize() override final;
            StatusCode finalize() override final;
            StatusCode execute() override final;
            unsigned int cardinality() const override final { return 1; }
        
        private:
            MuonVal::MuonTesterTree m_tree{"MuonHitTest", "MuonR4HitTest"};

            Gaudi::Property<bool> m_isMC{this, "isMC", true};

            SG::ReadHandleKey<xAOD::EventInfo> m_evtKey{this, "EvtInfoKey", "EventInfo"};
            Gaudi::Property<bool> m_writePileUp{this, "dumpPileUp", false};

            /**
             *  @brief Toggle whether the simHit collection of each sub detector shall
             *         be written to disk
             */
            StatusCode setupSimHits();
            Gaudi::Property<bool> m_writeSimHits{this, "dumpSimHits", true, 
                                                "Master switch toggling the sim hits"};
            Gaudi::Property<bool> m_writeMdtSim{this, "dumpMdtSimHits", true};
            Gaudi::Property<bool> m_writeRpcSim{this, "dumpRpcSimHits", true};
            Gaudi::Property<bool> m_writeTgcSim{this, "dumpTgcSimHits", true};
            Gaudi::Property<bool> m_writesTgcSim{this, "dumpStgcSimHits", true};
            Gaudi::Property<bool> m_writeMmSim{this, "dumpMmSimHits", true};

            /**
             *  @brief Container keys of the particular sim hit collections
             */
            Gaudi::Property<std::string> m_mdtSimHitKey{this, "MdtSimHitKey", "xMdtSimHits"};
            Gaudi::Property<std::string> m_rpcSimHitKey{this, "RpcSimHitKey", "xRpcSimHits"};
            Gaudi::Property<std::string> m_tgcSimHitKey{this, "TgcSimHitKey", "xTgcSimHits"};
            Gaudi::Property<std::string> m_mmSimHitKey{this, "MmSimHitKey", "xMmSimHits"};
            Gaudi::Property<std::string> m_sTgcSimHitKey{this, "sTgcSimHitKey", "xStgcSimHits"};
            /**
             *  @brief Toggle whether the digit collections shall be tested
             */
            StatusCode setupDigits();
            Gaudi::Property<bool> m_writeDigits{this, "dumpDigits", false,
                                                "Master switch toggling the digit collections"};
            
            Gaudi::Property<bool> m_writeMdtDigits{this, "dumpMdtDigits", true};
            Gaudi::Property<bool> m_writeRpcDigits{this, "dumpRpcDigits", true};
            Gaudi::Property<bool> m_writeTgcDigits{this, "dumpTgcDigits", true};
            Gaudi::Property<bool> m_writesTgcDigits{this, "dumpStgcDigits", true};
            Gaudi::Property<bool> m_writeMmDigits{this, "dumpMmDigits", true};

            /**
             *  @brief Container keys of the particular digit collections
             */
            Gaudi::Property<std::string> m_mdtDigitKey{this, "MdtDigitKey", "MDT_DIGITS"};
            Gaudi::Property<std::string> m_rpcDigitKey{this, "RpcDigitKey", "RPC_DIGITS"};
            Gaudi::Property<std::string> m_tgcDigitKey{this, "TgcDigitKey", "TGC_DIGITS"};
            Gaudi::Property<std::string> m_mmDigitKey{this, "MmDigitKey", "MM_DIGITS"};
            Gaudi::Property<std::string> m_sTgcDigitKey{this, "sTgcDigitKey", "sTGC_DIGITS"};
            /**
             *  @brief Toggle whether the uncalibrated measurement collections shall be tested
             * */
            StatusCode setupPrds();
            Gaudi::Property<bool> m_writePrds{this, "dumpPrds", false,
                                              "Master switch toggling the prd collection dump"};
   
            Gaudi::Property<bool> m_writeMdtPrds{this, "dumpMdtPrds", true};
            Gaudi::Property<bool> m_writeRpcPrds{this, "dumpRpcPrds", true};
            Gaudi::Property<bool> m_writeTgcPrds{this, "dumpTgcPrds", true};
            Gaudi::Property<bool> m_writeMmPrds{this, "dumpMmPrds", true};
            /**
             *  @brief Prd collection names
             */
            Gaudi::Property<std::string> m_mdtPrdKey{this, "MdtPrdKey", "xMdtDriftCircles"};
            Gaudi::Property<std::string> m_mdtTwinPrdKey{this, "MdtTwinPrdKey", "xMdtTwinDriftCircles"};
            Gaudi::Property<std::string> m_rpcPrdKey{this, "RpcPrdKey", "xRpcMeasurements"};
            Gaudi::Property<std::string> m_tgcPrdKey{this, "TgcPrdKey", "xTgcStrips"};
            Gaudi::Property<std::string> m_mmPrdKey{this, "MmPrdKey", "xAODMMClusters"};

            
            StatusCode setupTruth();
            /** @brief Flag toggling whether the truth particle container shall be written  */
            Gaudi::Property<bool> m_writeTruthMuon{this, "dumpTruthMuon", false};
            /** @brief Name of the truth particle container */
            Gaudi::Property<std::string> m_truthMuonCont{this, "TruthMuons", "MuonTruthParticles"};
            /** @brief Flag toggling whether the truth segment container shall be written */
            Gaudi::Property<bool> m_writeTruthSeg{this, "dumpTruthSegment", false};
            /** @brief Name of the truth segment container */
            Gaudi::Property<std::string> m_truthSegCont{this, "TruthSegments", "MuonTruthSegments"};

            std::shared_ptr<MuonPRDTest::ParticleVariables> m_truthParts;
            std::shared_ptr<MuonPRDTest::SegmentVariables> m_truthSegs;

            const MuonGMR4::MuonDetectorManager* m_detMgr{};
            
        };
}
#endif