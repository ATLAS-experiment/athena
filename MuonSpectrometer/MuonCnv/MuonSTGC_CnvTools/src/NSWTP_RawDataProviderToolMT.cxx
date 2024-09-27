/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "NSWTP_RawDataProviderToolMT.h"
#include "xAODMuonRDO/NSWTPRDOAuxContainer.h"

namespace Muon {

//=====================================================================
NSWTP_RawDataProviderToolMT::NSWTP_RawDataProviderToolMT(const std::string& type, const std::string& name, const IInterface* parent)
: AthAlgTool(type, name, parent), m_robDataProvider("ROBDataProviderSvc", name) 
{
  declareInterface<IMuonRawDataProviderTool>(this);
}

//=====================================================================
StatusCode NSWTP_RawDataProviderToolMT::initialize() 
{
  ATH_CHECK(m_decoder.retrieve());
  ATH_CHECK(m_idHelperSvc.retrieve());
  ATH_CHECK(m_robDataProvider.retrieve());
  ATH_CHECK(m_rdoContainerKey.initialize());
  
  return StatusCode::SUCCESS;
}


//=====================================================================
StatusCode NSWTP_RawDataProviderToolMT::convert() const 
{
  ATH_MSG_ERROR(__PRETTY_FUNCTION__ << " not yet implemented!");
  return StatusCode::FAILURE;
}

StatusCode NSWTP_RawDataProviderToolMT::convert(const ROBFragmentList&) const 
{
  ATH_MSG_ERROR(__PRETTY_FUNCTION__ << " not implemented!");
  return StatusCode::FAILURE;
}

StatusCode NSWTP_RawDataProviderToolMT::convert(const std::vector<IdentifierHash>&) const 
{
  ATH_MSG_ERROR(__PRETTY_FUNCTION__ << " not yet implemented!");
  return StatusCode::FAILURE;
}

StatusCode NSWTP_RawDataProviderToolMT::convert(const ROBFragmentList&, const std::vector<IdentifierHash>&) const 
{
  ATH_MSG_ERROR(__PRETTY_FUNCTION__ << " not implemented!");
  return StatusCode::FAILURE;
}


//=====================================================================
StatusCode NSWTP_RawDataProviderToolMT::convert(const ROBFragmentList& fragments, const EventContext& ctx) const 
{
  ATH_MSG_DEBUG(__PRETTY_FUNCTION__ << ": Got " << fragments.size() << " fragments");
  SG::WriteHandle<xAOD::NSWTPRDOContainer> rdoContainerHandle{m_rdoContainerKey, ctx};
  xAOD::NSWTPRDOContainer* pContainer{nullptr};

  // Retrieve container, if it exists in the event store; otherwise, create one
 
  ATH_CHECK(rdoContainerHandle.record(std::make_unique<xAOD::NSWTPRDOContainer>(), std::make_unique<xAOD::NSWTPRDOAuxContainer>()));
  pContainer = rdoContainerHandle.ptr();
  for (const auto *const fragment : fragments) {
    // TODO should an error here be a hard failure?
    ATH_CHECK(m_decoder->fillCollection(*fragment, *pContainer));
  }

  return StatusCode::SUCCESS;
}


//=====================================================================
// Convert all Pad Trigger ROBFragments within the given EventContext
StatusCode NSWTP_RawDataProviderToolMT::convert(const EventContext& ctx) const 
{
  // Get all ROBs!
  std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*> fragments;
  std::vector<uint32_t> robIDs;

  // Generate all possible ROB IDs, aka Source Identifiers
  // Valid values are: 0x00{6d,6e}002[0..f]
  for (uint32_t detectorID : {eformat::MUON_STGC_ENDCAP_A_SIDE, eformat::MUON_STGC_ENDCAP_C_SIDE}) {  // 0x6D, 0x6E
    for (uint8_t sector{}; sector < 16; sector++) {
      uint16_t moduleID = (0x1 << 4) | sector;
      eformat::helper::SourceIdentifier sourceID{static_cast<eformat::SubDetector>(detectorID), moduleID};
      robIDs.push_back(sourceID.simple_code());
    }
  }

  m_robDataProvider->getROBData(ctx, robIDs, fragments);
  return convert(fragments, ctx);
}

}  // namespace Muon

