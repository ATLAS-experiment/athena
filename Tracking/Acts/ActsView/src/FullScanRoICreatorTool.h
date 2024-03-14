/* 
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FULL_SCAN_ROI_CREATOR_TOOL_H
#define FULL_SCAN_ROI_CREATOR_TOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "ActsView/IRoICreatorTool.h"

class FullScanRoICreatorTool : public extends<AthAlgTool, ::IRoICreatorTool> {
 public:
  FullScanRoICreatorTool(const std::string& type,
			 const std::string& name,
			 const IInterface* parent);
  virtual ~FullScanRoICreatorTool() = default;

  virtual 
    StatusCode initialize() override;

   virtual
     StatusCode defineRegionsOfInterest(const EventContext& ctx,
					std::vector< ElementLink< TrigRoiDescriptorCollection > >& ELs) const override;
   
 private:
  SG::WriteHandleKey< TrigRoiDescriptorCollection > m_roiCollectionKey {this, "RoIs", ""};
};

#endif
