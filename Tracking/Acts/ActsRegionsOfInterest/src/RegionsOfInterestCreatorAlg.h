/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ROI_CREATOR_ALG_H
#define ROI_CREATOR_ALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "ActsToolInterfaces/IRoICreatorTool.h"
#include "TrigSteeringEvent/TrigRoiDescriptorCollection.h"
#include "StoreGate/WriteHandleKey.h"

namespace ActsTrk {
  
class RegionsOfInterestCreatorAlg
  : public AthReentrantAlgorithm {
 public:
  RegionsOfInterestCreatorAlg(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~RegionsOfInterestCreatorAlg() = default;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

 private:
  ToolHandle< ActsTrk::IRoICreatorTool > m_roiTool {this, "RoICreatorTool", "",
      "Tool for creating RoIs"};
  
  SG::WriteHandleKey< TrigRoiDescriptorCollection > m_roiCollectionKey {this, "RoIs", "",
    "The created RoI"};
};

}
#endif
