/* 
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TEST_ROI_CREATOR_TOOL_H
#define TEST_ROI_CREATOR_TOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "ActsToolInterfaces/IRoICreatorTool.h"

namespace ActsTrk {
  
class TestRoICreatorTool
: public extends<AthAlgTool, ActsTrk::IRoICreatorTool> {
 public:
  TestRoICreatorTool(const std::string& type,
		     const std::string& name,
		     const IInterface* parent);
  virtual ~TestRoICreatorTool() = default;

  virtual 
    StatusCode initialize() override;

   virtual
     StatusCode defineRegionsOfInterest(const EventContext& ctx,
					TrigRoiDescriptorCollection& collectionRoI) const override;
   
 private:
   Gaudi::Property< std::vector<double> > m_eta_center_rois {this, "EtaCenters", {}, "Center of the RoI - eta coordinate"};
   Gaudi::Property< std::vector<double> > m_phi_center_rois {this, "PhiCenters", {}, "Center of the RoI - phi coordinate"};
   Gaudi::Property< std::vector<double> > m_z_center_rois {this, "ZCenters", {}, "Center of the RoI - z coordinate"};

   Gaudi::Property< std::vector<double> > m_half_eta_width_rois {this, "HalfEtaWidths", {}, "Half width of the RoI - eta coordinate"};
   Gaudi::Property< std::vector<double> > m_half_phi_width_rois {this, "HalfPhiWidths", {}, "Half width of the RoI - phi coordinate"};
   Gaudi::Property< std::vector<double> > m_half_z_width_rois {this, "HalfZWidths", {}, "Half width of the RoI - z coordinate"};
};

}
#endif
