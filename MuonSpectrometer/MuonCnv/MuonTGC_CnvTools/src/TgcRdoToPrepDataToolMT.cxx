/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "TgcRdoToPrepDataToolMT.h"

#include "GeoPrimitives/GeoPrimitivesHelpers.h"

#include "GaudiKernel/ThreadLocalContext.h"
#include "MuonDigitContainer/TgcDigit.h"
#include "MuonReadoutGeometry/TgcReadoutElement.h"
#include "MuonTrigCoinData/TgcCoinData.h"
#include "TrkSurfaces/Surface.h"
#include <FourMomUtils/xAODP4Helpers.h>
#include <algorithm>
#include <cfloat>
#include <memory>
#include "xAODMuonPrepData/TgcStripAuxContainer.h"

namespace{
    using namespace xAOD::P4Helpers;
}
//================ Initialization =================================================

StatusCode Muon::TgcRdoToPrepDataToolMT::initialize()
{
  ATH_CHECK(m_idHelperSvc.retrieve());

  m_outputprepdataKeys.resize(NBC_HIT+1);
  for (int ibc=0; ibc < NBC_HIT+1; ibc++) {
    int bcTag = ibc+1;
    std::ostringstream location;
    location << m_outputCollectionLocation.value()
             << (bcTag==TgcDigit::BC_PREVIOUS ? "PriorBC" : "")
             << (bcTag==TgcDigit::BC_CURRENT ? "" : "")
         << (bcTag==TgcDigit::BC_NEXT ? "NextBC" : "")
             << (bcTag==(NBC_HIT+1) ? "AllBCs" : "");    
    m_outputprepdataKeys.at(ibc) = location.str();
  }

  m_outputCoinKeys.resize(NBC_TRIG);
  for (int ibc=0; ibc < NBC_TRIG; ibc++) {
    int bcTag = ibc+1;
    std::ostringstream location;
    location << m_outputCoinCollectionLocation.value()
             << (bcTag==TgcDigit::BC_PREVIOUS ? "PriorBC" : "")
             << (bcTag==TgcDigit::BC_NEXT ? "NextBC" : "")
             << (bcTag==TgcDigit::BC_NEXTNEXT ? "NextNextBC" : "");
    m_outputCoinKeys.at(ibc) = location.str();
  }

  ATH_CHECK(m_outputCoinKeys.initialize());
  ATH_CHECK(m_outputprepdataKeys.initialize());
  ATH_CHECK(m_muDetMgrKey.initialize());

  // check if initializing of DataHandle objects success
  ATH_CHECK( m_rdoContainerKey.initialize() );

  //try to configure the cabling service
  if (!getCabling()) {
    // ??? Is this delayed initialization still needed?
    ATH_MSG_INFO("MuonTGC_CablingSvc not yet retrieved; postpone TGCcabling initialization at first event.");
  }

  // Build names for the keys same as done for output containers
  if (not m_prdContainerCacheKeyStr.empty()) {
    m_prdContainerCacheKeys.resize(NBC_HIT+1);
    for (int ibc=0; ibc < NBC_HIT+1; ibc++) {
      int bcTag = ibc+1;
      std::ostringstream location;
      location << m_prdContainerCacheKeyStr.value()
               << (bcTag==TgcDigit::BC_PREVIOUS ? "PriorBC" : "")
               << (bcTag==TgcDigit::BC_CURRENT ? "" : "")
               << (bcTag==TgcDigit::BC_NEXT ? "NextBC" : "")
               << (bcTag==(NBC_HIT+1) ? "AllBCs" : "");    
      m_prdContainerCacheKeys.at(ibc) = location.str();
      ATH_MSG_DEBUG(location.str());      
    }
  }

  if (not m_coinContainerCacheKeyStr.empty()){
    m_coinContainerCacheKeys.resize(NBC_TRIG);
    for (int ibc=0; ibc < NBC_TRIG; ibc++) {
      int bcTag = ibc+1;
      std::ostringstream location;
      location << m_coinContainerCacheKeyStr.value()
               << (bcTag==TgcDigit::BC_PREVIOUS ? "PriorBC" : "")
               << (bcTag==TgcDigit::BC_NEXT ? "NextBC" : "")
               << (bcTag==TgcDigit::BC_NEXTNEXT ? "NextNextBC" : "");
      m_coinContainerCacheKeys.at(ibc) = location.str();
      ATH_MSG_DEBUG(location.str());
    }
  }

  // Only initialise if we passed in the cache name
  ATH_CHECK( m_prdContainerCacheKeys.initialize( not m_prdContainerCacheKeyStr.empty() ) );
  ATH_CHECK( m_coinContainerCacheKeys.initialize( not m_coinContainerCacheKeyStr.empty() ) );
  
  ATH_CHECK(m_xAODKey.initialize(!m_xAODKey.empty()));
  return StatusCode::SUCCESS;
}

//================ Finalization =================================================

StatusCode Muon::TgcRdoToPrepDataToolMT::finalize()
{
  ATH_MSG_INFO("finalize(): input RDOs->output PRDs ["  << 
           "Hit: "          << m_nHitRDOs          << "->" << m_nHitPRDs          << ", " << 
           "Tracklet: "     << m_nTrackletRDOs     << "->" << m_nTrackletPRDs     << ", " << 
           "TrackletEIFI: " << m_nTrackletEIFIRDOs << "->" << m_nTrackletEIFIPRDs << ", " << 
           "HiPt: "         << m_nHiPtRDOs         << "->" << m_nHiPtPRDs         << ", " << 
           "SL: "           << m_nSLRDOs           << "->" << m_nSLPRDs           << "]"); 

  return AthAlgTool::finalize();
}

//================ Decoding =================================================
StatusCode Muon::TgcRdoToPrepDataToolMT::decode(const EventContext&, const std::vector<uint32_t>& /*robIds*/ ) const {
   ATH_MSG_FATAL("ROB based decoding is not supported....");
   return StatusCode::FAILURE;
}
template<class ContType, class CollType> 
  StatusCode Muon::TgcRdoToPrepDataToolMT::transferData(ContType& container,
                                                        std::vector<std::unique_ptr<CollType>>&& coll) const {
  for (std::unique_ptr<CollType>& toMove : coll) {
    if (!toMove) continue;
    const IdentifierHash hash = toMove->identifyHash();
    auto lock = container.getWriteHandle(hash);
    if(!lock.alreadyPresent()) {
      ATH_CHECK(lock.addOrDelete(std::move(toMove)));
    }
  }  
  return StatusCode::SUCCESS;
}

StatusCode Muon::TgcRdoToPrepDataToolMT::provideEmptyContainer(const EventContext& ctx) const {
  if (!m_xAODKey.empty()){
    SG::WriteHandle handle{m_xAODKey, ctx};
    ATH_CHECK(handle.record(std::make_unique<xAOD::TgcStripContainer>(), 
                            std::make_unique<xAOD::TgcStripAuxContainer>()));
  }
   State state{};
   return setupState(ctx, state);
}

StatusCode Muon::TgcRdoToPrepDataToolMT::setupState(const EventContext& ctx, State& state) const{
  /// clean up containers for Hits
  const unsigned hashMax = m_idHelperSvc->tgcIdHelper().module_hash_max();

  for(unsigned int ibc=0; ibc < NBC_HIT+1; ibc++) {   //  +1 for AllBCs
    // initialize with false
    SG::WriteHandle<TgcPrepDataContainer>  handle(m_outputprepdataKeys[ibc], ctx);

    const bool externalCachePRD = m_prdContainerCacheKeys.size()>ibc &&
                                  !m_prdContainerCacheKeys[ibc].key().empty();
    if (!externalCachePRD) {
      // record the container in storeGate
      ATH_CHECK(handle.record(std::make_unique<TgcPrepDataContainer>(hashMax)));      
      ATH_MSG_DEBUG("TGC PrepData Container recorded in StoreGate with key " << m_outputprepdataKeys[ibc].key());
      
      // cache the pointer, storegate retains ownership
      state.tgcPrepDataContainer[ibc] = handle.ptr();
    } else {
      // use the cache to get the container
      SG::UpdateHandle<TgcPrepDataCollection_Cache> update(m_prdContainerCacheKeys[ibc], ctx);
      ATH_CHECK(update.isValid());

      ATH_CHECK(handle.record(std::make_unique<Muon::TgcPrepDataContainer>(update.ptr())));
     
      state.tgcPrepDataContainer[ibc] = handle.ptr();
      ATH_MSG_DEBUG("Created container using cache for " << m_prdContainerCacheKeys[ibc].key());
     }
     state.tgcPrepDataCollections[ibc].resize(hashMax);
   }

  /// clean up containers for Coincidence
  for (unsigned int ibc=0; ibc < NBC_TRIG; ibc++) {
    // prepare write handle for this BC
    SG::WriteHandle<TgcCoinDataContainer>  handle(m_outputCoinKeys[ibc], ctx);

    const bool externalCacheCoin = m_coinContainerCacheKeys.size()>ibc && 
                                   !m_coinContainerCacheKeys[ibc].key().empty();
    if(!externalCacheCoin) {
      // No cache (offline case), just record container into store gate
      ATH_CHECK(handle.record(std::make_unique<TgcCoinDataContainer>(hashMax)));
    } else {
      // Using the cache (trigger case)
      SG::UpdateHandle<TgcCoinDataCollection_Cache> update(m_coinContainerCacheKeys[ibc], ctx);
      ATH_CHECK(update.isValid());
      ATH_CHECK(handle.record(std::make_unique<TgcCoinDataContainer>(update.ptr())));    
      ATH_MSG_DEBUG("Created container using cache for " << m_coinContainerCacheKeys[ibc].key());

    }//external cache

    // cache the pointer for duration of function, storegate retains ownership
    state.tgcCoinDataContainer[ibc] = handle.ptr();
    state.tgcCoinDataCollections[ibc].resize(hashMax);

  }  // loop on BC_TRIG
 
  SG::ReadCondHandle<MuonGM::MuonDetectorManager> detMgr{m_muDetMgrKey, ctx};
  ATH_CHECK(detMgr.isValid());
  state.muDetMgr = detMgr.cptr();
  return StatusCode::SUCCESS;
}
StatusCode Muon::TgcRdoToPrepDataToolMT::decode(const EventContext& ctx, 
                                                const std::vector<IdentifierHash>& requestedIdHashVect) const {
  // Object to hold the containers for this decode call
  State state{};
  ATH_CHECK(setupState(ctx, state));

  SG::WriteHandle<xAOD::TgcStripContainer> xAODHandle{};
  if (!m_xAODKey.empty()) {
    xAODHandle =  SG::WriteHandle<xAOD::TgcStripContainer>{m_xAODKey, ctx};
    ATH_CHECK(xAODHandle.record(std::make_unique<xAOD::TgcStripContainer>(), 
                                std::make_unique<xAOD::TgcStripAuxContainer>()));
  }

  int sizeVectorRequested = requestedIdHashVect.size();
  ATH_MSG_DEBUG("decode for " << sizeVectorRequested << " offline collections called");

  const CablingInfo* cinfo = getCabling();
  if (!cinfo) {
    return StatusCode::FAILURE;
  }
  const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};

  std::set<const TgcRdo*> decodedRdoCollVec{}, rdoCollVec{};
  std::vector<bool> decodedOnlineId (cinfo->m_MAX_N_ROD, false);

  // if TGC decoding is switched off stop here
  if(!m_decodeData) {
    ATH_MSG_DEBUG("Stored empty container. Decoding TGC RDO into TGC PrepRawData is switched off");
    return StatusCode::SUCCESS;
  }

  ATH_MSG_DEBUG("Decoding TGC RDO into TGC PrepRawData");

  // retrieve the collection of RDO
  ATH_MSG_DEBUG("Retriving TGC RDO container from the store");
  SG::ReadHandle<TgcRdoContainer> rdoContainer{m_rdoContainerKey, ctx};
  ATH_CHECK(rdoContainer.isValid());
  ///////////// here the RDO container is retrieved and filled -whatever input type we start with- => check the size
  if(rdoContainer->empty()) {
    // empty csm container - no tgc rdo in this event
    ATH_MSG_DEBUG("Empty rdo container - no tgc rdo in this event");
    return StatusCode::SUCCESS;
  }

  ATH_MSG_DEBUG("Not empty rdo container in this event, the container size is " << rdoContainer->size());
  
  // select RDOs to be decoded when seeded mode is used
  if(sizeVectorRequested!=0) {
    unsigned int nRdo = 0;
    for (IdentifierHash offlineCollHash : requestedIdHashVect) {
      uint16_t onlineId = cinfo->m_hashToOnlineId.at(static_cast<unsigned int>(offlineCollHash));

      if(decodedOnlineId.at(onlineId)) {
          ATH_MSG_DEBUG("The ROB with onlineId " << onlineId << " which contains hash "
                      << static_cast<unsigned int>(offlineCollHash)
                      << " is already decoded and skipped");
          continue;
      }

      decodedOnlineId.at(onlineId) = true; // The ROB with this onlineId will be decoded only once
   
      for(const TgcRdo* rdoColl : *rdoContainer) {
        if(rdoColl->identify()==onlineId) {
            if(!decodedRdoCollVec.count(rdoColl)) {
              rdoCollVec.insert(rdoColl);
              nRdo++;
            }
            break;
        }
      }
    }
    ATH_MSG_DEBUG("Number of RDOs to be converted is " << nRdo);
  } // End of selection of RDOs to be decoded

  // Decode Hits
  if(sizeVectorRequested!=0) {
    ATH_MSG_DEBUG("Start loop over rdos - seeded mode");
    // for each RDO collection we collect up all the PRD collections and then write them once filled
    // need a vector because we have the collections for the different bunch crosssings
    for (const TgcRdo* rdo : rdoCollVec) {
      for (const TgcRawData* rd : *rdo) {
        //Since OnlineIds are not unique, need some additional filtering on offline hashId
        //to avoid decoding RDO outside of an RoI
        Identifier offlineId{};
        if(!cinfo->m_tgcCabling->getElementIDfromReadoutID(offlineId, rd->subDetectorId(), 
                                                           rd->rodId(), rd->sswId(), rd->slbId(), 
                                                           rd->bitpos())){
            continue;
        }
        const IdentifierHash tgcHashId = m_idHelperSvc->moduleHash(offlineId);
        if (std::find(requestedIdHashVect.begin(), requestedIdHashVect.end(), tgcHashId) == requestedIdHashVect.end()){
            continue;
        }
        selectDecoder(state,*rd, rdo);
      }
      decodedRdoCollVec.insert(rdo);
    }   
  } else {
    ATH_MSG_DEBUG("Start loop over rdos - unseeded mode");
    for(const TgcRdo* rdoColl : *rdoContainer) {
      if (rdoColl->empty() || decodedRdoCollVec.count(rdoColl) || rdoCollVec.count(rdoColl)) {
          continue;
      }
      ATH_MSG_DEBUG(" Number of RawData in this rdo " << rdoColl->size());
      for (const TgcRawData* rd : *rdoColl) {
          selectDecoder(state,*rd, rdoColl);
      }
      decodedRdoCollVec.insert(rdoColl);
    }
  }
  // first collect up all the HashIDs in any of the containers
  std::set<IdentifierHash> hashesInAnyBC;
  for(unsigned int ibc = 0; ibc < NBC_HIT; ++ibc) {

      uint16_t bcBitMap = 0;
      if(ibc == 0) bcBitMap = TgcPrepData::BCBIT_PREVIOUS;
      else if (ibc == 1) bcBitMap = TgcPrepData::BCBIT_CURRENT;
      else if (ibc == 2) bcBitMap = TgcPrepData::BCBIT_NEXT;
      
      for (std::unique_ptr<TgcPrepDataCollection>& bcColl : state.tgcPrepDataCollections[ibc]) {
        if (!bcColl) continue;
        std::unique_ptr<TgcPrepDataCollection>& allBcColl = state.tgcPrepDataCollections[NBC_HIT][bcColl->identifyHash()];
        if (!allBcColl) {
           allBcColl = std::make_unique<TgcPrepDataCollection>(bcColl->identifyHash());
           allBcColl->setIdentifier(bcColl->identify());
        }
        hashesInAnyBC.insert(bcColl->identifyHash());
        for (const TgcPrepData* prdToUpdate : *bcColl) {
          auto search_itr = std::find_if(allBcColl->begin(), allBcColl->end(),
                                         [prdToUpdate](const TgcPrepData* prd){
                                            return prd->identify() == prdToUpdate->identify();
                                         });
          if (search_itr == allBcColl->end())  {
            auto allBcPrd = std::make_unique<TgcPrepData>(*prdToUpdate);
            allBcPrd->setHashAndIndex(allBcColl->identifyHash(), allBcColl->size());
            allBcPrd->setBcBitMap(bcBitMap);
            allBcColl->push_back(std::move(allBcPrd));
          } else {
            TgcPrepData* allBcPrd = (*search_itr);
            const uint16_t bcBitMap_current = allBcPrd->getBcBitMap();
            ATH_MSG_VERBOSE(m_idHelperSvc->toString(allBcPrd->identify())<<" Old bitmap " << bcBitMap_current 
                          << " adding " << bcBitMap << " to get " << (bcBitMap_current | bcBitMap));
            allBcPrd->setBcBitMap((bcBitMap_current | bcBitMap));
          }
        }
     }
  }

  if (!m_xAODKey.empty()) {
    for (std::unique_ptr<TgcPrepDataCollection>& allBcColl : state.tgcPrepDataCollections[NBC_HIT]) {
        if (!allBcColl) continue;
        for (const TgcPrepData* allBcPrd : *allBcColl) {
          xAOD::TgcStrip* tgcStrip = xAODHandle->push_back(std::make_unique<xAOD::TgcStrip>());
          tgcStrip->setMeasuresPhi(idHelper.isStrip(allBcPrd->identify()));
          tgcStrip->setGasGap(idHelper.gasGap(allBcPrd->identify()));
          tgcStrip->setChannelNumber(idHelper.channel(allBcPrd->identify()));
          tgcStrip->setBcBitMap(allBcPrd->getBcBitMap());
          tgcStrip->setIdentifier(allBcPrd->identify().get_compact());
          /// In the R4 layout both phi & eta measurements are expressed on the same surface. However, the
          //  rotation from eta -> phi is clockwise  --> minus sign in prd creation
          const double locPos = (tgcStrip->measuresPhi() ? -1. : 1.) * allBcPrd->localPosition().x();
          tgcStrip->setMeasurement(m_idHelperSvc->moduleHash(allBcPrd->identify()),
                                  xAOD::MeasVector<1>(locPos),
                                  xAOD::MeasMatrix<1>(allBcPrd->localCovariance()(0,0)));
        }
    }
}

  for (unsigned int k = 0 ; k < state.tgcPrepDataContainer.size(); ++k){
    ATH_CHECK(transferData(*state.tgcPrepDataContainer[k], std::move(state.tgcPrepDataCollections[k])));
  }
  for (unsigned int k = 0 ; k < state.tgcCoinDataContainer.size(); ++k) {
    ATH_CHECK(transferData(*state.tgcCoinDataContainer[k], std::move(state.tgcCoinDataCollections[k])));
  }
  ATH_MSG_DEBUG("Found " << hashesInAnyBC.size() << " hashes that must be added to AllBC container");
  
  return StatusCode::SUCCESS;
}

void Muon::TgcRdoToPrepDataToolMT::selectDecoder(State& state,                                            
                                                 const TgcRawData& rd,
                                                 const TgcRdo* rdoColl) const {

  const CablingInfo* cinfo = getCabling();
  if (!cinfo) {
    return;
  }

  if(!rd.isCoincidence()) {
    if(!decodeHits(state, rd).isSuccess()) {
      ATH_MSG_WARNING("Cannot decode TGC Hits");
    }
  } else if(rd.isCoincidence() && m_fillCoinData) {  // coincidence start
    StatusCode status = StatusCode::SUCCESS;
    if (rd.type()==TgcRawData::TYPE_TRACKLET) {
      if(rd.slbType()==TgcRawData::SLB_TYPE_DOUBLET_WIRE || 
         rd.slbType()==TgcRawData::SLB_TYPE_DOUBLET_STRIP) {
        status = decodeTracklet(state, rd);
      } else if(rd.slbType()==TgcRawData::SLB_TYPE_INNER_WIRE ||
                rd.slbType()==TgcRawData::SLB_TYPE_INNER_STRIP) {
        status = decodeTrackletEIFI(state, rd);
      }
    } // rd.rodId()<13 for Run2, rd.rodId()>12 for Run3
      else if( rd.type()==TgcRawData::TYPE_HIPT && 
               ( (rd.isHipt() && rd.rodId()<13) || rd.rodId()>12) ) { 
      status = decodeHiPt(state, rd);
    } else if( (rd.rodId()<13 && rd.type()==TgcRawData::TYPE_HIPT && (rd.sector() & 4)!=0) || // Run2
               (rd.rodId()>12 && ( rd.type()==TgcRawData::TYPE_INNER_NSW  || // Run3
                                   rd.type()==TgcRawData::TYPE_INNER_BIS  || // Run3
                                   rd.type()==TgcRawData::TYPE_INNER_EIFI || // Run3
                                   rd.type()==TgcRawData::TYPE_INNER_TMDB )) ){ // Run3
      status = decodeInner(state, rd);
    } else if(rd.type()==TgcRawData::TYPE_SL) {
      status = decodeSL(state, rd, rdoColl);
    }
    if(!status.isSuccess()) {
      ATH_MSG_WARNING("Cannot decode TGC Coincidences");
    }
  }
}

StatusCode Muon::TgcRdoToPrepDataToolMT::decodeHits(State& state, const TgcRawData& rd ) const {
  // The channel hit by hardware-ROD supports only three-bunch readout. Data of TgcDigit::BC_NEXTNEXT should be skipped in this function.
  if (rd.bcTag() == TgcDigit::BC_NEXTNEXT) return StatusCode::SUCCESS;

  m_nHitRDOs++; // Count the number of input Hit RDOs.
  bool isConverted{false}, isDuplicated{false}, isInvalid{false}; 
  
  ATH_MSG_DEBUG("decodeHits() :"<<__LINE__<< " sub=" << rd.subDetectorId()
                                << " rod=" << rd.rodId() << " ssw=" << rd.sswId()
                                << " slb=" << rd.slbId() << " bitpos=" << rd.bitpos());

  const CablingInfo* cinfo = getCabling();

  const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};
  
  // select current Hits, =0 for backward compatibility
  // BC_CURRENT=2, BC_UNDEFINED=0
  int locId = (rd.bcTag()==TgcDigit::BC_CURRENT || rd.bcTag()==TgcDigit::BC_UNDEFINED) ? 1 : rd.bcTag()-1;
 
  // repeat two times for ORed channel
  for(int iOr=0; iOr<2; iOr++) {
    bool orFlag = false;
    // check if this channel has ORed partner only when 2nd time
    if(iOr != 0) {
      bool o_found = cinfo->m_tgcCabling->isOredChannel(rd.subDetectorId(), rd.rodId(), rd.sswId(),
                                                        rd.slbId(), rd.bitpos());
      // set OR flag
      if(o_found) orFlag = true;
      else continue;
    }
    
    // get element ID
    Identifier elementId{};
    bool e_found = cinfo->m_tgcCabling->getElementIDfromReadoutID(elementId,
                                                                  rd.subDetectorId(), rd.rodId(),
                                                                  rd.sswId(),  rd.slbId(),
                                                                  rd.bitpos(), orFlag);
    if(!e_found) {
      if(!orFlag) {
        bool show_warning_level = true;

        /* One invalid channel in sector A09: 
           sub=103 rod=9 ssw=6 slb=20 bitpos=151 +offset=0 orFlag=0
           was always seen in 2008 data, at least run 79772 - 91800.
           bug #48828 */
        /* One invalid channel in sector A11: 
           sub=103 rod=11 ssw=2 slb=8 bitpos=41 orFlag=0 
           was seen 5 times in 1,059,867 events of run 159179. */ 
        /* EIFI of MC ByteStream without correction issue : bug 57051 */ 
        if( (rd.subDetectorId()==103 && rd.rodId()==9 && rd.sswId()==6 && rd.slbId()==20 && rd.bitpos()==151) || 
            (rd.subDetectorId()==103 && rd.rodId()==11 && rd.sswId()==2 &&rd.slbId()==8 && rd.bitpos()==41) ||
            (rd.rodId()%3==2 && rd.sswId()==8)) {
          show_warning_level = m_show_warning_level_invalid_A09_SSW6_hit;
          isInvalid = true; 
        } 
        if (msgLvl(show_warning_level ? MSG::WARNING : MSG::DEBUG)){
          msg(show_warning_level ? MSG::WARNING : MSG::DEBUG) << "ElementID not found for "
              << " sub=" << rd.subDetectorId() << " rod=" << rd.rodId() << " ssw=" << rd.sswId()
              << " slb=" << rd.slbId() << " bitpos=" << rd.bitpos() << " orFlag=" << orFlag << endmsg;
        }
      }
      continue;
    }
    const IdentifierHash tgcHashId = m_idHelperSvc->moduleHash(elementId);

    Identifier channelId{};    
    bool c_found = cinfo->m_tgcCabling->getOfflineIDfromReadoutID(channelId,
                                                                  rd.subDetectorId(), rd.rodId(),
                                                                  rd.sswId(), rd.slbId(),
                                                                  rd.bitpos(), orFlag);
    if(!c_found) {
      if(!orFlag) {
        ATH_MSG_WARNING("OfflineID not found for "
                        << " sub=" << rd.subDetectorId()<< " rod=" << rd.rodId()
                        << " ssw=" << rd.sswId()<< " slb=" << rd.slbId()
                        << " bitpos=" << rd.bitpos()<< " orFlag=" << orFlag);
      }
      continue;
    }
    

    std::unique_ptr<TgcPrepDataCollection>& collection = state.tgcPrepDataCollections[locId][tgcHashId];
    if (!collection) {
      collection = std::make_unique<TgcPrepDataCollection>(tgcHashId);
      collection->setIdentifier(elementId);
    }
    const bool duplicate = std::find_if(collection->begin(), collection->end(),
                                        [&channelId](const TgcPrepData* prd){
                                          return prd->identify() == channelId;
                                        }) != collection->end();
    if(duplicate) {
      isDuplicated = true; // A converted PRD of this RDO is duplicated. 
      continue;
    }
    
    const MuonGM::TgcReadoutElement* descriptor = state.muDetMgr->getTgcReadoutElement(channelId);
    if(!isOfflineIdOKForTgcReadoutElement(descriptor, channelId)) {
      ATH_MSG_WARNING("decodeHits: MuonGM::TgcReadoutElement is invalid.");
      continue;
    }
    ATH_MSG_DEBUG("TGC RDO->PrepRawdata: " << m_idHelperSvc->toString(channelId));
    
    std::vector<Identifier> identifierList{channelId};

    Amg::Vector3D position = descriptor->channelPos(channelId);
    Amg::Vector2D hitPos{Amg::Vector2D::Zero()};
    bool onSurface = descriptor->surface(channelId).globalToLocal(position,position,hitPos);
    // the globalToLocal should not fail, if it does produce a WARNING
    if(!onSurface) {
      ATH_MSG_WARNING("Muon::TgcRdoToPrepDataToolMT::decodeHits Amg::Vector2D* hitPos is null."); 
      continue; 
    }
    

    int gasGap  = idHelper.gasGap(channelId);
    int channel = idHelper.channel(channelId);
    const double width = !idHelper.isStrip(channelId) ? descriptor->gangRadialLength(gasGap, channel)
                                                      : descriptor->stripWidth(gasGap, channel);
   
    if(width<s_cutDropPrdsWithZeroWidth && m_dropPrdsWithZeroWidth) { // Invalid PRD's whose widths are zero are dropped. 
      ATH_MSG_WARNING("decodeHits: width= " << width << " is smaller than s_cutDropPrdsWithZeroWidth= " 
                     << s_cutDropPrdsWithZeroWidth<<" "<<m_idHelperSvc->toString(channelId));
      continue; 
    }
    double errPos = width/std::sqrt(12.);
    
    Amg::MatrixX mat(1,1);
    mat.setIdentity();
    mat *= errPos*errPos;
    
    // add the digit to the collection
    // new TgcPrepRawData
    TgcPrepData* newPrepData = new TgcPrepData(channelId, // Readout ID -> Offline ID
                                               tgcHashId, // Readout ID -> Element ID -> Hash 
                                               hitPos, // determined from channelId
                                               identifierList, // holds channelId only
                                               mat, // determined from channelId
                                               descriptor); // determined from channelId
    newPrepData->setHashAndIndex(collection->identifyHash(), collection->size());
    collection->push_back(newPrepData);
    isConverted = true; // This RDO is converted to at least one PRD.  
  }

  if(isConverted) m_nHitPRDs++; // Count the number of output Hit PRDs.
  else if(isDuplicated || isInvalid) m_nHitRDOs--; // Reduce the number of input RDOs. 

  return StatusCode::SUCCESS;
}

StatusCode Muon::TgcRdoToPrepDataToolMT::decodeTracklet(State& state, const TgcRawData& rd) const {
  ++m_nTrackletRDOs; // Count the number of input Tracklet RDOs. 

  const CablingInfo* cinfo = getCabling();

  bool found = false;
  
  //*** Get OfflineId of pivot plane (TGC3) start ***//
  Identifier channelIdOut{};
  found = cinfo->m_tgcCabling->getOfflineIDfromLowPtCoincidenceID(channelIdOut, 
                                                                  rd.subDetectorId(), rd.rodId(),
                                                                  rd.sswId(), rd.slbId(), rd.subMatrix(), 
                                                                  rd.position(), false);
  if(!found) {
    ATH_MSG_DEBUG("decodeTracklet: can't get the OfflineIdOut");
    return StatusCode::SUCCESS;
  }
  //*** Get OfflineId of pivot plane (TGC3) end ***//
  
  //*** Get OfflineId of non-pivot plane (TGC2) start ***//
  int tmp_slbId{0}, tmp_subMatrix{0}, tmp_position{0};
  found = getTrackletInfo(rd, tmp_slbId, tmp_subMatrix, tmp_position);
  if(!found) {
    return StatusCode::SUCCESS;
  }
  Identifier channelIdIn{};
  found = cinfo->m_tgcCabling->getOfflineIDfromLowPtCoincidenceID(channelIdIn, 
                                                                  rd.subDetectorId(), rd.rodId(),
                                                                  rd.sswId(), tmp_slbId, 
                                                                  tmp_subMatrix, tmp_position, true);          
  if(!found) {
    ATH_MSG_DEBUG("decodeTracklet: can't get the OfflineIdIn");
    return StatusCode::SUCCESS;
  }
  //*** Get OfflineId of non-pivot plane (TGC2) end ***//
  
  const IdentifierHash tgcHashId = m_idHelperSvc->moduleHash(channelIdOut);
  

  int locId = (rd.bcTag()==TgcDigit::BC_CURRENT || rd.bcTag()==TgcDigit::BC_UNDEFINED)? 1 : rd.bcTag()-1;

  std::unique_ptr<Muon::TgcCoinDataCollection>& coincollection = state.tgcCoinDataCollections[locId][tgcHashId];
  if (!coincollection) {
      coincollection = std::make_unique<Muon::TgcCoinDataCollection>(tgcHashId);
      coincollection->setIdentifier(m_idHelperSvc->chamberId(channelIdOut));
  }

  int subMatrix = static_cast<int>(rd.subMatrix());
  int trackletId = static_cast<int>(2*rd.slbId()+subMatrix); 
  int delta = static_cast<int>(rd.delta());

  // Check duplicate digits
  for (const TgcCoinData* tgcCoinData : *coincollection) {
    if(TgcCoinData::TYPE_TRACKLET==tgcCoinData->type() && // coincidence type
       channelIdOut==tgcCoinData->identify() && // channelIdOut, identify returns channelIdOut for Tracklet
       channelIdIn==tgcCoinData->channelIdIn() &&  // channelIdIn
       trackletId==tgcCoinData->trackletId() && // trackletId
       delta==tgcCoinData->delta() && // delta
       subMatrix==tgcCoinData->sub()) { // subMatrix 
      
      ATH_MSG_DEBUG("Duplicated TgcCoinData (Tracklet) = "<< m_idHelperSvc->toString(channelIdIn));
      return StatusCode::SUCCESS;
    }
  }

  ATH_MSG_DEBUG("TGC RDO->Coindata for LowPT: " << m_idHelperSvc->toString(channelIdOut)); 

  //*** Get geometry of pivot plane (TGC3) start ***//
  const MuonGM::TgcReadoutElement* descriptor_o = state.muDetMgr->getTgcReadoutElement(channelIdOut);
  if(!isOfflineIdOKForTgcReadoutElement(descriptor_o, channelIdOut)) {
    return StatusCode::SUCCESS;
  }
  
  const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};
  int gasGap_o = idHelper.gasGap(channelIdOut);
  int channel_o = idHelper.channel(channelIdOut);
  double width_o = !idHelper.isStrip(channelIdOut) ? descriptor_o->gangRadialLength(gasGap_o, channel_o)
                                                   : descriptor_o->stripWidth(gasGap_o, channel_o);
 
  if(width_o<s_cutDropPrdsWithZeroWidth && m_dropPrdsWithZeroWidth) { // Invalid PRD's whose widths are zero are dropped.
    return StatusCode::SUCCESS;
  }
  
  Amg::Vector3D position_o = descriptor_o->channelPos(channelIdOut);
  Amg::Vector2D hitPos_o{Amg::Vector2D::Zero()};
  bool onSurface_o = descriptor_o->surface(channelIdOut).globalToLocal(position_o,position_o,hitPos_o);
  // the globalToLocal should not fail, if it does produce a WARNING
  if(!onSurface_o) {
    ATH_MSG_WARNING("Muon::TgcRdoToPrepDataToolMT::decodeTracklet Amg::Vector2D* hitPos_o is null.");
    return StatusCode::SUCCESS;
  }  
  //*** Get geometry of pivot plane (TGC3) end ***//
  
  //*** Get geometry of non-pivot plane (TGC2) start ***//
  const MuonGM::TgcReadoutElement* descriptor_i = state.muDetMgr->getTgcReadoutElement(channelIdIn);
  if(!isOfflineIdOKForTgcReadoutElement(descriptor_i, channelIdIn)) {
    return StatusCode::SUCCESS;
  }
  
  int gasGap_i = idHelper.gasGap(channelIdIn);
  int channel_i = idHelper.channel(channelIdIn);
  double width_i = !idHelper.isStrip(channelIdIn) ? descriptor_i->gangRadialLength(gasGap_i, channel_i)
                                                  : descriptor_i->stripWidth(gasGap_i, channel_i);
  
  if(width_i<s_cutDropPrdsWithZeroWidth && m_dropPrdsWithZeroWidth) { // Invalid PRD's whose widths are zero are dropped.
    return StatusCode::SUCCESS;
  }

  Amg::Vector3D position_i = descriptor_i->channelPos(channelIdIn);
  Amg::Vector2D hitPos_i{Amg::Vector2D::Zero()};
  bool onSurface_i = descriptor_i->surface(channelIdIn).globalToLocal(position_i,position_i,hitPos_i);
  // the globalToLocal should not fail, if it does produce a WARNING
  if(!onSurface_i) {
    ATH_MSG_WARNING("Muon::TgcRdoToPrepDataToolMT::decodeTracklet Amg::Vector2D* hitPos_i is null."); 
    return StatusCode::SUCCESS;
  }  
  //*** Get geometry of non-pivot plane (TGC2) end ***//
  const Amg::Vector2D* hitPosition_o = new Amg::Vector2D(hitPos_o);
  const Amg::Vector2D* hitPosition_i = new Amg::Vector2D(hitPos_i);
  // Add the digit to the collection
  TgcCoinData* newCoinData = new TgcCoinData(channelIdIn,
                                             channelIdOut,
                                             tgcHashId, // determined from channelIdOut
                                             descriptor_i, // determined from channelIdIn
                                             descriptor_o, // determined from channelIdOut
                                             TgcCoinData::TYPE_TRACKLET, 
                                             rd.subDetectorId()==ASIDE, // isAside
                                             idHelper.stationPhi(channelIdOut), // phi
                                             0, // isInner
                                             rd.sswId()==7||rd.sswId()==2, // isForward
                                             idHelper.isStrip(channelIdOut), // isStrip
                                             trackletId, // trackletId
                                             hitPosition_i, // determined from channelIdIn
                                             hitPosition_o, // determined from channelIdOut
                                             width_i, // determined from channelIdIn 
                                             width_o, // determined from channelIdOut
                                             delta, // delta
                                             subMatrix,
                                             0); // subMatrix
  newCoinData->setHashAndIndex(coincollection->identifyHash(), coincollection->size());  
  coincollection->push_back(newCoinData);
  
  ATH_MSG_DEBUG("coincollection->push_back done (for LowPT)");

  m_nTrackletPRDs++; // Count the number of output Tracklet PRDs. 
  
  return StatusCode::SUCCESS;
}

StatusCode Muon::TgcRdoToPrepDataToolMT::decodeTrackletEIFI(State& state, const TgcRawData& rd) const {
  // Count the number of input TrackletEIFI RDOs. 
  m_nTrackletEIFIRDOs++; 

  const CablingInfo* cinfo = getCabling();

  const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};


  // Determine chamber type 
  bool isStrip = rd.slbType()==TgcRawData::SLB_TYPE_INNER_STRIP;
  bool isAside = rd.subDetectorId()==ASIDE; 
  // https://twiki.cern.ch/twiki/pub/Main/TgcDocument/EIFI_PSB_SSW_ConnectionTable_v20080808.pdf
  bool isForward = (rd.slbId()%2==0);
  // Assuming RXID in the above file is equal to slbId 
  // rodId:     2                 5                 8                11
  // slbId FI:  0  2  4  6  8 10  0  2  4  6  8 10  0  2  4  6  8 10  0  2  4  6  8 10 
  //       EI:  1  3  5  7  9 11  1  3  5 XX  9 11  1  3  5  7  9 XX  1  3  5 XX  9 11  
  // slot:     24  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23
  // stationPhi
  //       FI: 24  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23
  //       EI: 21  1  2  3  4  5  6  7  8 XX  9 10 11 12 13 14 15 XX 16 17 18 XX 19 20  
  int slot = ((rd.slbId()/2) + (rd.rodId()-2)*2 + 23)%24 + 1; 

  bool isBackward = false; 
  if(!isForward) { // EI
    // Special case of EI11
    if(slot==15) {
      isBackward = !isAside;
    } else if(slot==16) {
      isBackward =  isAside;
    } else {
      //  A-side phi0 F: phi1 F: phi2 B
      //  C-side phi0 B: phi1 B: phi2 F
      if(isAside) isBackward = (slot%3==2);
      else        isBackward = (slot%3!=2);
    }
  } else { // FI
    isBackward = isAside;
  }

  // Determine bitpos based on wire/strip and subMatrix 
  // Input is chosen so that gasGap is 1 and channel is 4,12,20,28.  
  // One subMatrix covers 8 channels (=BIT_POS_ASD_SIZE/2)
  uint16_t bitpos = 0;
  if(!isStrip) { // Wire
    // For wires, D-Input is gasGap=1 

    int tmpsubMatrix = static_cast<int>(rd.subMatrix());
    if(!isForward) { // If an edge channel of EI fires, subMatrix of the tracklet can be greater than one of the hit channel.  
      if(tmpsubMatrix==3) {
        if(slot== 1 || slot== 3 || slot== 4 || slot== 5 || slot== 6 || slot== 7 || slot== 8 || 
           slot==10 || slot==11 || slot==13 || slot==18 || slot==19 || slot==20) {
          // These slots have only 24 wire channels (=3 submatrixes)  
          tmpsubMatrix = 2;
        }
      } else if(tmpsubMatrix==2) {
        if(slot== 2 || slot==12 || slot==14 || slot==15 || slot==16 || slot==22 || slot==23 || slot==24) {
          // These slots have only 16 wire channels (=2 submatrixes)  
          tmpsubMatrix = 1;
        }
      }
    }

    bitpos = BIT_POS_D_INPUT_ORIGIN - BIT_POS_INPUT_SIZE + 1 + BIT_POS_ASD_SIZE/4*(tmpsubMatrix*2+1); 
  } else { // Strip
    if(isBackward) {
      // For Backward chambers, B-Input is gasGap 1
      bitpos = BIT_POS_B_INPUT_ORIGIN - BIT_POS_INPUT_SIZE + (isAside ? 1 : 0) + BIT_POS_ASD_SIZE/4*(rd.subMatrix()*2+1); 
    } else {
      // For Forward  chambers, A-Input is gasGap 1
      bitpos = BIT_POS_A_INPUT_ORIGIN - BIT_POS_INPUT_SIZE + (isAside ? 0 : 1) + BIT_POS_ASD_SIZE/4*(rd.subMatrix()*2+1); 
    }
  }

  // Retrieve OfflineID from ReadoutID
  Identifier channelIdIn;
  bool o_found = cinfo->m_tgcCabling->getOfflineIDfromReadoutID(channelIdIn,
                                                                rd.subDetectorId(), rd.rodId(),
                                                                rd.sswId(),rd.slbId(), bitpos,
                                                                false/*orflag*/);
  if(!o_found) {
    ATH_MSG_WARNING("Muon::TgcRdoToPrepDataToolMT::decodeTrackletEIFI OfflineID not found for "
                  << " subDetectorId=" << rd.subDetectorId() << " rodId=" << rd.rodId()
                  << " sswId=" << rd.sswId()<< " slbId=" << rd.slbId() << " slbType=" << rd.slbType() 
                  << " subMatrix=" << rd.subMatrix() << " bitpos=" << bitpos << " isStrip=" << isStrip
                  << " isAside=" << isAside<< " isForward=" << isForward<< " slot=" << slot
                  << " isBackward=" << isBackward);
    return StatusCode::SUCCESS;
  }

 
  // Retrieve Hash from ElementID
  const IdentifierHash tgcHashId = m_idHelperSvc->moduleHash(channelIdIn);
  // Index is determined based on bcTag. 
  int locId = (rd.bcTag()==TgcDigit::BC_CURRENT || rd.bcTag()==TgcDigit::BC_UNDEFINED) ? 1 : rd.bcTag()-1;

  std::unique_ptr<Muon::TgcCoinDataCollection>& coincollection = state.tgcCoinDataCollections[locId][tgcHashId];
  if (!coincollection) {
      coincollection = std::make_unique<Muon::TgcCoinDataCollection>(tgcHashId);
      coincollection->setIdentifier(m_idHelperSvc->chamberId(channelIdIn));
  }

  // Check duplicate digits
  for (const TgcCoinData* tgcCoinData : *coincollection) {
    if(TgcCoinData::TYPE_TRACKLET_EIFI==tgcCoinData->type() && // coincidence type
       channelIdIn==tgcCoinData->channelIdIn() && // channelIdIn
       static_cast<int>(rd.subMatrix())==tgcCoinData->sub()) { // sub
      ATH_MSG_DEBUG("Duplicated TgcCoinData (TrackletEIFI) = "<< m_idHelperSvc->toString(channelIdIn));
      return StatusCode::SUCCESS;
    }
  }
  // Get MuonGM::TgcReadoutElement from channelIdIn
  const MuonGM::TgcReadoutElement* descriptor = state.muDetMgr->getTgcReadoutElement(channelIdIn);
  if(!isOfflineIdOKForTgcReadoutElement(descriptor, channelIdIn)) {
    ATH_MSG_WARNING("Muon::TgcRdoToPrepDataToolMT::decodeTrackletEIFI descriptor doesn't contain " 
            << m_idHelperSvc->toString(channelIdIn));
    return StatusCode::SUCCESS;
  }

  // Get Amg::Vector2D from channelIdIn //here I am
  Amg::Vector3D position = descriptor->channelPos(channelIdIn);
  Amg::Vector2D hitPos{Amg::Vector2D::Zero()};
  bool onSurface = descriptor->surface(channelIdIn).globalToLocal(position,position,hitPos);
  // the globalToLocal should not fail, if it does produce a WARNING
  if(!onSurface) {
    ATH_MSG_WARNING("Muon::TgcRdoToPrepDataToolMT::decodeTrackletEIFI Amg::Vector2D* hitPos is null.");
    return StatusCode::SUCCESS;
  }

  // Get width from channelIdIn, one subMatrix covers 8 channels per gasGap 
  int gasGap = idHelper.gasGap(channelIdIn); // 1
  int channel = idHelper.channel(channelIdIn); // 4, 12, 20, 28
  double width {0};
  if(isStrip) { // Strip
    double localZ = (descriptor->transform(channelIdIn).inverse()*descriptor->channelPos(channelIdIn)).z();
    double stripMaxX  = descriptor->stripHighEdgeLocX(gasGap, channel+4, localZ);
    double stripMinX  = descriptor->stripLowEdgeLocX(gasGap, channel-3, localZ);
    width = std::abs(stripMaxX - stripMinX);
  } else { // Wire
    int positiveOffset = +4;
    if(isForward && (slot%3==2) && channel==28) positiveOffset = +2; // T10S has only 30 channels.  
    double gangMaxZ = descriptor->gangLongWidth(gasGap, channel+positiveOffset);
    double gangMinZ = descriptor->gangShortWidth(gasGap, channel-3);
    width = std::abs(gangMaxZ - gangMinZ);
  }

  const Amg::Vector2D* hitPosition = new Amg::Vector2D(hitPos);
  // Add the digit to the collection
  TgcCoinData* newCoinData = new TgcCoinData(channelIdIn, 
                                             tgcHashId, // determined from channelIdIn
                                             descriptor, // determined from channelIdIn
                                             TgcCoinData::TYPE_TRACKLET_EIFI,
                                             isAside, 
                                             m_idHelperSvc->tgcIdHelper().stationPhi(channelIdIn), 
                                             isForward, 
                                             isStrip, 
                                             hitPosition, // determined from channelIdIn
                                             width, // determined from channelIdIn
                                             static_cast<int>(rd.subMatrix())); // subMatrix
  newCoinData->setHashAndIndex(coincollection->identifyHash(), coincollection->size());  
  coincollection->push_back(newCoinData);

  // Count the number of output Tracklet EIFI PRDs.
  m_nTrackletEIFIPRDs++; 
  return StatusCode::SUCCESS;
}

StatusCode Muon::TgcRdoToPrepDataToolMT::decodeHiPt(State& state, const TgcRawData& rd0) const {
  m_nHiPtRDOs++; // Count the number of input HiPt RDOs. 

  const CablingInfo* cinfo = getCabling();

  // conversion for Run3
  uint16_t tmprodId{}, tmpsector{};
  convertToRun2(rd0,tmprodId,tmpsector);
  const TgcRawData rd(rd0.bcTag(), rd0.subDetectorId(), tmprodId,
                      rd0.l1Id(), rd0.bcId(), rd0.isStrip(),
                      rd0.isForward(), tmpsector, rd0.chip(),
                      rd0.index(), rd0.isHipt(), rd0.hitId(),
                      rd0.hsub(), rd0.delta(), rd0.inner());

  // Protection against invalid subDetectorId and isForward
  if((rd.subDetectorId()!=ASIDE && rd.subDetectorId()!=CSIDE)) {
    ATH_MSG_DEBUG("TgcRdoToPrepDataToolMT::decodeHiPt::Unknown subDetectorId!!");
    return StatusCode::SUCCESS;
  }

  // Protection against invalid hitId
  if(rd.hitId()==0) {
    ATH_MSG_DEBUG("Invalid hitId_rdo_hipt, hitId == 0!! skip to convert this RDO to PRD");
    return StatusCode::SUCCESS;
  }
  int slbsubMatrix = 0;
  bool isBackward = isBackwardBW(rd);  // Backward or Forward
  int deltaBeforeConvert = getDeltaBeforeConvert(rd);
  
  bool found = false;
  
  Identifier channelIdOut_tmp{};
  std::array<Identifier, 2> channelIdOut{};
  std::array<int, 2>  bitpos_o{}, slbchannel_o{};
  int sswId_o{0}, sbLoc_o{0}, slbId_o{0};
  
  Identifier channelIdIn_tmp{};
  std::array<Identifier, 4> channelIdIn{};
  std::array<int, 4> bitpos_i{}, slbchannel_i{};
  int sswId_i{0}, sbLoc_i{0};
  
  std::array<int, 4> slbId_in {}, sbLoc_in{}; // only used for wire

  std::array<int, 4> gasGap_i{}, channel_i{}; 
  double width_i{0.},hit_position_i{0};
  Amg::Vector2D tmp_hitPos_i{Amg::Vector2D::Zero()};

  std::array<int, 2> gasGap_o{}, channel_o{};
  double width_o{0.}, hit_position_o{0.};
  Amg::Vector2D tmp_hitPos_o{Amg::Vector2D::Zero()};

  const MuonGM::TgcReadoutElement* descriptor_ii = nullptr;
  const MuonGM::TgcReadoutElement* descriptor_oo = nullptr;

  //*** TGC3 start ***//
  // RDOHighPtID --> (Sim)HighPtID --> OfflineID --> ReadoutID --> getSLBID
  found = getHiPtIds(rd, sswId_o, sbLoc_o, slbId_o);
  if(!found) {
    return StatusCode::SUCCESS;
  }

  // get the OfflineID of cernter of ROI of TGC3
  if(!rd.isStrip()) { // wire
    getBitPosOutWire(rd, slbsubMatrix, bitpos_o);
  } else { // strip
    getBitPosOutStrip(rd, slbsubMatrix, bitpos_o);
  }
  for(int i=0; i<2; i++) {
    found = cinfo->m_tgcCabling->getOfflineIDfromReadoutID(channelIdOut[i], rd.subDetectorId(), rd.rodId(), 
                                                           sswId_o, sbLoc_o, bitpos_o[i]);
    if(!found) {
      ATH_MSG_DEBUG("Failed to get OfflineID from ReadoutID for Pivot " << (rd.isStrip() ? "Strip" : "Wire") << ".");
      return StatusCode::SUCCESS;
    }
  }
   
  //}
  //*** TGC3 end ***//

  //*** TGC1 start ***//
  // get the OfflineID of cernter of ROI of TGC1
  if(!rd.isStrip()) { // wire
    getBitPosInWire(rd, deltaBeforeConvert, bitpos_i, slbchannel_i, slbId_in, sbLoc_in, sswId_i, bitpos_o, 
                    slbchannel_o, slbId_o);
  } else { // strip
    getBitPosInStrip(rd, deltaBeforeConvert, bitpos_i, slbchannel_i, sbLoc_i, sswId_i, bitpos_o, slbchannel_o);
  }
  for(int i=0; i<4; i++) {
    found = cinfo->m_tgcCabling->getOfflineIDfromReadoutID(channelIdIn[i], rd.subDetectorId(), rd.rodId(), sswId_i,
                                                           rd.isStrip() ? sbLoc_i : sbLoc_in[i], 
                                                           bitpos_i[i]);
    if(!found) {
      ATH_MSG_DEBUG("Failed to get OfflineID from ReadoutID for Pivot "
                    << (rd.isStrip() ? "Strip" : "Wire") << ".");
      return StatusCode::SUCCESS;
    }
  }
  //}
  //*** TGC1 end ***//
  
  ATH_MSG_DEBUG("TGC RDO->Coindata for HIPT: " << m_idHelperSvc->toString(channelIdOut[1])); 

  const IdentifierHash tgcHashId = m_idHelperSvc->moduleHash(channelIdOut[1]);

  int locId = (rd.bcTag()==TgcDigit::BC_CURRENT || rd.bcTag()==TgcDigit::BC_UNDEFINED)  ? 1 : rd.bcTag()-1;

  std::unique_ptr<Muon::TgcCoinDataCollection>& coincollection = state.tgcCoinDataCollections[locId][tgcHashId];
  if (!coincollection) {
      coincollection = std::make_unique<Muon::TgcCoinDataCollection>(tgcHashId);
      coincollection->setIdentifier(m_idHelperSvc->chamberId(channelIdOut[1]));
  }

  //*** TGC3 start ***// 
  // Get geometry of pivot plane 
  std::array<const MuonGM::TgcReadoutElement*, 2> descriptor_o{state.muDetMgr->getTgcReadoutElement(channelIdOut[0]), 
                                                               state.muDetMgr->getTgcReadoutElement(channelIdOut[1])};
  for(int i=0; i<2; i++) {
    if(!isOfflineIdOKForTgcReadoutElement(descriptor_o[i], channelIdOut[i])) {
      return StatusCode::SUCCESS;
    }
  }

  const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};
  for(int i=0; i<2; i++) { 
    gasGap_o[i] = idHelper.gasGap(channelIdOut[i]);
    channel_o[i] = idHelper.channel(channelIdOut[i]);
  }

  if(!rd.isStrip()) { // wire
    found = getPosAndIdWireOut(descriptor_o, channelIdOut,
                               gasGap_o, channel_o,
                               width_o, hit_position_o, tmp_hitPos_o,
                               channelIdOut_tmp);
  } else { //strip
    found = getPosAndIdStripOut(descriptor_o, channelIdOut,
                                gasGap_o, channel_o,
                                width_o, hit_position_o, tmp_hitPos_o,
                                channelIdOut_tmp,
                                isBackward, rd.subDetectorId()==ASIDE);
  }
  if(!found) {
    return StatusCode::SUCCESS;
  }
  if(width_o<s_cutDropPrdsWithZeroWidth && m_dropPrdsWithZeroWidth) { // Invalid PRD's whose widths are zero are dropped.
    return StatusCode::SUCCESS;
  }

  descriptor_oo = state.muDetMgr->getTgcReadoutElement(channelIdOut_tmp);
  if(!isOfflineIdOKForTgcReadoutElement(descriptor_oo, channelIdOut_tmp)) {
    return StatusCode::SUCCESS;
  }
  //}
  //*** TGC3 end ***// 
  
  //*** TGC1 start ***// 
  // Get geometry of non-pivot plane 
  std::array<const MuonGM::TgcReadoutElement*, 4> descriptor_i{state.muDetMgr->getTgcReadoutElement(channelIdIn[0]), 
                                                               state.muDetMgr->getTgcReadoutElement(channelIdIn[1]), 
                                                               state.muDetMgr->getTgcReadoutElement(channelIdIn[2]), 
                                                               state.muDetMgr->getTgcReadoutElement(channelIdIn[3])};
  for(int i=0; i<4; i++) {
    if(!isOfflineIdOKForTgcReadoutElement(descriptor_i[i], channelIdIn[i])) {
      return StatusCode::SUCCESS;
    }
  }
  for(int i=0; i<4; i++) { 
    gasGap_i[i] = idHelper.gasGap(channelIdIn[i]); 
    channel_i[i] = idHelper.channel(channelIdIn[i]); 
  }
    
  if(!rd.isStrip()) { // WIRE
    found = getPosAndIdWireIn(descriptor_i, channelIdIn,
                              gasGap_i, channel_i,
                              width_i, hit_position_i, tmp_hitPos_i,
                              channelIdIn_tmp);
  } else { // STRIP
    found = getPosAndIdStripIn(descriptor_i, channelIdIn,
                               gasGap_i, channel_i,
                               width_i, hit_position_i, tmp_hitPos_i,
                               channelIdIn_tmp,
                               isBackward, (rd.subDetectorId()==ASIDE));
  }
  if(!found) {
    return StatusCode::SUCCESS;
  }
  if(width_i<s_cutDropPrdsWithZeroWidth && m_dropPrdsWithZeroWidth) { // Invalid PRD's whose widths are zero are dropped.
    return StatusCode::SUCCESS;
  }

  descriptor_ii = state.muDetMgr->getTgcReadoutElement(channelIdIn_tmp);
  if(!isOfflineIdOKForTgcReadoutElement(descriptor_ii, channelIdIn_tmp)) {
    return StatusCode::SUCCESS;
  }
  //}
  //*** TGC1 end ***// 

  int trackletId = 2*sbLoc_o + slbsubMatrix;
  int delta = static_cast<int>(rd.delta());
  int hsub = static_cast<int>(rd.hsub());
  int inner = static_cast<int>(rd.inner());

  // check duplicate digits
  for (const TgcCoinData* tgcCoinData : *coincollection) {
    if((TgcCoinData::TYPE_HIPT==tgcCoinData->type()) && // Coincidence type
       (channelIdOut_tmp==tgcCoinData->identify()) && // channelIdOut, identify returns channelIdOut for HiPt
       (channelIdIn_tmp==tgcCoinData->channelIdIn()) && // channelIdIn
       (trackletId==tgcCoinData->trackletId()) && // trackletId 
       (delta==tgcCoinData->delta()) && // delta
       (hsub==tgcCoinData->sub()) && // hsub
       (inner==tgcCoinData->inner())) { 
      if(38<=trackletId && trackletId<=41) {
        // This drop is most probably due to the fix of the HiPt Endcap Strip Board bug. 
        m_nHiPtRDOs--; // Reduce the number of input RDOs. 
      }
      ATH_MSG_DEBUG("Duplicated TgcCoinData (HiPt) = "<< m_idHelperSvc->toString(channelIdOut_tmp));
      return StatusCode::SUCCESS;
    }
  }
  
  auto hitPos_o = std::make_unique<Amg::Vector2D>(tmp_hitPos_o);
  auto hitPos_i = std::make_unique<Amg::Vector2D>(tmp_hitPos_i);
 
  TgcCoinData* newCoinData = new TgcCoinData(channelIdIn_tmp,
                                             channelIdOut_tmp,
                                             tgcHashId, // determined from channelIdOut[1]
                                             descriptor_ii, // determined from channelIdIn_tmp
                                             descriptor_oo, // determined from channelIdOut_tmp
                                             TgcCoinData::TYPE_HIPT, // Coincidence type
                                             rd.subDetectorId()==ASIDE, // isAside
                                             idHelper.stationPhi(channelIdOut_tmp), // phi
                                             0, // isInner
                                             rd.isForward(), // isForward
                                             rd.isStrip(), // isStrip
                                             trackletId, // trackletId
                                             hitPos_i.release(),
                                             hitPos_o.release(),
                                             width_i,
                                             width_o,
                                             delta, // delta
                                             hsub,  // hsub
                                             inner);
  // add the digit to the collection
  newCoinData->setHashAndIndex(coincollection->identifyHash(), coincollection->size());  
  coincollection->push_back(newCoinData);

  ATH_MSG_DEBUG("coincollection->push_back done (for HIPT)");

  m_nHiPtPRDs++; // Count the number of output HiPt PRDs. 
  
  return StatusCode::SUCCESS;
}

StatusCode Muon::TgcRdoToPrepDataToolMT::decodeInner(State& state,const TgcRawData& rd) const {
  m_nHiPtRDOs++; // Count the number of input HiPt RDOs. 

  const CablingInfo* cinfo = getCabling();
  if (!cinfo) {
    return StatusCode::FAILURE;
  }

  int subDetectorId = rd.subDetectorId();
  // Protection against invalid subDetectorId and isForward
  if(subDetectorId!=ASIDE && subDetectorId!=CSIDE) {
    ATH_MSG_DEBUG("TgcRdoToPrepDataToolMT::decodeHiPt::Unknown subDetectorId!!");
    return StatusCode::SUCCESS;
  }

  bool isInner = ((rd.sector() & 4) != 0 ); // Inner flag for EIFI and Tilecal

 
  Identifier channelIdIn{};
  Identifier channelIdOut{};
  int sswId_o = 9;
  int sbLoc_o = rd.sector() & 3;
  int inner   = rd.inner();
  bool isStrip = rd.isStrip();

  int phi = 0; bool isAside = false; bool isEndcap = false;
  if(rd.rodId() < 13){ // Run2
    cinfo->m_tgcCabling->getSLIDfromReadoutID(phi, isAside, isEndcap, subDetectorId,
                                              rd.rodId(), sswId_o, sbLoc_o);
  }else{ // Run3
    sbLoc_o = rd.sector();
    cinfo->m_tgcCabling->getSLIDfromSReadoutID(phi, isAside, subDetectorId, rd.rodId(), sbLoc_o, rd.isForward());
    isEndcap = !rd.isForward();
    if(rd.type()==TgcRawData::TYPE_INNER_NSW){
      isInner = true; isStrip = false;
      inner = (rd.nsweta()    << Muon::TgcCoinData::INNER_NSW_R_BITSHIFT)
            + (rd.nswphi()    << Muon::TgcCoinData::INNER_NSW_PHI_BITSHIFT)
            + (rd.nswdtheta() << Muon::TgcCoinData::INNER_NSW_DTHETA_BITSHIFT)
            + (rd.nswphires() << Muon::TgcCoinData::INNER_NSW_PHIRES_BITSHIFT)
            + (rd.nswlowres() << Muon::TgcCoinData::INNER_NSW_LOWRES_BITSHIFT)
            + (rd.nswid()     << Muon::TgcCoinData::INNER_NSW_ID_BITSHIFT)
            + (((rd.nswcand()>>TgcRawData::NSW_BCID_BITSHIFT)&TgcRawData::NSW_BCID_BIT)   << Muon::TgcCoinData::INNER_NSW_BCID_BITSHIFT)
            + (((rd.nswcand()>>TgcRawData::NSW_INPUT_BITSHIFT)&TgcRawData::NSW_INPUT_BIT) << Muon::TgcCoinData::INNER_NSW_INPUT_BITSHIFT);
    } else if(rd.type()==TgcRawData::TYPE_INNER_BIS){
      isInner = true; isStrip = true;
      inner = (rd.rpceta()  << Muon::TgcCoinData::INNER_RPC_ETA_BITSHIFT)
            + (rd.rpcphi()  << Muon::TgcCoinData::INNER_RPC_PHI_BITSHIFT)
            + (rd.rpcdeta() << Muon::TgcCoinData::INNER_RPC_DETA_BITSHIFT)
            + (rd.rpcdphi() << Muon::TgcCoinData::INNER_RPC_DPHI_BITSHIFT)
            + (((rd.rpcflag()>>TgcRawData::RPC_FLAG_BITSHIFT)&TgcRawData::RPC_FLAG_BIT) << Muon::TgcCoinData::INNER_RPC_FLAG_BITSHIFT)
            + (((rd.rpcflag()>>TgcRawData::RPC_BCID_BITSHIFT)&TgcRawData::RPC_BCID_BIT) << Muon::TgcCoinData::INNER_RPC_BCID_BITSHIFT);
    }else if(rd.type()==TgcRawData::TYPE_INNER_EIFI){
      isInner = false; isStrip = false;
      inner = (rd.ei()  << Muon::TgcCoinData::INNER_EIFI_EI_BITSHIFT)
            + (rd.fi()  << Muon::TgcCoinData::INNER_EIFI_FI_BITSHIFT)
            + (rd.cid() << Muon::TgcCoinData::INNER_EIFI_CID_BITSHIFT);
    }else if(rd.type()==TgcRawData::TYPE_INNER_TMDB){
      isInner = false; isStrip = true;
      inner = (rd.tmdbmod()  << Muon::TgcCoinData::INNER_TILE_MODULE_BITSHIFT)
            + (rd.tmdbbcid() << Muon::TgcCoinData::INNER_TILE_BCID_BITSHIFT);
    }
  }

  int locId = (rd.bcTag()==TgcDigit::BC_CURRENT || rd.bcTag()==TgcDigit::BC_UNDEFINED) ? 1 : rd.bcTag()-1;
  
  auto hitPos_o = std::make_unique< Amg::Vector2D >(Amg::Vector2D::Zero());
  auto hitPos_i = std::make_unique< Amg::Vector2D >(Amg::Vector2D::Zero());

  const MuonGM::TgcReadoutElement* descriptor_ii = nullptr;
  const MuonGM::TgcReadoutElement* descriptor_oo = nullptr;

  std::string stationName = "T3E";
  int stationEta = isAside ? 1 : -1;
  int stationPhi = phi;
  bool isValid{false};
  Identifier elementId = m_idHelperSvc->tgcIdHelper().elementID(stationName, stationEta, stationPhi, isValid);
  const IdentifierHash tgcHashId = m_idHelperSvc->moduleHash(elementId);

  std::unique_ptr<Muon::TgcCoinDataCollection>& coincollection = state.tgcCoinDataCollections[locId][tgcHashId];
  if (!coincollection) {
      coincollection = std::make_unique<Muon::TgcCoinDataCollection>(tgcHashId);
      coincollection->setIdentifier(elementId);
  }
  
  ATH_MSG_DEBUG("Inner Data Word, phi: " << phi << " isAside: " << isAside << " isEndcap: " << isEndcap
                << " subDetectorId: " << subDetectorId << " isStrip: " << rd.isStrip()
                << " rodId: " << rd.rodId() << " slbId: " << sbLoc_o << " inner:"<< rd.inner());

  TgcCoinData* newCoinData = new TgcCoinData(channelIdIn,  // empty
                                             channelIdOut, // empty
                                             tgcHashId, // determined from channelIdOut[1]
                                             descriptor_ii, // determined from channelIdIn_tmp
                                             descriptor_oo, // determined from channelIdOut_tmp
                                             (rd.rodId()<13)?(TgcCoinData::TYPE_HIPT):(TgcCoinData::TYPE_UNKNOWN), // Coincidence type: rd.rodId()<13 for Run2, rd.rodId()>12 for Run3
                                             isAside, // isAside
                                             phi, // phi
                                                                 isInner, // Selection for NSW/BIS/EIFI/TMDB
                                             !isEndcap, // isForward
                                             isStrip, // Selection for NSW/BIS/EIFI/TMDB
                                             0, // trackletId
                                             hitPos_i.release(),
                                             hitPos_o.release(),
                                             0., // width_i,
                                             0., // width_o,
                                             0, // delta,
                                             0, // hsub,
                                             inner);
  // add the digit to the collection
  newCoinData->setHashAndIndex(coincollection->identifyHash(), coincollection->size());  
  coincollection->push_back(newCoinData);

  ATH_MSG_DEBUG("coincollection->push_back done (for Inner)");

  m_nHiPtPRDs++; // Count the number of output HiPt PRDs.

  return StatusCode::SUCCESS;
}

StatusCode Muon::TgcRdoToPrepDataToolMT::decodeSL(State& state, const TgcRawData& rd0, const TgcRdo* rdoColl) const {
  m_nSLRDOs++; // Count the number of input SL RDOs. 

  // conversion for Run3
  uint16_t tmprodId, tmpsector;
  convertToRun2(rd0,tmprodId,tmpsector);
  const TgcRawData rd(rd0.bcTag(), rd0.subDetectorId(), tmprodId, rd0.l1Id(),rd0.bcId(),
                      rd0.isForward(), tmpsector, rd0.innerflag(), rd0.coinflag(),
                      rd0.isMuplus(), rd0.threshold(), rd0.roi());

  // Protection against invalid subDetectorId
  if(rd.subDetectorId()!=ASIDE && rd.subDetectorId()!=CSIDE) {
    ATH_MSG_DEBUG("TgcRdoToPrepDataToolMT::decodeSL::Unknown subDetectorId!!");
    return StatusCode::SUCCESS;
  }
  
  bool found = false;

  std::array<Identifier, 3> channelId_wire{};
  int index_w{0}, chip_w{0}, hitId_w{0}, sub_w{0}, sswId_w{0}, sbLoc_w{0}, subMatrix_w{0};
  std::array<int, 3> bitpos_w{};

  //***  get OfflineID, center of ROI of R (wire) ***//
  found = getSLIds(false, rd, channelId_wire, index_w, chip_w, hitId_w, sub_w, sswId_w, sbLoc_w, 
                   subMatrix_w, bitpos_w);
  if(!found) {
    return StatusCode::SUCCESS;
  }

  std::array<Identifier, 3> channelId_strip{};
  int index_s{0}, chip_s{0}, hitId_s{0}, sub_s{0}, sswId_s{0}, sbLoc_s{0}, subMatrix_s{0};
  std::array<int, 3> bitpos_s{};

  //***  get OfflineID, center of ROI of phi (strip) ***//
  found = getSLIds(true, rd, channelId_strip, index_s, chip_s, hitId_s, sub_s, sswId_s, sbLoc_s, subMatrix_s, 
                   bitpos_s, isIncludedInChamberBoundary(rd), rdoColl, index_w, chip_w, hitId_w, sub_w);
  if(!found) {
    return StatusCode::SUCCESS;
  }

  ATH_MSG_DEBUG("TGC RDO->TgcCoindata(SL): " << m_idHelperSvc->toString(channelId_wire[1])); 
  
  const IdentifierHash tgcHashId= m_idHelperSvc->moduleHash(channelId_wire[1]);
 

  int locId = (rd.bcTag()==TgcDigit::BC_CURRENT || rd.bcTag()==TgcDigit::BC_UNDEFINED) ? 1 : rd.bcTag()-1;

  std::unique_ptr<Muon::TgcCoinDataCollection>& coincollection = state.tgcCoinDataCollections[locId][tgcHashId];
  if (!coincollection) {
      coincollection = std::make_unique<Muon::TgcCoinDataCollection>(tgcHashId);
      coincollection->setIdentifier(m_idHelperSvc->chamberId(channelId_wire[1]));
  }
  
  
  int trackletId = 2*sbLoc_w + subMatrix_w;
  int trackletIdStrip = 2*sbLoc_s + subMatrix_s;
  int roi = static_cast<int>(rd.roi());
  int pt = static_cast<int>(rd.threshold());
  if(rd0.rodId()>12){ // Run3:  pT 4 bit, CoinFlag 3 bit, InnerFlag 4 bit
    pt += (static_cast<int>( rd.coinflag() ) << 4 );
    pt += (static_cast<int>( rd.innerflag() ) << 7 );
  }
  bool veto = rd.isVeto();
  bool isPositiveDeltaR = rd.isMuplus(); // Current SL sets isMuplus flag based on sign of deltaR. 
                                              // Postive deltaR gives isMuplus=true.
  // check duplicate digits
  for (const TgcCoinData* tgcCoinData : *coincollection) {
    if(TgcCoinData::TYPE_SL==tgcCoinData->type() && channelId_wire[2]==tgcCoinData->identify() &&
       trackletId==tgcCoinData->trackletId() && trackletIdStrip==tgcCoinData->trackletIdStrip() &&       
       roi==tgcCoinData->roi() && pt==tgcCoinData->pt() &&  veto==tgcCoinData->veto() && 
       isPositiveDeltaR==tgcCoinData->isPositiveDeltaR()) {
      ATH_MSG_DEBUG("Duplicated TgcCoinData (SL) = "
            << m_idHelperSvc->toString(channelId_wire[2]));
      return StatusCode::SUCCESS;
    }
  }
 
  //*** R (wire) start ***//
  double width_w{0.}, tmp_r{0.}, tmp_wire_z{0.};
  found = getSLWireGeometry(channelId_wire, width_w, tmp_r, tmp_wire_z);
  if(!found) {
    return StatusCode::SUCCESS;
  }
  if(width_w<s_cutDropPrdsWithZeroWidth && m_dropPrdsWithZeroWidth) { // Invalid PRD's whose widths are zero are dropped.
    return StatusCode::SUCCESS;
  }

  double tmp_eta = 0.; 
  bool isGoodEta = getEtafromRZ(tmp_r, tmp_wire_z, tmp_eta); 
  if(!isGoodEta) { 
    ATH_MSG_WARNING("Conversion from r and z to eta by Muon::TgcRdoToPrepDataToolMT::getEtafromRZ failed."); 
    return StatusCode::SUCCESS; 
  } 
  if(tmp_wire_z<0.) tmp_eta *= -1.; 
  //*** R (wire) end ***//

  //*** Phi (strip) start ***//
  double width_s{0.}, tmp_phi{0.};
  found = getSLStripGeometry(channelId_strip, isBackwardBW(rd), (rd.subDetectorId()==ASIDE), width_s, tmp_phi);
  if(!found) {
    return StatusCode::SUCCESS;
  }
  if(width_s<s_cutDropPrdsWithZeroWidth && m_dropPrdsWithZeroWidth) { // Invalid PRD's whose widths are zero are dropped.
    return StatusCode::SUCCESS;
  }
  //*** Phi (strip) end ***//
  const MuonGM::TgcReadoutElement* descriptor_w2 = state.muDetMgr->getTgcReadoutElement(channelId_wire[2]);
  if(!isOfflineIdOKForTgcReadoutElement(descriptor_w2, channelId_wire[2])) {
    return StatusCode::SUCCESS;
  }
  
  Amg::Vector3D tmp_gp(tmp_r*std::cos(tmp_phi), tmp_r*std::sin(tmp_phi), tmp_wire_z);
  Amg::Vector2D tmp_hitPos{Amg::Vector2D::Zero()};
  bool onSurface = descriptor_w2->surface(channelId_wire[2]).globalToLocal(tmp_gp,tmp_gp,tmp_hitPos);
    // If TGC A-lines with rotations are used, the z-coordinate of a chamber depends on position. 
    // In this case, the global to local conversion fails. 
    // Obtain the local position in a different way. 
  const Amg::Vector2D* hitPos = !onSurface ? new Amg::Vector2D(tmp_hitPos) : 
                                             getSLLocalPosition(descriptor_w2, channelId_wire[2], tmp_eta, tmp_phi);


  Amg::MatrixX mat(2,2);
  mat.setIdentity();
  mat(0,0) = width_w;
  mat(1,1) = width_s;
  const Amg::MatrixX* errMat = new Amg::MatrixX(std::move(mat));


  // new TgcCoinData          
  TgcCoinData* newCoinData = new TgcCoinData(channelId_wire[2],
                                             tgcHashId, // determined from channelId_wire[1]
                                             descriptor_w2, // determined from channelId_wire[2]
                                             TgcCoinData::TYPE_SL, // Coincidence type
                                             rd.subDetectorId()==ASIDE, // isAside
                                             m_idHelperSvc->tgcIdHelper().stationPhi(channelId_wire[2]), // phi
                                             rd.isForward(), // isForward
                                             trackletId, // trackletId 
                                             trackletIdStrip, // trackletIdStrip
                                             hitPos,
                                             errMat,
                                             roi, // roi from RDO
                                             pt, // threshold from RDO
                                             veto, // veto flag from RDO
                                             isPositiveDeltaR); // isMuplus from RDO

  // add the digit to the collection
  newCoinData->setHashAndIndex(coincollection->identifyHash(), coincollection->size());  
  coincollection->push_back(newCoinData);
  ATH_MSG_DEBUG("coincollection->push_back done (for SL)");

  m_nSLPRDs++; // Count the number of output SL PRDs.
  return StatusCode::SUCCESS;
}

int Muon::TgcRdoToPrepDataToolMT::getbitpos(int channel, TgcRawData::SlbType slbType) 
{
  int bitpos = -BIT_POS_INPUT_SIZE +1;
  if(slbType==TgcRawData::SLB_TYPE_TRIPLET_WIRE) {
    if(channel<0 || channel>=WT_MAP_SIZE) return -1; // Invalid channel

    if(     channel%3==0) bitpos += BIT_POS_C_INPUT_ORIGIN; // C-Input
    else if(channel%3==1) bitpos += BIT_POS_B_INPUT_ORIGIN; // B-Input
    else                  bitpos += BIT_POS_A_INPUT_ORIGIN; // A-Input
    return bitpos + channel/3;
  } else if(slbType==TgcRawData::SLB_TYPE_TRIPLET_STRIP || 
        slbType==TgcRawData::SLB_TYPE_DOUBLET_STRIP) {
    if(channel<0 || channel>=ST_MAP_SIZE) return -1; // Invalid channel, SD_MAP_SIZE is equal to ST_MAP_SIZE. 

    if(channel%2==0) bitpos += BIT_POS_B_INPUT_ORIGIN; // B-Input
    else             bitpos += BIT_POS_A_INPUT_ORIGIN; // A-Input
    return bitpos + channel/2;
  } else if(slbType==TgcRawData::SLB_TYPE_DOUBLET_WIRE) {
    if(channel<0 || channel>=WD_MAP_SIZE) return -1; // Invalid channel

    if(channel%2==0) bitpos += BIT_POS_B_INPUT_ORIGIN; // B-Input
    else             bitpos += BIT_POS_A_INPUT_ORIGIN; // A-Input
    return bitpos + channel/2;
  } else {
    return -1;
  }
}

int Muon::TgcRdoToPrepDataToolMT::getchannel(int bitpos, TgcRawData::SlbType slbType) 
{
  int input = -1;
  if(     (bitpos<=BIT_POS_A_INPUT_ORIGIN) && (bitpos>BIT_POS_A_INPUT_ORIGIN-BIT_POS_INPUT_SIZE)) input = 2; // A-Input
  else if((bitpos<=BIT_POS_B_INPUT_ORIGIN) && (bitpos>BIT_POS_B_INPUT_ORIGIN-BIT_POS_INPUT_SIZE)) input = 1; // B-Input
  else if((bitpos<=BIT_POS_C_INPUT_ORIGIN) && (bitpos>BIT_POS_C_INPUT_ORIGIN-BIT_POS_INPUT_SIZE)) input = 0; // C-Input
  // D-Input is not implemented yet. 
  else return -1; // Invalid bitpos
  if(input==0 && slbType!=TgcRawData::SLB_TYPE_TRIPLET_WIRE) return -1; // Only Wire Triplet has C-input.
  
  int channel = 1-BIT_POS_INPUT_SIZE;
  if(slbType==TgcRawData::SLB_TYPE_TRIPLET_WIRE) {
    if(     input==2) channel += BIT_POS_A_INPUT_ORIGIN; // A-input
    else if(input==1) channel += BIT_POS_B_INPUT_ORIGIN; // B-input
    else              channel += BIT_POS_C_INPUT_ORIGIN; // C-input
    channel = 3*(bitpos - channel) + input;
    return channel; // C(0)->B(1)->A(2)->C(3)->B(4)->A(5)-> ... ->C(96-3)->B(96-2)->A(96-1)
  } else if(slbType==TgcRawData::SLB_TYPE_TRIPLET_STRIP ||
        slbType==TgcRawData::SLB_TYPE_DOUBLET_WIRE  ||
        slbType==TgcRawData::SLB_TYPE_DOUBLET_STRIP) {
    if(input==2) channel += BIT_POS_A_INPUT_ORIGIN; // A-input
    else         channel += BIT_POS_B_INPUT_ORIGIN; // B-input
    channel = 2*(bitpos - channel) + input-1;
    return channel; // B(0)->A(1)->B(2)->A(3)-> ... ->B(64-2)->A(64-1)
  } else {
    return -1;
  }
}

bool Muon::TgcRdoToPrepDataToolMT::getRfromEtaZ(const double eta, const double z, double& r) 
{
  r = exp(-eta); // tan(theta/2)
  r = atan(r); // theta/2
  r = tan(2.*r); // tan(theta)
  r *= std::abs(z); // r=|z|*tan(theta)

  return r >= 0.;
}

bool Muon::TgcRdoToPrepDataToolMT::getEtafromRZ(const double r, const double z, double& eta) 
{
  double r_tmp = std::abs(r);
  double z_tmp = std::abs(z);

  if((r_tmp<std::numeric_limits<double>::epsilon()) && (z_tmp<std::numeric_limits<double>::epsilon())) return false; 

  eta = std::abs(atan2(r_tmp,z_tmp)); // theta
  eta = tan(eta/2.); // tan(theta/2)
  eta = -log(eta); // rapidity=-log(tan(theta/2))
  return true;
}


bool Muon::TgcRdoToPrepDataToolMT::isOfflineIdOKForTgcReadoutElement(const MuonGM::TgcReadoutElement* descriptor, 
                                                                     const Identifier channelId) const {
  if(!descriptor || !descriptor->containsId(channelId)) {
    ATH_MSG_DEBUG("Illegal OfflineID for TgcReadoutElement" << m_idHelperSvc->toString(channelId));
    return false;
  }
  return true;
}

bool Muon::TgcRdoToPrepDataToolMT::getTrackletInfo(const TgcRawData& rd, int& tmp_slbId, 
                                                   int& tmp_subMatrix, int& tmp_position) const {
  tmp_subMatrix = rd.subMatrix();
  if(tmp_subMatrix!=0 && tmp_subMatrix!=1) {
    ATH_MSG_DEBUG("getTrackletInfo: subMatrix " << tmp_subMatrix << " is invalid.");
    return false;
  }
  
  int tmp_sswId = rd.sswId();
  tmp_slbId = rd.slbId();
  tmp_position = rd.position();
  
  int tmp_position_delta = tmp_position + rd.delta();
  int tmp_slbType = rd.slbType();

  if(tmp_position_delta>=BIT_POS_INPUT_SIZE) {
    tmp_position = tmp_position_delta-BIT_POS_INPUT_SIZE;
    if(tmp_subMatrix==1) {
      if(tmp_slbType==TgcRawData::SLB_TYPE_DOUBLET_STRIP) {
    tmp_position = BIT_POS_INPUT_SIZE-1;
    ATH_MSG_DEBUG("Expected TGC2 Strip position (" << tmp_position_delta <<  
              ") does not exist and is changed to the edge position " << tmp_position);
      } else if((tmp_slbType==TgcRawData::SLB_TYPE_DOUBLET_WIRE) && (tmp_sswId==7) && ((tmp_slbId==3) || (tmp_slbId==11))) { 
    // upper edge SLB of FWD:sbLoc3,11
    ATH_MSG_DEBUG("sbLoc " << tmp_slbId+1 << " doesn't exist for FWD!! (upper edge SLB of FWD:sbLoc3,11)");
        return false;
      } else if((tmp_slbType==TgcRawData::SLB_TYPE_DOUBLET_WIRE) && (tmp_sswId!=7) && (tmp_slbId==9)) { 
    // upper edge SLB of EWD:sbLoc9
    ATH_MSG_DEBUG("sbLoc " << tmp_slbId+1 << " doesn't exist for EWD!! (upper edge SLB of EWD:sbLoc9)");
        return false;
      } else { 
    // Valid sbLoc,delta,blockpos
        tmp_subMatrix = 0;
        tmp_slbId++;
      }
    } else {
      tmp_subMatrix = 1;
    }
  } else if(tmp_position_delta<0) {
    tmp_position = tmp_position_delta+BIT_POS_INPUT_SIZE;
    if(tmp_subMatrix==0) { 
      if(tmp_slbType==TgcRawData::SLB_TYPE_DOUBLET_STRIP) {
    tmp_position = 0;
    ATH_MSG_DEBUG("Expected TGC2 Strip position (" << tmp_position_delta <<  
              ") does not exist and is changed to the edge position " << tmp_position);
      } else if((tmp_slbType==TgcRawData::SLB_TYPE_DOUBLET_WIRE) && (tmp_sswId==7) && ((tmp_slbId==0) || (tmp_slbId==8))) { 
    // bottom edge SLB of FWD:sbLoc0,8
    if(tmp_position_delta==-1) { // This case is valid. 
      tmp_position = 0; 
    } else { 
      ATH_MSG_DEBUG("sbLoc " << tmp_slbId-1 << " doesn't exist for FWD!! (bottom edge SLB of FWD:sbLoc0,8)");
      return false;
    }
      } else if((tmp_slbType==TgcRawData::SLB_TYPE_DOUBLET_WIRE) &&(tmp_sswId!=7) && (tmp_slbId==0)) { 
    // bottom edge SLB of EWD:sbLoc0
    if(tmp_position_delta==-1) { // This case is valid. 
      tmp_position = 0; 
    } else { 
      ATH_MSG_DEBUG("sbLoc " << tmp_slbId-1 << " doesn't exist for EWD!! (bottom edge SLB of EWD:sbLoc0)");
      return false;
    }
      } else { 
    // Valid sbLoc,delta,
        tmp_subMatrix = 1;
        tmp_slbId--;
      }
    } else {
      tmp_subMatrix = 0;
    }
  } else {
    tmp_position = tmp_position_delta;
  }

  return true;
}

int Muon::TgcRdoToPrepDataToolMT::getRoiRow(const TgcRawData& rd) 
{
  int RoiRow = static_cast<int>(rd.roi()/4);
  return RoiRow;
}

bool Muon::TgcRdoToPrepDataToolMT::isIncludedInChamberBoundary(const TgcRawData& rd) const
{
  int RoiRow = getRoiRow(rd);

  if(!rd.isForward() && 
     (RoiRow== 3 || RoiRow== 4 || // SSC 2 : ROI 12-19
      RoiRow== 7 || RoiRow== 8 || // SSC 4 : ROI 28-35
      RoiRow==11 || RoiRow==12 || // SSC 6 : ROI 44-51
      RoiRow==23 || RoiRow==24    // SSC12 : ROI 92-99
      )) {
    return true;
  }
  return false;
}

void Muon::TgcRdoToPrepDataToolMT::getBitPosOutWire(const TgcRawData& rd, int& slbsubMatrix, 
                                                    std::array<int, 2>& bitpos_o) {
  // This method is used by decodeHiPt 
  if((rd.hitId()%2)==1) { // 1,3,5
    slbsubMatrix = 0;
    if((rd.hsub())==0) {
      bitpos_o[0] = BIT_POS_B_INPUT_LARGE_R_CH15; //  78
      bitpos_o[1] = BIT_POS_A_INPUT_LARGE_R_CH08; //  49
    } else if((rd.hsub())==1) {
      bitpos_o[0] = BIT_POS_B_INPUT_LARGE_R_CH07; //  86
      bitpos_o[1] = BIT_POS_A_INPUT_LARGE_R_CH00; //  57
    }
  } else { // 2,4,6
    slbsubMatrix = 1;
    if((rd.hsub())==0) {
      bitpos_o[0] = BIT_POS_B_INPUT_SMALL_R_CH15; //  94
      bitpos_o[1] = BIT_POS_A_INPUT_SMALL_R_CH08; //  65
    } else if((rd.hsub())==1) {
      bitpos_o[0] = BIT_POS_B_INPUT_SMALL_R_CH07; // 102
      bitpos_o[1] = BIT_POS_A_INPUT_SMALL_R_CH00; //  73
    }
  }

  // In case of WIRE, there are no bit assign.(EWD0 EWD4 FWD0 FWD1)-----------------------------
  // EWD0 sbLoc = 0     bitpos = 102 : does not exist and has 6 channels only (the largest  R, RoiRow== 0).
  // EWD4 sbLoc = 9     bitpos =  73 : does not exist and has 4 channels only (the smallest R, RoiRow==36). 
  // FWD0 sbLoc = 0, 8  bitpos =  78 : does not exist and has 5 channels only (the largest  R, RoiRow== 0). 
  // FWD1 sbLoc = 3, 11 bitpos =  73 : does not exist and has 5 channels only (the smallest R, RoiTow==15).
  // fixed it by following description.
  if(!rd.isForward()) { // Endcap
    if((rd.chip()==0) && (rd.hitId()==1) && (rd.hsub()==1)) { 
      // chip 0 should have these values and means EWD0's position.
      slbsubMatrix = 1;
      bitpos_o[0] = BIT_POS_B_INPUT_SMALL_R_CH05; // 104
      bitpos_o[1] = BIT_POS_A_INPUT_SMALL_R_CH00; //  73
    } else if((rd.chip()==3) && (rd.hitId()==6) && (rd.hsub()==1)) { // EWD4's position
      bitpos_o[0] = BIT_POS_B_INPUT_SMALL_R_CH07; // 102
      bitpos_o[1] = BIT_POS_A_INPUT_SMALL_R_CH04; //  69 
    }
  } else { // Forward
    if((rd.chip()==0) && (rd.hitId()==1) && (rd.hsub()==0)) { // FWD0
      bitpos_o[0] = BIT_POS_B_INPUT_LARGE_R_CH12; //  81
      bitpos_o[1] = BIT_POS_A_INPUT_LARGE_R_CH08; //  49
    } else if((rd.chip()==1) && (rd.hitId()==2) && (rd.hsub()==1)) { //FWD1
      bitpos_o[0] = BIT_POS_B_INPUT_SMALL_R_CH07; // 102
      bitpos_o[1] = BIT_POS_A_INPUT_SMALL_R_CH03; //  70
    }
  }// end of Forward
  // end of fixed----------------------------------------------------------------------------------
}

void Muon::TgcRdoToPrepDataToolMT::getBitPosInWire(const TgcRawData& rd, 
                                                  const int deltaBeforeConvert,
                                                  std::array<int,4>& bitpos_i, 
                                                  std::array<int,4>& slbchannel_i, 
                                                  std::array<int,4>& slbId_in, 
                                                  std::array<int,4>& sbLoc_in, 
                                                  int& sswId_i,
                                                  const std::array<int, 2>& bitpos_o, 
                                                  std::array<int, 2>& slbchannel_o, const int slbId_o) const {
  // This method is used by decodeHiPt 
  const int NUM_SLBID_SBLOC_OFFSET_WT = 8; // (see https://twiki.cern.ch/twiki/pub/Main/TgcDocument/sbloc-070701.xls)

  /*** get bitpos for TGC1 Station ***/

  int rdochIn_max = 0;
  int rdochIn_min = 0;
  int offset_dt = 0;
  if(!rd.isForward()) { // EWT
    rdochIn_max = 665;
    rdochIn_min = 78;
    offset_dt = 32;
  } else { // FWT
    rdochIn_max = 312;
    rdochIn_min = 0;
    offset_dt = 0;
  }

  int tmp_rdochannel_i = 0;
  int tmp_rdochannel_i2 = 0;
  for(int i=0; i<2; i++) {
    slbchannel_o[i] = getchannel(bitpos_o[i], TgcRawData::SLB_TYPE_DOUBLET_WIRE);
    tmp_rdochannel_i = WD_MAP_SIZE*slbId_o + slbchannel_o[i] + deltaBeforeConvert + offset_dt; 
    if(tmp_rdochannel_i>rdochIn_max) {
      tmp_rdochannel_i = rdochIn_max;
    } else if(tmp_rdochannel_i<rdochIn_min) {
      tmp_rdochannel_i = rdochIn_min;
    }

    //                               Large R <-----------------> Small R
    // tmp_rdochannel_i           :  0  1  2  3  4  5  6  7  8  9 10 ...
    //------------------------------------------------------------------- 
    // tmp_rdochannel_i/3         :  0  0  0  1  1  1  2  2  2  3  3 ...
    // (tmp_rdochannel_i/3)*3     :  0  0  0  3  3  3  6  6  6  9  9 ...
    // tmp_rdochannel_i2 (i==0)   :  2  2  2  5  5  5  8  8  8 11 11 ...
    //-------------------------------------------------------------------
    // tmp_rdochannel_i+1         :  1  2  3  4  5  6  7  8  9 10 11 ...
    // (tmp_rdochannel_i+1)/3     :  0  0  1  1  1  2  2  2  3  3  3 ...
    // ((tmp_rdochannel_i+1)/3)*3 :  0  0  3  3  3  6  6  6  9  9  9 ...
    // tmp_rdochannel_i2 (i==1)   : -1 -1  2  2  2  5  5  5  8  8  8 ... 
    
    if(i==0) { // get the lower R channel on L3 nearest from bitpos_o[0]
      tmp_rdochannel_i2 = (tmp_rdochannel_i/3)*3 + 2; 
    } else { // get the higher R channel on L3 nearest from bitpos_o[1] 
      tmp_rdochannel_i2 = ((tmp_rdochannel_i + 1)/3)*3 - 1; 
    }
    
    if(tmp_rdochannel_i2>rdochIn_max) {
      tmp_rdochannel_i2 = rdochIn_max;
    } else if(tmp_rdochannel_i2<rdochIn_min) {
      tmp_rdochannel_i2 = rdochIn_min + 2; // 2 is added for L3 
    }

    slbId_in[i]   = tmp_rdochannel_i /WT_MAP_SIZE;
    slbId_in[i+2] = tmp_rdochannel_i2/WT_MAP_SIZE;

    sbLoc_in[i]   = slbId_in[i];
    sbLoc_in[i+2] = slbId_in[i+2];
    if(rd.sector()%2==1) { 
      // phi1 and phi3 have offset (see https://twiki.cern.ch/twiki/pub/Main/TgcDocument/sbloc-070701.xls) 
      // Endcap sector=1, 3 are phi1 and phi3, and Forward sector=1 is phi2.
      sbLoc_in[i]   += NUM_SLBID_SBLOC_OFFSET_WT;
      sbLoc_in[i+2] += NUM_SLBID_SBLOC_OFFSET_WT;
    }

    slbchannel_i[i]   = tmp_rdochannel_i %WT_MAP_SIZE;
    slbchannel_i[i+2] = tmp_rdochannel_i2%WT_MAP_SIZE;

    bitpos_i[i]   = getbitpos(slbchannel_i[i],   TgcRawData::SLB_TYPE_TRIPLET_WIRE);
    bitpos_i[i+2] = getbitpos(slbchannel_i[i+2], TgcRawData::SLB_TYPE_TRIPLET_WIRE);
  }

  if(!rd.isForward()) { // EWT
    sswId_i = static_cast<int>(rd.sector()/2);
  } else { // FWT
    sswId_i = 2;
  }
}

void Muon::TgcRdoToPrepDataToolMT::getBitPosOutStrip(const TgcRawData& rd, 
                                                     int& slbsubMatrix, 
                                                     std::array<int,2 >& bitpos_o) {
  // This method is used by decodeHiPt
  if((rd.hitId()%2)==1) { // 1,3,5::hitId:1-6 for EC, 2-3 for Fw
    slbsubMatrix = 0;
    if((rd.hsub())==0) {
      bitpos_o[0] = BIT_POS_B_INPUT_LARGE_PHI_FOR_A_FWD_C_BWD_CH15; //  78 
      bitpos_o[1] = BIT_POS_A_INPUT_LARGE_PHI_FOR_A_FWD_C_BWD_CH08; //  49
    } else if((rd.hsub())==1) {
      bitpos_o[0] = BIT_POS_B_INPUT_LARGE_PHI_FOR_A_FWD_C_BWD_CH07; //  86
      bitpos_o[1] = BIT_POS_A_INPUT_LARGE_PHI_FOR_A_FWD_C_BWD_CH00; //  57
    }
  } else { // 2,4,6
    slbsubMatrix = 1;
    if((rd.hsub())==0) {
      bitpos_o[0] = BIT_POS_B_INPUT_SMALL_PHI_FOR_A_FWD_C_BWD_CH15; //  94
      bitpos_o[1] = BIT_POS_A_INPUT_SMALL_PHI_FOR_A_FWD_C_BWD_CH08; //  65
    } else if((rd.hsub())==1) {
      bitpos_o[0] = BIT_POS_B_INPUT_SMALL_PHI_FOR_A_FWD_C_BWD_CH07; // 102
      bitpos_o[1] = BIT_POS_A_INPUT_SMALL_PHI_FOR_A_FWD_C_BWD_CH00; //  73
    }
  }
}

void Muon::TgcRdoToPrepDataToolMT::getBitPosInStrip(const TgcRawData& rd, 
                                                    const int deltaBeforeConvert,
                                                    std::array<int, 4>& bitpos_i, 
                                                    std::array<int, 4>& slbchannel_i, 
                                                    int& sbLoc_i, 
                                                    int& sswId_i,
                                                    const std::array<int,2>& bitpos_o, 
                                                    std::array<int,2>& slbchannel_o) const {
  // This method is used by decodeHiPt 
  //*** get bitpos for TGC1 Station ***//
  // ST
  int rdochIn_max = ST_MAP_SIZE-1;
  int rdochIn_min = 0;

  if((rd.sector()%2)==0) { 
    if(rd.chip()==0) {
      sbLoc_i = 16; // EST0 (phi0 and phi2) or FST0 (phi0)
    } else if(rd.chip()==1) {
      sbLoc_i = 17; // EST1 (phi0 and phi2)
    }
  } else {
    if(rd.chip()==0) {
      sbLoc_i = 24; // EST0 (phi1 and phi3) or FST0 (phi2)
    } else if(rd.chip()==1) {
      sbLoc_i = 25; // EST1 (phi1 and phi3)
    }
  }

  for(int i=0; i<2; i++) {
    slbchannel_o[i] = getchannel(bitpos_o[i], TgcRawData::SLB_TYPE_DOUBLET_STRIP);
    slbchannel_i[i] = slbchannel_o[i] + deltaBeforeConvert;
    if(slbchannel_i[i]>rdochIn_max) {
      slbchannel_i[i] = rdochIn_max;
    } else if(slbchannel_i[i]<rdochIn_min) {
      slbchannel_i[i] = rdochIn_min;
    }

    if(i==0) {
      slbchannel_i[i+2] = slbchannel_i[i] + 1;
    } else {
      slbchannel_i[i+2] = slbchannel_i[i] - 1;
    }

    if(slbchannel_i[i+2] > rdochIn_max) {
      slbchannel_i[i+2] = rdochIn_max;
    } else if(slbchannel_i[i+2] < rdochIn_min) {
      slbchannel_i[i+2] = rdochIn_min;
    }

    bitpos_i[i]   = getbitpos(slbchannel_i[i],   TgcRawData::SLB_TYPE_TRIPLET_STRIP);
    bitpos_i[i+2] = getbitpos(slbchannel_i[i+2], TgcRawData::SLB_TYPE_TRIPLET_STRIP);
  }

  if(!rd.isForward()) { // EST
    sswId_i = static_cast<int>(rd.sector()/2);
  } else { // FST
    sswId_i = 2;
  }
}

void Muon::TgcRdoToPrepDataToolMT::getBitPosWire(const TgcRawData& rd, 
                                                 const int hitId_w, 
                                                 const int sub_w,
                                                 int& subMatrix_w, 
                                                 std::array<int, 3>& bitpos_w) const
{
  // This method is used by getSLIds 
  // This method assumes sub_w is 0 or 1.
  // Protection for other possibility is needed.

  // 0 : Index for the largest R channel 
  // 1 : Index for the smallest R channel 
  // 2 : Index for the "center" channel

  int RoiRow = getRoiRow(rd);
  bool isForward = rd.isForward();

  if(RoiRow==0 && !isForward) { // EWD0,SLB0 exception (It has 6 channels only, the largest R)
    subMatrix_w = 1;
    bitpos_w[0] = BIT_POS_B_INPUT_SMALL_R_CH05; // 104 
    bitpos_w[1] = BIT_POS_A_INPUT_SMALL_R_CH04; //  69
    bitpos_w[2] = BIT_POS_A_INPUT_SMALL_R_CH00; //  73
  } else if(RoiRow==36 && !isForward) { // EWD4,SLB1 exception (It has 4 channels only, the smallest R)
    subMatrix_w = 1;
    bitpos_w[0] = BIT_POS_B_INPUT_SMALL_R_CH07; // 102
    bitpos_w[1] = BIT_POS_A_INPUT_SMALL_R_CH04; //  69
    bitpos_w[2] = BIT_POS_A_INPUT_SMALL_R_CH04; //  69
  } else if(RoiRow==0 && isForward) { // FWD0,SLB0 exception (It has 5 channels only, the largest R)
    subMatrix_w = 0;
    bitpos_w[0] = BIT_POS_B_INPUT_LARGE_R_CH12; //  81
    bitpos_w[1] = BIT_POS_A_INPUT_LARGE_R_CH12; //  45
    bitpos_w[2] = BIT_POS_A_INPUT_LARGE_R_CH08; //  49
  } else if(RoiRow==15 && isForward) { // FWD1,SLB1 exception (It has 5 channels only, the smallest R)
    subMatrix_w = 1;
    bitpos_w[0] = BIT_POS_B_INPUT_SMALL_R_CH07; // 102
    bitpos_w[1] = BIT_POS_A_INPUT_SMALL_R_CH04; //  69
    bitpos_w[2] = BIT_POS_A_INPUT_SMALL_R_CH03; //  70
  } else {
    if((hitId_w%2)==0) { // 0, 2, 4
      subMatrix_w = 0;
      if(sub_w==0) {
        bitpos_w[0] = BIT_POS_B_INPUT_LARGE_R_CH15; //  78 
    bitpos_w[1] = BIT_POS_A_INPUT_LARGE_R_CH12; //  45 
    bitpos_w[2] = BIT_POS_A_INPUT_LARGE_R_CH08; //  49
      } else if(sub_w==1) {
        bitpos_w[0] = BIT_POS_B_INPUT_LARGE_R_CH07; //  86
    bitpos_w[1] = BIT_POS_A_INPUT_LARGE_R_CH04; //  53
    bitpos_w[2] = BIT_POS_A_INPUT_LARGE_R_CH00; //  57
      }
    } else { // 1, 3, 5
      subMatrix_w = 1;
      if(sub_w==0) {
        bitpos_w[0] = BIT_POS_B_INPUT_SMALL_R_CH15; //  94 
    bitpos_w[1] = BIT_POS_A_INPUT_SMALL_R_CH12; //  61 
    bitpos_w[2] = BIT_POS_A_INPUT_SMALL_R_CH08; //  65
      } else if(sub_w==1) {
        bitpos_w[0] = BIT_POS_B_INPUT_SMALL_R_CH07; // 102 
    bitpos_w[1] = BIT_POS_A_INPUT_SMALL_R_CH04; //  69
    bitpos_w[2] = BIT_POS_A_INPUT_SMALL_R_CH00; //  73
      }
    }
  }
}

void Muon::TgcRdoToPrepDataToolMT::getBitPosStrip(const int hitId_s, 
                                                  const int sub_s, 
                                                  int& subMatrix_s, 
                                                  std::array<int, 3>& bitpos_s) 
{
  // This method is used by getSLIds 
  // 0 : Index for the largest phi (for A-side forward and C-side backward) channel 
  // 1 : Index for the smallest phi (for A-side forward and C-side backward) channel 
  // 2 : Index for the "center" channel

  if((hitId_s%2)==0) { // 0, 2, 4
    subMatrix_s = 0;
    if(sub_s==0) {
      bitpos_s[0] = BIT_POS_B_INPUT_LARGE_PHI_FOR_A_FWD_C_BWD_CH15; //  78 
      bitpos_s[1] = BIT_POS_A_INPUT_LARGE_PHI_FOR_A_FWD_C_BWD_CH12; //  45
      bitpos_s[2] = BIT_POS_A_INPUT_LARGE_PHI_FOR_A_FWD_C_BWD_CH08; //  49
    } else if(sub_s==1) {
      bitpos_s[0] = BIT_POS_B_INPUT_LARGE_PHI_FOR_A_FWD_C_BWD_CH07; //  86
      bitpos_s[1] = BIT_POS_A_INPUT_LARGE_PHI_FOR_A_FWD_C_BWD_CH04; //  53
      bitpos_s[2] = BIT_POS_A_INPUT_LARGE_PHI_FOR_A_FWD_C_BWD_CH00; //  57
    }
  } else if((hitId_s%2)==1) { // 1, 3, 5
    subMatrix_s = 1;
    if(sub_s==0) {
      bitpos_s[0] = BIT_POS_B_INPUT_SMALL_PHI_FOR_A_FWD_C_BWD_CH15; //  94
      bitpos_s[1] = BIT_POS_A_INPUT_SMALL_PHI_FOR_A_FWD_C_BWD_CH12; //  61
      bitpos_s[2] = BIT_POS_A_INPUT_SMALL_PHI_FOR_A_FWD_C_BWD_CH08; //  65
    } else if(sub_s==1) {
      bitpos_s[0] = BIT_POS_B_INPUT_SMALL_PHI_FOR_A_FWD_C_BWD_CH07; // 102 
      bitpos_s[1] = BIT_POS_A_INPUT_SMALL_PHI_FOR_A_FWD_C_BWD_CH04; //  69 
      bitpos_s[2] = BIT_POS_A_INPUT_SMALL_PHI_FOR_A_FWD_C_BWD_CH00; //  73
    }
  }
}

int Muon::TgcRdoToPrepDataToolMT::getDeltaBeforeConvert(const TgcRawData& rd) 
{
  int deltaBeforeConvert = 0;

  if(rd.isStrip()) {// strip
    switch(rd.delta()) {
    case  5: deltaBeforeConvert =   6; break;
    case  6: deltaBeforeConvert =   8; break;
    case  7: deltaBeforeConvert =  10; break;
    case -4: deltaBeforeConvert =  -5; break;
    case -5: deltaBeforeConvert =  -7; break;
    case -6: deltaBeforeConvert =  -9; break;
    case -7: deltaBeforeConvert = -12; break;
    default: deltaBeforeConvert = rd.delta(); break;
    }
  } else {// wire
    switch (rd.delta()) {
    case  11: deltaBeforeConvert =  12; break;
    case  12: deltaBeforeConvert =  14; break;
    case  13: deltaBeforeConvert =  16; break;
    case  14: deltaBeforeConvert =  18; break;
    case  15: deltaBeforeConvert =  20; break;
    case -12: deltaBeforeConvert = -13; break;
    case -13: deltaBeforeConvert = -15; break;
    case -14: deltaBeforeConvert = -17; break;
    case -15: deltaBeforeConvert = -19; break;
    default:  deltaBeforeConvert = rd.delta(); break;
    }
  }

  return deltaBeforeConvert;
}

bool Muon::TgcRdoToPrepDataToolMT::isBackwardBW(const TgcRawData& rd) 
{
  bool isBackward = false;

  if(!rd.isForward()) { // Endcap
    if(((rd.subDetectorId()==ASIDE) && (rd.sector()%2==1)) ||
       ((rd.subDetectorId()==CSIDE) && (rd.sector()%2==0))) {
      // Aside,phi_odd::Backward
      // Cside,phi_even::Backward
      isBackward = true;
    } else {
      // Aside,phi_even::Forward
      // Cside,phi_odd::Forward
      isBackward = false;
    }
  } else { // Forward
    // Aide::Backward
    // Cside::Forward
    isBackward = (rd.subDetectorId()==ASIDE);
  }

  return isBackward;
}

bool Muon::TgcRdoToPrepDataToolMT::getSLWireGeometry(const std::array<Identifier, 3>& channelId_wire, 
                                                     double& width_wire, double& r_wire, double& z_wire) const
{
  SG::ReadCondHandle<MuonGM::MuonDetectorManager> muDetMgrHandle{m_muDetMgrKey};
  const MuonGM::MuonDetectorManager* muDetMgr = muDetMgrHandle.cptr();
  std::array<const MuonGM::TgcReadoutElement*, 3> descriptor_w{muDetMgr->getTgcReadoutElement(channelId_wire[0]), 
                                                               muDetMgr->getTgcReadoutElement(channelId_wire[1]), 
                                                               muDetMgr->getTgcReadoutElement(channelId_wire[2])};
  for(int i=0; i<3; i++) {
    if(!isOfflineIdOKForTgcReadoutElement(descriptor_w[i], channelId_wire[i])) {
      return false;
    }
  }

  Amg::Vector3D position_w = descriptor_w[2]->channelPos(channelId_wire[2]);
  Amg::Vector2D loc_hitPos_w{Amg::Vector2D::Zero()};
  bool onSurface_w = descriptor_w[2]->surface(channelId_wire[2]).globalToLocal(position_w,position_w,loc_hitPos_w);
  if(!onSurface_w) { 
    ATH_MSG_WARNING("Muon::TgcRdoToPrepDataToolMT::getSLWireGeometry Amg::Vector2D* loc_hitPos_w is null."); 
    return false; 
  } 
  Amg::Vector2D tmp_hitPos_w{Amg::Vector2D::Zero()};

  std::array<int, 3> gasGap_w{}, channel_w{};
  for(int i=0; i<3; i++) {
    gasGap_w[i] = m_idHelperSvc->tgcIdHelper().gasGap(channelId_wire[i]);
    channel_w[i] = m_idHelperSvc->tgcIdHelper().channel(channelId_wire[i]);
  }
  
  std::array<double,3> tmp_r_w{}, tmp_phi_w{}, tmp_eta_w{};
  
  std::array<Amg::Vector3D,3> tmp_position_w{make_array<Amg::Vector3D, 3>(Amg::Vector3D::Zero())};
  for(int i=0; i<3; i+=2) { // i=0 and 2
    tmp_position_w[i] = descriptor_w[i]->channelPos(channelId_wire[i]);
    tmp_r_w[i] = tmp_position_w[i].perp();
    tmp_phi_w[i] = tmp_position_w[i].phi();
    tmp_eta_w[i] = tmp_position_w[i].eta();
  
    double half_width = descriptor_w[i]->gangRadialLength(gasGap_w[i], channel_w[i])/2.;
    if(half_width<s_cutDropPrdsWithZeroWidth/2. && m_dropPrdsWithZeroWidth) { // Invalid PRD's whose widths are zero are dropped.
      return false;
    }
    // add half widths of edge channels
    if(i==0) {
      tmp_r_w[0] += half_width;
    } else if(i==2) {
      tmp_r_w[2] -= half_width;
    }
  }
  
  bool flag_geteta_w = getEtafromRZ(tmp_r_w[0], tmp_position_w[0].z(), tmp_eta_w[0]);
  bool flag_getr_w = getRfromEtaZ(tmp_eta_w[0], tmp_position_w[2].z(), tmp_r_w[0]);
  if(!flag_geteta_w || !flag_getr_w) {
    ATH_MSG_DEBUG("TgcRdoToPrepDataToolMT::getSLWireGeometry::failed to getR on L7!!");
    return false;
  }  
  width_wire = tmp_r_w[0] - tmp_r_w[2];
  double gang = descriptor_w[2]->gangShortWidth(gasGap_w[2],channel_w[2]) + width_wire/2.;
  tmp_hitPos_w[Trk::locX] = gang;
  tmp_hitPos_w[Trk::locY] = ((loc_hitPos_w)[Trk::loc2]);

  Amg::Vector3D tmp_wire_gp;
  descriptor_w[2]->surface(channelId_wire[2]).localToGlobal(tmp_hitPos_w,tmp_wire_gp,tmp_wire_gp);
  z_wire = tmp_wire_gp.z();
  r_wire = tmp_r_w[2] + width_wire/2.;

  return true;
}

bool Muon::TgcRdoToPrepDataToolMT::getSLStripGeometry(const std::array<Identifier, 3>& channelId_strip, 
                                                      const bool isBackward, const bool isAside, 
                                                      double& width_strip, double& theta_strip) const
{
  SG::ReadCondHandle<MuonGM::MuonDetectorManager> muDetMgrHandle{m_muDetMgrKey};
  const MuonGM::MuonDetectorManager* muDetMgr = muDetMgrHandle.cptr();
  std::array<const MuonGM::TgcReadoutElement*, 3> descriptor_s{muDetMgr->getTgcReadoutElement(channelId_strip[0]), 
                                                               muDetMgr->getTgcReadoutElement(channelId_strip[1]), 
                                                               muDetMgr->getTgcReadoutElement(channelId_strip[2])};
  for(int i=0; i<3; i++) {
    if(!isOfflineIdOKForTgcReadoutElement(descriptor_s[i], channelId_strip[i])) {
      return false;
    }
  }
  
  Amg::Vector3D position_s = Amg::Vector3D(descriptor_s[1]->channelPos(channelId_strip[1]));
  Amg::Vector2D loc_hitPos_s;
  bool onSurface_s = descriptor_s[1]->surface(channelId_strip[1]).globalToLocal(position_s,position_s,loc_hitPos_s);
  if(!onSurface_s) { 
    ATH_MSG_WARNING("Muon::TgcRdoToPrepDataToolMT::getSLStripGeometry Amg::Vector2D* loc_hitPos_s is null."); 
    return false; 
  } 
  Amg::Vector2D tmp_hitPos_s{Amg::Vector2D::Zero()};
  
  std::array<int, 3> gasGap_s, channel_s{};
  for(int i=0; i<3; i++) {
    gasGap_s[i] = m_idHelperSvc->tgcIdHelper().gasGap(channelId_strip[i]);
    channel_s[i] = m_idHelperSvc->tgcIdHelper().channel(channelId_strip[i]);
  }

  std::array<Amg::Vector3D, 3> localPos{Amg::Vector3D::Zero()};
  for(int i=0; i<3; i+=2) { // i=0 and 2
    localPos[i] = descriptor_s[i]->transform(channelId_strip[i]).inverse()
      *descriptor_s[i]->channelPos(channelId_strip[i]);
  }
  
  bool flag_reverse = false;
  std::array<int, 2> index_strip{};

  if(!isBackward) { // Forward chamber
    if(isAside) { // Aside/Forward Chamber
      index_strip[0] = 2; index_strip[1] = 0; flag_reverse = true;
    } else { // Cside/Forward Chamber
      index_strip[0] = 0; index_strip[1] = 2; flag_reverse = false;
    }
  } else { // Backward chamber   
    if(isAside) { // Aside/Backward Chamber
      index_strip[0] = 0; index_strip[1] = 2; flag_reverse = true;
    } else { // Cside/Backward Chamber
      index_strip[0] = 2; index_strip[1] = 0; flag_reverse = false;
    }
  }
  
  double stripMaxX = descriptor_s[index_strip[0]]->stripHighEdgeLocX(gasGap_s[index_strip[0]], channel_s[index_strip[0]], 
                                 localPos[index_strip[0]].z());
  double stripMinX = descriptor_s[index_strip[1]]->stripLowEdgeLocX(gasGap_s[index_strip[1]], channel_s[index_strip[1]], 
                                 localPos[index_strip[1]].z());
  width_strip = stripMaxX - stripMinX;
  double strip = stripMinX +  width_strip/2.;
  if(flag_reverse) strip *= -1.;
  
  tmp_hitPos_s[Trk::locX] = strip;
  tmp_hitPos_s[Trk::locY] = loc_hitPos_s[Trk::loc2];
  
  int index_strip_gp = 0;
  if(!isBackward) index_strip_gp = 0;
  else index_strip_gp = 2;
  Amg::Vector3D tmp_strip_gp; 
  descriptor_s[index_strip_gp]->surface(channelId_strip[index_strip_gp]).localToGlobal(tmp_hitPos_s,tmp_strip_gp,tmp_strip_gp);
  theta_strip = atan2(tmp_strip_gp.y(), tmp_strip_gp.x()); 

  
  return true;
}

bool Muon::TgcRdoToPrepDataToolMT::getPosAndIdWireOut(const std::array<const MuonGM::TgcReadoutElement*, 2>& descriptor_o, 
                                                      const std::array<Identifier, 2>& channelIdOut,
                                                      const std::array<int, 2>& gasGap_o, 
                                                      const std::array<int, 2>& channel_o,
                                                      double& width_o, 
                                                      double& hit_position_o, 
                                                      Amg::Vector2D& tmp_hitPos_o,
                                                      Identifier& channelIdOut_tmp) const
{
  // This method is used by decodeHiPt
  std::array<Amg::Vector3D, 2> position_o{make_array<Amg::Vector3D, 2>(Amg::Vector3D::Zero())};
  std::array<double, 2> tmp_phi_o{}, tmp_eta_o{}, tmp_r_o {};
  
  for(int i=0; i<2; i++) {
    position_o[i] = descriptor_o[i]->channelPos(channelIdOut[i]);
    tmp_r_o[i] = position_o[i].perp();
    tmp_phi_o[i] = position_o[i].phi();
    tmp_eta_o[i] = position_o[i].phi();
    // add half widths of edge channels
    double half_width = descriptor_o[i]->gangRadialLength(gasGap_o[i], channel_o[i])/2.;
    if(half_width<s_cutDropPrdsWithZeroWidth/2. && m_dropPrdsWithZeroWidth) { // Invalid PRD's whose widths are zero are dropped.
      return false;
    }
    if(i==0) tmp_r_o[0] += half_width;
    else     tmp_r_o[1] -= half_width;
  }
  
  bool flag_geteta_o = getEtafromRZ(tmp_r_o[0], position_o[0].z(), tmp_eta_o[0]);
  bool flag_getr_o = getRfromEtaZ(tmp_eta_o[0], position_o[1].z(), tmp_r_o[0]);
  if(!flag_geteta_o || !flag_getr_o) {
    ATH_MSG_DEBUG("TgcRdoToPrepDataToolMT::getPosAndIdWireOut::failed to getR on L7!!");
    return false;
  }
  
  width_o = tmp_r_o[0] - tmp_r_o[1];
  // X-coordinate
  hit_position_o = descriptor_o[1]->gangShortWidth(gasGap_o[1], channel_o[1]) + width_o/2;
  tmp_hitPos_o[Trk::locX] = (hit_position_o); 
  // Y-coordinate
  Amg::Vector3D position_out = Amg::Vector3D(descriptor_o[1]->channelPos(channelIdOut[1]));
  // dummy global pos
  Amg::Vector2D loc_hitPos_o;
  bool onSurface_o = descriptor_o[1]->surface(channelIdOut[1]).globalToLocal(position_out,position_out,loc_hitPos_o); 
  if(!onSurface_o) { 
    ATH_MSG_WARNING("Muon::TgcRdoToPrepDataToolMT::getPosAndIdWireOut Amg::Vector2D* loc_hitPos_o is null."); 
    return false; 
  } 
  tmp_hitPos_o[Trk::locY] = loc_hitPos_o[Trk::loc2];
  
  channelIdOut_tmp = channelIdOut[1];
  
  return true;
}

bool Muon::TgcRdoToPrepDataToolMT::getPosAndIdStripOut(const std::array<const MuonGM::TgcReadoutElement*, 2>& descriptor_o, 
                                                       const std::array<Identifier, 2>& channelIdOut,
                                                       const std::array<int, 2>& gasGap_o, 
                                                       const std::array<int, 2>& channel_o,
                                                       double& width_o, 
                                                       double& hit_position_o, 
                                                       Amg::Vector2D& tmp_hitPos_o,
                                                       Identifier& channelIdOut_tmp,
                                                       const bool isBackward, const bool isAside) const
{
  // This method is used by decodeHiPt
  // Currently, this method always returns true.
  std::array<Amg::Vector3D, 2> localpos_o{make_array<Amg::Vector3D, 2>(Amg::Vector3D::Zero())};
  for(int i=0; i<2; i++) {
    localpos_o[i] = descriptor_o[i]->transform(channelIdOut[i]).inverse()*descriptor_o[i]->channelPos(channelIdOut[i]);
  }
  
  std::array<int, 3> index{};
  bool flag_reverse = false;
  if(!isBackward) { // Forward chamber
    index[2] = 0;
    if(isAside) { // Aside/Forward Chamber
      index[0] = 1; index[1] = 0; flag_reverse = true;
    } else { // Cside/Forward Chamber
      index[0] = 0; index[1] = 1; flag_reverse = false;
    }
  } else { // Backward chamber   
    index[2] = 1;
    if(isAside) { // Aside/Backward Chamber
      index[0] = 0; index[1] = 1; flag_reverse = true;
    } else { // Cside/Backward Chamber
      index[0] = 1; index[1] = 0; flag_reverse = false;
    }
  }
  
  double stripMax = descriptor_o[index[0]]->stripHighEdgeLocX(gasGap_o[index[0]], channel_o[index[0]], 
                              localpos_o[index[0]].z());
  double stripMin = descriptor_o[index[1]]->stripLowEdgeLocX(gasGap_o[index[1]], channel_o[index[1]], 
                              localpos_o[index[1]].z());
  width_o = stripMax - stripMin;
  // X-coordinate
  hit_position_o = stripMin + width_o/2.;
  if(flag_reverse) hit_position_o *= -1.;
  tmp_hitPos_o[Trk::locX] = hit_position_o;
  // Y-coordinate
  Amg::Vector3D position_out = Amg::Vector3D(descriptor_o[1]->channelPos(channelIdOut[1]));
  // dummy global pos
  Amg::Vector2D loc_hitPos_o{Amg::Vector2D::Zero()};
  bool onSurface_o = descriptor_o[1]->surface(channelIdOut[1]).globalToLocal(position_out,position_out,loc_hitPos_o); 
  if(!onSurface_o) { 
    ATH_MSG_WARNING("Muon::TgcRdoToPrepDataToolMT::getPosAndIdStripOut Amg::Vector2D* loc_hitPos_o is null."); 
    return false; 
  } 
  tmp_hitPos_o[Trk::locY] = loc_hitPos_o[Trk::loc2];
  
  channelIdOut_tmp = channelIdOut[index[2]];
  
  return true;
}

bool Muon::TgcRdoToPrepDataToolMT::getPosAndIdWireIn(const std::array<const MuonGM::TgcReadoutElement*, 4>& descriptor_i, 
                                                     const std::array<Identifier, 4>& channelIdIn,
                                                     const std::array<int, 4>& gasGap_i, 
                                                     const std::array<int, 4>& channel_i,
                                                     double& width_i, double& hit_position_i, 
                                                     Amg::Vector2D& tmp_hitPos_i,
                                                     Identifier& channelIdIn_tmp) const
{
  // This method is used by decodeHiPt
  int flag_boundary_i = 0;
  if(descriptor_i[1]->chamberType()==descriptor_i[3]->chamberType()) { 
    // in case lower edge is not chamber boundary
    if(gasGap_i[1]==gasGap_i[3]) { // in case that edge channel is on L3; use [1]
      flag_boundary_i = 1;
    } else { // in case that edge channel is not on L3; use [3]
      flag_boundary_i = 3;
    }
  } else if(descriptor_i[0]->chamberType()==descriptor_i[2]->chamberType()) { 
    // in case higher edge is not chamber boundary
    if(gasGap_i[0]==gasGap_i[2]) { // in case that edge channel is on L3; use [0]
      flag_boundary_i = 0;
    } else { // in case that edge channel is not on L3; use [2]
      flag_boundary_i = 2;
    }
  } else {
    ATH_MSG_DEBUG("TgcRdoToPrepDataToolMT::getPosAndIdWireIn::ROI has 3 readout elements!!");
    return false;
  }
  
  channelIdIn_tmp = channelIdIn[flag_boundary_i];
  SG::ReadCondHandle<MuonGM::MuonDetectorManager> muDetMgrHandle{m_muDetMgrKey};
  const MuonGM::MuonDetectorManager* muDetMgr = muDetMgrHandle.cptr();
  const MuonGM::TgcReadoutElement* descriptor_iw = muDetMgr->getTgcReadoutElement(channelIdIn_tmp);
  if(!isOfflineIdOKForTgcReadoutElement(descriptor_iw, channelIdIn_tmp)) {
    return false;
  }
  
  std::array<double, 3> tmp_r_i{}, tmp_phi_i{}, tmp_eta_i{};
  
  std::array<Amg::Vector3D, 3> position_i {descriptor_i[0]->channelPos(channelIdIn[0]), 
                                           descriptor_i[1]->channelPos(channelIdIn[1]), 
                                           descriptor_iw->channelPos(channelIdIn_tmp)};

  for(int i=0; i<3; i++) {
    tmp_r_i[i] =position_i[i].perp();
    tmp_phi_i[i] = position_i[i].phi();
    tmp_eta_i[i] = position_i[i].eta();
    
    if(i<2) {
      // add half widths of edge channels
      double half_width = descriptor_i[i]->gangRadialLength(gasGap_i[i], channel_i[i])/2.;
      if(half_width<s_cutDropPrdsWithZeroWidth/2. && m_dropPrdsWithZeroWidth) { // Invalid PRD's whose widths are zero are dropped.
    return false;
      }
      if(i==0) tmp_r_i[0] += half_width;
      else     tmp_r_i[1] -= half_width;
      
      bool flag_geteta_i = getEtafromRZ(tmp_r_i[i], position_i[i].z(), tmp_eta_i[i]);
      bool flag_getr_i = getRfromEtaZ(tmp_eta_i[i], position_i[2].z(), tmp_r_i[i]); 
      
      if(!flag_geteta_i || !flag_getr_i) {
    ATH_MSG_DEBUG("TgcRdoToPrepDataToolMT::getPosAndIdWireIn::failed to getRIn" << i << " on L3!!");
    return false;
      }
    }
  }
  
  width_i = tmp_r_i[0] - tmp_r_i[1];
  if(width_i<0.) {
    ATH_MSG_DEBUG("TgcRdoToPrepDataToolMT::getPosAndIdWireIn::minus value width_i = " << width_i);
    return false;
  }
  
  int gasGap_tmp = m_idHelperSvc->tgcIdHelper().gasGap(channelIdIn_tmp);
  int channel_tmp = m_idHelperSvc->tgcIdHelper().channel(channelIdIn_tmp);
  double half_width = descriptor_iw->gangRadialLength(gasGap_tmp, channel_tmp)/2.;
  if(half_width<s_cutDropPrdsWithZeroWidth/2. && m_dropPrdsWithZeroWidth) { // Invalid PRD's whose widths are zero are dropped.
    return false;
  }
  if((flag_boundary_i%2)==1) { // in case lower edge is not chamber boundary
    tmp_r_i[2] -= half_width;
    hit_position_i = descriptor_iw->gangShortWidth(gasGap_tmp, channel_tmp)
      - (tmp_r_i[2] - tmp_r_i[1]) + width_i/2.;
  } else { // in case higher edge is not chamber boundary
    tmp_r_i[2] += half_width;
    hit_position_i = descriptor_iw->gangLongWidth(gasGap_tmp, channel_tmp)
      + (tmp_r_i[0] - tmp_r_i[2]) - width_i/2.;
  }
  
  // X-coordinate
  tmp_hitPos_i[Trk::locX] = hit_position_i;
  Amg::Vector3D position_in = descriptor_i[1]->channelPos(channelIdIn[1]);
  // dummy global pos
  Amg::Vector2D loc_hitPos_i{Amg::Vector2D::Zero()};
  bool onSurface_i = descriptor_i[1]->surface(channelIdIn[1]).globalToLocal(position_in,position_in,loc_hitPos_i); 
  if(!onSurface_i) { 
    ATH_MSG_WARNING("Muon::TgcRdoToPrepDataToolMT::getPosAndIdWireIn Amg::Vector2D* loc_hitPos_i is null."); 
    return false; 
  } 
  // Y-coordinate
  tmp_hitPos_i[Trk::locY] = loc_hitPos_i[Trk::loc2];
  
  return true;
}

bool Muon::TgcRdoToPrepDataToolMT::getPosAndIdStripIn(const std::array<const MuonGM::TgcReadoutElement*, 4>& descriptor_i, 
                                                      const std::array<Identifier, 4>& channelIdIn,
                                                      const std::array<int, 4>& gasGap_i, 
                                                      const std::array<int, 4>& channel_i,
                                                      double& width_i, 
                                                      double& hit_position_i, 
                                                      Amg::Vector2D& tmp_hitPos_i,
                                                      Identifier& channelIdIn_tmp,
                                                      const bool isBackward, const bool isAside) const
{
  // This method is used by decodeHiPt
  int flag_isL3 = -1;
  // which bitpos is Layer3?
  for(int i=0; i<4; i++) {
    if(gasGap_i[i]==3) {
      flag_isL3 = i;
      break;
    }
  }
  if(flag_isL3<0 || flag_isL3>=4) {
    ATH_MSG_DEBUG("getPosAndIdStripIn: Any bitpos is not at Layer3!");
    return false;
  }
  
  channelIdIn_tmp = channelIdIn[flag_isL3];
  SG::ReadCondHandle<MuonGM::MuonDetectorManager> muDetMgrHandle{m_muDetMgrKey};
  const MuonGM::MuonDetectorManager* muDetMgr = muDetMgrHandle.cptr();
  const MuonGM::TgcReadoutElement* descriptor_is = muDetMgr->getTgcReadoutElement(channelIdIn_tmp);
  if(!isOfflineIdOKForTgcReadoutElement(descriptor_is, channelIdIn_tmp)) {
    return true;
  }
  
  std::array<double, 3> tmp_r_i{}, tmp_phi_i{}, tmp_eta_i{};
  std::array<Amg::Vector3D, 3> position_is{make_array<Amg::Vector3D, 3>(Amg::Vector3D::Zero())};
  for(int i=0; i<3; i++) {
    if(i<2) {
      position_is[i] = descriptor_i[i]->channelPos(channelIdIn[i]);
    } else {
      position_is[i] = descriptor_is->channelPos(channelIdIn_tmp);
    }
    tmp_r_i[i] = position_is[i].perp();
    tmp_phi_i[i] = position_is[i].phi(); 
    tmp_eta_i[i] = position_is[i].eta();
  }
  
  std::array<int, 2> index{};
  bool flag_reverse = false;
  if(!isBackward) { // Forward chamber
    if(isAside) { // Aside/Forward Chamber
      index[0] = 1; index[1] = 0; flag_reverse = true;
    } else { // Cside/Forward Chamber
      index[0] = 0; index[1] = 1; flag_reverse = false;
    }
  } else { // Backward chamber
    if(isAside) { // Aside/Backward Chamber
      index[0] = 0; index[1] = 1; flag_reverse = true;
    } else { // Cside/Backward Chamber
      index[0] = 1; index[1] = 0; flag_reverse = false;
    }
  }
  
  std::array<Amg::Vector3D,2 > localpos_i{make_array<Amg::Vector3D, 2>(Amg::Vector3D::Zero())};
  for(int i=0; i<2; i++) {
    localpos_i[i] = descriptor_i[i]->transform(channelIdIn[i]).inverse()*descriptor_i[i]->channelPos(channelIdIn[i]);
  }
  double stripMaxX = descriptor_i[index[0]]->stripHighEdgeLocX(gasGap_i[index[0]], channel_i[index[0]], 
                               localpos_i[index[0]].z());
  double stripMinX = descriptor_i[index[1]]->stripLowEdgeLocX(gasGap_i[index[1]], channel_i[index[1]], 
                               localpos_i[index[1]].z());
  width_i = stripMaxX - stripMinX;
  // X-coordinate
  hit_position_i = stripMinX +  width_i/2.;
  if(flag_reverse) hit_position_i *= -1.;
  tmp_hitPos_i[Trk::locX] = hit_position_i;
  // Y-coordinate
  Amg::Vector3D position_in = Amg::Vector3D(descriptor_i[1]->channelPos(channelIdIn[1]));
  // dummy global pos
  Amg::Vector2D loc_hitPos_i;
  bool onSurface_i = descriptor_i[1]->surface(channelIdIn[1]).globalToLocal(position_in,position_in,loc_hitPos_i); 
  if(!onSurface_i) { 
    ATH_MSG_WARNING("Muon::TgcRdoToPrepDataToolMT::getPosAndIdStripIn Amg::Vector2D* loc_hitPos_i is null."); 
    return false; 
  } 
  tmp_hitPos_i[Trk::locY] = loc_hitPos_i[Trk::loc2];
  
  return true;
}

// RDOHighPtID --> (Sim)HighPtID --> OfflineID --> ReadoutID --> getSLBID
bool Muon::TgcRdoToPrepDataToolMT::getHiPtIds(const TgcRawData& rd, int& sswId_o, int& sbLoc_o, int& slbId_o) const
{
  const CablingInfo* cinfo = getCabling();
  if (!cinfo) {
    return false;
  }

  int index = static_cast<int>(rd.index());
  int chip = static_cast<int>(rd.chip());
  int hitId = static_cast<int>(rd.hitId());
  
  // getSimHighPtIDfromRDOHighPtID changes index, chip and hitId.
  bool found = cinfo->m_tgcCabling->getSimHighPtIDfromRDOHighPtID(rd.isForward(), rd.isStrip(), index, chip, hitId);
  if(!found) {
    ATH_MSG_DEBUG("Failed to get SimHighPtID from RDOHighPtID for Pivot "
          << (rd.isStrip() ? "Strip" : "Wire"));
    return false;
  }
  
  // conversion for Run3
  uint16_t tmprodId, tmpsector;
  convertToRun2(rd,tmprodId,tmpsector);

  Identifier dummyId;
  if (rd.rodId()>12){ // Run3
    found = cinfo->m_tgcCabling->getOfflineIDfromHighPtID(dummyId,
                                                 rd.subDetectorId(), tmprodId, tmpsector,
                                                 rd.isStrip(), rd.isForward(), index,
                                                 chip, hitId, rd.hsub());
  }else{
    found = cinfo->m_tgcCabling->getOfflineIDfromHighPtID(dummyId,
                                                 rd.subDetectorId(), rd.rodId(), rd.sector(),
                                                 rd.isStrip(), rd.isForward(), index,
                                                 chip, hitId, rd.hsub());
  }

  if(!found) {
    ATH_MSG_DEBUG("Failed to get offlineID from HighPtID for Pivot "
          << (rd.isStrip() ? "Strip" : "Wire"));
    return false;
  }
  
  std::array<int, 3> dummy_i{};
  found = cinfo->m_tgcCabling->getReadoutIDfromOfflineID(dummyId, dummy_i[0], dummy_i[1], sswId_o, sbLoc_o, dummy_i[2]);
  if(!found) {
    ATH_MSG_DEBUG("Failed to get ReadoutID from OfflineID for Pivot "
          << (rd.isStrip() ? "Strip" : "Wire"));
    return false;
  }
  
  std::array<int, 2> i_o{};
  std::array<bool, 2> b_o{false, false};
  if (rd.rodId()>12){ // Run3
    found = cinfo->m_tgcCabling->getSLBIDfromReadoutID(i_o[0], b_o[0], b_o[1], i_o[1], slbId_o, 
                                              rd.subDetectorId(), tmprodId, sswId_o, sbLoc_o);
  }else{
    found = cinfo->m_tgcCabling->getSLBIDfromReadoutID(i_o[0], b_o[0], b_o[1], i_o[1], slbId_o, 
                                              rd.subDetectorId(), rd.rodId(), sswId_o, sbLoc_o);
  }
  if(!found) {
    ATH_MSG_DEBUG("Failed to get SLBID from ReadoutID for Pivot "
          << (rd.isStrip() ? "Strip" : "Wire"));
    return false;
  }

  return true;
}
bool Muon::TgcRdoToPrepDataToolMT::getSLIds(const bool isStrip,
                                            const TgcRawData& rd,
                                            std::array<Identifier, 3>& channelId, 
                                            int& index, int& chip, int& hitId, int& sub, int& sswId, int& sbLoc, 
                                            int& subMatrix, 
                                            std::array<int, 3>& bitpos,
                                            const bool isBoundary, const TgcRdo* rdoColl, 
                                            const int index_w, const int chip_w, const int hitId_w, const int sub_w) const

{
  const CablingInfo* cinfo = getCabling();
  if (!cinfo) {
    return false;
  }

  bool found = cinfo->m_tgcCabling->getHighPtIDfromROINumber(rd.roi(),
                                                    rd.isForward(),
                                                    isStrip, // get HitID of HPT Board
                                                    index,
                                                    chip,
                                                    hitId,
                                                    sub);
  if(!found) {
    ATH_MSG_DEBUG("Failed to get HighPtID from ROINumber for "
          << (!isStrip ? "Wire" : "Strip"));
    return false;
  }
  
  found = cinfo->m_tgcCabling->getSimHighPtIDfromRDOHighPtID(rd.isForward(),
                                                    isStrip,
                                                    index,
                                                    chip,
                                                    hitId);
  if(!found) {
    ATH_MSG_DEBUG("Failed to get SimHighPtID from RDOHighPtID for "
          << (!isStrip ? "Wire" : "Strip"));
    return false;
  }
  
  Identifier offlineId;
  found = cinfo->m_tgcCabling->getOfflineIDfromHighPtID(offlineId,
                           rd.subDetectorId(),
                           rd.rodId(),
                           rd.sector(),
                           isStrip,
                           rd.isForward(),
                           index,
                           chip,
                           hitId,
                           sub);
  if(!found) {
    ATH_MSG_DEBUG("Failed to get OfflineID from HighPtID for "
          << (!isStrip ? "Wire" : "Strip"));
    return false;
  }
    
  if(!isStrip || !isBoundary) { // Wire or strip whose ROI not including chamber boundary
    channelId[1] = offlineId;
    std::array<int, 3> dummy_i{};
    found = cinfo->m_tgcCabling->getReadoutIDfromOfflineID(channelId[1], 
                                                  dummy_i[0],
                                                  dummy_i[1],
                                                  sswId,
                                                  sbLoc,
                                                  dummy_i[2]);
    if(!found) {
      ATH_MSG_DEBUG("Failed to get ReadoutID from OfflineID for "
            << (!isStrip ? "Wire" : "Strip"));
      return false;
    }
  } else { // --> solving the chamber boundary of strip 
    sswId = rd.sector()+3; // SSW3: M3-EC phi0, SSW4: M3-EC phi1, SSW5: M3-EC phi2, SSW6: M3-EC phi3 

    // Loop over all HiPt Strip to find an associated one to obtain sbLoc
    bool exist_hipt_s = getSbLocOfEndcapStripBoundaryFromHiPt(rd, sbLoc, rdoColl, index_w, chip_w, hitId_w, sub_w);
    if(!exist_hipt_s) { 
      // Loop over all Tracklet Strip to find an associated one to obtain sbLoc 
      bool exist_tracklet_s = getSbLocOfEndcapStripBoundaryFromTracklet(rd, sbLoc, rdoColl, index_w, chip_w, hitId_w, sub_w);
      if(!exist_tracklet_s) { 
    ATH_MSG_DEBUG("Failed to find correspond Tracklet_strip for SL!!");
    return false;
      }
    }
  }

  if(!isStrip) { // wire
    getBitPosWire(rd, hitId, sub, subMatrix, bitpos);
  } else { // strip
    getBitPosStrip(hitId, sub, subMatrix, bitpos);
  } 
  
  for(int i=0; i<3; i++) { 
    if(i==1 && (!isStrip || !isBoundary)) continue; 
    
    found = cinfo->m_tgcCabling->getOfflineIDfromReadoutID(channelId[i],
                                                  rd.subDetectorId(),
                                                  rd.rodId(),
                                                  sswId,
                                                  sbLoc,
                                                  bitpos[i]);
    if(!found) {
      ATH_MSG_DEBUG("Failed to get OfflineID from ReadoutID for " 
            << (!isStrip ? "Wire" : "Strip"));
      if(!isStrip || i==1) {
    ATH_MSG_DEBUG("subDetectorId = " << rd.subDetectorId()
              << ", rodId = " << rd.rodId()
              << ", sswId = " << sswId
              << ", slbId = " << sbLoc
              << ", bitpos_" << (!isStrip ? "w" : "s") 
              << "[" << i << "] = " << bitpos[i]);
      }
      return false;
    }
  }

  return true;
}


bool Muon::TgcRdoToPrepDataToolMT::getSbLocOfEndcapStripBoundaryFromHiPt(const TgcRawData& rd, int& sbLoc,
                                       const TgcRdo* rdoColl, 
                                       const int index_w, const int chip_w, const int hitId_w, const int sub_w) const
{ 
  const CablingInfo* cinfo = getCabling();
  if (!cinfo) {
    return false;
  }

  bool exist_hipt_s = false;
  
  // Get trackletIds of three candidates 
  int trackletIdStripFirst{-1}, trackletIdStripSecond{-1}, trackletIdStripThird{-1};
  getEndcapStripCandidateTrackletIds(static_cast<int>(rd.roi()), trackletIdStripFirst, 
                     trackletIdStripSecond, trackletIdStripThird); 

  // Loop over all HiPt Strip to find an associated one
  for (const TgcRawData* rdH0 : *rdoColl) {

    // conversion for Run3
    uint16_t tmprodId, tmpsector;
    convertToRun2(rdH0,tmprodId,tmpsector);
    const TgcRawData rdH(rdH0->bcTag(), rdH0->subDetectorId(), tmprodId,
                         rdH0->l1Id(), rdH0->bcId(), rdH0->isStrip(),
                         rdH0->isForward(), tmpsector, rdH0->chip(),
                         rdH0->index(),rdH0->isHipt(), rdH0->hitId(),
                         rdH0->hsub(), rdH0->delta(), rdH0->inner());

    if((rdH.type()==TgcRawData::TYPE_HIPT) && // HiPt
       (rdH.isHipt()) && // HiPt flag is required
       (rdH.isStrip()) && // Strip
       (!rdH.isForward()) && // Endcap
       (rdH.bcTag()==rd.bcTag()) && // The same timing
       (rdH.subDetectorId()==rd.subDetectorId()) && // The same side
       (rdH.rodId()==rd.rodId()) && // The same ROD
       (rdH.sector()==rd.sector()) // The same sector (1/48 phi)
       ) {

      // Get sbLoc
      int sswId_o{0}, sbLoc_o{0}, slbId_o{0};
      bool found = getHiPtIds(rdH, sswId_o, sbLoc_o, slbId_o);
      if(!found){
        continue;
      }

      // Get subMatrix
      int slbsubMatrix = 0;
      std::array<int, 2> bitpos_o{};
      getBitPosOutStrip(rdH, slbsubMatrix, bitpos_o);

      // Get trackletId 
      int trackletIdStrip = 2*sbLoc_o + slbsubMatrix;

      // Compare trackletIds 
      if(trackletIdStrip!=trackletIdStripFirst &&  trackletIdStrip!=trackletIdStripSecond && 
         trackletIdStrip!=trackletIdStripThird) continue; 

      // The third candidate is used only if any corresponding HiPt Strip is found so far.  
      if(exist_hipt_s && trackletIdStrip==trackletIdStripThird) continue;

      // getRDOHighPtIDfromSimHighPtID overwrites index, chip and hitId. 
      int index_w_tmp = index_w;
      int chip_w_tmp = chip_w;
      int hitId_w_tmp = hitId_w;
      // Get RDO HighPt ID from SimHighPtID for wire 
      found = cinfo->m_tgcCabling->getRDOHighPtIDfromSimHighPtID(false, // false for endcap
                                                                 false, // wire
                                                                 index_w_tmp,
                                                                 chip_w_tmp,
                                                                 hitId_w_tmp);
      if(!found) {
        ATH_MSG_DEBUG("Failed to get RDOHighPtID from SimHighPtID for Wire");
        continue;
      }

      // RDO High Pt ID of Strip 
      int chip_s = static_cast<int>(rdH.chip());
      int hitId_s = static_cast<int>(rdH.hitId());
      int hsub_s = static_cast<int>(rdH.hsub());

      int roi = 0;
      found = cinfo->m_tgcCabling->getROINumberfromHighPtID(roi,
                                                            false, // false for Endcap
                                                            index_w_tmp, // hpb_wire (not used)
                                                            chip_w_tmp, // chip_wire
                                                            hitId_w_tmp, // hitId_wire
                                                            sub_w, // sub_wire
                                                            chip_s, // chip_strip (not used)
                                                            hitId_s, // hitId_strip 
                                                            hsub_s); // sub_strip
      if(!found) {
        ATH_MSG_DEBUG("Failed to get ROINumber from HighPtID for Strip");
        continue;
      }
      
      if(roi==rd.roi()) { 
        sbLoc = sbLoc_o;
        exist_hipt_s = true;
        if(trackletIdStrip==trackletIdStripFirst) break; // If the first candidate is found, exit from this for loop
      }
    }
  }

  return exist_hipt_s;
}

bool Muon::TgcRdoToPrepDataToolMT::getSbLocOfEndcapStripBoundaryFromTracklet(const TgcRawData& rd, int& sbLoc,
                                                                             const TgcRdo* rdoColl,
                                                                             const int index_w, const int chip_w, 
                                                                             const int hitId_w, const int sub_w) const
{
  const CablingInfo* cinfo = getCabling();

  bool exist_tracklet_s = false;
  
  // Get trackletIds of three candidates 
  int trackletIdStripFirst{-1}, trackletIdStripSecond{-1}, trackletIdStripThird{-1};
  getEndcapStripCandidateTrackletIds(static_cast<int>(rd.roi()), trackletIdStripFirst, 
                     trackletIdStripSecond, trackletIdStripThird); 
  
  // Loop over all Tracklet Strip to find an associated one
  for (const TgcRawData* rdS0 : *rdoColl) {

    // conversion for Run3
    uint16_t tmprodId{0}, tmpsector{0};
    convertToRun2(rdS0,tmprodId,tmpsector);
    const TgcRawData rdS(rdS0->bcTag(), rdS0->subDetectorId(), tmprodId,
                         rdS0->sswId(), rdS0->slbId(), rdS0->l1Id(),
                         rdS0->bcId(), rdS0->slbType(), rdS0->delta(),
                         rdS0->segment(), rdS0->subMatrix(), rdS0->position());
    
    bool isForward_s = (rdS.sswId()==7); // Doublet, Forward
    if(isForward_s) continue; // Chamber boundaries exist in endcap only

    if((rdS.type()==TgcRawData::TYPE_TRACKLET) &&
       (rdS.slbType()==TgcRawData::SLB_TYPE_DOUBLET_STRIP) &&
       (rdS.bcTag()==rd.bcTag()) &&
       (rdS.subDetectorId()==rd.subDetectorId()) &&
       (rdS.rodId()==rd.rodId()) &&
       (rdS.sswId()-3==rd.sector()) // SSW3: M3-EC phi0, SSW4: M3-EC phi1, SSW5: M3-EC phi2, SSW6: M3-EC phi3
       ) {

      // Compare trackletIds 
      int trackletIdStrip = static_cast<int>(2*rdS.slbId()+rdS.subMatrix()); // trackletId of Tracklet Strip
      if(trackletIdStrip!=trackletIdStripFirst && 
     trackletIdStrip!=trackletIdStripSecond && 
     trackletIdStrip!=trackletIdStripThird) continue; 

      // The third candidate is used only if any corresponding Tracklet Strip is found so far.
      if(exist_tracklet_s && trackletIdStrip==trackletIdStripThird) continue;
      
      Identifier offlineId;
      bool found = cinfo->m_tgcCabling->getOfflineIDfromLowPtCoincidenceID(offlineId, rdS.subDetectorId(), rdS.rodId(),
                                                                  rdS.sswId(), rdS.slbId(), rdS.subMatrix(),
                                                                  rdS.position(), false);
      if(!found) {
        ATH_MSG_DEBUG("Failed to get OfflineID from LowPtCoincidenceID for Strip");
        continue;
      }
      
      std::array<int, 7> i{};
      std::array<bool, 2> b{};
      found = cinfo->m_tgcCabling->getHighPtIDfromOfflineID(offlineId,i[0],i[1],i[2],b[0],b[1],i[3],i[4],i[5],i[6]);
      // i[0] subDetectorID, i[1] rodID, i[2] sectorInReadout, b[0] isStrip, 
      // b[1] isForward, i[3] hpb, i[4] chip, i[5] hitID, i[6] pos
      
      if(!found) {
        ATH_MSG_DEBUG("Failed to get HighPtID from OfflineID for Strip");
        continue;
      }

      // getRDOHighPtIDfromSimHighPtID overwrites index, chip and hitId. 
      int index_w_tmp = index_w;
      int chip_w_tmp = chip_w;
      int hitId_w_tmp = hitId_w;
      found = cinfo->m_tgcCabling->getRDOHighPtIDfromSimHighPtID(rd.isForward(), // false for endcap
                                                                 false, // wire
                                                                 index_w_tmp, // hpb-index
                                                                 chip_w_tmp, // chip-chip
                                                                 hitId_w_tmp); // hitID-hitId
      if(!found) {
        ATH_MSG_DEBUG("Failed to get RDOHighPtID from SimHighPtID for Wire");
        continue;
      }
      
      found = cinfo->m_tgcCabling->getRDOHighPtIDfromSimHighPtID(rd.isForward(), // false for endcap
                                                                 true, // strip
                                                                 i[3], // hpb-index
                                                                 i[4], // chip-chip
                                                                 i[5]); // hitID-hitId
      if(!found) {
        ATH_MSG_DEBUG("Failed to get RDOHighPtID from SimHighPtID for Strip");
        continue;
      }
      
      int roi = 0;
      found = cinfo->m_tgcCabling->getROINumberfromHighPtID(roi, 
                                                            rd.isForward(), // false for endcap
                                                            index_w_tmp, // hpb_wire (not used)
                                                            chip_w_tmp, // chip_wire
                                                            hitId_w_tmp, // hitId_wire
                                                            sub_w, // sub_wire
                                                            i[4], // chip_strip (not used)
                                                            i[5], // hitId_strip
                                                            i[6]); // sub_strip
      if(!found) {
        ATH_MSG_DEBUG("Failed to get ROINumber from HighPtID for Strip");
        continue;
      }
      
      if(roi==rd.roi()) { 
        sbLoc = rdS.slbId();
        exist_tracklet_s = true;
        if(trackletIdStrip==trackletIdStripFirst) break; // If the first candidate is found, exit from this for loop 
      }
    }
  }

  return exist_tracklet_s;
}

void Muon::TgcRdoToPrepDataToolMT::getEndcapStripCandidateTrackletIds(const int roi, int &trackletIdStripFirst, 
                                                                      int &trackletIdStripSecond, 
                                                                      int &trackletIdStripThird) {
  constexpr int T9SscMax =  2; // SSC 0 to SSC 2
  constexpr int T8SscMax =  4; // SSC 3 to SSC 4
  constexpr int T7SscMax =  6; // SSC 5 to SSC 6
  constexpr int T6SscMax = 12; // SSC 7 to SSC12
  constexpr int T5SscMax = 18; // SSC13 to SSC18

  constexpr int T9Offset = 32 + 0; 
  constexpr int T8Offset = 32 + 2; 
  constexpr int T7Offset = 32 + 4; 
  constexpr int T6Offset = 32 + 6; 
  constexpr int T5Offset = 32 + 8; 
  
  // ROI     :  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 ...
  // SSC     :  0  0  0  0  1  1  1  1  1  1  1  1  2  2  2  2  2  2 ...
  // Half SSC:  0  0  1  1  0  0  1  1  0  0  1  1  0  0  1  1  0  0 ... 
  int ssc = (roi+4)/8; 
  int halfSsc = (roi%4)/2; 
  
  if(       ssc< T9SscMax) { // SSC 0 to SSC 1 
    trackletIdStripFirst  = T9Offset + halfSsc; // T9
    trackletIdStripSecond = -1;
    trackletIdStripThird  = -1;
  } else if(ssc==T9SscMax) { // SSC 2 (boundary)
    trackletIdStripFirst  = T8Offset + halfSsc; // T8
    trackletIdStripSecond = T9Offset + halfSsc; // T9
    trackletIdStripThird  = -1;
  } else if(ssc< T8SscMax) { // SSC 3
    trackletIdStripFirst  = T8Offset + halfSsc; // T8
    trackletIdStripSecond = -1;
    trackletIdStripThird  = -1;
  } else if(ssc==T8SscMax) { // SSC 4 (boundary)
    trackletIdStripFirst  = T7Offset + halfSsc; // T7
    trackletIdStripSecond = T8Offset + halfSsc; // T8
    trackletIdStripThird  = -1;
  } else if(ssc< T7SscMax) { // SSC 5
    trackletIdStripFirst  = T7Offset + halfSsc; // T7
    trackletIdStripSecond = -1;
    trackletIdStripThird  = -1;
  } else if(ssc==T7SscMax) { // SSC 6 (boundary)
    trackletIdStripFirst  = T6Offset + halfSsc; // T6
    trackletIdStripSecond = T7Offset + halfSsc; // T7
    trackletIdStripThird  = T5Offset + halfSsc; // T5, HiPt Endcap Strip board bug
  } else if(ssc< T6SscMax) { // SSC 7 to SSC11 
    trackletIdStripFirst  = T6Offset + halfSsc; // T6
    trackletIdStripSecond = T5Offset + halfSsc; // T5, HiPt Endcap Strip board bug 
    trackletIdStripThird  = -1;
  } else if(ssc==T6SscMax) { // SSC12 (boundary)
    trackletIdStripFirst  = T6Offset + halfSsc; // T6
    trackletIdStripSecond = T5Offset + halfSsc; // T5
    trackletIdStripThird  = -1;
  } else if(ssc<=T5SscMax) { // SSC13 to SSC18 
    trackletIdStripFirst  = T5Offset + halfSsc; // T5
    trackletIdStripSecond = T6Offset + halfSsc; // T6, HiPt Endcap Strip board bug 
    trackletIdStripThird  = -1;
  } else {
    trackletIdStripFirst  = -1;
    trackletIdStripSecond = -1;
    trackletIdStripThird  = -1;
  }
}

const Muon::TgcRdoToPrepDataToolMT::CablingInfo*
Muon::TgcRdoToPrepDataToolMT::getCabling() const
{
  if (m_cablingInfo.isValid()) {
    return m_cablingInfo.ptr();
  }

  // get TGC Cabling Svc
  CablingInfo cinfo;
  if (cinfo.m_tgcCabling.retrieve().isFailure()) {
    ATH_MSG_ERROR( "Could not get MuonTGC_CablingSvc!" );
    return nullptr;
  }

  ATH_MSG_DEBUG(cinfo.m_tgcCabling->name() << " is OK");  

  // check the relation between hash and onlineId (onlineId depends on cabling)  
  unsigned int hashId_max = m_idHelperSvc->tgcIdHelper().module_hash_max();  
  cinfo.m_hashToOnlineId.reserve(hashId_max);  
  IdContext tgcContext = m_idHelperSvc->tgcIdHelper().module_context(); // TGC context   
  Identifier elementId;  
  int subDetectorId = 0; // 103(=0x67) for A side, 104(=0x68) for C side  
  int rodId = 0; // 1 to 12 (12-fold cabling)
  TgcRdo tgcRdo; // For onlineId conversion  
  for(unsigned int hashId=0; hashId<hashId_max; hashId++) {  
    IdentifierHash hash(hashId);  
    m_idHelperSvc->tgcIdHelper().get_id(hash, elementId, &tgcContext);  
    cinfo.m_tgcCabling->getReadoutIDfromElementID(elementId, subDetectorId, rodId);  
    // onlineId: 0 to 11 on A side and 12 to 23 on C side (12-fold cabling)  
    uint16_t onlineId = TgcRdo::calculateOnlineId(subDetectorId, rodId);   
    cinfo.m_hashToOnlineId.push_back(onlineId);  
  } 

  // initialize with false  
  cinfo.m_MAX_N_ROD = 2*12;

  m_cablingInfo.set (std::move (cinfo));
  return m_cablingInfo.ptr();
}

const Amg::Vector2D* Muon::TgcRdoToPrepDataToolMT::getSLLocalPosition(const MuonGM::TgcReadoutElement* readout, 
                                                                      const Identifier identify,  
                                                                      const double eta, 
                                                                      const double phi) { 
  if(!readout) return nullptr;  
  
  // Obtain the local coordinate by the secant method
  constexpr double length = 100.; // 100 mm
  constexpr unsigned int nRep = 10; // 10 repetitions
  constexpr double dRAccuracy = 1.0E-20; // recuqired dR accuracy
  constexpr double maxLocR = 10000.; // 10 m
  
  double locX = 0.;  
  double locY = 0.;  
  for(unsigned int iRep=0; iRep<nRep; iRep++) { 
    Amg::Vector2D loc_hitPos_c(locX, locY); // center or current position 
    Amg::Vector3D tmp_glob_hitPos_c{Amg::Vector3D::Zero()};
    readout->surface(identify).localToGlobal(loc_hitPos_c,tmp_glob_hitPos_c,tmp_glob_hitPos_c);
    const Amg::Vector3D &glob_hitPos_c  = tmp_glob_hitPos_c;

    double glob_eta_c = glob_hitPos_c.eta();
    double glob_phi_c = glob_hitPos_c.phi();

    Amg::Vector2D vector{eta - glob_eta_c,  deltaPhi(phi, glob_phi_c)};
      
    double dR =  std::hypot(vector[0], vector[1]);
    if(dR<dRAccuracy) {
    break;
    }
      
    Amg::Vector2D loc_hitPos_w(locX+length, locY       ); // slightly shifted position in the wire direction 
    Amg::Vector2D loc_hitPos_s(locX,        locY+length); // slightly shifted position in the strip direction 
    Amg::Vector3D tmp_glob_hitPos_w{Amg::Vector3D::Zero()}; 
    readout->surface(identify).localToGlobal(loc_hitPos_w,tmp_glob_hitPos_w,tmp_glob_hitPos_w);
    const Amg::Vector3D &glob_hitPos_w = tmp_glob_hitPos_w;
    Amg::Vector3D tmp_glob_hitPos_s{Amg::Vector3D::Zero()}; 
    readout->surface(identify).localToGlobal(loc_hitPos_s,tmp_glob_hitPos_s,tmp_glob_hitPos_s);
    const Amg::Vector3D &glob_hitPos_s = tmp_glob_hitPos_s;
      
    AmgSymMatrix(2) matrix{AmgSymMatrix(2)::Identity()};
    matrix(0,0) = glob_hitPos_w.eta() - glob_eta_c;
    matrix(1,0) = deltaPhi(glob_hitPos_w.phi(), glob_phi_c);   
    matrix(0,1) = glob_hitPos_s.eta() - glob_eta_c;
    matrix(1,1) = deltaPhi(glob_hitPos_s.phi(), glob_phi_c);
   
    bool invertible = matrix.determinant();
    if(invertible) {
      Amg::MatrixX invertedMatrix = matrix.inverse();
      Amg::Vector2D solution = invertedMatrix * vector;
      locX += length * solution(0,0);
      locY += length * solution(1,0);
      
      double locR = sqrt(locX*locX + locY*locY);
      if(locR>maxLocR) {
        locX *= maxLocR/locR;
        locY *= maxLocR/locR;
      }
    }
  }
  
  return new Amg::Vector2D(locX, locY); 
} 

const double Muon::TgcRdoToPrepDataToolMT::s_cutDropPrdsWithZeroWidth = 0.1;  // 0.1 mm
