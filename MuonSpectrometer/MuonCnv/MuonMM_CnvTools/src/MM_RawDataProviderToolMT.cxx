/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// MM_RawDataProviderToolMT.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "MM_RawDataProviderToolMT.h"
#include "MuonRDO/MM_RawDataContainer.h"
#include "eformat/SourceIdentifier.h"
using eformat::helper::SourceIdentifier;

//==============================================================================
Muon::MM_RawDataProviderToolMT::MM_RawDataProviderToolMT(const std::string& t, const std::string& n, const IInterface*  p)
: MM_RawDataProviderToolCore(t, n, p)
{
  declareInterface<IMuonRawDataProviderTool>(this);
 
}


//==============================================================================
StatusCode Muon::MM_RawDataProviderToolMT::initialize()
{
  // generate all the Source Identifiers for MicroMegas to request the fragments.
  // assume 16 RODs per side (one per sector) and that ROB ID = ROD ID.
  for (uint32_t detID : {eformat::MUON_MMEGA_ENDCAP_A_SIDE, eformat::MUON_MMEGA_ENDCAP_C_SIDE}) { //0x6B, 0x6C
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
       
    }
  }

  ATH_CHECK(m_rdoContainerCacheKey.initialize(!m_rdoContainerCacheKey.key().empty()));
  ATH_CHECK(MM_RawDataProviderToolCore::initialize());
  return StatusCode::SUCCESS;
}


//==============================================================================
StatusCode Muon::MM_RawDataProviderToolMT::initRdoContainer(const EventContext& ctx, MM_RawDataContainer*& rdoContainer) const
{
  // Create the identifiable RdoContainer in StoreGate to be filled with decoded fragment contents.
  SG::WriteHandle<MM_RawDataContainer> rdoContainerHandle(m_rdoContainerKey, ctx); 


  const bool externalCacheRDO = !m_rdoContainerCacheKey.key().empty();
  if(!externalCacheRDO){
    ATH_CHECK(rdoContainerHandle.record(std::make_unique<MM_RawDataContainer>(m_maxhashtoUse)));
    ATH_MSG_DEBUG("Created MM container");
  } else {
    SG::UpdateHandle<MM_RawDataCollection_Cache> update(m_rdoContainerCacheKey, ctx);
    ATH_CHECK(update.isValid());
    ATH_CHECK(rdoContainerHandle.record(std::make_unique<MM_RawDataContainer>(update.ptr())));
    ATH_MSG_DEBUG("Created MM container using cache for " << m_rdoContainerCacheKey.key());
  }

  // this should never happen, but since we dereference the pointer, we should check
  if (!(rdoContainer = rdoContainerHandle.ptr())) {
    ATH_MSG_ERROR("the MM RDO container is null, cannot decode MM data");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

//==============================================================================
StatusCode Muon::MM_RawDataProviderToolMT::convert(const std::vector<uint32_t>& robIds, const EventContext& ctx) const {
  // method for RoI-seeded mode via ROB IDs
  MM_RawDataContainer* rdoContainer{nullptr};
  ATH_CHECK(initRdoContainer(ctx, rdoContainer));

  if (robIds.empty() || m_skipDecoding) return StatusCode::SUCCESS;
  
  std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*> vecRobf;

  m_robDataProvider->getROBData(ctx, robIds, vecRobf);

  // pass empty list of ID hashes, every ROB ID in list will be decoded
  const std::vector<IdentifierHash> hashIDList;
  return convertIntoContainer(ctx, vecRobf, hashIDList, *rdoContainer);
}

//==============================================================================
StatusCode Muon::MM_RawDataProviderToolMT::convert(const std::vector<IdentifierHash>& rdoIdhVect, const EventContext& ctx) const
{
  // method for RoI-seeded mode via hash IDs. we don't let empty hash containers reach the decoder, 
  // since an empty container means unseeded mode (decode everything).

  MM_RawDataContainer* rdoContainer{nullptr};
  ATH_CHECK(initRdoContainer(ctx, rdoContainer));

  if (rdoIdhVect.empty() || m_skipDecoding) return StatusCode::SUCCESS;
  
  std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*> vecRobf;
  m_robDataProvider->getROBData(ctx, m_allRobIds, vecRobf);

  return convertIntoContainer(ctx, vecRobf, rdoIdhVect, *rdoContainer);
}


//==============================================================================
StatusCode Muon::MM_RawDataProviderToolMT::convert(const EventContext& ctx) const
{
  // method for unseeded mode. just decode everything.

  MM_RawDataContainer* rdoContainer{nullptr};
  ATH_CHECK(initRdoContainer(ctx, rdoContainer));
  if(m_skipDecoding) return StatusCode::SUCCESS;

  std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*> vecRobf;
  m_robDataProvider->getROBData(ctx, m_allRobIds, vecRobf);
  
  // dummy hashID vector for the decoder (empty = unseeded mode)
  const std::vector<IdentifierHash> rdoIdhVect;

  return convertIntoContainer(ctx, vecRobf, rdoIdhVect, *rdoContainer);
}


//==============================================================================
StatusCode Muon::MM_RawDataProviderToolMT::convert() const //call decoding function using list of all detector ROBId's
{
  ATH_MSG_ERROR("MM_RawDataProviderToolMT::convert() Not implemented.");
  return StatusCode::FAILURE;
}

StatusCode Muon::MM_RawDataProviderToolMT::convert(const std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>&) const
{
  ATH_MSG_ERROR("MM_RawDataProviderToolMT::convert(const ROBFragmentList&) not implemented.");
  return StatusCode::FAILURE;
}

StatusCode Muon::MM_RawDataProviderToolMT::convert(const std::vector<IdentifierHash>&) const
{
  ATH_MSG_ERROR("MM_RawDataProviderToolMT::convert(const std::vector<IdentifierHash>&) not implemented.");
  return StatusCode::FAILURE;
}

StatusCode Muon::MM_RawDataProviderToolMT::convert( const std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>&, const std::vector<IdentifierHash>&) const
{
  ATH_MSG_ERROR("MM_RawDataProviderToolMT::convert(const ROBFragmentList&) not implemented.");
  return StatusCode::FAILURE;
}

