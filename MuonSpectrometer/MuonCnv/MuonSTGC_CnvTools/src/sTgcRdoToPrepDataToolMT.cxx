/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "sTgcRdoToPrepDataToolMT.h"

#include "MuonReadoutGeometry/MuonStation.h"
#include "MuonReadoutGeometry/sTgcReadoutElement.h"

#include "xAODMuonPrepData/sTgcMeasurement.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"

using namespace MuonGM;
using namespace Trk;

namespace {
    std::atomic<bool> hitNegativeCharge{false};
}


namespace Muon{


 sTgcRdoToPrepDataToolMT::DataCache::DataCache(const std::size_t hashMax,
                                               const PrdKey_t& key,
                                               const EventContext& ctx):
        prdWriteHandle{key, ctx}{
    
    collections.resize(hashMax);

}

void sTgcRdoToPrepDataToolMT::DataCache::translateAndSort(sTgcPrepDataCollection& coll) {
    if (!detMgr) {
        return;
    }
    std::sort(coll.begin(), coll.end(),[](const sTgcPrepData*a, const sTgcPrepData* b){
        return a->identify() < b->identify();
    });
    const IMuonIdHelperSvc* idHelperSvc = detMgr->idHelperSvc();
    
    const sTgcIdHelper& id_helper = idHelperSvc->stgcIdHelper();
    for (const sTgcPrepData* prd : coll) {
        const Identifier prdId = prd->identify();
        const int gasGap = id_helper.gasGap(prdId);
        const int channel = id_helper.channel(prdId);
        const int chType = id_helper.channelType(prdId);
        xAOD::sTgcMeasurement* outHit{nullptr};
        if (chType == sTgcIdHelper::sTgcChannelTypes::Pad) {    
            if (!pads.hasHandle()) {
                continue;
            }          
            xAOD::MeasMatrix<2> lCov{xAOD::MeasMatrix<2>::Identity()};
            lCov(1,1) = prd->localCovariance()(0,0);
            /// Calculate the eta covariance from the geometry for now for 
            const MuonGMR4::sTgcReadoutElement* readoutEle =  detMgr->getsTgcReadoutElement(prdId);
            lCov(0,0) = Acts::square(0.5* readoutEle->padHeight(readoutEle->measurementHash(prdId)));

            //skip for now measurements with zero covariance
            if (lCov.determinant() == 0) {
                continue;               
            }
            outHit = pads->push_back(std::make_unique<xAOD::sTgcPadHit>());
            outHit->setMeasurement<2>(idHelperSvc->detElementHash(prdId), 
                                      xAOD::toStorage(Eigen::Rotation2D{-M_PI_2}*prd->localPosition()),
                                      std::move(lCov));
        } else if (chType == sTgcIdHelper::sTgcChannelTypes::Wire) {
            if (!wires.hasHandle()){
                continue;
            }
            outHit = wires->push_back(std::make_unique<xAOD::sTgcWireHit>());
        } else if (chType == sTgcIdHelper::sTgcChannelTypes::Strip) {
            if (!strips.hasHandle()) {
                continue;
            }
            auto stripHit = strips->push_back(std::make_unique<xAOD::sTgcStripCluster>());
            stripHit->setStripCharges(prd->stripCharges());
            stripHit->setStripNumbers(prd->stripNumbers());
            stripHit->setStripTimes(prd->stripTimes());
            outHit = stripHit;
        }
        if (!outHit) {
            continue;
        }
        if (chType != sTgcIdHelper::sTgcChannelTypes::Pad){
            /// In the R4 layout both phi & eta measurements are expressed on the same surface. However, the
            ///  rotation from eta -> phi is clockwise  --> minus sign in prd creation
            const double locPos = (chType == sTgcIdHelper::sTgcChannelTypes::Wire ? -1. : 1.) * prd->localPosition().x();
            xAOD::MeasVector<1> lPos = locPos * xAOD::MeasVector<1>::UnitX();
            xAOD::MeasMatrix<1> lCov{};
            lCov(0,0) = prd->localCovariance()(0,0);
            outHit->setMeasurement<1>(idHelperSvc->detElementHash(prdId),
                                      std::move(lPos),
                                      std::move(lCov));

        }
        outHit->setChannelNumber(channel);
        outHit->setGasGap(gasGap);
        outHit->setAuthor(prd->author());
        outHit->setTime(prd->time());
        outHit->setCharge(prd->charge());
        outHit->setReadoutElement(detMgr->getsTgcReadoutElement(prdId));
    }
}
sTgcRdoToPrepDataToolMT::DataCache::~DataCache() {
    if (!isValid) {
        return;
    }
    
    for (std::size_t hash = 0; hash < collections.size(); ++hash) {
        std::unique_ptr<sTgcPrepDataCollection>& coll = collections[hash];
        if (!coll) {
            continue;
        }
        sTgcPrepDataContainer::IDC_WriteHandle lock = prdWriteHandle->getWriteHandle( hash );
        if (lock.OnlineAndPresentInAnotherView()){
            continue;
        }

        translateAndSort(*coll);

        lock.addOrDelete(std::move(coll)).ignore();
    }
}
//============================================================================
StatusCode sTgcRdoToPrepDataToolMT::initialize()
{  
    ATH_MSG_DEBUG(" in initialize()");
    ATH_CHECK( m_idHelperSvc.retrieve() );
    // check if the initialization of the data container is success
    ATH_CHECK(m_stgcPrepDataContainerKey.initialize());
    ATH_CHECK(m_rdoContainerKey.initialize());
    ATH_CHECK(m_muDetMgrKey.initialize());
    ATH_CHECK(m_calibTool.retrieve());
    ATH_CHECK(m_prdContainerCacheKey.initialize(!m_prdContainerCacheKey.key().empty()) );

    if (m_useNewGeo) {
        ATH_CHECK(detStore()->retrieve(m_detMgrR4));
    }

    ATH_CHECK(m_xAODStripKey.initialize(!m_xAODStripKey.empty()));
    ATH_CHECK(m_xAODWireKey.initialize(!m_xAODWireKey.empty()));
    ATH_CHECK(m_xAODPadKey.initialize(!m_xAODPadKey.empty()));
    return StatusCode::SUCCESS;
}


//============================================================================
StatusCode sTgcRdoToPrepDataToolMT::processCollection(const EventContext& ctx,
                                                            DataCache& cache,
                                                            const STGC_RawDataCollection *rdoColl) const {

    const sTgcIdHelper& id_helper = m_idHelperSvc->stgcIdHelper();
    const IdentifierHash hash = rdoColl->identifyHash();

    ATH_MSG_DEBUG(" ***************** Start of process STGC Collection with hash Id: " << hash);
  
    // check if the collection already exists, otherwise add it
    if ( cache.prdWriteHandle->indexFindPtr(hash) != nullptr ) {
        ATH_MSG_DEBUG("In processCollection: collection already contained in the sTGC PrepData container");
        return StatusCode::FAILURE;

    } 

    // Get write handle for this collection
    sTgcPrepDataContainer::IDC_WriteHandle lock = cache.prdWriteHandle->getWriteHandle( hash );
    // Check if collection already exists (via the cache, i.e. in online trigger mode)
    if( lock.OnlineAndPresentInAnotherView() ) {
      ATH_MSG_DEBUG("In processCollection: collection already available in the sTgc PrepData container (via cache)");
      return StatusCode::SUCCESS;
    }

    // Make the PRD collection (will be added to container later
    std::unique_ptr<sTgcPrepDataCollection>& prdColl = cache.collections[hash];
    if (!prdColl) {
       prdColl = std::make_unique<sTgcPrepDataCollection>(hash);
    }

    // set the offline identifier of the collection Id
    IdContext  context = id_helper.module_context();
    Identifier moduleId;
    int getId = id_helper.get_id(hash, moduleId, &context);
    if ( getId != 0 ) {
      ATH_MSG_ERROR("Could not convert the hash Id: " << hash << " to identifier");
    } else {
      prdColl->setIdentifier(moduleId);
    }

    // vectors to hold PRDs decoded for this RDO collection
    std::vector<sTgcPrepData> sTgcStripPrds;
    std::vector<sTgcPrepData> sTgcWirePrds;
    std::vector<sTgcPrepData> sTgcPadPrds;
    sTgcStripPrds.reserve(rdoColl->size());
    sTgcPadPrds.reserve(rdoColl->size());
    sTgcWirePrds.reserve(rdoColl->size());
    
    // Count hits with negative charge, which indicates bad calibration
    
  
    // MuonDetectorManager from the conditions store
    const MuonGM::MuonDetectorManager* muonDetMgr{nullptr};
    ATH_CHECK(SG::get(muonDetMgr,m_muDetMgrKey,ctx));
    // convert the RDO collection to a PRD collection
    for ( const STGC_RawData* rdo : * rdoColl) {

        ATH_MSG_DEBUG("Adding a new sTgc PrepRawData");

        const Identifier  rdoId = rdo->identify();

        std::vector<Identifier> rdoList{rdoId};
    
        // get the local and global positions
        const MuonGM::sTgcReadoutElement* detEl = muonDetMgr->getsTgcReadoutElement(rdoId);
        Amg::Vector2D localPos{Amg::Vector2D::Zero()};

        int channelType = id_helper.channelType(rdoId);
        if (channelType < 0 || channelType > 2) {
            ATH_MSG_ERROR("Unknown sTGC channel type");
            return StatusCode::FAILURE;
        }
        if (!detEl->stripPosition(rdoId, localPos)) {
            ATH_MSG_ERROR("Could not get the local strip position for "<<m_idHelperSvc->toString(rdoId));
            return StatusCode::FAILURE;
        } 

        // get the resolution from strip width
        // to be fixed: for now do not set the resolution, it will be added in the next update    
        const int     gasGap = id_helper.gasGap(rdoId);
        const int    channel = id_helper.channel(rdoId);

        NSWCalib::CalibratedStrip calibStrip;
        ATH_CHECK (m_calibTool->calibrateStrip(ctx, rdo, calibStrip));
        int calibratedCharge = static_cast<int>(calibStrip.charge);
        if (calibratedCharge < 0 && channelType == 1) { // we only want to protect against negatively charged strips and we should not lose wire or pad hits because of bad calibrations since charge does not matter for them in reco. 
            if (!hitNegativeCharge) {
                ATH_MSG_DEBUG("One sTGC RDO or more, such as one with pdo = "<<rdo->charge() << " counts, corresponds to a negative charge (" << calibratedCharge << "). Skipping these RDOs");
                hitNegativeCharge = true; 
            }
            continue;
        }
        
        double width{0.};
        if (channelType == sTgcIdHelper::sTgcChannelTypes::Pad) { // Pads
            const MuonGM::MuonPadDesign* design = detEl->getPadDesign(rdoId);
            if (!design) {
                ATH_MSG_WARNING("Failed to get design for sTGC pad" );
            } else {
                width = design->channelWidth(localPos, true);
            } 
        } else { // Strips and wires
            const MuonGM::MuonChannelDesign* design = detEl->getDesign(rdoId);
            if (!design) {
                ATH_MSG_WARNING("Failed to get design for sTGC strip/wire" );
            } else {
                width = design->channelWidth();
            }
        }
                
        const double resolution = width/ std::sqrt(12.); 
        auto   cov = Amg::MatrixX(1,1);
        cov.setIdentity();
        (cov)(0,0) = resolution*resolution;  

        ATH_MSG_DEBUG("Adding a new STGC PRD, gasGap: " << gasGap << " channel: " << channel << " type: " << channelType << " resolution " << resolution );

        if(m_merge) {
            std::vector<sTgcPrepData>& sTgcPrds = channelType == sTgcIdHelper::Pad ? sTgcPadPrds : 
                                                  (channelType == sTgcIdHelper::Strip ? sTgcStripPrds : sTgcWirePrds);
        
            // check if the same RdoId is already present; keep the one with the smallest time
            auto it = std::find_if(sTgcPrds.begin(), sTgcPrds.end(), [&rdoId](const sTgcPrepData& prd) { 
                                                                        return (prd.identify() == rdoId); 
                                                                    });
            if (it == sTgcPrds.end()) {
                sTgcPrds.emplace_back(rdoId, hash, std::move(localPos), std::move(rdoList), std::move(cov), detEl, calibratedCharge, calibStrip.time);
                sTgcPrds.back().setAuthor(sTgcPrepData::Author::RdoToPrdConverter);
            } else if (it->time() > calibStrip.time) {
                *it = sTgcPrepData(rdoId, hash, std::move(localPos), std::move(rdoList), std::move(cov), detEl, calibratedCharge, calibStrip.time);
                it->setAuthor(sTgcPrepData::Author::RdoToPrdConverter);
            }
            
            // TODO - add merging for xAOD

        } else {
          // if not merging just add the PRD to the collection
          prdColl->push_back(std::make_unique<sTgcPrepData>(rdoId, 
                                                            hash, 
                                                            std::move(localPos), 
                                                            std::move(rdoList),
                                                            std::move(cov), 
                                                            detEl, 
                                                            calibratedCharge, 
                                                            calibStrip.time));
        }
    }

    if(m_merge) {
        // merge strip prds that fire closeby channels (not clusterizing wires and pads)
        std::vector<std::unique_ptr<sTgcPrepData>> sTgcStripClusters;
        ATH_CHECK(m_clusterBuilderTool->getClusters(ctx, std::move(sTgcStripPrds), sTgcStripClusters)); // Clusterize strips

        for ( std::unique_ptr<sTgcPrepData>& it : sTgcStripClusters ) {
            it->setHashAndIndex(prdColl->identifyHash(), prdColl->size());
            prdColl->push_back(std::move(it));
        } 
        for ( sTgcPrepData& prd : sTgcWirePrds ) {
            prd.setHashAndIndex(prdColl->identifyHash(), prdColl->size());
            prdColl->push_back(std::make_unique<sTgcPrepData>(std::move(prd)));
        }
        for (sTgcPrepData& prd : sTgcPadPrds ) {
            prd.setHashAndIndex(prdColl->identifyHash(), prdColl->size());
            prdColl->push_back(std::make_unique<sTgcPrepData>(std::move(prd)));
        }
    }

    // now add the collection to the container
    ATH_MSG_DEBUG("PRD hash " << hash << " has been moved to container");

    return StatusCode::SUCCESS;
}


//============================================================================
const STGC_RawDataContainer* sTgcRdoToPrepDataToolMT::getRdoContainer(const EventContext& ctx) const 
{
    auto rdoContainerHandle  = SG::makeHandle(m_rdoContainerKey, ctx);
    if(rdoContainerHandle.isValid()) {
        ATH_MSG_DEBUG("STGC_getRdoContainer success");
        return rdoContainerHandle.cptr();  
    }
    ATH_MSG_WARNING("Retrieval of STGC_RawDataContainer failed !");

    return nullptr;
}


//============================================================================
void sTgcRdoToPrepDataToolMT::processRDOContainer(const EventContext& ctx, 
                                                        DataCache& cache,
                                                        const std::vector<IdentifierHash>& idsToDecode) const
{
    ATH_MSG_DEBUG("In processRDOContainer");
    const STGC_RawDataContainer* rdoContainer = getRdoContainer(ctx);
    if (!rdoContainer) return;
  
    // run in unseeded mode
    for (const STGC_RawDataCollection* rdoColl : *rdoContainer) {
        if (rdoColl->empty()) continue;
        ATH_MSG_DEBUG("New RDO collection with " << rdoColl->size() << "STGC Hits");

        const IdentifierHash hash = rdoColl->identifyHash();

        // check if we actually want to decode this RDO collection
        if(!idsToDecode.empty() and std::find(idsToDecode.begin(), idsToDecode.end(), hash)==idsToDecode.end()) {
            ATH_MSG_DEBUG("Hash ID " << hash << " not in input list, ignore");
            continue;
        } else ATH_MSG_DEBUG("Going to decode " << hash);

        if(processCollection(ctx, cache, rdoColl).isFailure()) {
            ATH_MSG_DEBUG("processCsm returns a bad StatusCode - keep going for new data collections in this event");
        }
    } 
}

// methods for ROB-based decoding
//============================================================================
StatusCode sTgcRdoToPrepDataToolMT::decode(const EventContext& ctx,
                                                 const std::vector<IdentifierHash>& idVect) const {
    ATH_MSG_DEBUG("Size of the input hash id vector: " << idVect.size());

    DataCache outCache = setupOutputContainers(ctx);
    if (!outCache.isValid) return StatusCode::FAILURE;

    processRDOContainer(ctx, outCache, idVect);
    return StatusCode::SUCCESS;
} 


//============================================================================
StatusCode sTgcRdoToPrepDataToolMT::decode(const EventContext&, const std::vector<uint32_t>& ) const {
   ATH_MSG_FATAL("ROB based decoding is not supported....");
   return StatusCode::FAILURE;
}
StatusCode sTgcRdoToPrepDataToolMT::provideEmptyContainer(const EventContext& ctx) const {
    return setupOutputContainers(ctx).isValid ? StatusCode::SUCCESS : StatusCode::FAILURE;
}


sTgcRdoToPrepDataToolMT::DataCache
  sTgcRdoToPrepDataToolMT::setupOutputContainers(const EventContext& ctx) const {
      
    
      const std::size_t hashMax = m_idHelperSvc->stgcIdHelper().module_hash_max();
      DataCache containers{hashMax, m_stgcPrepDataContainerKey, ctx};

      containers.detMgr = m_detMgrR4;
      if(m_prdContainerCacheKey.key().empty()) {
         // No external cache, just record the container

         if (!containers.prdWriteHandle.record(std::make_unique<sTgcPrepDataContainer>(hashMax)).isSuccess()){
             ATH_MSG_FATAL("Faile to record "<<m_stgcPrepDataContainerKey.fullKey());
             return containers; 
         }
      } else {
        ///use the cache to get the container
        SG::UpdateHandle update{m_prdContainerCacheKey, ctx};
        if (!update.isValid()) {
          ATH_MSG_FATAL("Invalid UpdateHandle " << m_prdContainerCacheKey.key());
          return containers;
        }
        if (!containers.prdWriteHandle.record(std::make_unique<sTgcPrepDataContainer>(update.ptr())).isSuccess()) {
            ATH_MSG_FATAL("Failed to record "<<m_stgcPrepDataContainerKey.fullKey()
                        <<" from "<<m_prdContainerCacheKey.fullKey());
            return containers;
        }
      }
      containers.isValid = containers.strips.record(m_xAODStripKey, ctx).isSuccess()
                         && containers.wires.record(m_xAODWireKey, ctx).isSuccess()
                         && containers.pads.record(m_xAODPadKey, ctx).isSuccess();
      return containers;
}
}