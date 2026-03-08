/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#include "STGC_RawDataProviderTool.h"
#include "MuonRDO/STGC_RawDataContainer.h"
#include "eformat/SourceIdentifier.h"
using eformat::helper::SourceIdentifier;

using namespace OFFLINE_FRAGMENTS_NAMESPACE;


//==============================================================================
StatusCode Muon::STGC_RawDataProviderTool::initialize() {


  ATH_CHECK(m_idHelperSvc.retrieve());
  ATH_CHECK(m_decoder.retrieve());
  ATH_CHECK(m_robDataProvider.retrieve()); // ROBDataProviderSvc
  ATH_CHECK(m_rdoContainerKey.initialize());

  m_maxhashtoUse = m_idHelperSvc->stgcIdHelper().module_hash_max();  
  // generate all the Source Identifiers to request the fragments.
  // assume 16 RODs per side (one per sector) and that ROB ID = ROD ID.
  for (uint32_t detID : {eformat::MUON_STGC_ENDCAP_A_SIDE, eformat::MUON_STGC_ENDCAP_C_SIDE}) { //0x6D, 0x6E
    for (uint8_t sectorID(0); sectorID < 16; ++sectorID) {
       // for now lets build all the possible ROB ids of all possible readout configurations
       // maybe later we can come up with a smart way to detect which readout sheme is running and only request the relevant ROB ids from the ROBDataProviderSvc
       // reference: slide 6 of https://indico.cern.ch/event/1260377/contributions/5294286/attachments/2603399/4495810/NSW-SwRod-Felix-v3.pdf
 
       uint16_t moduleID = (0x0 << 8) | sectorID; // combined/single ROB
       SourceIdentifier sid(static_cast<eformat::SubDetector>(detID), moduleID);
       m_allRobIds.push_back(sid.simple_code());
       
       moduleID = (0x1 << 8) | sectorID; // full device ROB (split configuration)
       sid = SourceIdentifier(static_cast<eformat::SubDetector>(detID), moduleID);
       m_allRobIds.push_back(sid.simple_code());
       
       moduleID = (0x2 << 8) | sectorID; // shared device ROB (split configuration)
       sid = SourceIdentifier(static_cast<eformat::SubDetector>(detID), moduleID);
       m_allRobIds.push_back(sid.simple_code());
       
       moduleID = (0x3 << 8) | sectorID; // spare device ROB (split configuration)
       sid = SourceIdentifier(static_cast<eformat::SubDetector>(detID), moduleID);
       m_allRobIds.push_back(sid.simple_code());
    }
  }

  ATH_CHECK(m_rdoContainerCacheKey.initialize(!m_rdoContainerCacheKey.key().empty()));
  return StatusCode::SUCCESS;
}


//==============================================================================
StatusCode Muon::STGC_RawDataProviderTool::convertIntoContainer(const EventContext&ctx, 
                                                                const ROBFragmentList& vecRobs, 
                                                                const std::vector<IdentifierHash>& rdoIdhVect, 
                                                                STGC_RawDataContainer& stgcRdoContainer) const
{
  // Since there can be multiple ROBFragments contributing to the same RDO collection a temporary cache is setup and passed to fillCollection by reference. Once all ROBFragments are processed the collections are added into the rdo container
  std::unordered_map<IdentifierHash, std::unique_ptr<STGC_RawDataCollection>> rdo_map;


  // Loop on the passed ROB fragments, and call the decoder for each one to fill the RDO container.
  for (const ROBFragment* fragment : vecRobs)
    ATH_CHECK( m_decoder->fillCollection(ctx, *fragment, rdoIdhVect, rdo_map) ); // always returns StatusCode::SUCCESS

  
  // error counters
  int nerr_duplicate{0}, nerr_rdo{0};

  // add the RDO collections created from the data of this ROB into the identifiable container.
  for (auto& [hash, collection]: rdo_map) {

    if ((!collection) or collection->empty()) continue; // skip empty collections

    STGC_RawDataContainer::IDC_WriteHandle lock = stgcRdoContainer.getWriteHandle(hash);

    if (lock.alreadyPresent()) {
      ++nerr_duplicate;
    } else if (!lock.addOrDelete(std::move(collection)).isSuccess()) {
      // since we prevent duplicates above, this error should never happen.
      ++nerr_rdo;
    }
  }


  // error summary (to reduce the number of messages)
  if (nerr_duplicate) ATH_MSG_WARNING(nerr_duplicate << " elinks skipped since the same module hash has been added by a previous ROB fragment");
  if (nerr_rdo) {
      ATH_MSG_ERROR("Failed to add "<<nerr_rdo<<" RDOs into the identifiable container");
      return StatusCode::FAILURE;
   }

  ATH_MSG_DEBUG("Size of sTgcRdoContainer is " << stgcRdoContainer.size());
  return StatusCode::SUCCESS;
}


//==============================================================================
StatusCode Muon::STGC_RawDataProviderTool::initRdoContainer(const EventContext& ctx, STGC_RawDataContainer*& rdoContainer) const
{
  // Create the identifiable RdoContainer in StoreGate to be filled with decoded fragment contents.
  SG::WriteHandle rdoContainerHandle(m_rdoContainerKey, ctx); 

  const bool externalCacheRDO = !m_rdoContainerCacheKey.key().empty();
  if(!externalCacheRDO){
    ATH_CHECK(rdoContainerHandle.record(std::make_unique<STGC_RawDataContainer>(m_maxhashtoUse)));
    ATH_MSG_DEBUG("Created STGC RDO container");
  } else {
    SG::UpdateHandle<STGC_RawDataCollection_Cache> update(m_rdoContainerCacheKey, ctx);
    ATH_CHECK(update.isValid());
    ATH_CHECK(rdoContainerHandle.record(std::make_unique<STGC_RawDataContainer>(update.ptr())));
    ATH_MSG_DEBUG("Created STGC RDO container using cache for " << m_rdoContainerCacheKey.key());
  }

  // this should never happen, but since we dereference the pointer, we should check
  if (!(rdoContainer = rdoContainerHandle.ptr())) {
    ATH_MSG_ERROR("The STGC RDO container is null, cannot decode STGC data");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}


//==============================================================================
StatusCode Muon::STGC_RawDataProviderTool::convert(const std::vector<IdentifierHash>& rdoIdhVect, const EventContext& ctx) const
{
  // method for RoI-seeded mode. we don't let empty hash containers reach the decoder, 
  // since an empty container means unseeded mode (decode everything).

  STGC_RawDataContainer* rdoContainer{nullptr};
  ATH_CHECK(initRdoContainer(ctx, rdoContainer));

  if (rdoIdhVect.empty() || m_skipDecoding) return StatusCode::SUCCESS;

  ROBFragmentList vecRobf;
  m_robDataProvider->getROBData(ctx, m_allRobIds, vecRobf);

  return convertIntoContainer(ctx, vecRobf, rdoIdhVect, *rdoContainer);
}


//==============================================================================
StatusCode  Muon::STGC_RawDataProviderTool::convert(const EventContext& ctx) const {
  // method for unseeded mode. just decode everything.

  STGC_RawDataContainer* rdoContainer{nullptr};
  ATH_CHECK(initRdoContainer(ctx, rdoContainer));
  if(m_skipDecoding) return StatusCode::SUCCESS;

  ROBFragmentList vecRobf;
  m_robDataProvider->getROBData(ctx, m_allRobIds, vecRobf);
  
  // dummy hashID vector for the decoder (empty = unseeded mode)
  const std::vector<IdentifierHash> rdoIdhVect;

  return convertIntoContainer(ctx, vecRobf, rdoIdhVect, *rdoContainer);
}

StatusCode Muon::STGC_RawDataProviderTool::convert(const std::vector<uint32_t>& robIds, const EventContext& ctx) const
{
  STGC_RawDataContainer* rdoContainer{nullptr};
  ATH_CHECK(initRdoContainer(ctx, rdoContainer));
  
  if (robIds.empty() || m_skipDecoding) return StatusCode::SUCCESS;
  
  ROBFragmentList vecRobf;

  m_robDataProvider->getROBData(ctx, robIds, vecRobf);

  // pass empty list of ID hashes, every ROB ID in list will be decoded
  const std::vector<IdentifierHash> hashIDList; 

  return convertIntoContainer(ctx, vecRobf, hashIDList, *rdoContainer);

}