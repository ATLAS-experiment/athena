/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "NSWTP_RawDataProviderTool.h"
#include "xAODMuonRDO/NSWTPRDOAuxContainer.h"

namespace Muon {

//=====================================================================
StatusCode NSWTP_RawDataProviderTool::initialize() 
{
  ATH_CHECK(m_decoder.retrieve());
  ATH_CHECK(m_idHelperSvc.retrieve());
  ATH_CHECK(m_robDataProvider.retrieve());
  ATH_CHECK(m_rdoContainerKey.initialize());
  
  return StatusCode::SUCCESS;
}


//=====================================================================
// Convert all Pad Trigger ROBFragments within the given EventContext
StatusCode NSWTP_RawDataProviderTool::convert(const EventContext& ctx) const {
  // Get all ROBs!
  ROBFragmentList fragments;
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


StatusCode NSWTP_RawDataProviderTool::convert(const std::vector<IdentifierHash>& , 
                                              const EventContext&) const {
  ATH_MSG_ERROR(__PRETTY_FUNCTION__<<" not implemented");
  return StatusCode::FAILURE;
}
StatusCode NSWTP_RawDataProviderTool::convert(const std::vector<uint32_t>& , 
                                              const EventContext& ) const {
   ATH_MSG_ERROR(__PRETTY_FUNCTION__<<" not implemented");
  return StatusCode::FAILURE;
}


}  // namespace Muon

