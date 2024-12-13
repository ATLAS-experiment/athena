/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "RpcRdoToPrepDataToolMT.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonReadoutGeometry/RpcReadoutElement.h"
#include "MuonTrigCoinData/RpcCoinDataContainer.h"
#include "TrkSurfaces/Surface.h"
#include "MuonCnvToolInterfaces/IDC_Helper.h"
#include "MuonRPC_CnvTools/IRPC_RDO_Decoder.h"

using namespace MuonGM;
using namespace Trk;

/////////////////////////////////////////////////////////////////////////////
Muon::RpcRdoToPrepDataToolMT::State::State(const RpcIdHelper& idHelper) :
   m_rpcIdHelper{idHelper} {

}

Muon::RpcPrepDataCollection* Muon::RpcRdoToPrepDataToolMT::State::getPrepCollection(const IdentifierHash& rpcHashId, MsgStream& msg) {
    std::unique_ptr<Muon::RpcPrepDataCollection>& coll = m_rpcPrepDataCollections[rpcHashId];
    if (!coll) {
        Identifier id{0};
        if (m_rpcIdHelper.get_id(rpcHashId, id, &m_modContext)) {
            msg << MSG::ERROR << "Module hash creation failed. " << id << endmsg;
            return nullptr;
        }
        coll = std::make_unique<Muon::RpcPrepDataCollection>(rpcHashId);
        coll->setIdentifier(id);
    }
    return coll.get();
}
Muon::RpcCoinDataCollection* Muon::RpcRdoToPrepDataToolMT::State::getCoinCollection(const IdentifierHash& rpcHashId, MsgStream& msg) {
    std::unique_ptr<Muon::RpcCoinDataCollection>& coll = m_rpcCoinDataCollections[rpcHashId];
    if (!coll) {
        Identifier id{0};
        if (m_rpcIdHelper.get_id(rpcHashId, id, &m_modContext)) {
            msg << MSG::ERROR << "Module hash creation failed. " << id << endmsg;
            return nullptr;
        }
        coll = std::make_unique<Muon::RpcCoinDataCollection>(rpcHashId);
        coll->setIdentifier(id);
    }
    return coll.get();
}

///
Muon::RpcRdoToPrepDataToolMT::RpcRdoToPrepDataToolMT(const std::string& type, const std::string& name, const IInterface* parent) :
    base_class(type, name, parent){}

//___________________________________________________________________________
StatusCode Muon::RpcRdoToPrepDataToolMT::initialize() {
    // perform necessary one-off initialization

    ATH_MSG_INFO("properties are ");
    ATH_MSG_INFO("produceRpcCoinDatafromTriggerWords " << m_producePRDfromTriggerWords);
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
    ATH_CHECK(m_nRpcCablingKey.initialize(!m_rdoNrpcContainerKey.empty()));  // If we don't configure the NRPC RDO key, the cabling is needed either.
    ATH_CHECK(m_rpcPrepDataContainerKey.initialize());
    ATH_CHECK(m_rpcCoinDataContainerKey.initialize());
    ATH_CHECK(m_eventInfo.initialize());
    ATH_CHECK(m_muDetMgrKey.initialize());
    ATH_CHECK(m_prdContainerCacheKey.initialize(SG::AllowEmpty));
    ATH_CHECK(m_coindataContainerCacheKey.initialize(SG::AllowEmpty));
    return StatusCode::SUCCESS;
}
StatusCode Muon::RpcRdoToPrepDataToolMT::loadProcessedChambers(const EventContext& ctx, State& state) const {
    if (!m_prdContainerCacheKey.key().empty()) {
        SG::UpdateHandle<RpcPrepDataCollection_Cache> update{m_prdContainerCacheKey, ctx};
        if (!update.isValid()) {
            ATH_MSG_FATAL("Invalid UpdateHandle " << m_prdContainerCacheKey.key());
            return StatusCode::FAILURE;
        }
        state.m_prepDataCont = std::make_unique<Muon::RpcPrepDataContainer>(update.ptr());
        for (const RpcPrepDataCollection* coll : *state.m_prepDataCont) {
                state.m_decodedOfflineHashIds.insert(coll->identifyHash());
        }
     } else state.m_prepDataCont = std::make_unique<Muon::RpcPrepDataContainer>(m_idHelperSvc->rpcIdHelper().module_hash_max());

    if (m_coindataContainerCacheKey.key().empty()) {
        // without the cache we just record the container
        state.m_coinDataCont = std::make_unique<Muon::RpcCoinDataContainer>(m_idHelperSvc->rpcIdHelper().module_hash_max());
    } else {
        // use the cache to get the container
        SG::UpdateHandle<RpcCoinDataCollection_Cache> update{m_coindataContainerCacheKey, ctx};
        if (!update.isValid()) {
            ATH_MSG_FATAL("Invalid UpdateHandle " << m_coindataContainerCacheKey.key());
            return StatusCode::FAILURE;
        }
        state.m_coinDataCont = std::make_unique<Muon::RpcCoinDataContainer>(update.ptr());
        for (const RpcCoinDataCollection* coll : *state.m_coinDataCont) {
                state.m_decodedOfflineHashIds.insert(coll->identifyHash());
        }        
    }

    return StatusCode::SUCCESS;
}
/// This code is thread-safe as we will propagate local thread collection contents to a thread-safe one
StatusCode Muon::RpcRdoToPrepDataToolMT::decode(const EventContext&ctx,
                                                std::vector<IdentifierHash>& idVect, 
                                                std::vector<IdentifierHash>& selectedIdVect) const {
    ATH_MSG_DEBUG("Calling Core decode function from MT decode function (hash vector)");
    State state{m_idHelperSvc->rpcIdHelper()};
    ATH_CHECK(loadProcessedChambers(ctx, state));

    ATH_CHECK(decodeImpl(ctx, state, idVect, selectedIdVect, true));
    ATH_CHECK(processNrpcRdo(ctx, state) );
    ATH_MSG_DEBUG("Core decode processed in MT decode (hash vector)");
    
    ATH_CHECK(transferAndRecordPrepData(ctx, state));
    ATH_CHECK(transferAndRecordCoinData(ctx, state));

    return StatusCode::SUCCESS;
}

/// This code is thread-safe as we will propagate local thread collection contents to a thread-safe one
StatusCode Muon::RpcRdoToPrepDataToolMT::decode(const EventContext& ctx,
                                                const std::vector<uint32_t>& robIds) const {
    ATH_MSG_DEBUG("Calling Core decode function from MT decode function (ROB vector)");
    State state{m_idHelperSvc->rpcIdHelper()};
    ATH_CHECK(loadProcessedChambers(ctx, state));

    ATH_CHECK(decodeImpl(ctx, state,  robIds, true));
    ATH_CHECK(processNrpcRdo(ctx, state) );
    ATH_MSG_DEBUG("Core decode processed in MT decode (ROB vector)");

    ATH_CHECK(transferAndRecordPrepData(ctx, state));
    ATH_CHECK(transferAndRecordCoinData(ctx, state));

    return StatusCode::SUCCESS;
}
StatusCode Muon::RpcRdoToPrepDataToolMT::provideEmptyContainer(const EventContext& ctx) const {
   State state{m_idHelperSvc->rpcIdHelper()};
   ATH_CHECK(loadProcessedChambers(ctx, state));
   ATH_CHECK(transferAndRecordPrepData(ctx, state));
   ATH_CHECK(transferAndRecordCoinData(ctx, state));
   return StatusCode::SUCCESS;
}

StatusCode Muon::RpcRdoToPrepDataToolMT::transferAndRecordPrepData(const EventContext& ctx, State& state) const {
    
    for (auto& [hash, collection] : state.m_rpcPrepDataCollections) {
        if (!collection || collection->empty()) continue;       
        // If not present, get a write lock for the hash and move collection
        RpcPrepDataContainer::IDC_WriteHandle lock = state.m_prepDataCont->getWriteHandle(hash);
        if (lock.alreadyPresent()) {
           ATH_MSG_DEBUG("RpcPrepDataCollection already contained in IDC " <<m_idHelperSvc->toString(collection->identify()));
           continue;          
        }
        ATH_CHECK(lock.addOrDelete(std::move(collection)));
        ATH_MSG_DEBUG("PRD hash " << hash << " has been moved to cache container");
    }
    state.m_rpcPrepDataCollections.clear();

    if (msgLvl(MSG::DEBUG)) {
        for (const auto& [hash, ptr] : state.m_prepDataCont->GetAllHashPtrPair()) {
            ATH_MSG_DEBUG("Contents of CONTAINER in this view : " << hash);
        }
    }
    SG::WriteHandle<Muon::RpcPrepDataContainer> rpcPRDHandle{m_rpcPrepDataContainerKey, ctx};
    ATH_CHECK(rpcPRDHandle.record(std::move(state.m_prepDataCont)));
    ATH_MSG_DEBUG("Created container " << m_rpcPrepDataContainerKey.key());
    return StatusCode::SUCCESS;
}

StatusCode Muon::RpcRdoToPrepDataToolMT::transferAndRecordCoinData(const EventContext& ctx, State& state) const {
    if (!m_producePRDfromTriggerWords) { return StatusCode::SUCCESS; }

    // Take localContainer and transfer contents to rpcCoinHandle
    for (auto& [hash, collection] : state.m_rpcCoinDataCollections) {
        if (!collection || collection->empty()) continue;       
        // If not present, get a write lock for the hash and move collection
        RpcCoinDataContainer::IDC_WriteHandle lock = state.m_coinDataCont->getWriteHandle(hash);
        if (lock.alreadyPresent()) {
           ATH_MSG_DEBUG("RpcCoinDataCollection already contained in IDC " <<m_idHelperSvc->toString(collection->identify()));
           continue;            
        }
        ATH_CHECK(lock.addOrDelete(std::move(collection)));
        ATH_MSG_DEBUG("Coin hash " << hash << " has been moved to cache container");
    }
    state.m_rpcCoinDataCollections.clear();
    if (msgLvl(MSG::DEBUG)) {
        for (const auto& [hash, ptr] : state.m_coinDataCont->GetAllHashPtrPair()) { ATH_MSG_DEBUG("Contents of LOCAL in this view : " << hash); }
    }
    SG::WriteHandle<Muon::RpcCoinDataContainer> rpcCoinHandle{m_rpcCoinDataContainerKey, ctx};
    ATH_CHECK(rpcCoinHandle.record(std::move(state.m_coinDataCont)));

    ATH_MSG_DEBUG("Created container " << m_rpcCoinDataContainerKey.key());
    // For additional information on the contents of the cache-based container, this function can be used
    // printMTCoinData (*rpcCoinHandle);

    return StatusCode::SUCCESS;
}

void Muon::RpcRdoToPrepDataToolMT::printMTPrepData(const Muon::RpcPrepDataContainer& prepData) const {
    msg(MSG::INFO) << "********************************************************************************************************" << endmsg;
    msg(MSG::INFO) << "***************** Listing RpcPrepData collections content **********************************************" << endmsg;

    if (prepData.empty()) msg(MSG::INFO) << "No RpcPrepRawData collections found" << endmsg;

    int ncoll{0}, ict{0}, ictphi{0}, icteta{0}, icttrg{0};
    msg(MSG::INFO) << "--------------------------------------------------------------------------------------------" << endmsg;
    for (const Muon::RpcPrepDataCollection* rpcColl : prepData) {
        if (!rpcColl->empty()) {
            msg(MSG::INFO) << "PrepData Collection ID " << m_idHelperSvc->toString(rpcColl->identify()) << endmsg;            
            int icc{0}, iccphi{0}, icceta{0};
            for (const RpcPrepData* rpc : *rpcColl) {
                icc++;
                ict++;
                if (m_idHelperSvc->rpcIdHelper().measuresPhi(rpc->identify())) {
                    iccphi++;
                    ictphi++;
                    
                } else {
                    icceta++;
                    icteta++;
                }
                msg(MSG::INFO) << ict << " in this coll. " << icc
                               << " prepData id = " << m_idHelperSvc->toString(rpc->identify()) << " time "
                               << rpc->time() << " ambiguityFlag " << rpc->ambiguityFlag() << endmsg;
            }
            ncoll++;
            msg(MSG::INFO) << "*** Collection " << ncoll << " Summary: " << iccphi << " phi hits / " << icceta << " eta hits " << endmsg;
            msg(MSG::INFO) << "--------------------------------------------------------------------------------------------" << endmsg;
        }
    }
    msg(MSG::INFO) << "*** Event  Summary: " << ncoll << " Collections / " << icttrg << " trigger hits / " << ictphi << " phi hits / "
                   << icteta << " eta hits " << endmsg;
    msg(MSG::INFO) << "--------------------------------------------------------------------------------------------" << endmsg;
}

void Muon::RpcRdoToPrepDataToolMT::printMTCoinData(const Muon::RpcCoinDataContainer& coinData) const {
    msg(MSG::INFO) << "********************************************************************************************************" << endmsg;
    msg(MSG::INFO) << "***************** Listing RpcCoinData collections content **********************************************" << endmsg;

    if (coinData.empty()) msg(MSG::INFO) << "No RpcCoinData collections found" << endmsg;

    int ncoll{0}, ict{0}, ictphi{0}, icteta{0}, ictphilc{0}, ictphihc{0}, ictetalc{0}, ictetahc{0};
    msg(MSG::INFO) << "--------------------------------------------------------------------------------------------" << endmsg;
    for (const Muon::RpcCoinDataCollection* rpcColl: coinData) {
       
        if (!rpcColl->empty()) {
            msg(MSG::INFO) << "CoinData Collection ID " << m_idHelperSvc->toString(rpcColl->identify()) << endmsg;
            int icc{0}, iccphi{0}, icceta{0}, iccphilc{0}, iccetahc{0}, iccphihc{0}, iccetalc{0};
            for (const RpcCoinData* rpc : *rpcColl) {
                icc++;
                ict++;

                if (m_idHelperSvc->rpcIdHelper().measuresPhi(rpc->identify())) {
                    iccphi++;
                    ictphi++;
                    if (rpc->isLowPtCoin()) {
                        iccphilc++;
                        ictphilc++;
                    } else if (rpc->isHighPtCoin()) {
                        iccphihc++;
                        ictphihc++;
                    }
                } else {
                    icceta++;
                    icteta++;
                    if (rpc->isLowPtCoin()) {
                        iccetalc++;
                        ictetalc++;
                    } else if (rpc->isHighPtCoin()) {
                        iccetahc++;
                        ictetahc++;
                    }
                }
                msg(MSG::INFO) << ict << " in this coll. " << icc
                               << " coinData id = " << m_idHelperSvc->toString(rpc->identify()) << " time "
                               << rpc->time() << " ijk = " << rpc->ijk() << " cm/pad/sl ids = " << rpc->parentCmId() << "/"
                               << rpc->parentPadId() << "/" << rpc->parentSectorId() << "/"
                               << " isLowPtCoin/HighPtCoin/LowPtInputToHighPt " << rpc->isLowPtCoin() << "/" << rpc->isHighPtCoin() << "/"
                               << rpc->isLowPtInputToHighPtCm() << endmsg;
            }
            ncoll++;
            msg(MSG::INFO) << "*** Collection " << ncoll << " Summary: " << iccphi << " phi coin. hits / " << icceta << " eta coin. hits \n"
                           << iccphilc << " phi lowPt / " << iccphihc << " phi highPt / " << iccetalc << " eta lowPt / " << iccetahc
                           << " eta highPt coincidences  " << endmsg;
            msg(MSG::INFO) << "--------------------------------------------------------------------------------------------" << endmsg;
        }
    }
    msg(MSG::INFO) << "*** Event  Summary: " << ncoll << " Collections / " << ictphi << " phi coin. hits / " << icteta
                   << " eta coin. hits \n"
                   << ictphilc << " phi lowPt / " << ictphihc << " phi highPt / " << ictetalc << " eta lowPt / " << ictetahc
                   << " eta highPt coincidences  " << endmsg;
    msg(MSG::INFO) << "--------------------------------------------------------------------------------------------" << endmsg;
}

void Muon::RpcRdoToPrepDataToolMT::printPrepData(const EventContext& ctx) const {
    printPrepDataImpl(*SG::makeHandle(m_rpcPrepDataContainerKey, ctx), *SG::makeHandle(m_rpcCoinDataContainerKey, ctx));
}

//___________________________________________________________________________
StatusCode Muon::RpcRdoToPrepDataToolMT::decodeImpl(const EventContext& ctx, State& state, std::vector<IdentifierHash>& idVect,
                                                      std::vector<IdentifierHash>& idWithDataVect, bool firstTimeInTheEvent) const {
    int sizeVectorRequested = idVect.size();
    ATH_MSG_DEBUG("Decode method called for " << sizeVectorRequested << " offline collections");
    if (sizeVectorRequested == 0) ATH_MSG_DEBUG("Decoding the entire event");

    

    // clear output vector of selected data collections containing data
    idWithDataVect.clear();

    // create an empty vector of hash ids to be decoded (will be filled if RoI-based and left empty if full-scan)
    std::vector<IdentifierHash> idVectToBeDecoded; 
    idVectToBeDecoded.reserve(idVect.size());

    if (firstTimeInTheEvent) {
        state.m_fullEventDone = sizeVectorRequested == 0;
    } else {
        if (state.m_fullEventDone) {
            ATH_MSG_DEBUG("Whole event has already been decoded; nothing to do.");
            return StatusCode::SUCCESS;
        }
        if (sizeVectorRequested == 0) state.m_fullEventDone = true;
    }

    if (sizeVectorRequested != 0) {
        // the program goes in here only if RoI-based decoding has been called and the full event is not already decoded
        // this code ensures decoding of every offline hash id is called only once
        for (const IdentifierHash& itHashId:  idVect) {
            if (state.m_decodedOfflineHashIds.insert(itHashId).second) idVectToBeDecoded.push_back(itHashId);
        }
        if (idVectToBeDecoded.empty()) {
            ATH_MSG_DEBUG("All requested offline collections have already been decoded; nothing to do.");
            return StatusCode::SUCCESS;
        } else {
            ATH_MSG_DEBUG(idVectToBeDecoded.size() << " offline collections have not yet been decoded and will be decoded now.");
            if (msgLvl(MSG::VERBOSE)) {
                ATH_MSG_VERBOSE("The list of offline collection hash ids to be decoded:");
                for (const IdentifierHash& itHashId : idVectToBeDecoded)
                    ATH_MSG_VERBOSE(itHashId << " ");
            }
        }
    }

    // if RPC decoding is switched off stop here
    if (!m_decodeData) {
        ATH_MSG_DEBUG("Stored empty container. Decoding RPC RDO into RPC PrepRawData is switched off");
        return StatusCode::SUCCESS;
    }

    ATH_MSG_DEBUG("Decoding RPC RDO into RPC PrepRawData");

    SG::ReadCondHandle<RpcCablingCondData> cablingCondData{m_rpcReadKey, ctx};
    const RpcCablingCondData* rpcCabling{*cablingCondData};

    // if the vector requested has size 0, we need to perform a scan of the entire RDO container
    // otherwise select the pads to be decoded
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
        ATH_MSG_ERROR("Retrieval of RPC RDO container failed !");
        return StatusCode::FAILURE;
    }

    ///////////// here the RDO container is retrieved and filled -whatever input type we start with- => check the size
    if (rdoContainerHandle->numberOfCollections() == 0) {
        // empty pad container - no rpc rdo in this event
        ATH_MSG_DEBUG("Empty pad container - no rpc rdo in this event ");
        return StatusCode::SUCCESS;
    }
    ATH_MSG_DEBUG("Not empty pad container in this event ");

    // start here to process the RDO (for the full event of for a fraction of it)
    bool processingetaview{true}, processingphiview{false};
    if (!m_solvePhiAmbiguities) processingetaview = false;
    bool doingSecondLoopAmbigColls = false;
    while (processingetaview || processingphiview || (!m_solvePhiAmbiguities)) {
        int ipad{0}, nPrepRawData{0},nPhiPrepRawData{0}, nEtaPrepRawData{0};
        if (processingphiview) state.m_ambiguousCollections.clear();
        if (msgLvl(MSG::DEBUG)) {
            if (processingetaview)
                ATH_MSG_DEBUG("*** Processing eta view ");
            else
                ATH_MSG_DEBUG("*** Processing phi view ");
        }
        // seeded decoding
        if (sizeVectorRequested != 0) {
            ATH_MSG_DEBUG("Start loop over pads hashes - seeded mode ");
            for (const IdentifierHash& iPadHash: rdoHashVec) {
                const RpcPad* rdoColl = rdoContainerHandle->indexFindPtr(iPadHash);
                if (!rdoColl) {
                    ATH_MSG_DEBUG("Requested pad with online id " << iPadHash << " not found in the rdoContainerHandle.");
                    continue;
                }
                ++ipad;

                ATH_MSG_DEBUG("A new pad here n. " << ipad << ", online id " << (int)(rdoColl->identifyHash()) << ", with "
                                                << rdoColl->size() << " CM inside ");
                ATH_CHECK(processPad(ctx, state, rdoColl, processingetaview, processingphiview, nPrepRawData,
                                   idVectToBeDecoded, idWithDataVect,  doingSecondLoopAmbigColls));                    
               
            }      // end loop over requested pads hashes
        } else {   // unseeded // whole event
            ATH_MSG_DEBUG("Start loop over pads - unseeded mode ");
            for (const RpcPad* rdoColl: *rdoContainerHandle) {
                // loop over all elements of the pad container
                if (rdoColl->empty()) continue;
                ++ipad;
                ATH_MSG_DEBUG("A new pad here n. " << ipad << ", online id " << (int)(rdoColl->identifyHash()) << ", with "
                                                   << rdoColl->size() << " CM inside ");
                
                ATH_CHECK(processPad(ctx, state, rdoColl, processingetaview, processingphiview, nPrepRawData,
                               idVectToBeDecoded, idWithDataVect,  doingSecondLoopAmbigColls));               
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
                for (const IdentifierHash&  itAmbiColl : state.m_ambiguousCollections) {
                    ATH_MSG_DEBUG((int)itAmbiColl << " ");
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
    // remove empty collections listed in idWithDataVect
    std::vector<IdentifierHash> temIdWithDataVect;
    for (const IdentifierHash& idhash : idWithDataVect) {
        Muon::RpcPrepDataCollection* rpcColl = state.getPrepCollection(idhash, msgStream());
        if (!rpcColl->empty()) {
            temIdWithDataVect.push_back(idhash);
            ATH_MSG_VERBOSE("Accepting non empty coll. " << m_idHelperSvc->toString(rpcColl->identify())
                                                             << " hash = " <<idhash << " in PREPDATA container");
        }
    }    
    idWithDataVect = std::move(temIdWithDataVect);

    // sort and remove duplicate entries in idWithDataVect
    ATH_MSG_DEBUG("sorting and removing duplicates in the accepted collections vector");
    std::sort(idWithDataVect.begin(), idWithDataVect.end());
   
    if (msgLvl(MSG::DEBUG)) {
        for (IdentifierHash hashId : idWithDataVect) { ATH_MSG_DEBUG("Accepted collection with hashId: " << (unsigned int)hashId); }
    }

    return StatusCode::SUCCESS;
}

//___________________________________________________________________________
StatusCode Muon::RpcRdoToPrepDataToolMT::decodeImpl(const EventContext& ctx, State& state, const std::vector<uint32_t>& robIds,
                                                      bool firstTimeInTheEvent) const {
    // ROB-based decoding is only applied in seeded mode. Full scan should use the hashId-based method with empty requested collections
    // vector.

    int sizeVectorRequested = robIds.size();
    ATH_MSG_DEBUG("Decode method called for " << sizeVectorRequested << " ROBs");
   
    std::vector<uint32_t> robIdsToBeDecoded;
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
        if (state.m_decodedRobIds.insert(robid).second) robIdsToBeDecoded.push_back(robid);
    }

    if (robIdsToBeDecoded.empty()) {
        ATH_MSG_DEBUG("All requested ROBs have already been decoded; nothing to do.");
        return StatusCode::SUCCESS;
    }
    ATH_MSG_DEBUG(robIdsToBeDecoded.size() << " ROBs have not yet been decoded and will be decoded now.");
    if (msgLvl(MSG::VERBOSE)) {
        ATH_MSG_VERBOSE("The list of ROB Ids to be decoded:");
        for (uint32_t robid : robIdsToBeDecoded) ATH_MSG_VERBOSE("0x" << MSG::hex << robid << MSG::dec << " ");
    }
    

    SG::ReadCondHandle<RpcCablingCondData> cablingCondData{m_rpcReadKey, ctx};
    const RpcCablingCondData* rpcCabling{*cablingCondData};

    // if all robs will be decoded after the current execution of the method, set the flag m_fullEventDone
    if (state.m_decodedRobIds.size() == rpcCabling->giveFullListOfRobIds().size()) state.m_fullEventDone = true;

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
        ATH_MSG_WARNING("Retrieval of RPC RDO container failed !");
        return StatusCode::SUCCESS;
    } 

    // here the RDO container is retrieved and filled -whatever input type we start with- => check the size
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

    std::vector<IdentifierHash> idWithDataVect;  // vector passed to processPad - filled with IDs of created PrepRawData collections

    // start here to process the RDOs
    bool processingetaview = true;
    bool processingphiview = false;
    if (!m_solvePhiAmbiguities) processingetaview = false;
    while (processingetaview || processingphiview || (!m_solvePhiAmbiguities)) {
        int ipad{0}, nPrepRawData{0}, nPhiPrepRawData{0}, nEtaPrepRawData{0};
        if (processingphiview) state.m_ambiguousCollections.clear();

        if (processingetaview)
            ATH_MSG_DEBUG("*** Processing eta view ");
        else
            ATH_MSG_DEBUG("*** Processing phi view ");

        // seeded decoding (for full scan, use the hashId-based method)
        ATH_MSG_DEBUG("Start loop over pads hashes - seeded mode ");

        for (const IdentifierHash& padHashId : rdoHashVec) {
            const RpcPad* rdoColl = rdoContainerHandle->indexFindPtr(padHashId);
            if (!rdoColl) {
                ATH_MSG_DEBUG("Requested pad with online id " << (unsigned int)padHashId << " not found in the rdoContainerHandle.");
                continue;
            }
            ++ipad;
            ATH_MSG_DEBUG("A new pad here n." << ipad << ", online id " << (int)(rdoColl->identifyHash()) << ", with " << rdoColl->size()
                                              << " CM inside ");
            CHECK(processPad(ctx, state,  rdoColl, processingetaview, processingphiview, nPrepRawData,
                             rdoHashVec, idWithDataVect, false));
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
    // remove empty collections listed in idWithDataVect
    std::vector<IdentifierHash> temIdWithDataVect;
    for (const IdentifierHash& hashId : idWithDataVect) {
        const RpcPrepDataCollection* rpcColl = state.getPrepCollection(hashId, msgStream());
        if (!rpcColl->empty()) {
            temIdWithDataVect.push_back(hashId);
            ATH_MSG_VERBOSE("Accepting non empty coll. " << m_idHelperSvc->toString(rpcColl->identify())
                                                            << " hashId = " << (unsigned int)hashId << " in PREPDATA container");
        } 
    }
    idWithDataVect = std::move(temIdWithDataVect);

    // sort and remove duplicate entries in idWithDataVect
    ATH_MSG_DEBUG("sorting and removing duplicates in the accepted collections vector");
    std::sort(idWithDataVect.begin(), idWithDataVect.end());
    if (msgLvl(MSG::DEBUG)) {
        for (const IdentifierHash& hashId : idWithDataVect) { ATH_MSG_DEBUG("Accepted collection with hashId: " << (unsigned int)hashId); }
    }

    return StatusCode::SUCCESS;;
}

//___________________________________________________________________________
void Muon::RpcRdoToPrepDataToolMT::printInputRdo(const EventContext& ctx) const {
    ATH_MSG_INFO("********************************************************************************************************");
    ATH_MSG_INFO("***************** Listing RpcPad Collections --- i.e. input RDO ****************************************");

    /// RPC context
    IdContext rpcContext = m_idHelperSvc->rpcIdHelper().module_context();
    /// RPC RDO container --- assuming it is available
    ATH_MSG_DEBUG("Retrieving Rpc PAD container from the store");
    auto rdoContainerHandle = SG::makeHandle(m_rdoContainerKey, ctx);
    if (!rdoContainerHandle.isValid()) {
        ATH_MSG_WARNING("Retrieval of RPC RDO container failed !");
        return;
    }

    if (rdoContainerHandle->empty()) ATH_MSG_INFO("No RpcPad collections found");

    int ncoll = 0;
    int ictphi = 0;
    int icteta = 0;
    int icttrg = 0;
    ATH_MSG_INFO("--------------------------------------------------------------------------------------------");

    int ipad = 0;
    const RpcPad* rdoColl = nullptr;
    SG::ReadHandle<xAOD::EventInfo> evtInfo(m_eventInfo, ctx);
    for (RpcPadContainer::const_iterator rdoColli = rdoContainerHandle->begin(); rdoColli != rdoContainerHandle->end(); ++rdoColli) {
        // loop over all elements of the pad container
        rdoColl = *rdoColli;
        if (rdoColl->empty()) continue;
        ++ipad;
        uint16_t padId = rdoColl->onlineId();
        uint16_t sectorId = rdoColl->sector();
        ATH_MSG_INFO("*** Pad online Id " << padId << " m_logic sector ID " << sectorId << " # of CM inside is " << rdoColl->size());
        IdentifierHash rpcHashId = rdoColl->identifyHash();
        Identifier rdoId;
        int code = m_idHelperSvc->rpcIdHelper().get_id(rpcHashId, rdoId, &rpcContext);
        if (code != 0) ATH_MSG_INFO(" A problem in hash -> id conversion for hashId= " << (int)rpcHashId);
        std::string extIdstring = m_idHelperSvc->toString(rdoId);
        ATH_MSG_INFO("*** Offine HashId = " << static_cast<unsigned int>(rpcHashId) << " extended = " << extIdstring);

        // For each pad, loop on the coincidence matrices
        RpcPad::const_iterator itCM = rdoColl->begin();
        RpcPad::const_iterator itCM_e = rdoColl->end();
        int icphi = 0;
        int iceta = 0;
        int ictrg = 0;
        for (; itCM != itCM_e; ++itCM) {
            bool etaview = false;
            if (!evtInfo->eventType(xAOD::EventInfo::IS_SIMULATION)) etaview = true;
            bool highPtCm = false;
            // Get CM online Id
            uint16_t cmaId = (*itCM)->onlineId();
            if (cmaId < 4) {
                if (cmaId < 2) {
                    etaview = true;
                    if (!evtInfo->eventType(xAOD::EventInfo::IS_SIMULATION)) { etaview = false; }
                }
            } else {
                highPtCm = true;
                if (cmaId < 6) {
                    etaview = true;
                    if (!evtInfo->eventType(xAOD::EventInfo::IS_SIMULATION)) { etaview = false; }
                }
            }
            ATH_MSG_INFO("*** CM online Id " << cmaId << " eta view = " << etaview << " high pT = " << highPtCm
                                             << " # of hits = " << (*itCM)->size());

            // For each CM, loop on the fired channels
            RpcCoinMatrix::const_iterator itD = (*itCM)->begin();
            RpcCoinMatrix::const_iterator itD_e = (*itCM)->end();
            if (itD == itD_e) { ATH_MSG_INFO("Empty CM"); }
            for (; itD != itD_e; ++itD) {

                const RpcFiredChannel* rpcChan = (*itD);
                ATH_MSG_INFO("***** RpcFiredChannel: bcid " << rpcChan->bcid() << " time " << rpcChan->time() << " ijk " << rpcChan->ijk());
                if (rpcChan->ijk() < 7) ATH_MSG_INFO(" ch " << rpcChan->channel());
                if (rpcChan->ijk() == 6) ++ictrg;
            }  // end loop over hits
        }      // end loop over CM
        icttrg += ictrg;
        ictphi += icphi;
        icteta += iceta;
    }  // end loop over pads
    ncoll = ipad;
    ATH_MSG_INFO("*** Event  Summary: " << ncoll << " Collections / " << icttrg << " trigger hits / " << ictphi << " phi hits / " << icteta
                                        << " eta hits ");
    ATH_MSG_INFO("--------------------------------------------------------------------------------------------");
}

void Muon::RpcRdoToPrepDataToolMT::printPrepDataImpl(const Muon::RpcPrepDataContainer& rpcPrepDataContainer,
                                                       const Muon::RpcCoinDataContainer& rpcCoinDataContainer) const {
    ATH_MSG_INFO("********************************************************************************************************");
    ATH_MSG_INFO("***************** Listing RpcPrepData collections content **********************************************");

    if (rpcPrepDataContainer.empty()) ATH_MSG_INFO("No RpcPrepRawData collections found");

    int ncoll{0}, ict{0}, ictphi{0}, icteta{0}, icttrg{0};
    ATH_MSG_INFO("--------------------------------------------------------------------------------------------");
    for (const Muon::RpcPrepDataCollection* rpcColl : rpcPrepDataContainer) {
        if (!rpcColl->empty()) {
            ATH_MSG_INFO("PrepData Collection ID " << m_idHelperSvc->toString(rpcColl->identify()));
            int icc{0}, iccphi{0}, icceta{0};
            for (const RpcPrepData* rpc : *rpcColl) {
                icc++;
                ict++;
                if (m_idHelperSvc->rpcIdHelper().measuresPhi(rpc->identify())) {
                    iccphi++;
                    ictphi++;
                    
                } else {
                    icceta++;
                    icteta++;
                }
                ATH_MSG_INFO(ict << " in this coll. " << icc
                                 << " prepData id = " << m_idHelperSvc->toString(rpc->identify()) << " time "
                                 << rpc->time() /*<<" triggerInfo "<<rpc->triggerInfo()*/
                                 << " ambiguityFlag " << rpc->ambiguityFlag());
            }
            ncoll++;
            ATH_MSG_INFO("*** Collection " << ncoll << " Summary: " << iccphi << " phi hits / " << icceta << " eta hits ");
            ATH_MSG_INFO("--------------------------------------------------------------------------------------------");
        }
    }
    ATH_MSG_INFO("*** Event  Summary: " << ncoll << " Collections / " << icttrg << " trigger hits / " << ictphi << " phi hits / " << icteta
                                        << " eta hits ");
    ATH_MSG_INFO("--------------------------------------------------------------------------------------------");

    // and now coincidence data
    printCoinDataImpl(rpcCoinDataContainer);
}

void Muon::RpcRdoToPrepDataToolMT::printCoinDataImpl(const Muon::RpcCoinDataContainer& rpcCoinDataContainer) const {
    ATH_MSG_INFO("********************************************************************************************************");
    ATH_MSG_INFO("***************** Listing RpcCoinData collections content **********************************************");

    if (rpcCoinDataContainer.empty()) ATH_MSG_INFO("No RpcCoinData collections found");

    int ncoll{0}, ict{0}, ictphi{0}, icteta{0}, ictphilc{0}, ictphihc{0}, ictetalc{0}, ictetahc{0};
    ATH_MSG_INFO("--------------------------------------------------------------------------------------------");
    for (const Muon::RpcCoinDataCollection* rpcColl: rpcCoinDataContainer) {
        if (!rpcColl->empty()) {
            ATH_MSG_INFO("CoinData Collection ID " << m_idHelperSvc->toString(rpcColl->identify()));
            int icc{0}, iccphi{0}, icceta{0}, iccphilc{0}, iccetahc{0}, iccphihc{0}, iccetalc{0};
            for (const RpcCoinData* rpc : *rpcColl) {
                icc++;
                ict++;
                if (m_idHelperSvc->rpcIdHelper().measuresPhi(rpc->identify())) {
                    iccphi++;
                    ictphi++;
                    if (rpc->isLowPtCoin()) {
                        iccphilc++;
                        ictphilc++;
                    } else if (rpc->isHighPtCoin()) {
                        iccphihc++;
                        ictphihc++;
                    }
                } else {
                    icceta++;
                    icteta++;
                    if (rpc->isLowPtCoin()) {
                        iccetalc++;
                        ictetalc++;
                    } else if (rpc->isHighPtCoin()) {
                        iccetahc++;
                        ictetahc++;
                    }
                }
                ATH_MSG_INFO(
                    ict << " in this coll. " << icc << " coinData id = " << m_idHelperSvc->toString(rpc->identify())
                        << " time " << rpc->time() << " ijk = " << rpc->ijk() /*<<" triggerInfo "<<rpc->triggerInfo()*/
                        << " cm/pad/sl ids = " << rpc->parentCmId() << "/" << rpc->parentPadId() << "/" << rpc->parentSectorId() << "/"
                        << " isLowPtCoin/HighPtCoin/LowPtInputToHighPt " << rpc->isLowPtCoin() << "/" << rpc->isHighPtCoin() << "/"
                        << rpc->isLowPtInputToHighPtCm());
            }
            ncoll++;
            ATH_MSG_INFO("*** Collection " << ncoll << " Summary: " << iccphi << " phi coin. hits / " << icceta << " eta coin. hits \n"
                                           << iccphilc << " phi lowPt / " << iccphihc << " phi highPt / " << iccetalc << " eta lowPt / "
                                           << iccetahc << " eta highPt coincidences");
            ATH_MSG_INFO("--------------------------------------------------------------------------------------------");
        }
    }
    ATH_MSG_INFO("*** Event  Summary: " << ncoll << " Collections / " << ictphi << " phi coin. hits / " << icteta << " eta coin. hits \n"
                                        << ictphilc << " phi lowPt / " << ictphihc << " phi highPt / " << ictetalc << " eta lowPt / "
                                        << ictetahc << " eta highPt coincidences");
    ATH_MSG_INFO("--------------------------------------------------------------------------------------------");
}

StatusCode Muon::RpcRdoToPrepDataToolMT::processPad(const EventContext& ctx,
    State& state, const RpcPad* rdoColl,
    bool& processingetaview, bool& processingphiview, int& nPrepRawData,
    std::vector<IdentifierHash>& idVect,          // if empty, turns off decoding of additional RDOs for ambiguity solving
    std::vector<IdentifierHash>& idWithDataVect,  // filled with IDs of created PrepRawData collections
    bool doingSecondLoopAmbigColls) const {
    
    std::set<IdentifierHash>& ambiguousCollections{state.m_ambiguousCollections};
    ATH_MSG_DEBUG("***************** Start of processPad eta/phiview "
                  << processingetaview << "/" << processingphiview << " ---# of coll.s with data until now is " << idWithDataVect.size());
    //{processPad
    // Get pad online id and sector id
    uint16_t padId = rdoColl->onlineId();
    uint16_t sectorId = rdoColl->sector();
    ATH_MSG_DEBUG("***************** for Pad online Id " << padId << " m_logic sector ID " << sectorId);

    // Create an RPC PrepDataCollection
    Identifier oldId{0}, oldIdTrg{0};
    ATH_MSG_VERBOSE("Init pointer to RpcPrepDataCollection ");
    RpcPrepDataCollection* collection{nullptr};
    RpcCoinDataCollection* collectionTrg{nullptr};
    IdentifierHash rpcHashId{0};

    SG::ReadCondHandle<RpcCablingCondData> cablingCondData{m_rpcReadKey, ctx};
    const RpcCablingCondData* rpcCabling{*cablingCondData};

    // For each pad, loop on the coincidence matrices
    RpcPad::const_iterator itCM = rdoColl->begin();
    RpcPad::const_iterator itCM_e = rdoColl->end();
    int icm = 0;
    SG::ReadHandle<xAOD::EventInfo> evtInfo{m_eventInfo, ctx};
    for (; itCM != itCM_e; ++itCM) {
        icm++;
        bool etaview = false;
        if (!evtInfo->eventType(xAOD::EventInfo::IS_SIMULATION)) etaview = true;
        bool highPtCm = false;
        // Get CM online Id
        uint16_t cmaId = (*itCM)->onlineId();
        ATH_MSG_DEBUG("A new CM here n. " << icm << " CM online ID " << cmaId << " with n. of hits =  " << (*itCM)->size());
        if (cmaId < 4) {
            ATH_MSG_DEBUG(" low pt ");
            if (cmaId < 2) {
                etaview = true;
                if (!evtInfo->eventType(xAOD::EventInfo::IS_SIMULATION)) { etaview = false; }
                ATH_MSG_DEBUG(" eta view = " << etaview);
            }

            else {
                ATH_MSG_DEBUG(" eta view = " << etaview);
            }
        } else {
            ATH_MSG_DEBUG(" high pt ");
            highPtCm = true;
            if (cmaId < 6) {
                etaview = true;
                if (!evtInfo->eventType(xAOD::EventInfo::IS_SIMULATION)) { etaview = false; }
                ATH_MSG_DEBUG(" eta view = " << etaview);
            } else {
                ATH_MSG_DEBUG(" eta view = " << etaview);
            }
        }

        if (processingetaview && (!etaview)) continue;
        if (processingphiview && etaview) continue;

        // For each CM, loop on the fired channels
        RpcCoinMatrix::const_iterator itD = (*itCM)->begin();
        RpcCoinMatrix::const_iterator itD_e = (*itCM)->end();
        int idata = 0;
        if (itD == itD_e) { ATH_MSG_DEBUG("Empty CM"); }
        for (; itD != itD_e; ++itD) {
            idata++;
            // trigger related quantities
            unsigned short threshold = 99;
            unsigned short overlap = 99;

            // flags defining the processing mode of this hit
            bool solvePhiAmb_thisHit = m_solvePhiAmbiguities;
            bool reduceCablOvl_thisHit = m_reduceCablingOverlap;

            ATH_MSG_DEBUG("A new CM Hit " << idata);
            const RpcFiredChannel* rpcChan = (*itD);
            if (msgLvl(MSG::DEBUG)) {
                ATH_MSG_DEBUG("RpcFiredChannel: bcid " << rpcChan->bcid() << " time " << rpcChan->time() << " ijk " << rpcChan->ijk());
                if (rpcChan->ijk() < 7) ATH_MSG_DEBUG(" ch " << rpcChan->channel());
            }

            // check if trigger hit
            // select the cases: ijk = 0 and high p, ijk= 6, ijk=7
            bool triggerHit = false;
            bool toSkip = false;
            processTriggerHitHypothesis(itD, itD_e, highPtCm, triggerHit, threshold, overlap, toSkip);
            if (toSkip) continue;
            if (triggerHit) {
                // here ijk = 6 or ijk = 0 in high pt cm
                // keep all pivot + trigger info (even if duplicated [should never happen, for pivot hits])
                solvePhiAmb_thisHit = false;
                reduceCablOvl_thisHit = false;
                ATH_MSG_DEBUG("RpcFiredChannel: it's a triggerHit or a lowPt coinc. in a high pt CM \n"
                              << "    ijk = " << rpcChan->ijk() << " isHighPtCM " << highPtCm << " thr/ovl = " << threshold << "/"
                              << overlap);
            }

            // here decode (get offline ids for the online indices of this hit)
            double time = 0.;
            std::vector<Identifier> digitVec{m_rpcRdoDecoderTool->getOfflineData(rpcChan, sectorId, padId, cmaId, time, rpcCabling)};
            time += (double)m_timeShift;

          
            int nMatchingEtaHits = 0;
            int nDuplicatePhiHits = 0;
            bool unsolvedAmbiguity = false;
            bool notFinished = true;
            // allow for 2 iterations in case there are phi digits without matching eta (eta inefficiency)
            // all eta digits, not already recorded, will be registered as PrepRawData
            // and all phi digits, not yet recorded and with a eta digit in the same module
            // and gap, will produce a PrepRawData.
            // Phi digits without a eta match will not be recorded at the first iteration.
            // If all phi digits do not have a eta match, they will be all recorded as PrepRawData
            // in the second iteration (the ambiguity will remain unsolved)
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
                    Identifier parentId = m_idHelperSvc->rpcIdHelper().elementID(channelId);
                    if (m_idHelperSvc->rpcIdHelper().get_module_hash(parentId, rpcHashId)) {
                        ATH_MSG_WARNING("Unable to get RPC hash id from RPC collection "<<m_idHelperSvc->toString(parentId));                       
                    }

                    // There is some ambiguity in the channel/sectorId's, so need to explicitly filter
                    // out hashIDs outside of the RoI in seeded decoding mode
                    if (!idVect.empty()) {
                        if (std::find(idVect.begin(), idVect.end(), rpcHashId) == idVect.end()) continue;
                    }
                    if (msgLvl(MSG::DEBUG)) {
                        ATH_MSG_DEBUG("CM Hit decoded into offline Id " << m_idHelperSvc->toString(channelId)
                                                                        << " time " << time);
                        ATH_MSG_DEBUG("           Parent collection   "
                                      << m_idHelperSvc->toString(parentId)
                                      << " oldID = " << m_idHelperSvc->toString(oldId)
                                      << " oldIDtrg = " << m_idHelperSvc->toString(oldIdTrg));
                    }
                    bool hasAMatchingEtaHit = 0;
                    // current collection has Id "parentId"; get it from the container !
                    if (triggerHit) {
                        if ((oldIdTrg != parentId) || !collectionTrg) {
                            // Get collection from IDC if it exists, or create it and add it if not.
                            ATH_MSG_DEBUG(" Looking/Creating a collection with ID = "
                                          << m_idHelperSvc->toString(parentId)
                                          << " hash = " << static_cast<unsigned int>(rpcHashId));
                            collectionTrg = state.getCoinCollection(rpcHashId, msgStream());
                            oldIdTrg = parentId;
                            ATH_MSG_DEBUG(
                                " Resetting oldIDtrg to current parentID = " << m_idHelperSvc->toString(oldIdTrg));
                        }
                    } else if ((oldId != parentId) || !collection) {
                        // Get collection from IDC if it exists, or create it and add it if not.
                        ATH_MSG_DEBUG(" Looking/Creating a collection with ID = "
                                        << m_idHelperSvc->toString(parentId)
                                        << " hash = " << static_cast<unsigned int>(rpcHashId));
                        collection = state.getPrepCollection(rpcHashId, msgStream());
                        if (std::find(idWithDataVect.begin(),idWithDataVect.end(), rpcHashId) == idWithDataVect.end()){
                            idWithDataVect.push_back(rpcHashId);
                        }
                        oldId = parentId;
                        ATH_MSG_DEBUG(" Resetting oldID to current parentID = " << m_idHelperSvc->toString(oldId));
                    }

                    // check if the data has already ben recorded
                    // (if you want to reduce the redundancy due to cabling overlap and if the collection is not empty)
                    bool duplicate = false;
                    if (reduceCablOvl_thisHit) {
                        if (collection->begin() != collection->end()) {
                            RpcPrepDataCollection::iterator it_rpcPrepData;
                            ATH_MSG_VERBOSE("Check for duplicates in coll. with size " << collection->size());
                            int current_dbphi{0}, current_dbz{0}, current_gg{0};
                            if (processingphiview) {
                                current_dbphi = m_idHelperSvc->rpcIdHelper().doubletPhi(channelId);
                                current_dbz = m_idHelperSvc->rpcIdHelper().doubletZ(channelId);
                                current_gg = m_idHelperSvc->rpcIdHelper().gasGap(channelId);
                                ATH_MSG_VERBOSE("Check also for eta hits matching dbz, dbphi, gg  " << current_dbz << " " << current_dbphi
                                                                                                    << " " << current_gg);
                            }

                            for (RpcPrepData* rpc : *collection) {
                                if (channelId == rpc->identify() && fabs(time - rpc->time()) < m_overlap_timeTolerance) {
                                    duplicate = true;
                                    hasAMatchingEtaHit = false;  // we don't want to increment the number of strips with
                                    // a matching eta due to a cabling overlap
                                    ATH_MSG_VERBOSE("Duplicated RpcPrepData(not recorded) = "
                                                    << m_idHelperSvc->toString(channelId));
                                    float previous_time = rpc->time();
                                    // choose the smallest time within timeTolerance
                                    if (time < previous_time) {
                                        rpc->m_time = time;
                                        ATH_MSG_DEBUG("time of the prd previously stored is now updated with current hit time: "
                                                      << previous_time << " -> " << rpc->time());
                                    }
                                    break;  // this break is why we cannot have
                                            //        solvePhiAmb_thisHit = true and reduceCablOvl_thisHit= false
                                }
                                if (processingphiview) {
                                    if (solvePhiAmb_thisHit) {
                                        if (!unsolvedAmbiguity) {
                                            if (m_idHelperSvc->rpcIdHelper().measuresPhi(rpc->identify()) == 0) {
                                                // check if there's a eta hit in the same gap
                                                // of the RPC module (doubletZ, doubletPhi, gg)
                                                if (current_dbz == m_idHelperSvc->rpcIdHelper().doubletZ(rpc->identify())) {
                                                    if (current_dbphi == m_idHelperSvc->rpcIdHelper().doubletPhi(rpc->identify())) {
                                                        if (current_gg == m_idHelperSvc->rpcIdHelper().gasGap(rpc->identify())) {
                                                            if (fabs(time - rpc->time()) < m_etaphi_coincidenceTime) {
                                                                hasAMatchingEtaHit = true;
                                                                ATH_MSG_VERBOSE(
                                                                    "There's a matching eta hit with id "
                                                                    << m_idHelperSvc->toString(rpc->identify()));
                                                                // here there can be a break ? NO, we need to keep looping in order to check
                                                                //  if this preprawdata has been already recorded (due to cabling overlaps)
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            if (hasAMatchingEtaHit) nMatchingEtaHits++;  // Number of phi strips (possibly corresponding to this CM hit)
                            // with a matching eta
                            if (processingphiview && duplicate)
                                nDuplicatePhiHits++;  // Number of phi strips (possibly corresponding to this CM hit)
                            // already in the collection
                        }  // if collection is not empty
                    }      // end of if reduceCablingOverlap

                    if (msgLvl(MSG::VERBOSE)) {
                        if (solvePhiAmb_thisHit && (!etaview))
                            ATH_MSG_VERBOSE("nMatchingEtaHits = " << nMatchingEtaHits << " hasAMatchingEtaHit = " << hasAMatchingEtaHit);
                    }

                    if (!duplicate) {
                        ATH_MSG_VERBOSE(" solvePhiAmb_thisHit, processingetaview, processingphiview, hasAMatchingEtaHit, unsolvedAmbiguity "
                                        << solvePhiAmb_thisHit << " " << processingetaview << " " << processingphiview << " "
                                        << hasAMatchingEtaHit << " " << unsolvedAmbiguity);
                        if ((!solvePhiAmb_thisHit) || processingetaview ||
                            (processingphiview && (hasAMatchingEtaHit || unsolvedAmbiguity))) {
                            if (unsolvedAmbiguity) {
                                if (idVect.empty()) {  // full-scan mode
                                    ATH_MSG_DEBUG("storing data even if unsolvedAmbiguity");
                                } else {
                                    // if in RoI mode and the collection was not requested in this event, add it to ambiguousCollections
                                    ATH_MSG_DEBUG("unsolvedAmbiguity is true, adding collection with hash = "
                                                  << (int)rpcHashId << " to ambiguous collections vector");
                                    if (!state.m_decodedOfflineHashIds.empty() &&
                                        state.m_decodedOfflineHashIds.find(rpcHashId) == state.m_decodedOfflineHashIds.end()) {
                                        ambiguousCollections.insert(rpcHashId);
                                        ATH_MSG_DEBUG(
                                            "collection not yet processed; added to ambiguous collection vector; going to the next offline "
                                            "channel ID");
                                        continue;  // go to the next possible offline channel ID
                                    } else if (!doingSecondLoopAmbigColls) {
                                        ambiguousCollections.insert(rpcHashId);
                                        ATH_MSG_DEBUG(
                                            "collection already processed and doingSecondLoopAmbigColls=false; added to ambiguous "
                                            "collection vector; going to the next offline channel ID");
                                        continue;
                                    } else {
                                        ATH_MSG_DEBUG(
                                            "collection already processed and doingSecondLoopAmbigColls=true; trying to store data even if "
                                            "unsolvedAmbiguity");
                                    }
                                }
                            }
                            SG::ReadCondHandle<MuonGM::MuonDetectorManager> muDetMgrHandle{m_muDetMgrKey, ctx};
                            const MuonGM::MuonDetectorManager* muDetMgr = muDetMgrHandle.cptr();
                            const RpcReadoutElement* descriptor = muDetMgr->getRpcReadoutElement(channelId);

                            // here check validity
                            // if invalid, reset flags
                            if (!descriptor) {
                                hasAMatchingEtaHit = false;
                                duplicate = false;
                                ATH_MSG_WARNING("Detector Element not found for Identifier from the cabling service <"
                                                << m_idHelperSvc->toString(channelId) << ">  =>>ignore this hit");
                                continue;
                            } else if (!descriptor->containsId(channelId)) {
                                hasAMatchingEtaHit = false;
                                duplicate = false;
                                if (m_idHelperSvc->rpcIdHelper().stationNameString(m_idHelperSvc->rpcIdHelper().stationName(channelId)) ==
                                    "BOG")
                                    ATH_MSG_DEBUG("Identifier from the cabling service <"
                                                  << m_idHelperSvc->toString(channelId)
                                                  << "> inconsistent with the geometry of detector element <"
                                                  << m_idHelperSvc->toString(descriptor->identify())
                                                  << ">  =>>ignore this hit /// there are unmasked channels in BOG");
                                else
                                    ATH_MSG_WARNING("Identifier from the cabling service <"
                                                    << m_idHelperSvc->toString(channelId)
                                                    << "> inconsistent with the geometry of detector element <"
                                                    << m_idHelperSvc->toString(descriptor->identify())
                                                    << ">  =>>ignore this hit");
                                continue;
                            }

                            //
                            // Global position
                            Amg::Vector3D tempGlobalPosition = descriptor->stripPos(channelId);
                            ATH_MSG_VERBOSE("RPC RDO->PrepRawdata: global position ("
                                            << tempGlobalPosition.x() << ", " << tempGlobalPosition.y() << ", " << tempGlobalPosition.z()
                                            << ") ");
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
                                if (processingetaview) ambiguityFlag = 1;
                                if (unsolvedAmbiguity)
                                    ambiguityFlag =  digitVec.size();
                                else if (hasAMatchingEtaHit)
                                    ambiguityFlag = nMatchingEtaHits;
                            }

                            // correct prd time from cool db
                            if (m_RPCInfoFromDb) {
                                SG::ReadCondHandle<RpcCondDbData> readHandle{m_readKey, ctx};
                                const RpcCondDbData* readCdo{*readHandle};
                                ATH_MSG_DEBUG(" Time correction from COOL "
                                              << " size of  RPC_TimeMapforStrip " << readCdo->getStripTimeMap().size());
                                std::vector<double> StripTimeFromCool;
                                if (readCdo->getStripTimeMap().find(channelId) != readCdo->getStripTimeMap().end()) {
                                    StripTimeFromCool = readCdo->getStripTimeMap().find(channelId)->second;
                                    ATH_MSG_DEBUG(" Time " << time << " Time correction from COOL for jstrip " << StripTimeFromCool.at(0));
                                    time -= StripTimeFromCool.at(0);
                                }
                            }

                            if (triggerHit) {
                                ATH_MSG_DEBUG("producing a new  RpcCoinData");

                                RpcCoinData* newCoinData = new RpcCoinData(
                                    channelId, rpcHashId, pointLocPos, identifierList, Amg::MatrixX(mat), descriptor, (float)time,
                                    ambiguityFlag, rpcChan->ijk(), threshold, overlap, cmaId, padId, sectorId, !(highPtCm));

                                // record the new data in the collection
                                ATH_MSG_DEBUG(" Adding RpcCoinData @ "
                                              << newCoinData << " to collection "
                                              << m_idHelperSvc->toString(collectionTrg->identify()));

                                newCoinData->setHashAndIndex(collectionTrg->identifyHash(), collectionTrg->size());
                                collectionTrg->push_back(newCoinData);
                            }  // end of to be stored now for RpcCoinData
                            else {
                                ATH_MSG_DEBUG("producing a new  RpcPrepData with "
                                              << "ambiguityFlag = " << ambiguityFlag);

                                RpcPrepData* newPrepData = new RpcPrepData(channelId, rpcHashId, pointLocPos, identifierList,
                                                                           Amg::MatrixX(mat), descriptor, (float)time, ambiguityFlag);

                                // record the new data in the collection
                                ATH_MSG_DEBUG(" Adding digit @ " << newPrepData << " to collection "
                                                                 << m_idHelperSvc->toString(collection->identify()));

                                newPrepData->setHashAndIndex(collection->identifyHash(), collection->size());
                                collection->push_back(newPrepData);
                                // here one should reset ambiguityFlag for the prepdata registered before the current one
                                // (from the same RDO hit) if nMatchingEtaHits > 1
                                nPrepRawData++;
                            }  // end of to be stored now for RpcPrepData
                        }      // end of to be stored now
                    }          // this hit was not yet recorded
                    else {
                        // this hit was already recorded
                        ATH_MSG_DEBUG("digit already in the collection ");
                    }
                }  // end loop over possible offline identifiers corresponding to this CM hit
                ATH_MSG_VERBOSE("processingphiview, nMatchingEtaHits, nDuplicatePhiHits, unsolvedAmbiguity, solvePhiAmb_thisHit : "
                                << processingphiview << ", " << nMatchingEtaHits << ", " << nDuplicatePhiHits << ", " << unsolvedAmbiguity
                                << ", " << solvePhiAmb_thisHit);
                if ((processingphiview && (nMatchingEtaHits == 0)) && (nDuplicatePhiHits == 0) && (!unsolvedAmbiguity) &&
                    (solvePhiAmb_thisHit)) {
                    unsolvedAmbiguity = true;
                    // no eta hits matching any phi digit
                    // loop once again and store all phi digits potentially generating this CM hit
                    ATH_MSG_DEBUG("No eta prepData matching any phi hit from this CM hit \n"
                                  << "loop once again and store all phi digits potentially generating this CM hit");
                } else if (unsolvedAmbiguity)
                    notFinished = false;
                else
                    notFinished = false;
            }  // end of not finished            
        }  // end loop over CM hits
    }      // end loop over CMs

    ATH_MSG_DEBUG("***************** Stop  of processPad eta/phiview "
                  << processingetaview << "/" << processingphiview << " ---# of coll.s with data now is " << idWithDataVect.size()
                  << "***************** for Pad online Id " << padId << " m_logic sector ID " << sectorId);

    return StatusCode::SUCCESS;
}

StatusCode Muon::RpcRdoToPrepDataToolMT::processNrpcRdo(const EventContext& ctx, State& state) const {
    if (m_rdoNrpcContainerKey.empty()){
         ATH_MSG_DEBUG("The NRPC processing is disabled.");
         return StatusCode::SUCCESS;
    }
    
    ATH_MSG_DEBUG("Retrieving Nrpc RDO container from the store");
    SG::ReadHandle<xAOD::NRPCRDOContainer> rdoNrpcContainerHandle{m_rdoNrpcContainerKey, ctx};
    if (!rdoNrpcContainerHandle.isPresent()) {
        ATH_MSG_ERROR("Retrieval of NRPC RDO "<<m_rdoNrpcContainerKey.fullKey()<<" container failed !");
        return StatusCode::FAILURE;
    }
    
    if (rdoNrpcContainerHandle->empty()) {
        // empty NRPC RDO container - no nrpc rdo in this event
        ATH_MSG_DEBUG("Empty NRPC RDO container - no nrpc rdo in this event ");
        return StatusCode::SUCCESS;
    }
    ATH_MSG_DEBUG("Not empty NRPC RDO container in this event ");


    SG::ReadCondHandle<RpcCablingMap> readCdo{m_nRpcCablingKey, ctx};
    if (!readCdo.isValid()) {
        ATH_MSG_ERROR("Could not retrieve "<<m_nRpcCablingKey.fullKey()<<" from the conditions store");
        return StatusCode::FAILURE;
    }
    /// No need to check for its validity. It's done at other places for sure
    SG::ReadCondHandle<MuonGM::MuonDetectorManager> muDetMgr{m_muDetMgrKey, ctx};
  
    for (const xAOD::NRPCRDO* nrpcrdo: *rdoNrpcContainerHandle) {
        /// Convert from online to offline cabling
        RpcCablingData cabling_data{};
        cabling_data.subDetector = nrpcrdo->subdetector();
        cabling_data.tdcSector = nrpcrdo->tdcsector();
        cabling_data.tdc = nrpcrdo->tdc();
        cabling_data.channelId = nrpcrdo->channel();
        Identifier chanId{};
        if (!readCdo->getOfflineId(cabling_data, msgStream()) ||
            !readCdo->convert(cabling_data, chanId, false)){
             ATH_MSG_FATAL("Failed to retrieve the offline Identifier");
             return StatusCode::FAILURE;
        }
        const IdentifierHash  rpcHashId = m_idHelperSvc->moduleHash(chanId);
        RpcPrepDataCollection* collection = state.getPrepCollection(rpcHashId, msgStream());

        const RpcReadoutElement* descriptor = muDetMgr->getRpcReadoutElement(chanId);
        
        // List of Digits in the cluster (self)
        std::vector<Identifier> identifierList{chanId};
        // Global position
        const Amg::Vector3D stripPos{descriptor->stripPos(chanId)};
        ATH_MSG_DEBUG("RPC RDO->PrepRawdata "<<m_idHelperSvc->toString(chanId)<<" global position "<<Amg::toString(stripPos,2));
        // Local position
        Amg::Vector2D pointLocPos{Amg::Vector2D::Zero()};
        descriptor->surface(chanId).globalToLocal(stripPos, stripPos, pointLocPos);
        // width of the cluster (self)
        
        const double stripWidth = descriptor->StripWidth(m_idHelperSvc->rpcIdHelper().measuresPhi(chanId));
        // Error matrix
        const double errPos = stripWidth / std::sqrt(12.0);
        Amg::MatrixX mat(1, 1);
        mat.setIdentity();
        mat *= errPos * errPos;

        int ambiguityFlag = 0; // Ambiguity flag not checked for BIS RPCs
        
        const float time = nrpcrdo->time();
        /// That needs to be parsed to the constructor of the PRD somehow...
        const float timeoverthr = nrpcrdo->timeoverthr();

        RpcPrepData* newPrepData = new RpcPrepData(chanId, rpcHashId, pointLocPos, identifierList,
                                                   mat, descriptor, time, timeoverthr, 0, ambiguityFlag);

        newPrepData->setHashAndIndex(collection->identifyHash(), collection->size());        
        collection->push_back(newPrepData);        
    }    
    return StatusCode::SUCCESS;
}

void Muon::RpcRdoToPrepDataToolMT::processTriggerHitHypothesis(RpcCoinMatrix::const_iterator itD, RpcCoinMatrix::const_iterator itD_end,
                                                                 bool highPtCm,
                                                                 // the previous arg.s are inputs
                                                                 bool& triggerHit, unsigned short& threshold, unsigned short& overlap,
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
            std::string cmtype;
            // look for the subsequent ijk  7 to define threshold and overlap
            if (msgLvl(MSG::VERBOSE)) {
                cmtype = " in low  pT CM ";
                if (highPtCm) cmtype = " in high pT CM ";
                ATH_MSG_VERBOSE("This hit: ijk = " << rpcChan->ijk() << cmtype << " bcid is " << rpcChan->bcid() << " time is "
                                                   << rpcChan->time() << " ch " << rpcChan->channel());
            }
            RpcCoinMatrix::const_iterator itDnext = itD + 1;
            while (itDnext != itD_end) {
                const RpcFiredChannel* rpcChanNext = (*itDnext);
                if (msgLvl(MSG::VERBOSE)) {
                    ATH_MSG_VERBOSE("Next hit: ijk = " << rpcChanNext->ijk() << cmtype << " bcid is " << rpcChan->bcid() << " time is "
                                                       << rpcChanNext->time());
                    if (rpcChanNext->ijk() < 7) ATH_MSG_VERBOSE(" ch " << rpcChanNext->channel());
                }
                if (rpcChanNext->ijk() == 7) {
                    ATH_MSG_VERBOSE("next has ijk=7 ");
                    if (rpcChanNext->bcid() == rpcChan->bcid() && rpcChanNext->time() == rpcChan->time()) {
                        ATH_MSG_VERBOSE("bdid/tick match; assigning thr/overlap  = " << rpcChanNext->thr() << "/" << rpcChanNext->ovl());
                        threshold = rpcChanNext->thr();
                        overlap = rpcChanNext->ovl();
                    } else {
                        ATH_MSG_WARNING("ijk =7 after a ijk = 6 BUT bdid/tick don't match - will not assign threshold/overlap ");
                    }
                    break;
                } else {
                    // std::cout<<"processing trigger hit with ijk = 6; next is not ijk 7"<<std::endl;
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
    ATH_MSG_VERBOSE("RPC trigger hit; ijk = " << rpcChan->ijk() << " threshold / overlap = " << threshold << "/" << overlap);
}
