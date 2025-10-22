/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONRDOTOPREPDATA_RPCRDOTOPREPDATATOOLMT_H
#define MUONRDOTOPREPDATA_RPCRDOTOPREPDATATOOLMT_H

#include <set>
#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonCnvToolInterfaces/IMuonRdoToPrepDataTool.h"
#include "MuonCondData/RpcCondDbData.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonRDO/RpcCoinMatrix.h"
#include "MuonRDO/RpcPadContainer.h"
#include "xAODMuonRDO/NRPCRDOContainer.h"
#include "MuonRPC_CnvTools/IRPC_RDO_Decoder.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonTrigCoinData/RpcCoinDataContainer.h"
#include "RPC_CondCabling/RpcCablingCondData.h"
#include "MuonCablingData/RpcCablingMap.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "xAODEventInfo/EventInfo.h"
#include "MuonPrepRawData/MuonPrepDataCollection_Cache.h"
#include "MuonTrigCoinData/MuonTrigCoinData_Cache.h"

namespace Muon {

    /////////////////////////////////////////////////////////////////////////////

    class RpcRdoToPrepDataToolMT : public extends<AthAlgTool, IMuonRdoToPrepDataTool> {
    public:        

        RpcRdoToPrepDataToolMT(const std::string&, const std::string&, const IInterface*);

        // setup/teardown functions, similar like those for Algorithm/Service
        virtual StatusCode initialize() override;

        virtual StatusCode finalize() override;

        // debugging
        virtual void printInputRdo(const EventContext& ctx) const override;
        void printPrepDataImpl(const Muon::RpcPrepDataContainer& rpcPrepDataContainer,
                               const Muon::RpcCoinDataContainer& rpcCoinDataContainer) const;
        void printCoinDataImpl(const Muon::RpcCoinDataContainer& rpcCoinDataContainer) const;

        virtual StatusCode decode(const EventContext& ctx, std::vector<IdentifierHash>& idVect, std::vector<IdentifierHash>& selectedIdVect) const override;
        virtual StatusCode decode(const EventContext& ctx, const std::vector<uint32_t>& robIds) const override;
        virtual StatusCode provideEmptyContainer(const EventContext& ctx) const override;
        virtual void printPrepData(const EventContext& ctx) const override;

    protected:
        struct State;

        /// Stores the PrepData container into store gate
        StatusCode transferAndRecordPrepData(const EventContext& ctx, State& state) const;
        /// Stores the CoinData container into store gate
        StatusCode transferAndRecordCoinData(const EventContext& ctx, State& state) const;
        /// Load the hashes of the processed chambers 
        StatusCode loadProcessedChambers(const EventContext& ctx, State& state) const;
        
        void printMTPrepData(const Muon::RpcPrepDataContainer& prepData) const;
        void printMTCoinData(const Muon::RpcCoinDataContainer& prepData) const;

   
      

        // decoding method
        StatusCode decodeImpl(const EventContext& ctx, State& state, 
                              std::vector<IdentifierHash>& idVect, std::vector<IdentifierHash>& selectedIdVect,
                              bool firstTimeInTheEvent) const;
        StatusCode decodeImpl(const EventContext& ctx, State& state, 
                              const std::vector<uint32_t>& robIds, bool firstTimeInTheEvent) const;

        StatusCode processPad(const EventContext& ctx, State& state, 
                              const RpcPad* rdoColl, bool& processingetaview, bool& processingphiview, int& nPrepRawData,
                              std::vector<IdentifierHash>& idVect, std::vector<IdentifierHash>& idWithDataVect,
                              bool doingSecondLoopAmbigColls) const;

        void processTriggerHitHypothesis(RpcCoinMatrix::const_iterator itD, RpcCoinMatrix::const_iterator itD_end,
                                         bool highptpad,  // these are inputs
                                         bool& triggerHit, unsigned short& threshold, unsigned short& overlap, bool& toSkip) const;

        StatusCode processNrpcRdo(const EventContext& ctx, State& state) const;

        struct State {
            State(const RpcIdHelper& idHelper);
           
            Muon::RpcPrepDataCollection* getPrepCollection(const IdentifierHash& hash, MsgStream& msg);
            Muon::RpcCoinDataCollection* getCoinCollection(const IdentifierHash& hash, MsgStream& msg);

            
            const RpcIdHelper& m_rpcIdHelper;

            std::map<IdentifierHash, std::unique_ptr<Muon::RpcPrepDataCollection>> m_rpcPrepDataCollections{};
            std::map<IdentifierHash, std::unique_ptr<Muon::RpcCoinDataCollection>> m_rpcCoinDataCollections{};

            /// Pointer of the prep container stored in store gate
            std::unique_ptr<Muon::RpcPrepDataContainer> m_prepDataCont{nullptr};            
            /// Pointer of the coin container stored in store gate
            std::unique_ptr<Muon::RpcCoinDataContainer> m_coinDataCont{nullptr};
            // keepTrackOfFullEventDecoding
            bool m_fullEventDone{false};

            // the set of already requested and decoded offline (PrepRawData) collections
            std::set<IdentifierHash> m_decodedOfflineHashIds{};

            // the set of unrequested collections with phi hits stored with ambiguityFlag > 1
            std::set<IdentifierHash> m_ambiguousCollections{};

            // the set of already requested and decoded ROBs
            std::set<uint32_t> m_decodedRobIds{};

            const IdContext m_modContext{m_rpcIdHelper.module_context()};
        };
      
        //!< 15 ns should be the max.diff. in prop.time in phi and eta strips
        Gaudi::Property<float> m_etaphi_coincidenceTime{this, "etaphi_coincidenceTime", 20., "time for phi*eta coincidence"};
        //!<  3 ns is the resolution of the RPC readout electronics
        Gaudi::Property<float> m_overlap_timeTolerance{this,"overlap_timeTolerance", 10., "tolerance of the timing calibration"};
        Gaudi::Property<bool> m_producePRDfromTriggerWords{this, "produceRpcCoinDatafromTriggerWords", true, "tore as prd the trigger hits"};
        Gaudi::Property<bool> m_solvePhiAmbiguities{this, "solvePhiAmbiguities", true, "toggle on/off the removal of phi ambiguities" };
        Gaudi::Property<bool> m_reduceCablingOverlap{this, "reduceCablingOverlap", true, "toggle on/off the overlap removal"}; 
        Gaudi::Property<float> m_timeShift{this, "timeShift", -12.5, "any global time shift ?!" };
        Gaudi::Property<bool> m_decodeData{this, "DecodeData", true};                  //!< toggle on/off the decoding of RPC RDO into RpcPerpData
        Gaudi::Property<bool> m_RPCInfoFromDb{this, "RPCInfoFromDb", false};               //!< correct time prd from cool db
        // end of configurable options

        SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_muDetMgrKey{this, "DetectorManagerKey", "MuonDetectorManager",
                                                                         "Key of input MuonDetectorManager condition data"};

        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
        
        /// RpcPrepData containers
        SG::WriteHandleKey<Muon::RpcPrepDataContainer> m_rpcPrepDataContainerKey{this,"OutputCollection","RPC_Measurements"};
        /// RpcCoinData containers
        SG::WriteHandleKey<Muon::RpcCoinDataContainer> m_rpcCoinDataContainerKey{this,"TriggerOutputCollection", "RPC_triggerHits" };

        SG::ReadHandleKey<RpcPadContainer> m_rdoContainerKey{this, "RDOContainer", "RPCPAD"};

        SG::ReadHandleKey<xAOD::NRPCRDOContainer> m_rdoNrpcContainerKey{this, "NrpcInputCollection", "NRPCRDO"};

        // Rob Data Provider handle
        ToolHandle<Muon::IRPC_RDO_Decoder> m_rpcRdoDecoderTool{this, "RdoDecoderTool", "Muon::RpcRDO_Decoder"};

        SG::ReadCondHandleKey<RpcCondDbData> m_readKey{this, "ReadKey", "RpcCondDbData", "Key of RpcCondDbData"};
        SG::ReadCondHandleKey<RpcCablingCondData> m_rpcReadKey{this, "RpcCablingKey", "RpcCablingCondData", "Key of RpcCablingCondData"};
        SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo{this, "EventInfoContName", "EventInfo", "event info key"};
        SG::ReadCondHandleKey<RpcCablingMap> m_nRpcCablingKey{this, "NrpcCablingKey", "MuonNRPC_CablingMap", "Key of MuonNRPC_CablingMap"};
    
         /// This is the key for the cache for the MDT PRD containers, can be empty
         SG::UpdateHandleKey<RpcPrepDataCollection_Cache> m_prdContainerCacheKey{this,"RpcPrdContainerCacheKey", "", "Optional external cache for the RPC PRD container"};
         SG::UpdateHandleKey<RpcCoinDataCollection_Cache> m_coindataContainerCacheKey{this, "RpcCoinDataContainerCacheKey", "" , "Optional external cache for the RPC coin data container"};

    private:
        mutable std::atomic_int m_spuriousHitCounter;
    };
}  // namespace Muon

#endif  // !ATHEXJOBOPTIONS_CONCRETETOOL_H
