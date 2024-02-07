/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_DATAPREPARATION_PIXEL_CLUSTERING_TOOL_H
#define ACTSTRK_DATAPREPARATION_PIXEL_CLUSTERING_TOOL_H


#include "ActsToolInterfaces/IPixelClusteringTool.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetRawData/InDetRawDataCollection.h"
#include "InDetRawData/PixelRDORawData.h"
#include "SiClusterizationTool/ClusterMakerTool.h"
#include "SiClusterizationTool/PixelRDOTool.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"

namespace InDet {

// Helper functions for use with ACTS clusterization
// Put these in the InDet namespace so that ACTS can find them
// via ADL.
//
inline int getCellRow(const InDet::UnpackedPixelRDO& cell)
{
    return cell.ROW;
}
    
inline int getCellColumn(const InDet::UnpackedPixelRDO& cell)
{
    return cell.COL;
}

inline int& getCellLabel(InDet::UnpackedPixelRDO& cell)
{
    return cell.NCL;
}


} // namespace InDet


namespace ActsTrk {


class PixelClusteringTool : public extends<AthAlgTool,IPixelClusteringTool> {
public:

    using Cell = InDet::UnpackedPixelRDO;
    using CellCollection = std::vector<Cell>;




    struct Cluster {
	std::vector<Identifier> ids;
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
    StatusCode makeCluster(const PixelClusteringTool::Cluster &cluster,
			   const PixelID& pixelID,
			   const InDetDD::SiDetectorElement* element,
			   xAOD::PixelCluster& container) const;


    ToolHandle< InDet::PixelRDOTool > m_pixelRDOTool
      {this, "PixelRDOTool", "InDet::PixelRDOTool"};
    ToolHandle< InDet::ClusterMakerTool > m_clusterMakerTool
      {this, "ClusterMakerTool", "InDet::ClusterMakerTool"};

    Gaudi::Property<bool> m_addCorners{this, "AddCorners", true};
    Gaudi::Property<int> m_errorStrategy{this, "ErrorStrategy", 1};

    SG::ReadCondHandleKey<PixelChargeCalibCondData> m_chargeDataKey
      {this, "PixelChargeCalibCondData", "ITkPixelChargeCalibCondData",
       "Pixel charge calibration data"};

    SG::ReadCondHandleKey<PixelCalib::PixelOfflineCalibData> m_offlineCalibDataKey
      {this, "PixelOfflineCalibData", "ITkPixelOfflineCalibData",
       "Pixel offline calibration data"};
};

} // namespace ActsTrk 

#endif // ACTS_PIXEL_CLUSTERING_TOOL_H
