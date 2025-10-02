/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_DATAPREPARATION_PIXEL_CLUSTERING_TOOL_H
#define ACTSTRK_DATAPREPARATION_PIXEL_CLUSTERING_TOOL_H


#include "ActsToolInterfaces/IPixelClusteringTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetRawData/PixelRDORawData.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "InDetCondTools/ISiLorentzAngleTool.h"
#include "PixelReadoutGeometry/IPixelReadoutManager.h"
#include "PixelConditionsData/PixelChargeCalibCondData.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"


namespace ActsTrk {

class PixelClusteringTool : public extends<AthAlgTool,IPixelClusteringTool> {
public:
    PixelClusteringTool(const std::string& type,
			const std::string& name,
			const IInterface* parent);

    virtual StatusCode
    clusterize(const EventContext& ctx,
               const RawDataCollection& RDOs,
               const InDet::SiDetectorElementStatus& pixelDetElStatus,
               const InDetDD::SiDetectorElement& element,
               Acts::Ccl::ClusteringData& data,
               std::vector<ClusterCollection>& collection) const override;
  
    virtual StatusCode
    makeClusters(const EventContext& ctx,
                 typename IPixelClusteringTool::ClusterCollection& clusters,
                 const InDetDD::SiDetectorElement& element,
		 typename ClusterContainer::iterator itrContainer) const override;
  
    virtual StatusCode initialize() override;
  
private:
    // N.B. the cluster is added to the container
    // and the tots and charges vectors will be moved to the xAOD object
  
  StatusCode makeCluster(const EventContext& ctx,
			 PixelClusteringTool::Cluster &cluster,
			 const InDetDD::SiDetectorElement* element,
			 const InDetDD::PixelModuleDesign& design,
			 const PixelChargeCalibCondData *calibData,
			 const PixelChargeCalibCondData::CalibrationStrategy calibStrategy,
			 xAOD::PixelCluster& container) const;

  typename IPixelClusteringTool::CellCollection
  unpackRDOs(const RawDataCollection& RDOs,
	     const InDet::SiDetectorElementStatus& stripDetElStatus,
	     const InDetDD::SiDetectorElement& element) const;

  static inline
  std::optional<Identifier>
  isGanged(const Identifier& rdoID,
	   const InDetDD::SiDetectorElement& element);

private:  
  ServiceHandle< InDetDD::IPixelReadoutManager > m_pixelReadout {this, "PixelReadoutManager", "InDetDD::ITk::PixelReadoutManager",
      "Pixel readout manager" };
  
  ToolHandle< ISiLorentzAngleTool > m_pixelLorentzAngleTool {this, "PixelLorentzAngleTool", "", "Tool to retreive Lorentz angle of Pixel"};
  
  SG::ReadCondHandleKey<PixelChargeCalibCondData> m_chargeDataKey {this, "PixelChargeCalibCondData", "",
    "Pixel charge calibration data"};
  
  Gaudi::Property<bool> m_addCorners {this, "AddCorners", true};
  Gaudi::Property<bool> m_useWeightedPos {this, "UseWeightedPosition", false};
  Gaudi::Property<bool> m_broadErrors {this, "UseBroadErrors", false};
  Gaudi::Property<bool> m_checkGanged {this, "CheckGanged", false};
  Gaudi::Property<bool> m_isITk {this, "isITk", true, "True if running in ITk"};
  const PixelID* m_pixelID {nullptr};
};
  
} // namespace ActsTrk 

#endif // ACTS_PIXEL_CLUSTERING_TOOL_H
