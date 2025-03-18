/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "HLTCaloCellSumMaker.h"


StatusCode HLTCaloCellSumMaker::initialize() {
  ATH_CHECK( m_cellContainerKey.initialize( m_roiMode ) );
  ATH_CHECK( m_cellContainerVKey.initialize( !m_roiMode ) );
  return StatusCode::SUCCESS;
}


StatusCode HLTCaloCellSumMaker::execute( const EventContext& context ) const {

  if ( m_roiMode ) {
    auto roiCollection = SG::makeHandle( m_cellContainerKey, context );
    if ( !roiCollection.isValid() ){
      ATH_MSG_INFO( "empty container");
      return StatusCode::SUCCESS;
    }
    float sum=0.;
    for(auto c : *roiCollection ) {sum+=c->et();}
    ATH_MSG_DEBUG ( "REGTEST: Executing " << name() << "... size : " << roiCollection->size() << "; Energy Et Sum : " << sum );

  } else {
    auto roiCollection= SG::makeHandle( m_cellContainerVKey, context );
    for(const CaloCellContainer* roiDescriptor : *roiCollection ) {
      if ( !roiDescriptor->empty() ) {
        float sum=0.;
        for(auto c : *roiDescriptor ) {sum+=c->et();}
        ATH_MSG_DEBUG("REGTEST: Executing " << name() << "; size : " << roiDescriptor->size() << "; Energy ET Sum : " << sum );
      } else { ATH_MSG_INFO( "empty container"); }
    }
  }
  return StatusCode::SUCCESS;
}
