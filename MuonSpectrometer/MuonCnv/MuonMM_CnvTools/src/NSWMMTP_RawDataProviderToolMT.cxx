/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "NSWMMTP_RawDataProviderToolMT.h"
#include "xAODMuonRDO/NSWMMTPRDOAuxContainer.h"

namespace Muon {

  //=====================================================================
  NSWMMTP_RawDataProviderToolMT::NSWMMTP_RawDataProviderToolMT(const std::string& type, const std::string& name, const IInterface* parent)
    : AthAlgTool(type, name, parent), m_robDataProvider("ROBDataProviderSvc", name) 
  {
    declareInterface<IMuonRawDataProviderTool>(this);
  }

  //=====================================================================
  StatusCode NSWMMTP_RawDataProviderToolMT::initialize() 
  {
    ATH_CHECK(m_decoder.retrieve());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_robDataProvider.retrieve());
    ATH_CHECK(m_rdoContainerKey.initialize());
  
    return StatusCode::SUCCESS;
  }


  //=====================================================================
  StatusCode NSWMMTP_RawDataProviderToolMT::convert() const 
  {
    ATH_MSG_ERROR(__PRETTY_FUNCTION__ << " not yet implemented!");
    return StatusCode::FAILURE;
  }

  StatusCode NSWMMTP_RawDataProviderToolMT::convert(const ROBFragmentList&) const 
  {
    ATH_MSG_ERROR(__PRETTY_FUNCTION__ << " not implemented!");
    return StatusCode::FAILURE;
  }

  StatusCode NSWMMTP_RawDataProviderToolMT::convert(const std::vector<IdentifierHash>&) const 
  {
    ATH_MSG_ERROR(__PRETTY_FUNCTION__ << " not yet implemented!");
    return StatusCode::FAILURE;
  }

  StatusCode NSWMMTP_RawDataProviderToolMT::convert(const ROBFragmentList&, const std::vector<IdentifierHash>&) const 
  {
    ATH_MSG_ERROR(__PRETTY_FUNCTION__ << " not implemented!");
    return StatusCode::FAILURE;
  }


  //=====================================================================
  StatusCode NSWMMTP_RawDataProviderToolMT::convert(const ROBFragmentList& fragments, const EventContext& ctx) const 
  {
    ATH_MSG_DEBUG(__PRETTY_FUNCTION__ << ": Got " << fragments.size() << " fragments");
    SG::WriteHandle<xAOD::NSWMMTPRDOContainer> rdoContainerHandle{m_rdoContainerKey, ctx};
    xAOD::NSWMMTPRDOContainer* pContainer{nullptr};

    // Retrieve container, if it exists in the event store; otherwise, create one
 
    ATH_CHECK(rdoContainerHandle.record(std::make_unique<xAOD::NSWMMTPRDOContainer>(), std::make_unique<xAOD::NSWMMTPRDOAuxContainer>()));
    pContainer = rdoContainerHandle.ptr();
    for (const auto *const fragment : fragments) {
      // NSW Common Decoder has already a try/catch inside and it's reporting errors/exceptions: wanna handle them here too?
      ATH_CHECK(m_decoder->fillCollection(*fragment, *pContainer));
    }

    return StatusCode::SUCCESS;
  }


  //=====================================================================
  StatusCode NSWMMTP_RawDataProviderToolMT::convert(const EventContext& ctx) const 
  {
    // Get all ROBs!
    std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*> fragments;
    std::vector<uint32_t> robIDs;

    // Generate all possible ROB IDs, aka Source Identifiers
    // Valid values are: 0x00{6b,6c}001[0..f]
    robIDs.reserve(32);
    for (uint32_t detectorID : {eformat::MUON_MMEGA_ENDCAP_A_SIDE, eformat::MUON_MMEGA_ENDCAP_C_SIDE}) {  // 0x6B, 0x6C
      for (uint8_t sector{}; sector < 16; sector++) {
	uint16_t moduleID = (0x1 << 4) | sector;
	eformat::helper::SourceIdentifier sourceID{static_cast<eformat::SubDetector>(detectorID), moduleID};
	robIDs.push_back(sourceID.simple_code());
      }
    }

    m_robDataProvider->getROBData(ctx, robIDs, fragments);
    return convert(fragments, ctx);
  }

}
