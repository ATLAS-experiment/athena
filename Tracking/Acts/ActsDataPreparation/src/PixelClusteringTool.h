/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_DATAPREPARATION_PIXEL_CLUSTERING_TOOL_H
#define ACTSTRK_DATAPREPARATION_PIXEL_CLUSTERING_TOOL_H


#include "ActsToolInterfaces/IPixelClusteringTool.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetRawData/InDetRawDataCollection.h"
#include "InDetRawData/PixelRDORawData.h"
#include "SiClusterizationTool/PixelRDOTool.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "InDetCondTools/ISiLorentzAngleTool.h"
#include "PixelReadoutGeometry/IPixelReadoutManager.h"
#include "PixelConditionsData/PixelChargeCalibCondData.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"


namespace ActsTrk {

class PixelClusteringTool : public extends<AthAlgTool,IPixelClusteringTool> {
public:

    using Cell = InDet::UnpackedPixelRDO;
    using CellCollection = std::vector<Cell>;

    struct Cluster {
        std::vector<Identifier::value_type> ids;
	std::vector<int> tots;
	int lvl1min = std::numeric_limits<int>::max();
    };

    using ClusterCollection = std::vector<Cluster>;

    PixelClusteringTool(const std::string& type,
			const std::string& name,
			const IInterface* parent);

    virtual StatusCode
    clusterize(const InDetRawDataCollection<PixelRDORawData>& RDOs,
	       const PixelID& pixelID,
	       const EventContext& ctx,
	       xAOD::PixelClusterContainer& container) const override;

    virtual StatusCode initialize() override;

private:
    // N.B. the cluster is added to the container
    // and the tots and charges vectors will be moved to the xAOD object
  
  StatusCode makeCluster(const EventContext& ctx,
			 PixelClusteringTool::Cluster &cluster,
			 const PixelID& pixelID,
			 const InDetDD::SiDetectorElement* element,
			 const InDetDD::PixelModuleDesign& design,
			 const PixelChargeCalibCondData *calibData,
			 const PixelChargeCalibCondData::CalibrationStrategy calibStrategy,
			 xAOD::PixelCluster& container) const;

private:  
  ServiceHandle< InDetDD::IPixelReadoutManager > m_pixelReadout {this, "PixelReadoutManager", "ITkPixelReadoutManager",
      "Pixel readout manager" };
  
  ToolHandle< InDet::PixelRDOTool > m_pixelRDOTool {this, "PixelRDOTool", "", "The Pixel RDO tool"};
  ToolHandle< ISiLorentzAngleTool > m_pixelLorentzAngleTool {this, "PixelLorentzAngleTool", "", "Tool to retreive Lorentz angle of Pixel"};
  
  SG::ReadCondHandleKey<PixelChargeCalibCondData> m_chargeDataKey {this, "PixelChargeCalibCondData", "ITkPixelChargeCalibCondData",
    "Pixel charge calibration data"};
  
  Gaudi::Property<bool> m_addCorners {this, "AddCorners", true};
  Gaudi::Property<bool> m_useWeightedPos {this, "UseWeightedPosition", false};
  Gaudi::Property<bool> m_broadErrors {this, "UseBroadErrors", false};
};
  
} // namespace ActsTrk 

#endif // ACTS_PIXEL_CLUSTERING_TOOL_H
