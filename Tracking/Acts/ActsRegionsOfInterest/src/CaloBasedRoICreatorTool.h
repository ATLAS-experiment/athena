
/* 
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALO_BASED_ROI_CREATOR_TOOL_H
#define CALO_BASED_ROI_CREATOR_TOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "ActsToolInterfaces/IRoICreatorTool.h"
#include "TrkCaloClusterROI/ROIPhiRZContainer.h"
#include "BeamSpotConditionsData/BeamSpotData.h"

namespace ActsTrk {

class
CaloBasedRoICreatorTool
  : public extends<AthAlgTool, ActsTrk::IRoICreatorTool> {
 public:
  CaloBasedRoICreatorTool(const std::string& type,
			  const std::string& name,
			  const IInterface* parent);
  virtual ~CaloBasedRoICreatorTool() = default;

  virtual 
    StatusCode initialize() override;

   virtual
   StatusCode defineRegionsOfInterest(const EventContext& ctx,
				      TrigRoiDescriptorCollection& collectionRoI) const override;
  
private:
  SG::ReadHandleKey< ROIPhiRZContainer > m_caloClusterROIKey
    {this, "CaloClusterRoIContainer", "",
	"Name of the calo cluster ROIs in Phi,R,Z parameterization"};
  
  SG::ReadCondHandleKey< InDet::BeamSpotData > m_beamSpotKey
    {this, "BeamSpotKey", "BeamSpotData",
	"SG key for beam spot"};

  Gaudi::Property< double > m_deltaEta
    {this, "DeltaEtaCaloRoI", .1};
  Gaudi::Property< double > m_deltaPhi
    {this, "DeltaPhiCaloRoI", .25};
  Gaudi::Property< double > m_deltaZ
    {this, "DeltaZCaloRoI", 300.};
};

}

#endif
