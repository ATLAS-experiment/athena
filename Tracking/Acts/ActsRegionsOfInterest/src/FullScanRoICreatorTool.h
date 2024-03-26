/* 
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FULL_SCAN_ROI_CREATOR_TOOL_H
#define FULL_SCAN_ROI_CREATOR_TOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "ActsToolInterfaces/IRoICreatorTool.h"

namespace ActsTrk {

class FullScanRoICreatorTool : public extends<AthAlgTool, ActsTrk::IRoICreatorTool> {
 public:
  FullScanRoICreatorTool(const std::string& type,
			 const std::string& name,
			 const IInterface* parent);
  virtual ~FullScanRoICreatorTool() = default;

   virtual
     StatusCode defineRegionsOfInterest(const EventContext& ctx,
					TrigRoiDescriptorCollection& collectionRoI) const override;
};

}
#endif
