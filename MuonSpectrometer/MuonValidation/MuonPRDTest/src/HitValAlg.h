/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONPRDTEST_MuonHitValAlg_H
#define MUONPRDTEST_MuonHitValAlg_H

#include "AthenaBaseComps/AthHistogramAlgorithm.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonTesterTree/MuonTesterTree.h"
#include "MuonTGC_Cabling/MuonTGC_CablingSvc.h"
#include "MuonCSC_CnvTools/ICSC_RDO_Decoder.h"
#include "xAODEventInfo/EventInfo.h"

namespace MuonVal{
class HitValAlg : public AthHistogramAlgorithm {
public:

    using AthHistogramAlgorithm::AthHistogramAlgorithm;

    StatusCode initialize() override;
    StatusCode finalize() override;
    StatusCode execute() override;
    unsigned int cardinality() const override final { return 1; }

private:
    
    StatusCode setupSimHits();
    StatusCode setupSDOs();
    StatusCode setupDigits();
    StatusCode setupRDOs();
    StatusCode setupPRDs();
    
    MuonVal::MuonTesterTree m_tree{"MuonHitValidTree", "MUONHITVALIDSTREAM"};

    ServiceHandle<MuonTGC_CablingSvc> m_tgcCabling{this, "TGCCablingSvc", "MuonTGC_CablingSvc"};

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    PublicToolHandle<Muon::ICSC_RDO_Decoder> m_csc_decoder{this, "CscRDODecoder", "Muon::CscRDO_Decoder/CSC_RDODecoder"};

    Gaudi::Property<bool> m_isData{this, "isData", false};        // if false use MuonDetectorManager from detector store everywhere
    Gaudi::Property<bool> m_doTruth{this, "doTruth", false};      // switch on the output of the MC truth
    Gaudi::Property<bool> m_doMuEntry{this, "doMuEntry", false};  // switch on the output of the Muon Entry Layer
  
    Gaudi::Property<bool> m_doSimHits{this, "doSimHits", false}; /// Switch to toggle the dumping of sim hits in general
    Gaudi::Property<bool> m_doSDO{this, "doSDOs", false};        /// Switch to toggle the dumping of SDOs in general
    Gaudi::Property<bool> m_doDigits{this, "doDigits", false};   /// Switch to toggle the dumping of Digits in general
    Gaudi::Property<bool> m_doRDOs{this, "doRDOs", false};       /// Switch to toggle the dumping of RDOs in general
    Gaudi::Property<bool> m_doPRDs{this, "doPRDs", false};       /// Switch to toggle the dumping of PRDs in general
    
    Gaudi::Property<bool> m_doSTGCHit{this, "doSTGCHit", false};     // switch on the output of the Small TGC simulated hits
    Gaudi::Property<bool> m_doSTGCSDO{this, "doSTGCSDO", false};     // switch on the output of the sTGC SDO
    Gaudi::Property<bool> m_doSTGCDigit{this, "doSTGCDigit", false}; // swicth on the output of the Small TGC digit
    Gaudi::Property<bool> m_doSTGCRDO{this, "doSTGCRDO", false};     // switch on the output of the Small TGC RDO
    Gaudi::Property<bool> m_doSTGCPRD{this, "doSTGCPRD", false};     // swicth on the output of the Small TGC prepdata

    Gaudi::Property<bool> m_doMMHit{this, "doMMHit", false};                  // switch on the output of the MicroMegas simulated hits
    Gaudi::Property<bool> m_doMMSDO{this, "doMMSDO", false};                  // switch on the output of the MicroMegas SDO
    Gaudi::Property<bool> m_doMMDigit{this, "doMMDigit", false};              // switch on the output of the MicroMegas digitization
    Gaudi::Property<bool> m_doMMRDO{this, "doMMRDO", false};                  // switch on the output of the MicroMegas RDO
    Gaudi::Property<bool> m_doMMPRD{this, "doMMPRD", false};                  // switch on the output of the MicroMegas prepdata

    Gaudi::Property<bool> m_doCSCHit{this, "doCSCHit", false};                // switch on the output of the CSC simulated hits
    Gaudi::Property<bool> m_doCSCSDO{this, "doCSCSDO", false};                // switch on the output of the CSC SDO
    Gaudi::Property<bool> m_doCSCDigit{this, "doCSCDigit", false};            // switch on the output of the CSC digitization
    Gaudi::Property<bool> m_doCSCRDO{this, "doCSCRDO", false};                // switch on the output of the CSC RDO
    Gaudi::Property<bool> m_doCSCPRD{this, "doCSCPRD", false};                // switch on the output of the CSC prepdata

    Gaudi::Property<bool> m_doMDTHit{this, "doMDTHit", false};                // switch on the output of the MDT simulated hits
    Gaudi::Property<bool> m_doMDTSDO{this, "doMDTSDO", false};                // switch on the output of the MDT SDO
    Gaudi::Property<bool> m_doMDTDigit{this, "doMDTDigit", false};            // switch on the output of the MDT digitization
    Gaudi::Property<bool> m_doMDTPRD{this, "doMDTPRD", false};                // switch on the output of the MDT prepdata

    Gaudi::Property<bool> m_doRPCHit{this, "doRPCHit", false};                // switch on the output of the RPC simulated hits
    Gaudi::Property<bool> m_doRPCSDO{this, "doRPCSDO", false};                // switch on the output of the RPC SDO
    Gaudi::Property<bool> m_doRPCDigit{this, "doRPCDigit", false};            // switch on the output of the RPC digitization
    Gaudi::Property<bool> m_doRPCPRD{this, "doRPCPRD", false};                // switch on the output of the RPC prepdata

    Gaudi::Property<bool> m_doTGCHit{this, "doTGCHit", false};                // switch on the output of the TGC simulated hits
    Gaudi::Property<bool> m_doTGCSDO{this, "doTGCSDO", false};                // switch on the output of the TGC SDO
    Gaudi::Property<bool> m_doTGCDigit{this, "doTGCDigit", false};            // switch on the output of the TGC digitization
    Gaudi::Property<bool> m_doTGCRDO{this, "doTGCRDO", false};                // switch on the output of the TGC RDO
    Gaudi::Property<bool> m_doTGCPRD{this, "doTGCPRD", false};                // switch on the output of the TGC prepdata


    SG::ReadHandleKey<xAOD::EventInfo> m_evtInfo{this, "EvtInfo", "EventInfo"};
    Gaudi::Property<std::string> m_Truth_ContainerName{this, "Truth_ContainerName", "TruthEvent"};
    Gaudi::Property<std::string> m_MuEntry_ContainerName{this, "MuonEntryLayer_ContainerName", "MuonEntryLayer"};

    Gaudi::Property<std::string> m_sTgcSimKey{this, "sTgcSimKey", "sTGC_Hits"};
    Gaudi::Property<std::string> m_sTgcSdoKey{this, "sTgcSdoKey", "sTGC_SDO"};
    Gaudi::Property<std::string> m_sTgcDigitKey{this, "sTgcDigitKey", "sTGC_DIGITS"};
    Gaudi::Property<std::string> m_sTgcRdoKey{this, "sTgcRdoKey", "sTGCRDO"};
    Gaudi::Property<std::string> m_sTgcPRDKey{this, "sTgcPrdKey", "STGC_Measurements"};

    Gaudi::Property<std::string> m_MmSimKey{this, "MmSimKey", "MM_Hits"};
    Gaudi::Property<std::string> m_MmSdoKey{this, "MmSdoKey", "MM_SDO"};
    Gaudi::Property<std::string> m_MmDigitKey{this, "MmDigitKey", "MM_DIGITS"};
    Gaudi::Property<std::string> m_MmRdoKey{this, "MmRdoKey", "MMRDO"};
    Gaudi::Property<std::string> m_MmPrdKey{this, "MmPrdKey", "MM_Measurements"};

    Gaudi::Property<std::string> m_CSC_SimContainerName{this, "CSC_SimContainerName", "CSC_Hits"};
    Gaudi::Property<std::string> m_CSC_SDOContainerName{this, "CSC_SDOContainerName", "CSC_SDO"};
    Gaudi::Property<std::string> m_CSC_DigitContainerName{this, "CSC_DigitContainerName", "CSC_DIGITS"};
    Gaudi::Property<std::string> m_CSC_RDOContainerName{this, "CSC_RDOContainerName", "CSCRDO"};
    Gaudi::Property<std::string> m_CSC_PRDContainerName{this, "CSC_PRDContainerName", "CSC_Clusters"};

    Gaudi::Property<std::string> m_MdtSimHitKey{this, "MdtSimKey", "MDT_Hits"};
    Gaudi::Property<std::string> m_MdtSdoKey{this, "MdtSdoKey", "MDT_SDO"};
    Gaudi::Property<std::string> m_MdtDigitKey{this, "MdtDigitKey", "MDT_DIGITS"};
    Gaudi::Property<std::string> m_MdtPrdKey{this, "MdtPrdKey", "MDT_DriftCircles"};

    Gaudi::Property<std::string> m_RpcSimHitKey{this, "RpcSimKey", "RPC_Hits"};
    Gaudi::Property<std::string> m_RpcSdoKey{this, "RpcSdoKey", "RPC_SDO"};
    Gaudi::Property<std::string> m_RpcDigitKey{this, "RpcDigitKey", "RPC_DIGITS"};
    Gaudi::Property<std::string> m_RpcPrdKey{this, "RpcPrdKey", "RPC_Measurements"};

    Gaudi::Property<std::string> m_TgcSimHitKey{this, "TgcSimKey", "TGC_Hits"};
    Gaudi::Property<std::string> m_TgcSdoKey{this, "TgcSdoKey", "TGC_SDO"};
    Gaudi::Property<std::string> m_TgcDigitKey{this, "TgcDigitKey", "TGC_DIGITS"};
    Gaudi::Property<std::string> m_TgcRdoKey{this, "TgcRdoKey", "TGCRDO"};
    Gaudi::Property<std::string> m_TgcPrdKey{this, "TgcPrdKey", "TGC_Measurements"};

    // Matching algorithm

    Gaudi::Property<bool> m_doNSWMatching{this, "doNSWMatchingAlg", false};
    Gaudi::Property<bool> m_doNSWMatchingMuon{this, "doNSWMatchingMuonOnly", false};
    Gaudi::Property<uint> m_maxStripDiff{this, "setMaxStripDistance", 3};
};
}
#endif  // MuonHitValAlg_H
