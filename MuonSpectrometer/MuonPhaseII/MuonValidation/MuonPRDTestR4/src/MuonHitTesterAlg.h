/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONPRDTESTR4_MUONHITTESTERALH_H
#define MUONPRDTESTR4_MUONHITTESTERALH_H

#include "AthenaBaseComps/AthHistogramAlgorithm.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonTesterTree/MuonTesterTree.h"

namespace MuonValR4 {
    class MuonHitTesterAlg: public AthHistogramAlgorithm{
        public:
            MuonHitTesterAlg(const std::string& alg_name,
                             ISvcLocator* pSvcLocator);
            
            StatusCode initialize() override final;
            StatusCode finalize() override final;
            StatusCode execute() override final;
            unsigned int cardinality() const override final { return 1; }
        
        private:
            MuonVal::MuonTesterTree m_tree{"MuonHitTest", "MuonR4HitTest"};

            Gaudi::Property<bool> m_isMC{this, "isMC", true};

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
            Gaudi::Property<std::string> m_sTgcDigitKey{this, "sTgcDigitKey", "STGC_DIGITS"};  

    };
}
#endif