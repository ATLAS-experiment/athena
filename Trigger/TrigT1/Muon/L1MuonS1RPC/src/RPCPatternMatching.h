/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef RPCPatternMatching_H
#define RPCPatternMatching_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "AthenaKernel/IAthRNGSvc.h"

#include "StoreGate/ReadHandleKey.h"
#include "xAODMuonRDO/NRPCRDOContainer.h"
#include "MuonCablingData/RpcCablingMap.h"

#include "xAODMuonSimHit/MuonSimHitContainer.h"
#include "xAODMuonSimHit/MuonSimHit.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "xAODTrigL1Muon/L1RPCCandData.h"
#include "xAODTrigL1Muon/L1RPCCandDataContainer.h"
#include "xAODTrigL1Muon/L1RPCCandDataAuxContainer.h"

#include "L1MuonS1RPC/RPCMuonPattern.h"
#include "L1MuonS1RPC/RPCSLStripConverter.h"
#include <GaudiKernel/PhysicalConstants.h>

#include "TFile.h"
#include "TTree.h"

namespace L1Muon {

    class RPCPatternMatching : public ::AthReentrantAlgorithm {
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;
            virtual StatusCode initialize() override;
            virtual StatusCode execute(const EventContext& ctx) const override;
            
        private:
            using RdoPairs = std::array<const xAOD::NRPCRDO*, 2>;
            using PatternContainer = std::map<uint16_t, std::map<uint16_t, std::vector<RPCMuonPattern>>>; // map to hold patterns for each sector and eta bin

            StatusCode initializePatterns(); // read pattern from file (once ready, move pattern lists to ATLAS storage)
            StatusCode loadPatternFile(const std::string& filePath, const std::string& patternType, PatternContainer& patternContainer);
            void getStripIdforAlg(const RdoPairs& rdoPairs, 
                          std::vector<uint32_t>& stripIDs, 
                          std::map<uint32_t, uint32_t>& strip_coordinates,
                          std::map<uint32_t, uint32_t>& strip_etabin,
                          const Muon::RpcCablingData& cabling, 
                          const Identifier& offId, 
                          const ActsTrk::GeometryContext& geoCtx) const;

            uint32_t getSLStripId(bool side, int sector, int doubletPhi, Muon::MuonStationIndex::StIndex RPClayer,
                                 int doubletR, int stationEta, int doubletZ, int stripNumber, bool measuresPhi) const;

            int getVirtualBIPhiStripNumber(float loc_pos_along_BIstrip, const MuonGMR4::RpcReadoutElement& readoutElement) const;

            uint32_t getVirtualBIPhiStripCoordinate(const Amg::Vector3D& stripPos, float loc_pos_along_BIstrip) const;

            uint32_t getBMBOPhiStripCoordinate(const Amg::Vector3D& stripPos) const;

            void matchPattern( const RPCMuonPattern& pattern, const std::vector<uint32_t>& stripIDs,
                                std::vector<RPCMuonPattern>& matchedPatterns, const uint16_t& sector_header) const;

            void matchEtaAndPhiPatterns(const RPCMuonPattern& etaPattern, const RPCMuonPattern& phiPattern, std::vector<RPCMuonPattern>& matchedPatterns) const;

            bool checkStripCoincidence(std::vector<uint32_t>& stripIDs) const;

            StatusCode createCandidate(const std::map<uint16_t, std::vector<RPCMuonPattern>>& matchedPatterns, xAOD::L1RPCCandDataContainer& outputCandHandle) const;

            void getBestCandidatesPerEtaBin(const std::vector<RPCMuonPattern>& matchedPatterns,
                                         std::map<uint32_t, RPCMuonPattern>& bestCandidates) const;

            bool patternSanityCheck(const RPCMuonPattern& pattern) const;

            // Patterns for each sector are stored in maps with the following structure:
            // m_etaPatterns[sector_header][etabin] = vector of RPCMuonPattern
            // m_phiPatterns[sector_header][etabin] = vector of RPCMuonPattern
            // This structure allows for efficient access to the patterns based on sector and eta binning,
            // minimizing the need for repeated searches through the patterns.

            PatternContainer m_etaPatterns; // map to hold patterns for each sector 
            PatternContainer m_phiPatterns; // map to hold patterns for each sector 
            
            /// RPC Rdo
            SG::ReadHandleKey<xAOD::NRPCRDOContainer> m_keyRpcRdo{this,"NrpcRdoKey","NRPCRDO","Location of input RpcRDO"};

            /// Output Trigger candidates
            SG::WriteHandleKey<xAOD::L1RPCCandDataContainer> m_outputCandKey{this, "xRPCCandKey", "L1RPCCandData",
                                                                        "LVL1 xAOD Barrel trigger candidates in the Muon RPC"};

            /// RPC cabling map
            SG::ReadCondHandleKey<Muon::RpcCablingMap> m_cablingKey{this, "CablingKey", "MuonNRPC_CablingMap",
                                                                            "Key of MuonNRPC_CablingMap"};
            
            /// RPC Id helper
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{"Muon::MuonIdHelperSvc/MuonIdHelperSvc", name()};

            const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
            SG::ReadHandleKey<ActsTrk::GeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

    };

} // end of namespace L1Muon


#endif // RPCPatternMatching_H