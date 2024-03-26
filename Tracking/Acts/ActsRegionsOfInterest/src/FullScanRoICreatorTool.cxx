/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "src/FullScanRoICreatorTool.h"

namespace ActsTrk {
  
FullScanRoICreatorTool::FullScanRoICreatorTool(const std::string& type,
					       const std::string& name,
					       const IInterface* parent)
  : base_class(type, name, parent) 
{}

StatusCode FullScanRoICreatorTool::defineRegionsOfInterest(const EventContext& /*ctx*/,
							   TrigRoiDescriptorCollection& collectionRoI) const
{
  // Add a Full Scan RoI
  collectionRoI.push_back( new TrigRoiDescriptor(true) );
  return StatusCode::SUCCESS;
}

}
