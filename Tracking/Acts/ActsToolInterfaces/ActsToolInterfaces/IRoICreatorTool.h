/* 
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IROI_CREATOR_TOOL_H
#define IROI_CREATOR_TOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "AthLinks/ElementLink.h"
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"

#include <vector>

namespace ActsTrk {
  
class IRoICreatorTool 
: virtual public IAlgTool {
 public:
  DeclareInterfaceID(IRoICreatorTool, 1, 0);

  virtual
    StatusCode
    defineRegionsOfInterest(const EventContext& ctx,
			    TrigRoiDescriptorCollection& collectionRoI) const = 0;
};

}
#endif
