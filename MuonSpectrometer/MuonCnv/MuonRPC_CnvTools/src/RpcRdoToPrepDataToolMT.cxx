/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "RpcRdoToPrepDataToolMT.h"

#include "GaudiKernel/ThreadLocalContext.h"
#include "MuonCnvToolInterfaces/IDC_Helper.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonRPC_CnvTools/IRPC_RDO_Decoder.h"
#include "MuonReadoutGeometry/RpcReadoutElement.h"
#include "MuonTrigCoinData/RpcCoinDataContainer.h"
#include "TrkSurfaces/Surface.h"
#include "xAODMuonPrepData/RpcStripAuxContainer.h"
#include "MuonIdHelpers/IdentifierByDetElSorter.h"
#include "GeoModelKernel/throwExcept.h"
using namespace MuonGM;
using namespace Trk;
namespace Muon{
/////////////////////////////////////////////////////////////////////////////
RpcRdoToPrepDataToolMT::State::State(const IMuonIdHelperSvc* idHelperSvc):    
    m_idHelperSvc{idHelperSvc} {
    const IdentifierHash hashMax = m_idHelperSvc->rpcIdHelper().module_hash_max();
    rpcPrepDataCollections.resize(hashMax);
    rpcCoinDataCollections.resize(hashMax);
}
RpcPrepDataCollection* RpcRdoToPrepDataToolMT::State::getPrepCollection(const Identifier& chanId) {
  
  const IdentifierHash  rpdModHash = m_idHelperSvc->moduleHash(chanId);
  std::unique_ptr<RpcPrepDataCollection>& coll = rpcPrepDataCollections[rpdModHash];
  if (!coll) {    
    coll = std::make_unique<RpcPrepDataCollection>(rpdModHash);
    coll->setIdentifier(m_idHelperSvc->chamberId(chanId));
  }
  return coll.get();
}
RpcCoinDataCollection* RpcRdoToPrepDataToolMT::State::getCoinCollection(const Identifier& chanId) {
    const IdentifierHash  rpdModHash = m_idHelperSvc->moduleHash(chanId);
    std::unique_ptr<RpcCoinDataCollection>& coll = rpcCoinDataCollections[rpdModHash];
    if (!coll) {    
      coll = std::make_unique<RpcCoinDataCollection>(rpdModHash);
      coll->setIdentifier(m_idHelperSvc->chamberId(chanId));
  }
  return coll.get();
}


//___________________________________________________________________________
StatusCode RpcRdoToPrepDataToolMT::initialize() {
  // perform necessary one-off initialization

  ATH_MSG_INFO("properties are ");
  ATH_MSG_INFO("produceRpcCoinDatafromTriggerWords " <<m_producePRDfromTriggerWords);
  ATH_MSG_INFO("reduceCablingOverlap               " << m_reduceCablingOverlap);
  ATH_MSG_INFO("solvePhiAmbiguities                " << m_solvePhiAmbiguities);
  ATH_MSG_INFO("timeShift                          " << m_timeShift);
  if (m_solvePhiAmbiguities && (!m_reduceCablingOverlap)) {
    ATH_MSG_WARNING("Inconsistent setting of properties (solvePhiAmbiguities entails reduceCablingOverlap)");
    ATH_MSG_WARNING("Resetting reduceCablingOverlap to true");
    m_reduceCablingOverlap = true;
  }
  ATH_MSG_INFO("etaphi_coincidenceTime             " << m_etaphi_coincidenceTime);
  ATH_MSG_INFO("overlap_timeTolerance              " << m_overlap_timeTolerance);
  ATH_MSG_INFO("Correct prd time from cool db      " << m_RPCInfoFromDb);
  ATH_CHECK(m_rpcRdoDecoderTool.retrieve());
  ATH_CHECK(m_idHelperSvc.retrieve());
  ATH_CHECK(m_rpcReadKey.initialize());
  ATH_CHECK(m_readKey.initialize(m_RPCInfoFromDb));
  ATH_CHECK(m_rdoContainerKey.initialize());
  ATH_CHECK(m_rdoNrpcContainerKey.initialize(!m_rdoNrpcContainerKey.empty()));
  ATH_CHECK(m_nRpcCablingKey.initialize(!m_rdoNrpcContainerKey.empty()));  
  // If we don't configure the NRPC RDO
  // key, the cabling is needed either.
  ATH_CHECK(m_rpcPrepDataContainerKey.initialize());
  ATH_CHECK(m_rpcCoinDataContainerKey.initialize(m_producePRDfromTriggerWords));
  ATH_CHECK(m_muDetMgrKey.initialize());
  ATH_CHECK(m_prdContainerCacheKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_coindataContainerCacheKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_xAODKey.initialize(!m_xAODKey.empty()));
  return StatusCode::SUCCESS;
}
StatusCode RpcRdoToPrepDataToolMT::loadProcessedChambers(const EventContext& ctx, State& state) const {
  const int modHashMax = m_idHelperSvc->rpcIdHelper().module_hash_max();
  if (!m_prdContainerCacheKey.key().empty()) {
    SG::UpdateHandle update{m_prdContainerCacheKey, ctx};
    if (!update.isValid()) {
      ATH_MSG_FATAL("Invalid UpdateHandle " << m_prdContainerCacheKey.key());
      return StatusCode::FAILURE;
    }
    state.prepDataCont = std::make_unique<RpcPrepDataContainer>(update.ptr());
    for (const RpcPrepDataCollection* coll : *state.prepDataCont) {
      state.m_decodedOfflineHashIds.insert(coll->identifyHash());
    }
  } else{
    state.prepDataCont = std::make_unique<RpcPrepDataContainer>(modHashMax);
  }
  if (m_coindataContainerCacheKey.key().empty()) {
    // without the cache we just record the container
    state.coinDataCont = std::make_unique<RpcCoinDataContainer>(modHashMax);
  } else {
    // use the cache to get the container
    SG::UpdateHandle update{m_coindataContainerCacheKey, ctx};
    if (!update.isValid()) {
      ATH_MSG_FATAL("Invalid UpdateHandle "<< m_coindataContainerCacheKey.key());
      return StatusCode::FAILURE;
    }
    state.coinDataCont = std::make_unique<RpcCoinDataContainer>(update.ptr());
    for (const RpcCoinDataCollection* coll : *state.coinDataCont) {
      state.m_decodedOfflineHashIds.insert(coll->identifyHash());
    }
  }
  return StatusCode::SUCCESS;
}
/// This code is thread-safe as we will propagate local thread collection
/// contents to a thread-safe one
StatusCode RpcRdoToPrepDataToolMT::decode(const EventContext& ctx, 
                                          const std::vector<IdentifierHash>& idVect) const {
  ATH_MSG_DEBUG("Calling Core decode function from MT decode function (hash vector)");
  State state{m_idHelperSvc.get()};
  ATH_CHECK(loadProcessedChambers(ctx, state));

  ATH_CHECK(decodeImpl(ctx, state, idVect, true));
  ATH_CHECK(processNrpcRdo(ctx, state));
  ATH_MSG_DEBUG("Core decode processed in MT decode (hash vector)");

  ATH_CHECK(transferAndRecordPrepData(ctx, state));
  ATH_CHECK(transferAndRecordCoinData(ctx, state));

  return StatusCode::SUCCESS;
}

/// This code is thread-safe as we will propagate local thread collection
/// contents to a thread-safe one
StatusCode RpcRdoToPrepDataToolMT::decode(const EventContext& ctx,
                                          const std::vector<uint32_t>& robIds) const {
  ATH_MSG_DEBUG("Calling Core decode function from MT decode function (ROB vector)");
  State state{m_idHelperSvc.get()};
  ATH_CHECK(loadProcessedChambers(ctx, state));

  ATH_CHECK(decodeImpl(ctx, state, robIds, true));
  ATH_CHECK(processNrpcRdo(ctx, state));
  ATH_MSG_DEBUG("Core decode processed in MT decode (ROB vector)");

  ATH_CHECK(transferAndRecordPrepData(ctx, state));
  ATH_CHECK(transferAndRecordCoinData(ctx, state));

  return StatusCode::SUCCESS;
}
StatusCode RpcRdoToPrepDataToolMT::provideEmptyContainer(const EventContext& ctx) const {
  State state{m_idHelperSvc.get()};
  ATH_CHECK(loadProcessedChambers(ctx, state));
  ATH_CHECK(transferAndRecordPrepData(ctx, state));
  ATH_CHECK(transferAndRecordCoinData(ctx, state));
  return StatusCode::SUCCESS;
}

StatusCode RpcRdoToPrepDataToolMT::transferAndRecordPrepData(const EventContext& ctx, State& state) const {

  SG::WriteHandle<xAOD::RpcStripContainer> writeHandleXAOD{};
  if (!m_xAODKey.empty()) {
    writeHandleXAOD = SG::WriteHandle{m_xAODKey, ctx};
    ATH_CHECK(writeHandleXAOD.record(std::make_unique<xAOD::RpcStripContainer>(),
                                     std::make_unique<xAOD::RpcStripAuxContainer>()));
  }
  const RpcIdHelper& idHelper{m_idHelperSvc->rpcIdHelper()};
  for (std::unique_ptr<RpcPrepDataCollection>& collection : state.rpcPrepDataCollections) {
    if (!collection || collection->empty()) {
        continue;
    }
    if (!m_xAODKey.empty()) {
      /// Before converting the PrepData into the xAOD container, sort them by detectorElement
      /// allowing for ChamberView accesses layer
      std::vector<const RpcPrepData*> sortMe{collection->begin(), collection->end()};
      std::ranges::sort(sortMe, IdentifierByDetElSorter{m_idHelperSvc.get()});
      for (const RpcPrepData* prd : sortMe) {
        const Identifier id = prd->identify();
        xAOD::RpcStrip* strip = writeHandleXAOD->push_back(std::make_unique<xAOD::RpcStrip>());
        strip->setDoubletPhi(idHelper.doubletPhi(id));
        strip->setGasGap(idHelper.gasGap(id));
        strip->setMeasuresPhi(idHelper.measuresPhi(id));
        strip->setStripNumber(idHelper.channel(id));
        strip->setAmbiguityFlag(prd->ambiguityFlag());
        strip->setTimeOverThreshold(prd->timeOverThreshold());
        strip->setTime(prd->time());
        strip->setTimeCovariance(std::pow(m_stripTimeResolution, 2));
        strip->setTriggerInfo(prd->triggerInfo());
        xAOD::MeasVector<1> locPos{prd->localPosition().x()};
        xAOD::MeasMatrix<1> locCov{prd->localCovariance()(0,0)};
        strip->setMeasurement(m_idHelperSvc->detElementHash(id), std::move(locPos), std::move(locCov));
      }
    }
    
    const IdentifierHash hash = collection->identifyHash();
    // If not present, get a write lock for the hash and move collection
    RpcPrepDataContainer::IDC_WriteHandle lock =  state.prepDataCont->getWriteHandle(hash);
    if (lock.alreadyPresent()) {
      ATH_MSG_DEBUG("RpcPrepDataCollection already contained in IDC "
                    << m_idHelperSvc->toString(collection->identify()));
      continue;
    }
    ATH_CHECK(lock.addOrDelete(std::move(collection)));
    ATH_MSG_DEBUG("PRD hash " << hash << " has been moved to cache container");
  }
  state.rpcPrepDataCollections.clear();

  if (msgLvl(MSG::DEBUG)) {
    for (const auto& [hash, ptr] : state.prepDataCont->GetAllHashPtrPair()) {
      ATH_MSG_DEBUG("Contents of CONTAINER in this view : " << hash);
    }
  }
  SG::WriteHandle rpcPRDHandle{m_rpcPrepDataContainerKey, ctx};
  ATH_CHECK(rpcPRDHandle.record(std::move(state.prepDataCont)));
  ATH_MSG_DEBUG("Created container " << m_rpcPrepDataContainerKey.key());

  return StatusCode::SUCCESS;
}

StatusCode RpcRdoToPrepDataToolMT::transferAndRecordCoinData(const EventContext& ctx, State& state) const {
  if (!m_producePRDfromTriggerWords) {
    return StatusCode::SUCCESS;
  }

  // Take localContainer and transfer contents to rpcCoinHandle
  for (std::unique_ptr<RpcCoinDataCollection>& collection : state.rpcCoinDataCollections) {
    if (!collection || collection->empty()) {
      continue;
    }
    const IdentifierHash hash = collection->identifyHash();
    // If not present, get a write lock for the hash and move collection
    RpcCoinDataContainer::IDC_WriteHandle lock = state.coinDataCont->getWriteHandle(hash);
    if (lock.alreadyPresent()) {
      ATH_MSG_DEBUG("RpcCoinDataCollection already contained in IDC "
                    << m_idHelperSvc->toString(collection->identify()));
      continue;
    }
    ATH_CHECK(lock.addOrDelete(std::move(collection)));
    ATH_MSG_DEBUG("Coin hash " << hash << " has been moved to cache container");
  }
  state.rpcCoinDataCollections.clear();
  if (msgLvl(MSG::DEBUG)) {
    for (const auto& [hash, ptr] : state.coinDataCont->GetAllHashPtrPair()) {
      ATH_MSG_DEBUG("Contents of LOCAL in this view : " << hash);
    }
  }
  SG::WriteHandle<RpcCoinDataContainer> rpcCoinHandle{
      m_rpcCoinDataContainerKey, ctx};
  ATH_CHECK(rpcCoinHandle.record(std::move(state.coinDataCont)));

  ATH_MSG_DEBUG("Created container " << m_rpcCoinDataContainerKey.key());
  // For additional information on the contents of the cache-based container,
  // this function can be used printMTCoinData (*rpcCoinHandle);

  return StatusCode::SUCCESS;
}

//___________________________________________________________________________
StatusCode RpcRdoToPrepDataToolMT::decodeImpl(const EventContext& ctx, State& state, 
                                              const std::vector<IdentifierHash>& idVect,
                                              bool firstTimeInTheEvent) const {
  int sizeVectorRequested = idVect.size();
  ATH_MSG_DEBUG("Decode method called for " << sizeVectorRequested << " offline collections");
  if (sizeVectorRequested == 0){
    ATH_MSG_DEBUG("Decoding the entire event");
  }
  // create an empty vector of hash ids to be decoded (will be filled if
  // RoI-based and left empty if full-scan)
  std::vector<IdentifierHash> idVectToBeDecoded;
  idVectToBeDecoded.reserve(idVect.size());

  if (firstTimeInTheEvent) {
    state.m_fullEventDone = sizeVectorRequested == 0;
  } else {
    if (state.m_fullEventDone) {
      ATH_MSG_DEBUG("Whole event has already been decoded; nothing to do.");
      return StatusCode::SUCCESS;
    }
    if (sizeVectorRequested == 0) {
      state.m_fullEventDone = true;
    }
  }

  if (sizeVectorRequested != 0) {
    // the program goes in here only if RoI-based decoding has been called and
    // the full event is not already decoded this code ensures decoding of every
    // offline hash id is called only once
    for (const IdentifierHash& itHashId : idVect) {
      if (state.m_decodedOfflineHashIds.insert(itHashId).second)
        idVectToBeDecoded.push_back(itHashId);
    }

    if (idVectToBeDecoded.empty()) {
      ATH_MSG_DEBUG("All requested offline collections have already been decoded; nothing to do.");
      return StatusCode::SUCCESS;
    } else {
      ATH_MSG_DEBUG(idVectToBeDecoded.size()
                    << " offline collections have not yet been decoded and will be decoded now.");
      ATH_MSG_VERBOSE("The list of offline collection hash ids to be decoded:"<<std::endl<<idVectToBeDecoded);
    }
  }

  // if RPC decoding is switched off stop here
  if (!m_decodeData) {
    ATH_MSG_DEBUG("Stored empty container. Decoding RPC RDO into RPC PrepRawData is switched off");
    return StatusCode::SUCCESS;
  }

  ATH_MSG_DEBUG("Decoding RPC RDO into RPC PrepRawData");

  SG::ReadCondHandle rpcCabling{m_rpcReadKey, ctx};
  // if the vector requested has size 0, we need to perform a scan of the entire
  // RDO container otherwise select the pads to be decoded
  std::vector<IdentifierHash> rdoHashVec;
  if (sizeVectorRequested != 0) {
    ATH_MSG_DEBUG("Looking for pads IdHash to be decoded for the requested collection Ids");
    ATH_CHECK(rpcCabling->giveRDO_fromPRD(idVectToBeDecoded, rdoHashVec));
  }

  /// RPC context
  IdContext rpcContext = m_idHelperSvc->rpcIdHelper().module_context();

  // we come here if the rdo container is already in SG (for example in MC RDO!)
  ATH_MSG_DEBUG("Retrieving Rpc PAD container from the store");
  auto rdoContainerHandle = SG::makeHandle(m_rdoContainerKey, ctx);
  if (!rdoContainerHandle.isValid()) {
    ATH_MSG_ERROR("Retrieval of RPC RDO container failed ! "<<m_rdoContainerKey.fullKey());
    return StatusCode::FAILURE;
  }

  ///////////// here the RDO container is retrieved and filled -whatever input
  ///type we start with- => check the size
  if (rdoContainerHandle->numberOfCollections() == 0) {
    // empty pad container - no rpc rdo in this event
    ATH_MSG_DEBUG("Empty pad container - no rpc rdo in this event ");
    return StatusCode::SUCCESS;
  }
  ATH_MSG_DEBUG("Not empty pad container in this event ");

  // start here to process the RDO (for the full event of for a fraction of it)
  bool processingetaview{m_solvePhiAmbiguities}, processingphiview{false};
  bool doingSecondLoopAmbigColls = false;
  while (processingetaview || processingphiview || (!m_solvePhiAmbiguities)) {
    int ipad{0}, nPrepRawData{0}, nPhiPrepRawData{0}, nEtaPrepRawData{0};
    if (processingphiview) {
      state.m_ambiguousCollections.clear();
    }
    ATH_MSG_DEBUG("*** Processing "<<(processingetaview ? "eta" : "phi")<<" view ");
    // seeded decoding
    if (sizeVectorRequested != 0) {
      ATH_MSG_DEBUG("Start loop over pads hashes - seeded mode ");
      for (const IdentifierHash& iPadHash : rdoHashVec) {
        const RpcPad* rdoColl = rdoContainerHandle->indexFindPtr(iPadHash);
        if (!rdoColl) {
          ATH_MSG_DEBUG("Requested pad with online id "<< iPadHash << " not found in the rdoContainerHandle.");
          continue;
        }
        ++ipad;

        ATH_MSG_DEBUG("A new pad here n. " << ipad << ", online id "<< rdoColl->identifyHash()
                   << ", with " << rdoColl->size() << " CM inside ");
        ATH_CHECK(processPad(ctx, state, rdoColl, processingetaview,
                             processingphiview, nPrepRawData, idVectToBeDecoded, doingSecondLoopAmbigColls));

      }       // end loop over requested pads hashes
    } else {  // unseeded // whole event
      ATH_MSG_DEBUG("Start loop over pads - unseeded mode ");
      for (const RpcPad* rdoColl : *rdoContainerHandle) {
        // loop over all elements of the pad container
        if (rdoColl->empty()){
          continue;
        }
        ++ipad;
        ATH_MSG_DEBUG("A new pad here n. " << ipad << ", online id "<< rdoColl->identifyHash()
                    << ", with " << rdoColl->size() << " CM inside ");

        ATH_CHECK(processPad(ctx, state, rdoColl, processingetaview,
                             processingphiview, nPrepRawData, idVectToBeDecoded, doingSecondLoopAmbigColls));
      }  // end loop over pads
    }

    if (processingetaview) {
      processingetaview = false;
      processingphiview = true;
      nEtaPrepRawData = nPrepRawData;
      ATH_MSG_DEBUG("*** " << nEtaPrepRawData << " eta PrepRawData registered");
    } else {
      processingphiview = false;
      nPhiPrepRawData = nPrepRawData - nEtaPrepRawData;
      ATH_MSG_DEBUG("*** " << nPhiPrepRawData << " phi PrepRawData registered");
      if (!state.m_ambiguousCollections.empty()) {
        // loop again for unrequested collections stored with ambiguous phi hits
        doingSecondLoopAmbigColls = true;
        processingetaview = true;
        ATH_MSG_DEBUG(state.m_ambiguousCollections.size() << " ambiguous collections were stored:");
        idVectToBeDecoded.clear();
        rdoHashVec.clear();
        for (const IdentifierHash& itAmbiColl : state.m_ambiguousCollections) {
          ATH_MSG_DEBUG(itAmbiColl << " ");
          idVectToBeDecoded.push_back(itAmbiColl);
          state.m_decodedOfflineHashIds.insert(itAmbiColl);
        }
        ATH_CHECK(rpcCabling->giveRDO_fromPRD(idVectToBeDecoded, rdoHashVec));
      }
    }
    if (!m_solvePhiAmbiguities) {
      ATH_MSG_DEBUG("*** " << nPrepRawData << " PrepRawData registered");
      break;
    }
  }

  ATH_MSG_DEBUG("*** Final Cleanup ");
  return StatusCode::SUCCESS;
}

//___________________________________________________________________________
StatusCode RpcRdoToPrepDataToolMT::decodeImpl(const EventContext& ctx, State& state, 
                                              const std::vector<uint32_t>& robIds, bool firstTimeInTheEvent) const {
  // ROB-based decoding is only applied in seeded mode. Full scan should use the
  // hashId-based method with empty requested collections vector.

  int sizeVectorRequested = robIds.size();
  ATH_MSG_DEBUG("Decode method called for " << sizeVectorRequested << " ROBs");

  std::vector<uint32_t> robIdsToBeDecoded{};
  robIdsToBeDecoded.reserve(robIds.size());

  if (firstTimeInTheEvent) {
    state.m_fullEventDone = false;
  } else {
    if (state.m_fullEventDone) {
      ATH_MSG_DEBUG("Whole event has already been decoded; nothing to do.");
      return StatusCode::SUCCESS;
    }
  }

  // check which of the requested robs are not yet decoded
  for (uint32_t robid : robIds) {
    if (state.m_decodedRobIds.insert(robid).second) {
      robIdsToBeDecoded.push_back(robid);
    }
  }

  if (robIdsToBeDecoded.empty()) {
    ATH_MSG_DEBUG("All requested ROBs have already been decoded; nothing to do.");
    return StatusCode::SUCCESS;
  }
  ATH_MSG_DEBUG(robIdsToBeDecoded.size()<< " ROBs have not yet been decoded and will be decoded now.");
  if (msgLvl(MSG::VERBOSE)) {
    ATH_MSG_VERBOSE("The list of ROB Ids to be decoded:");
    for (uint32_t robid : robIdsToBeDecoded)
      ATH_MSG_VERBOSE("0x" << MSG::hex << robid << MSG::dec << " ");
  }

  SG::ReadCondHandle rpcCabling{m_rpcReadKey, ctx};

  // if all robs will be decoded after the current execution of the method, set
  // the flag m_fullEventDone
  if (state.m_decodedRobIds.size() == rpcCabling->giveFullListOfRobIds().size())
    state.m_fullEventDone = true;

  // if RPC decoding is switched off stop here
  if (!m_decodeData) {
    ATH_MSG_DEBUG("Stored empty container. Decoding RPC RDO into RPC PrepRawData is switched off");
    return StatusCode::SUCCESS;
  }

  ATH_MSG_DEBUG("Decoding RPC RDO into RPC PrepRawData");

  // we come here if the rdo container is already in SG (for example in MC RDO!)
  ATH_MSG_DEBUG("Retrieving Rpc PAD container from the store");
  auto rdoContainerHandle = SG::makeHandle(m_rdoContainerKey, ctx);
  if (!rdoContainerHandle.isValid()) {
    ATH_MSG_WARNING("Retrieval of RPC RDO container failed ! "<<m_rdoContainerKey.fullKey());
    return StatusCode::SUCCESS;
  }

  // here the RDO container is retrieved and filled -whatever input type we
  // start with- => check the size
  if (rdoContainerHandle->empty()) {
    // empty pad container - no rpc rdo in this event
    ATH_MSG_DEBUG("Empty pad container - no rpc rdo in this event ");
    return StatusCode::SUCCESS;
  }
  ATH_MSG_DEBUG("Not empty pad container in this event ");

  // obtain a list of PADs (RDOs) to be processed
  std::vector<IdentifierHash> rdoHashVec;
  rdoHashVec.reserve(13 * robIdsToBeDecoded.size());  // most ROBs have 13 RDOs, some have less
  ATH_CHECK(rpcCabling->giveRDO_fromROB(robIdsToBeDecoded, rdoHashVec));

  // start here to process the RDOs
  bool processingetaview = true;
  bool processingphiview = false;
  if (!m_solvePhiAmbiguities){
    processingetaview = false;
  }
  while (processingetaview || processingphiview || (!m_solvePhiAmbiguities)) {
    int ipad{0}, nPrepRawData{0}, nPhiPrepRawData{0}, nEtaPrepRawData{0};
    if (processingphiview){
      state.m_ambiguousCollections.clear();
    }
    ATH_MSG_DEBUG("*** Processing "<<(processingetaview? "eta" : "phi")<<" view ");

    // seeded decoding (for full scan, use the hashId-based method)
    ATH_MSG_DEBUG("Start loop over pads hashes - seeded mode ");

    for (const IdentifierHash& padHashId : rdoHashVec) {
      const RpcPad* rdoColl = rdoContainerHandle->indexFindPtr(padHashId);
      if (!rdoColl) {
        ATH_MSG_DEBUG("Requested pad with online id "<< padHashId << " not found in the rdoContainerHandle.");
        continue;
      }
      ++ipad;
      ATH_MSG_DEBUG("A new pad here n."<< ipad << ", online id " << rdoColl->identifyHash()
                    << ", with " << rdoColl->size() << " CM inside ");
      ATH_CHECK(processPad(ctx, state, rdoColl, processingetaview,
                           processingphiview, nPrepRawData, rdoHashVec, false));
    }

    if (processingetaview) {
      processingetaview = false;
      processingphiview = true;
      nEtaPrepRawData = nPrepRawData;
      ATH_MSG_DEBUG("*** " << nEtaPrepRawData << " eta PrepRawData registered");
    } else {
      processingphiview = false;
      nPhiPrepRawData = nPrepRawData - nEtaPrepRawData;
      ATH_MSG_DEBUG("*** " << nPhiPrepRawData << " phi PrepRawData registered");
    }
    if (!m_solvePhiAmbiguities) {
      ATH_MSG_DEBUG("*** " << nPrepRawData << " PrepRawData registered");
      break;
    }
  }

  ATH_MSG_DEBUG("*** Final Cleanup ");
  
  return StatusCode::SUCCESS;
  
}

StatusCode RpcRdoToPrepDataToolMT::processPad(const EventContext& ctx, State& state, const RpcPad* rdoColl,
                                              bool& processingetaview, bool& processingphiview, int& nPrepRawData,
                                              const std::vector<IdentifierHash>& idVect,     
                                              bool doingSecondLoopAmbigColls) const {

  const RpcIdHelper& idHelper = m_idHelperSvc->rpcIdHelper();

  std::unordered_set<IdentifierHash>& ambiguousCollections{state.m_ambiguousCollections};
  ATH_MSG_DEBUG("***************** Start of processPad eta/phiview "<< processingetaview << "/" << processingphiview);
  //{processPad
  // Get pad online id and sector id
  uint16_t padId = rdoColl->onlineId();
  uint16_t sectorId = rdoColl->sector();
  ATH_MSG_DEBUG("***************** for Pad online Id "<< padId << " m_logic sector ID " << sectorId);

  // Create an RPC PrepDataCollection
  Identifier oldId{}, oldIdTrg{};
  ATH_MSG_VERBOSE("Init pointer to RpcPrepDataCollection ");
  RpcPrepDataCollection* collection{nullptr};
  RpcCoinDataCollection* collectionTrg{nullptr};
  IdentifierHash rpcHashId{0};

  SG::ReadCondHandle rpcCabling{m_rpcReadKey, ctx};

  // For each pad, loop on the coincidence matrices
  int icm = 0;
  for (const RpcCoinMatrix* coinMat : *rdoColl) {
    ++icm;
    bool etaview = !m_isMC;
    bool highPtCm = false;
    // Get CM online Id
    uint16_t cmaId = coinMat->onlineId();
    ATH_MSG_DEBUG("A new CM here n. "<< icm << " CM online ID " << cmaId<< " with n. of hits =  " << coinMat->size()
              <<", empty: "<<coinMat->empty());
    ATH_MSG_DEBUG((cmaId < 4? "low": "high") <<"  pt ");
    if (cmaId < 2) {
      etaview = m_isMC;
    } else if (cmaId >=4) {
      highPtCm = true;
      if (cmaId < 6) {
        etaview = m_isMC;
      } 
    }
    ATH_MSG_DEBUG(" eta view = " << etaview<<", processingetaview: "<<processingetaview
                <<", processingphiview: "<<processingphiview);

    if (processingetaview && !etaview){
      continue;
    } else if (processingphiview && etaview) {
      continue;
    }
    if (coinMat->empty()) {
       ATH_MSG_DEBUG("Empty CM");
    }
    // For each CM, loop on the fired channels
    int idata = 0;
    for (RpcCoinMatrix::const_iterator itD = coinMat->begin(); itD != coinMat->end(); ++itD) {
      const RpcFiredChannel* rpcChan{*itD}; 
      idata++;
      // trigger related quantities
      unsigned short threshold = 99;
      unsigned short overlap = 99;

      // flags defining the processing mode of this hit
      bool solvePhiAmb_thisHit = m_solvePhiAmbiguities;
      bool reduceCablOvl_thisHit = m_reduceCablingOverlap;

      ATH_MSG_DEBUG("A new CM Hit " << idata);
      ATH_MSG_DEBUG("RpcFiredChannel: bcid " << rpcChan->bcid() << " time " << rpcChan->time() 
                 << " ijk "<< rpcChan->ijk() <<" ch " << ( rpcChan->ijk() < 7 ? rpcChan->channel() :  -1));
      // check if trigger hit
      // select the cases: ijk = 0 and high p, ijk= 6, ijk=7
      bool triggerHit = false;
      bool toSkip = false;
      processTriggerHitHypothesis(itD, coinMat->end(), highPtCm, triggerHit, threshold, overlap, toSkip);
      if (toSkip) {
        continue;
      }  
      if (triggerHit) {
        // here ijk = 6 or ijk = 0 in high pt cm
        // keep all pivot + trigger info (even if duplicated [should never
        // happen, for pivot hits])
        solvePhiAmb_thisHit = false;
        reduceCablOvl_thisHit = false;
        ATH_MSG_DEBUG("RpcFiredChannel: it's a triggerHit or a lowPt coinc. in a high pt CM \n"
                    << "    ijk = " << rpcChan->ijk() << " isHighPtCM " << highPtCm
                    << " thr/ovl = " << threshold << "/" << overlap);
      }

      // here decode (get offline ids for the online indices of this hit)
      double time = 0.;
      std::vector<Identifier> digitVec{m_rpcRdoDecoderTool->getOfflineData(rpcChan, sectorId, padId, 
                                                                           cmaId, time, rpcCabling.cptr())};
      time += m_timeShift;

      int nMatchingEtaHits{0}, nDuplicatePhiHits{0};
      bool unsolvedAmbiguity{false}, notFinished{true};
      // allow for 2 iterations in case there are phi digits without matching
      // eta (eta inefficiency) all eta digits, not already recorded, will be
      // registered as PrepRawData and all phi digits, not yet recorded and with
      // a eta digit in the same module and gap, will produce a PrepRawData. Phi
      // digits without a eta match will not be recorded at the first iteration.
      // If all phi digits do not have a eta match, they will be all recorded as
      // PrepRawData in the second iteration (the ambiguity will remain
      // unsolved)
      while (notFinished) {
        // Loop on the digits corresponding to the fired channel
        ATH_MSG_DEBUG("size of the corresponding list of ID = " << digitVec.size());
        if (digitVec.empty()) {
          ATH_MSG_DEBUG("going to next CM hit");
          notFinished = false;
          continue;
        }
        for (const Identifier& channelId : digitVec) {
          // Prepare the prepdata for this identifier
          // channel Id
          rpcHashId = m_idHelperSvc->moduleHash(channelId);
          const Identifier parentId = idHelper.parentID(channelId);

          // There is some ambiguity in the channel/sectorId's, so need to
          // explicitly filter out hashIDs outside of the RoI in seeded decoding
          // mode
          if (!idVect.empty() && std::find(idVect.begin(), idVect.end(), rpcHashId) == idVect.end()) {
              continue;
          }
          ATH_MSG_DEBUG("CM Hit decoded into offline Id "<< m_idHelperSvc->toString(channelId) << " time "<< time);
          ATH_MSG_DEBUG(" oldID = " << m_idHelperSvc->toString(oldId) <<" oldIDtrg = " << m_idHelperSvc->toString(oldIdTrg));
          bool hasAMatchingEtaHit = 0;
          // current collection has Id "parentId"; get it from the container !
          if (triggerHit) {
            if ((oldIdTrg != parentId) || !collectionTrg) {
              // Get collection from IDC if it exists, or create it and add it
              // if not.
              ATH_MSG_DEBUG(" Looking/Creating a collection with ID = "<< m_idHelperSvc->toString(parentId) 
                         << " hash = " << rpcHashId);
              collectionTrg = state.getCoinCollection(parentId);
              oldIdTrg = parentId;
              ATH_MSG_DEBUG(" Resetting oldIDtrg to current parentID = "<< m_idHelperSvc->toString(oldIdTrg));
            }
          } else if ((oldId != parentId) || !collection) {
            // Get collection from IDC if it exists, or create it and add it if
            // not.
            ATH_MSG_DEBUG(" Looking/Creating a collection with ID = "<< m_idHelperSvc->toString(parentId) << " hash = "<< rpcHashId);
            collection = state.getPrepCollection(parentId);
            oldId = parentId;
            ATH_MSG_DEBUG(" Resetting oldID to current parentID = "<< m_idHelperSvc->toString(oldId));
          }

          // check if the data has already been recorded
          // (if you want to reduce the redundancy due to cabling overlap and if
          // the collection is not empty)
          bool duplicate = false;
          if (reduceCablOvl_thisHit) {
            ATH_MSG_VERBOSE("Check for duplicates in coll. with size "<< collection->size());
            for (RpcPrepData* rpc : *collection) {
              const Identifier existId{rpc->identify()};
              if (channelId == existId &&
                  std::abs(time - rpc->time()) < m_overlap_timeTolerance) {
                duplicate = true;
                hasAMatchingEtaHit = false;  // we don't want to increment the
                                              // number of strips with
                // a matching eta due to a cabling overlap
                ATH_MSG_VERBOSE("Duplicated RpcPrepData(not recorded) = " << m_idHelperSvc->toString(channelId));
                float previous_time = rpc->time();
                // choose the smallest time within timeTolerance
                if (time < previous_time) {
                  rpc->m_time = time;
                  ATH_MSG_DEBUG("time of the prd previously stored is now updated with "
                      << "current hit time: "<< previous_time << " -> " << rpc->time());
                }
                break;  // this break is why we cannot have
                        //        solvePhiAmb_thisHit = true and
                        //        reduceCablOvl_thisHit= false
              }


              if (processingphiview && solvePhiAmb_thisHit && !unsolvedAmbiguity && !idHelper.measuresPhi(existId) && 
                  m_idHelperSvc->gasGapId(existId) == m_idHelperSvc->gasGapId(channelId) &&
                std::abs(time - rpc->time()) < m_etaphi_coincidenceTime) {
                hasAMatchingEtaHit = true;
                ATH_MSG_VERBOSE("There's a matching eta hit with id "<< m_idHelperSvc->toString(existId));
                // here there can be a break ? NO, we need to
                // keep looping in order to check
                //  if this preprawdata has been already
                //  recorded (due to cabling overlaps)
              }
            }
            if (hasAMatchingEtaHit)
              nMatchingEtaHits++;  // Number of phi strips (possibly
                                    // corresponding to this CM hit)
            // with a matching eta
            if (processingphiview && duplicate)
              nDuplicatePhiHits++;  // Number of phi strips (possibly
                                    // corresponding to this CM hit)
                                    // already in the collection
          }                           // end of if reduceCablingOverlap

          if (solvePhiAmb_thisHit && !etaview){
            ATH_MSG_VERBOSE("nMatchingEtaHits = " << nMatchingEtaHits
                         << " hasAMatchingEtaHit = "<< hasAMatchingEtaHit);
          }
          if (duplicate) {
            // this hit was already recorded
            ATH_MSG_DEBUG("digit already in the collection ");
            continue;
          }
          ATH_MSG_VERBOSE(" solvePhiAmb_thisHit: "<<solvePhiAmb_thisHit<<", processingetaview:"<<processingetaview
                        <<", processingphiview: "<<processingphiview<<", hasAMatchingEtaHit: "<<hasAMatchingEtaHit
                        <<", unsolvedAmbiguity: "<< unsolvedAmbiguity);
          if ( !solvePhiAmb_thisHit || processingetaview ||
               (processingphiview && (hasAMatchingEtaHit || unsolvedAmbiguity))) {
              if (unsolvedAmbiguity) {
                if (idVect.empty()) {  // full-scan mode
                  ATH_MSG_DEBUG("storing data even if unsolvedAmbiguity");
                } else {
                  // if in RoI mode and the collection was not requested in this
                  // event, add it to ambiguousCollections
                  ATH_MSG_DEBUG("unsolvedAmbiguity is true, adding collection with hash = "
                      << rpcHashId << " to ambiguous collections vector");
                  const auto& decode{state.m_decodedOfflineHashIds};
                  if (!decode.empty() && decode.find(rpcHashId) == decode.end()) {
                    ambiguousCollections.insert(rpcHashId);
                    ATH_MSG_DEBUG("collection not yet processed; added to ambiguous "
                                <<"collection vector; going to the next offline channel ID");
                    continue;  // go to the next possible offline channel ID
                  } else if (!doingSecondLoopAmbigColls) {
                    ambiguousCollections.insert(rpcHashId);
                    ATH_MSG_DEBUG("collection already processed and doingSecondLoopAmbigColls=false; added to ambiguous "
                                <<"collection vector; going to the next offline channel ID");
                    continue;
                  } else {
                    ATH_MSG_DEBUG("collection already processed and doingSecondLoopAmbigColls=true; trying to store data even if unsolvedAmbiguity");
                  }
                }
              }
              SG::ReadCondHandle muDetMgr{m_muDetMgrKey, ctx};
              const RpcReadoutElement* descriptor = muDetMgr->getRpcReadoutElement(channelId);

              // here check validity
              // if invalid, reset flags
              if (!descriptor->containsId(channelId)) {
                hasAMatchingEtaHit = false;
                duplicate = false;
                if (idHelper.stationNameString(idHelper.stationName(channelId)) == "BOG"){
                  ATH_MSG_DEBUG("Identifier from the cabling service <"<< m_idHelperSvc->toString(channelId)
                      << "> inconsistent with the geometry of detector element <"
                      << m_idHelperSvc->toStringDetEl(descriptor->identify())
                      << ">  =>>ignore this hit /// there are unmasked channels in BOG");
                }else{
                  ATH_MSG_WARNING("Identifier from the cabling service <"<< m_idHelperSvc->toString(channelId)
                                << "> inconsistent with the geometry of detector element <"
                                << m_idHelperSvc->toStringDetEl(descriptor->identify())<< ">  =>>ignore this hit");
                }
                continue;
              }

              //
              // Global position
              Amg::Vector3D tempGlobalPosition = descriptor->stripPos(channelId);
              ATH_MSG_VERBOSE("RPC RDO->PrepRawdata: global position "<<Amg::toString(tempGlobalPosition));
              // Local position
              Amg::Vector2D pointLocPos{Amg::Vector2D::Zero()};
              descriptor->surface(channelId).globalToLocal(tempGlobalPosition, tempGlobalPosition, pointLocPos);

              // List of Digits in the cluster (self)
              std::vector<Identifier> identifierList{channelId};

              // width of the cluster (self)
              float stripWidth = descriptor->StripWidth(m_idHelperSvc->rpcIdHelper().measuresPhi(channelId));

              // Error matrix
              double errPos = stripWidth / std::sqrt(12.0);
              Amg::MatrixX mat(1, 1);
              mat.setIdentity();
              mat *= errPos * errPos;
              // check if this is a triggerINFO rather then a real hit
              // Create a new PrepData
              int ambiguityFlag = 0;
              if (solvePhiAmb_thisHit) {
                if (processingetaview){
                  ambiguityFlag = 1;
                }
                if (unsolvedAmbiguity){
                  ambiguityFlag = digitVec.size();
                } else if (hasAMatchingEtaHit){
                  ambiguityFlag = nMatchingEtaHits;
                }
              }

              // correct prd time from cool db
              if (m_RPCInfoFromDb) {
                SG::ReadCondHandle readHandle{m_readKey, ctx};
                std::optional<double> StripTimeFromCool = readHandle->getStripTime(channelId);
                if (StripTimeFromCool) {
                  time -= (*StripTimeFromCool);
                }                
              }

              if (triggerHit) {
                ATH_MSG_DEBUG("producing a new  RpcCoinData");

                auto newCoinData = std::make_unique<RpcCoinData>(channelId, rpcHashId, 
                                                                std::move(pointLocPos), std::move(identifierList),
                                                                std::move(mat), descriptor, time, 
                                                                ambiguityFlag, rpcChan->ijk(), threshold, overlap, cmaId, padId, sectorId,
                                                                !(highPtCm));

                // record the new data in the collection
                ATH_MSG_DEBUG(" Adding RpcCoinData @ "<< newCoinData.get() << " to collection "
                              << m_idHelperSvc->toString(collectionTrg->identify()));

                newCoinData->setHashAndIndex(collectionTrg->identifyHash(),
                                             collectionTrg->size());
                collectionTrg->push_back(std::move(newCoinData));
              }  // end of to be stored now for RpcCoinData
              else {
                ATH_MSG_DEBUG("producing a new  RpcPrepData with "
                              << "ambiguityFlag = " << ambiguityFlag);

                auto newPrepData = std::make_unique<RpcPrepData>(channelId, rpcHashId,std::move(pointLocPos), 
                                                                std::move(identifierList),
                                                                std::move(mat), descriptor, time, ambiguityFlag);

                // record the new data in the collection
                ATH_MSG_DEBUG(" Adding digit @ "<< newPrepData.get() << " to collection "
                              << m_idHelperSvc->toString(collection->identify()));

                newPrepData->setHashAndIndex(collection->identifyHash(),
                                             collection->size());
                collection->push_back(std::move(newPrepData));
                // here one should reset ambiguityFlag for the prepdata
                // registered before the current one (from the same RDO hit) if
                // nMatchingEtaHits > 1
                nPrepRawData++;                
              }
            }    // end of to be stored now
        }  // end loop over possible offline identifiers corresponding to this
           // CM hit
        ATH_MSG_VERBOSE("processingphiview:"<<processingphiview<<", nMatchingEtaHits:"<<nMatchingEtaHits
                <<", nDuplicatePhiHits, unsolvedAmbiguity: "<<unsolvedAmbiguity<<", solvePhiAmb_thisHit : "
                << solvePhiAmb_thisHit);
        if ((processingphiview && (nMatchingEtaHits == 0)) && (nDuplicatePhiHits == 0) && 
            (!unsolvedAmbiguity) && (solvePhiAmb_thisHit)) {
          unsolvedAmbiguity = true;
          // no eta hits matching any phi digit
          // loop once again and store all phi digits potentially generating
          // this CM hit
          ATH_MSG_DEBUG("No eta prepData matching any phi hit from this CM hit \n"
              << "loop once again and store all phi digits potentially "
                 "generating this CM hit");
        } else if (unsolvedAmbiguity)
          notFinished = false;
        else
          notFinished = false;
      }  // end of not finished
    }    // end loop over CM hits
  }      // end loop over CMs

  ATH_MSG_DEBUG("***************** Stop  of processPad eta/phiview "
                << processingetaview << "/" << processingphiview
                << "***************** for Pad online Id " << padId
                << " m_logic sector ID " << sectorId);

  return StatusCode::SUCCESS;
}

StatusCode RpcRdoToPrepDataToolMT::processNrpcRdo(const EventContext& ctx,
                                                        State& state) const {
  if (m_rdoNrpcContainerKey.empty()) {
    ATH_MSG_DEBUG("The NRPC processing is disabled.");
    return StatusCode::SUCCESS;
  }

  ATH_MSG_DEBUG("Retrieving Nrpc RDO container from the store");
  SG::ReadHandle rdoNrpcContainerHandle{m_rdoNrpcContainerKey, ctx};
  if (!rdoNrpcContainerHandle.isPresent()) {
    ATH_MSG_ERROR("Retrieval of NRPC RDO " << m_rdoNrpcContainerKey.fullKey()
                                           << " container failed !");
    return StatusCode::FAILURE;
  }

  if (rdoNrpcContainerHandle->empty()) {
    // empty NRPC RDO container - no nrpc rdo in this event
    ATH_MSG_DEBUG("Empty NRPC RDO container - no nrpc rdo in this event ");
    return StatusCode::SUCCESS;
  }
  ATH_MSG_DEBUG("Not empty NRPC RDO container in this event ");

  SG::ReadCondHandle readCdo{m_nRpcCablingKey, ctx};
  if (!readCdo.isValid()) {
    ATH_MSG_ERROR("Could not retrieve " << m_nRpcCablingKey.fullKey()
                                        << " from the conditions store");
    return StatusCode::FAILURE;
  }
  /// No need to check for its validity. It's done at other places for sure
  SG::ReadCondHandle muDetMgr{m_muDetMgrKey, ctx};

  for (const xAOD::NRPCRDO* nrpcrdo : *rdoNrpcContainerHandle) {
    /// Convert from online to offline cabling
    RpcCablingData translateCache{};
    translateCache.subDetector = nrpcrdo->subdetector();
    translateCache.boardSector = nrpcrdo->boardsector();
    translateCache.board = nrpcrdo->board();
    translateCache.channelId = nrpcrdo->channel();
    Identifier chanId{};
    if (!readCdo->getOfflineId(translateCache, msgStream()) ||
        !readCdo->convert(translateCache, chanId, false)) {
      ATH_MSG_FATAL("Failed to retrieve the offline Identifier");
      return StatusCode::FAILURE;
    }

    RpcPrepDataCollection* collection = state.getPrepCollection(chanId);

    const RpcReadoutElement* descriptor = muDetMgr->getRpcReadoutElement(chanId);

    // List of Digits in the cluster (self)
    std::vector<Identifier> identifierList{chanId};
    // Global position
    const Amg::Vector3D stripPos{descriptor->stripPos(chanId)};
    ATH_MSG_DEBUG("RPC RDO->PrepRawdata " << m_idHelperSvc->toString(chanId)
                                          << " global position "
                                          << Amg::toString(stripPos, 2));
    // Local position
    Amg::Vector2D pointLocPos{Amg::Vector2D::Zero()};
    descriptor->surface(chanId).globalToLocal(stripPos, stripPos, pointLocPos);
    // width of the cluster (self)

    const double stripWidth = descriptor->StripWidth(
        m_idHelperSvc->rpcIdHelper().measuresPhi(chanId));
    // Error matrix
    const double errPos = stripWidth / std::sqrt(12.0);
    Amg::MatrixX mat(1, 1);
    mat.setIdentity();
    mat *= errPos * errPos;

    int ambiguityFlag = 0;  // Ambiguity flag not checked for BIS RPCs

    const float time = nrpcrdo->time();
    /// That needs to be parsed to the constructor of the PRD somehow...
    const float timeoverthr = nrpcrdo->timeoverthr();

    const IdentifierHash rpcHashId = m_idHelperSvc->moduleHash(chanId);
    auto newPrepData = std::make_unique<RpcPrepData>(chanId, rpcHashId, pointLocPos, identifierList, mat,
                                                     descriptor, time, timeoverthr, 0, ambiguityFlag);

    newPrepData->setHashAndIndex(collection->identifyHash(),
                                 collection->size());
    collection->push_back(std::move(newPrepData));
  }
  return StatusCode::SUCCESS;
}

void RpcRdoToPrepDataToolMT::processTriggerHitHypothesis(RpcCoinMatrix::const_iterator itD, 
                                                         RpcCoinMatrix::const_iterator itD_end,
                                                         bool highPtCm, bool& triggerHit, 
                                                         unsigned short& threshold, unsigned short& overlap,
                                                         bool& toSkip) const {
  toSkip = false;
  const RpcFiredChannel* rpcChan = (*itD);
  if ((highPtCm && rpcChan->ijk() < 2) || (rpcChan->ijk() > 5)) {
    ATH_MSG_VERBOSE("RpcFiredChannel: it's a trigger hit");
    triggerHit = true;

    // triggerHit
    if (!m_producePRDfromTriggerWords) {
      // skip if not storing the trigger info
      toSkip = true;
      return;
    }
    if (rpcChan->ijk() == 7) {
      // the info in ijk 7 refer to the previous CM hit with ijk  6 => skip
      toSkip = true;
      return;
    }
    if (rpcChan->ijk() == 6) {
      // look for the subsequent ijk  7 to define threshold and overlap
      ATH_MSG_VERBOSE("This hit: ijk = " << rpcChan->ijk() << "in "<<(highPtCm ? "high" : "low")
                      <<" pT CM, bcid is " << rpcChan->bcid()<< " time is " << rpcChan->time()
                                           << " ch " << rpcChan->channel());
      RpcCoinMatrix::const_iterator itDnext = itD + 1;
      while (itDnext != itD_end) {
        const RpcFiredChannel* rpcChanNext = (*itDnext);
        ATH_MSG_VERBOSE("Next hit: ijk = " << rpcChanNext->ijk() << "in "<<(highPtCm ? "high" : "low")
                       <<" pT CM, bcid is " << rpcChan->bcid() << " time is " << rpcChanNext->time()
                      <<", ch: "<<( rpcChanNext->ijk() < 7? rpcChanNext->channel() : -1));

        if (rpcChanNext->ijk() == 7) {
          if (rpcChanNext->bcid() == rpcChan->bcid() && rpcChanNext->time() == rpcChan->time()) {
            ATH_MSG_VERBOSE("bdid/tick match; assigning thr/overlap  = " << rpcChanNext->thr() << "/" << rpcChanNext->ovl());
            threshold = rpcChanNext->thr();
            overlap = rpcChanNext->ovl();
          } else {
            ATH_MSG_WARNING("ijk =7 after a ijk = 6 BUT bdid/tick don't match - will not assign threshold/overlap ");
          }
          break;
        } else {
          if (rpcChanNext->ijk() == 6) {
            ++itDnext;
            // std::cout<<"next has ijk 6; try next to next"<<std::endl;
          } else {
            ATH_MSG_WARNING("RPC cm hit with ijk = 6 not followed by ijk = 6 or 7 - will not assign threshold / overlap");
            break;
          }
        }
      }
    }
  } else {
    triggerHit = false;
    return;
  }
  ATH_MSG_VERBOSE("RPC trigger hit; ijk = "
                  << rpcChan->ijk() << " threshold / overlap = " << threshold
                  << "/" << overlap);
}
}
