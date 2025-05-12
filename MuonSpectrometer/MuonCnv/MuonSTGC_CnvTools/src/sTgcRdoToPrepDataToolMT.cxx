/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "sTgcRdoToPrepDataToolMT.h"

#include "MuonReadoutGeometry/MuonStation.h"
#include "MuonReadoutGeometry/sTgcReadoutElement.h"

using namespace MuonGM;
using namespace Trk;
using namespace Muon;

namespace {
    std::atomic<bool> hitNegativeCharge{false};
}
//============================================================================
Muon::sTgcRdoToPrepDataToolMT::sTgcRdoToPrepDataToolMT(const std::string& t, const std::string& n, const IInterface* p) 
: base_class(t,n,p){}


//============================================================================
StatusCode Muon::sTgcRdoToPrepDataToolMT::initialize()
{  
    ATH_MSG_DEBUG(" in initialize()");
    ATH_CHECK( m_idHelperSvc.retrieve() );
    // check if the initialization of the data container is success
    ATH_CHECK(m_stgcPrepDataContainerKey.initialize());
    ATH_CHECK(m_rdoContainerKey.initialize());
    ATH_CHECK(m_muDetMgrKey.initialize());
    ATH_CHECK(m_calibTool.retrieve());
    ATH_CHECK(m_prdContainerCacheKey.initialize(!m_prdContainerCacheKey.key().empty()) );
    ATH_MSG_INFO("initialize() successful in " << name());
    return StatusCode::SUCCESS;
}


//============================================================================
StatusCode Muon::sTgcRdoToPrepDataToolMT::processCollection(const EventContext& ctx,
                                                            Muon::sTgcPrepDataContainer* stgcPrepDataContainer, 
                                                            const STGC_RawDataCollection *rdoColl, 
                                                            std::vector<IdentifierHash>& idWithDataVect) const {

    const sTgcIdHelper& id_helper = m_idHelperSvc->stgcIdHelper();
    const IdentifierHash hash = rdoColl->identifyHash();

    ATH_MSG_DEBUG(" ***************** Start of process STGC Collection with hash Id: " << hash);
  
    // check if the collection already exists, otherwise add it
    if ( stgcPrepDataContainer->indexFindPtr(hash) != nullptr ) {
        ATH_MSG_DEBUG("In processCollection: collection already contained in the sTGC PrepData container");
        return StatusCode::FAILURE;

    } 

    // Get write handle for this collection
    sTgcPrepDataContainer::IDC_WriteHandle lock = stgcPrepDataContainer->getWriteHandle( hash );
    // Check if collection already exists (via the cache, i.e. in online trigger mode)
    if( lock.OnlineAndPresentInAnotherView() ) {
      ATH_MSG_DEBUG("In processCollection: collection already available in the sTgc PrepData container (via cache)");
      idWithDataVect.push_back(hash);
      return StatusCode::SUCCESS;
    }

    // Make the PRD collection (will be added to container later
    std::unique_ptr<sTgcPrepDataCollection> prdColl = std::make_unique<sTgcPrepDataCollection>(hash);
    idWithDataVect.push_back(hash);

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
    SG::ReadCondHandle<MuonGM::MuonDetectorManager> muonDetMgr{m_muDetMgrKey,ctx};
    if(!muonDetMgr.isValid()){
        ATH_MSG_ERROR("Null pointer to the read MuonDetectorManager conditions object");
        return StatusCode::FAILURE;
    }
    // convert the RDO collection to a PRD collection
    for ( const STGC_RawData* rdo : * rdoColl) {

        ATH_MSG_DEBUG("Adding a new sTgc PrepRawData");

        const Identifier  rdoId = rdo->identify();

        if (!m_idHelperSvc->issTgc(rdoId)) {
            ATH_MSG_WARNING("The given Identifier "<<rdoId.get_compact()<<" ("<<m_idHelperSvc->toString(rdoId)<<") is no sTGC Identifier, continuing");
            continue;
        }

        std::vector<Identifier> rdoList;
        rdoList.push_back(rdoId);

        // get the local and global positions
        const MuonGM::sTgcReadoutElement* detEl = muonDetMgr->getsTgcReadoutElement(rdoId);
        Amg::Vector2D localPos;

        int channelType = id_helper.channelType(rdoId);
        if (channelType < 0 || channelType > 2) {
            ATH_MSG_ERROR("Unknown sTGC channel type");
            return StatusCode::FAILURE;
        }

        bool getLocalPos = detEl->stripPosition(rdoId, localPos);
        if ( !getLocalPos ) {
            ATH_MSG_ERROR("Could not get the local strip position for sTgc");
            return StatusCode::FAILURE;
        } 

        // get the resolution from strip width
        // to be fixed: for now do not set the resolution, it will be added in the next update    
        const int     gasGap = id_helper.gasGap(rdoId);
        const int    channel = id_helper.channel(rdoId);
        const uint16_t bcTag = rdo->bcTag();

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
        if (channelType == 0) { // Pads
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
                sTgcPrds.emplace_back(rdoId, hash, std::move(localPos), std::move(rdoList), std::move(cov), detEl, calibratedCharge, calibStrip.time, bcTag);
                sTgcPrds.back().setAuthor(sTgcPrepData::Author::RdoToPrdConverter);
            } else if (it->time() > calibStrip.time) {
                *it = sTgcPrepData(rdoId, hash, std::move(localPos), std::move(rdoList), std::move(cov), detEl, calibratedCharge, calibStrip.time, bcTag);
                it->setAuthor(sTgcPrepData::Author::RdoToPrdConverter);
            }
        } else {
            // if not merging just add the PRD to the collection
            prdColl->push_back(new sTgcPrepData(rdoId,hash, std::move(localPos), std::move(rdoList), std::move(cov), 
                                                detEl, calibratedCharge, calibStrip.time, bcTag));
        } 
    }

    if(m_merge) {
        // merge strip prds that fire closeby channels (not clusterizing wires and pads)
        std::vector<std::unique_ptr<Muon::sTgcPrepData>> sTgcStripClusters;
        ATH_CHECK(m_clusterBuilderTool->getClusters(ctx, std::move(sTgcStripPrds), sTgcStripClusters)); // Clusterize strips

        for ( std::unique_ptr<Muon::sTgcPrepData>& it : sTgcStripClusters ) {
            it->setHashAndIndex(prdColl->identifyHash(), prdColl->size());
            prdColl->push_back(std::move(it));
        } 
        for ( Muon::sTgcPrepData& prd : sTgcWirePrds ) {
            prd.setHashAndIndex(prdColl->identifyHash(), prdColl->size());
            prdColl->emplace_back(new sTgcPrepData(std::move(prd)));
        }
        for (Muon::sTgcPrepData& prd : sTgcPadPrds ) {
            prd.setHashAndIndex(prdColl->identifyHash(), prdColl->size());
            prdColl->emplace_back(new sTgcPrepData(std::move(prd)));
        }
    }

    // now add the collection to the container
    ATH_CHECK( lock.addOrDelete(std::move( prdColl ) ) );
    ATH_MSG_DEBUG("PRD hash " << hash << " has been moved to container");

    return StatusCode::SUCCESS;
}


//============================================================================
const STGC_RawDataContainer* Muon::sTgcRdoToPrepDataToolMT::getRdoContainer(const EventContext& ctx) const 
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
void Muon::sTgcRdoToPrepDataToolMT::processRDOContainer(const EventContext& ctx, 
                                                          Muon::sTgcPrepDataContainer* stgcPrepDataContainer, 
                                                          const std::vector<IdentifierHash>& idsToDecode,
                                                          std::vector<IdentifierHash>& idWithDataVect ) const
{
    ATH_MSG_DEBUG("In processRDOContainer");
    const STGC_RawDataContainer* rdoContainer = getRdoContainer(ctx);
    if (!rdoContainer) return;
  
    // run in unseeded mode
    for (STGC_RawDataContainer::const_iterator it = rdoContainer->begin(); it != rdoContainer->end(); ++it ) {
        const auto *rdoColl = *it;
        if (rdoColl->empty()) continue;
        ATH_MSG_DEBUG("New RDO collection with " << rdoColl->size() << "STGC Hits");

        const IdentifierHash hash = rdoColl->identifyHash();

        // check if we actually want to decode this RDO collection
        if(!idsToDecode.empty() and std::find(idsToDecode.begin(), idsToDecode.end(), hash)==idsToDecode.end()) {
            ATH_MSG_DEBUG("Hash ID " << hash << " not in input list, ignore");
            continue;
        } else ATH_MSG_DEBUG("Going to decode " << hash);

        if(processCollection(ctx, stgcPrepDataContainer, rdoColl, idWithDataVect).isFailure()) {
            ATH_MSG_DEBUG("processCsm returns a bad StatusCode - keep going for new data collections in this event");
        }
    } 
}


// methods for ROB-based decoding
//============================================================================
StatusCode Muon::sTgcRdoToPrepDataToolMT::decode(const EventContext& ctx,
                                                   std::vector<IdentifierHash>& idVect, 
                                                   std::vector<IdentifierHash>& idWithDataVect ) const
{
    ATH_MSG_DEBUG("Size of the input hash id vector: " << idVect.size());

    // clear the output vector of selected data
    idWithDataVect.clear();

    Muon::sTgcPrepDataContainer* stgcPrepDataContainer = setupSTGC_PrepDataContainer(ctx);

    if (!stgcPrepDataContainer) return StatusCode::FAILURE;

    processRDOContainer(ctx, stgcPrepDataContainer, idVect, idWithDataVect);
    return StatusCode::SUCCESS;
} 


//============================================================================
StatusCode Muon::sTgcRdoToPrepDataToolMT::decode(const EventContext&, const std::vector<uint32_t>& ) const {
   ATH_MSG_FATAL("ROB based decoding is not supported....");
   return StatusCode::FAILURE;
}
StatusCode Muon::sTgcRdoToPrepDataToolMT::provideEmptyContainer(const EventContext& ctx) const {
    return setupSTGC_PrepDataContainer(ctx) ? StatusCode::SUCCESS : StatusCode::FAILURE;
}

// printout methods
void Muon::sTgcRdoToPrepDataToolMT::printInputRdo(const EventContext&) const { }
void Muon::sTgcRdoToPrepDataToolMT::printPrepData(const EventContext&) const { }

Muon::sTgcPrepDataContainer* Muon::sTgcRdoToPrepDataToolMT::setupSTGC_PrepDataContainer(const EventContext& ctx) const {

  SG::WriteHandle< Muon::sTgcPrepDataContainer > handle(m_stgcPrepDataContainerKey, ctx);
  if(m_prdContainerCacheKey.key().empty()) {
    // No external cache, just record the container
    StatusCode status = handle.record(std::make_unique<Muon::sTgcPrepDataContainer>(m_idHelperSvc->stgcIdHelper().module_hash_max()));
    
    if (status.isFailure() || !handle.isValid() )   {
      ATH_MSG_FATAL("Could not record container of STGC PrepData Container at " << m_stgcPrepDataContainerKey.key()); 
      return nullptr;
    }
  } else {
    //use the cache to get the container
    SG::UpdateHandle<sTgcPrepDataCollection_Cache> update(m_prdContainerCacheKey, ctx);
    if (!update.isValid()) {
      ATH_MSG_FATAL("Invalid UpdateHandle " << m_prdContainerCacheKey.key());
      return nullptr;
    }
    StatusCode status = handle.record(std::make_unique<Muon::sTgcPrepDataContainer>(update.ptr()));
    if (status.isFailure() || !handle.isValid()) {
      ATH_MSG_FATAL("Could not record container of sTGC PrepData Container using cache " << m_prdContainerCacheKey.key() << " - " << m_stgcPrepDataContainerKey.key());
      return nullptr;
    }
    ATH_MSG_DEBUG("Created container using cache for " << m_prdContainerCacheKey.key());
  }
  
  return handle.ptr();
}
