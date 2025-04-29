/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONRDOTOPREPDATA_RPCRDOTOPREPDATATOOLMT_H
#define MUONRDOTOPREPDATA_RPCRDOTOPREPDATATOOLMT_H

#include <set>

#include "AthenaBaseComps/AthAlgTool.h"
#include "MuonCablingData/RpcCablingMap.h"
#include "MuonCnvToolInterfaces/IMuonRdoToPrepDataTool.h"
#include "MuonCondData/RpcCondDbData.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonPrepRawData/MuonPrepDataCollection_Cache.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonRDO/RpcCoinMatrix.h"
#include "MuonRDO/RpcPadContainer.h"
#include "MuonRPC_CnvTools/IRPC_RDO_Decoder.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonTrigCoinData/MuonTrigCoinData_Cache.h"
#include "MuonTrigCoinData/RpcCoinDataContainer.h"
#include "RPC_CondCabling/RpcCablingCondData.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "xAODMuonPrepData/RpcStripContainer.h"
#include "xAODMuonRDO/NRPCRDOContainer.h"
namespace Muon {

/////////////////////////////////////////////////////////////////////////////

class RpcRdoToPrepDataToolMT
    : public extends<AthAlgTool, IMuonRdoToPrepDataTool> {
 public:
 using base_class::base_class;
  // setup/teardown functions, similar like those for Algorithm/Service
  virtual StatusCode initialize() override;

  virtual StatusCode decode(const EventContext& ctx, 
                            const std::vector<IdentifierHash>& idVect) const override;
 
  virtual StatusCode decode(const EventContext& ctx,
                            const std::vector<uint32_t>& robIds) const override;
  virtual StatusCode provideEmptyContainer(const EventContext& ctx) const override;
  
 protected:
   struct State {
        State(const IMuonIdHelperSvc* idHelperSvc);

        RpcPrepDataCollection* getPrepCollection(const Identifier& id);
        RpcCoinDataCollection* getCoinCollection(const Identifier& id);
        
        
        const IMuonIdHelperSvc* m_idHelperSvc{nullptr};

        std::vector<std::unique_ptr<RpcPrepDataCollection>> rpcPrepDataCollections{};
        std::vector<std::unique_ptr<RpcCoinDataCollection>> rpcCoinDataCollections{};

        /// Pointer of the prep container stored in store gate
        std::unique_ptr<RpcPrepDataContainer> prepDataCont{nullptr};
        /// Pointer of the coin container stored in store gate
        std::unique_ptr<RpcCoinDataContainer> coinDataCont{nullptr};


        // keepTrackOfFullEventDecoding
        bool m_fullEventDone{false};

        // the set of already requested and decoded offline (PrepRawData)
        // collections
        std::unordered_set<IdentifierHash> m_decodedOfflineHashIds{};

        // the set of unrequested collections with phi hits stored with
        // ambiguityFlag > 1
        std::unordered_set<IdentifierHash> m_ambiguousCollections{};

        // the set of already requested and decoded ROBs
        std::unordered_set<uint32_t> m_decodedRobIds{};
   };

  /// Stores the PrepData container into store gate
  StatusCode transferAndRecordPrepData(const EventContext& ctx,
                                       State& state) const;
  /// Stores the CoinData container into store gate
  StatusCode transferAndRecordCoinData(const EventContext& ctx,
                                       State& state) const;
  /// Load the hashes of the processed chambers
  StatusCode loadProcessedChambers(const EventContext& ctx, State& state) const;

  void printMTPrepData(const RpcPrepDataContainer& prepData) const;
  void printMTCoinData(const RpcCoinDataContainer& prepData) const;

  // decoding method
  StatusCode decodeImpl(const EventContext& ctx, State& state,
                        const std::vector<IdentifierHash>& idVect,
                        bool firstTimeInTheEvent) const;
  StatusCode decodeImpl(const EventContext& ctx, State& state,
                        const std::vector<uint32_t>& robIds,
                        bool firstTimeInTheEvent) const;

  StatusCode processPad(const EventContext& ctx, State& state,
                        const RpcPad* rdoColl, bool& processingetaview,
                        bool& processingphiview, int& nPrepRawData,
                        const std::vector<IdentifierHash>& idVect,
                        bool doingSecondLoopAmbigColls) const;

  void processTriggerHitHypothesis(RpcCoinMatrix::const_iterator itD,
                                   RpcCoinMatrix::const_iterator itD_end,
                                   bool highptpad,  // these are inputs
                                   bool& triggerHit, unsigned short& threshold,
                                   unsigned short& overlap, bool& toSkip) const;

  StatusCode processNrpcRdo(const EventContext& ctx, State& state) const;



  //!< 15 ns should be the max.diff. in prop.time in phi and eta strips
  Gaudi::Property<float> m_etaphi_coincidenceTime{
      this, "etaphi_coincidenceTime", 20., "time for phi*eta coincidence"};
  //!<  3 ns is the resolution of the RPC readout electronics
  Gaudi::Property<float> m_overlap_timeTolerance{
      this, "overlap_timeTolerance", 10.,
      "tolerance of the timing calibration"};
  Gaudi::Property<bool> m_producePRDfromTriggerWords{
      this, "produceRpcCoinDatafromTriggerWords", true,
      "tore as prd the trigger hits"};
  Gaudi::Property<bool> m_solvePhiAmbiguities{
      this, "solvePhiAmbiguities", true,
      "toggle on/off the removal of phi ambiguities"};
  Gaudi::Property<bool> m_reduceCablingOverlap{
      this, "reduceCablingOverlap", true, "toggle on/off the overlap removal"};
  Gaudi::Property<float> m_timeShift{this, "timeShift", -12.5,
                                     "any global time shift ?!"};
  Gaudi::Property<bool> m_decodeData{
      this, "DecodeData",
      true};  //!< toggle on/off the decoding of RPC RDO into RpcPerpData
  Gaudi::Property<bool> m_RPCInfoFromDb{this, "RPCInfoFromDb", false};  //!< correct time prd from cool db

  Gaudi::Property<bool> m_isMC{this, "isMC", false};

  SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_muDetMgrKey{
      this, "DetectorManagerKey", "MuonDetectorManager",
      "Key of input MuonDetectorManager condition data"};

  ServiceHandle<IMuonIdHelperSvc> m_idHelperSvc{
      this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

  /// RpcPrepData containers
  SG::WriteHandleKey<RpcPrepDataContainer> m_rpcPrepDataContainerKey{
      this, "OutputCollection", "RPC_Measurements"};
  /// RpcCoinData containers
  SG::WriteHandleKey<RpcCoinDataContainer> m_rpcCoinDataContainerKey{
      this, "TriggerOutputCollection", "RPC_triggerHits"};

  SG::ReadHandleKey<RpcPadContainer> m_rdoContainerKey{this, "RDOContainer",
                                                       "RPCPAD"};

  SG::ReadHandleKey<xAOD::NRPCRDOContainer> m_rdoNrpcContainerKey{
      this, "NrpcInputCollection", "NRPCRDO"};

  // Rob Data Provider handle
  ToolHandle<IRPC_RDO_Decoder> m_rpcRdoDecoderTool{
      this, "RdoDecoderTool", "Muon::RpcRDO_Decoder"};

  SG::ReadCondHandleKey<RpcCondDbData> m_readKey{
      this, "ReadKey", "RpcCondDbData", "Key of RpcCondDbData"};
  SG::ReadCondHandleKey<RpcCablingCondData> m_rpcReadKey{
      this, "RpcCablingKey", "RpcCablingCondData", "Key of RpcCablingCondData"};
  SG::ReadCondHandleKey<RpcCablingMap> m_nRpcCablingKey{this, "NrpcCablingKey","MuonNRPC_CablingMap",
                                                         "Key of MuonNRPC_CablingMap"};

  /// This is the key for the cache for the MDT PRD containers, can be empty
  SG::UpdateHandleKey<RpcPrepDataCollection_Cache> m_prdContainerCacheKey{
      this, "RpcPrdContainerCacheKey", "",
      "Optional external cache for the RPC PRD container"};
  SG::UpdateHandleKey<RpcCoinDataCollection_Cache> m_coindataContainerCacheKey{
      this, "RpcCoinDataContainerCacheKey", "",
      "Optional external cache for the RPC coin data container"};

  SG::WriteHandleKey<xAOD::RpcStripContainer> m_xAODKey{
      this, "xAODKey", "",
      "If empty, do not produce xAOD, otherwise this is the key of the output "
      "xAOD MDT PRD container"};
   
   Gaudi::Property<double> m_stripTimeResolution{this, "timeResolution", 0.6 * Gaudi::Units::nanosecond,
                                                          "Estimated time resolution of the strip readout"};
};
}  // namespace Muon

#endif  // !ATHEXJOBOPTIONS_CONCRETETOOL_H
