/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "src/RegionsOfInterestCreatorAlg.h"

namespace ActsTrk {

RegionsOfInterestCreatorAlg::RegionsOfInterestCreatorAlg(const std::string& name, 
							 ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator)
{}

StatusCode RegionsOfInterestCreatorAlg::initialize() 
{
  ATH_MSG_DEBUG("Initialising " << name() << " ...");

  ATH_CHECK(m_roiCollectionKey.initialize());
  ATH_CHECK(m_roiTool.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode RegionsOfInterestCreatorAlg::execute(const EventContext& ctx) const
{
  ATH_MSG_DEBUG("Executing " << name() << " ...");

  ATH_MSG_DEBUG("Creating ROI with key " << m_roiCollectionKey.key());
  SG::WriteHandle< TrigRoiDescriptorCollection > roiCollectionHandle = SG::makeHandle( m_roiCollectionKey, ctx );
  ATH_CHECK( roiCollectionHandle.record( std::make_unique< TrigRoiDescriptorCollection >() ) );
  TrigRoiDescriptorCollection *collectionRoI = roiCollectionHandle.ptr();
  ATH_CHECK(collectionRoI != nullptr);
  
  ATH_CHECK( m_roiTool->defineRegionsOfInterest(ctx, *collectionRoI) );
  return StatusCode::SUCCESS;
}

}
