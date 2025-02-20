/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "EndOfEventROIConfirmerAlg.h"


EndOfEventROIConfirmerAlg::EndOfEventROIConfirmerAlg(const std::string& name, ISvcLocator* pSvcLocator)
    : AthReentrantAlgorithm(name, pSvcLocator) {}


StatusCode EndOfEventROIConfirmerAlg::initialize() {
  ATH_MSG_DEBUG( "EndOfEventROIConfirmerAlg::initialize()" );
  ATH_CHECK( m_writeHandleKeyArray_ROIs.initialize() );
  return StatusCode::SUCCESS;
}


StatusCode EndOfEventROIConfirmerAlg::execute(const EventContext& context) const {
  ATH_MSG_DEBUG( "EndOfEventROIConfirmerAlg::execute()" );

  SG::ReadHandleKey<TrigRoiDescriptorCollection> rhk("temp");
  ATH_CHECK( rhk.initialize() );

  for (const auto& whk : m_writeHandleKeyArray_ROIs) {
    rhk = whk.key();  // update the key
    auto readHandle = SG::makeHandle(rhk, context);
    if ( readHandle.isValid() ) {
      ATH_MSG_DEBUG( "The " << whk.key() << " already present - this chain must have run as part of the trigger in this event" );
    } else {
      ATH_MSG_DEBUG( "The " << whk.key() << " is not here - we should create it such that we can run algorithms at the end of the HLT-accepted event" );
    
      auto handle = SG::makeHandle(whk, context);
      auto objp = std::make_unique<TrigRoiDescriptorCollection> (TrigRoiDescriptorCollection());
      ATH_CHECK( handle.record (std::move (objp)) );
      handle->push_back(new TrigRoiDescriptor(true));
    }
  }
  return StatusCode::SUCCESS;
}
