/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_DATAPREPARATION_PIXEL_CLUSTERING_TOOL_H
#define ACTSTRK_DATAPREPARATION_PIXEL_CLUSTERING_TOOL_H


#include "ActsToolInterfaces/IPixelClusteringTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetRawData/PixelRDORawData.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "InDetCondTools/ISiLorentzAngleTool.h"
#include "PixelConditionsData/PixelChargeCalibCondData.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "details/CellContainer.h"
#include "details/CellContainerProxy.h"
#include "details/InPlaceClusterization.h"
#include <string>


namespace ActsTrk {

class PixelClusteringTool : public extends<AthAlgTool,IPixelClusteringTool> {
public:
    PixelClusteringTool(const std::string& type,
			const std::string& name,
			const IInterface* parent);

    virtual StatusCode initialize() override;

    virtual std::pair<unsigned int, unsigned int>
    countCells(const RDOContainer& rdo_collection,
               const std::vector<IdentifierHash> &listOfIds,
               const InDetDD::SiDetectorElementCollection &detector_elements) const override;

   virtual StatusCode
   clusterize(const EventContext& ctx,
              const RawDataCollection& RDOs,
              const InDet::SiDetectorElementStatus& pixelDetElStatus,
              const InDetDD::SiDetectorElement& element,
              IPixelClusteringTool::CellContainer &cellContainer) const override;

    virtual std::any createEventDataCache(xAOD::PixelClusterContainer& cont,
                                          std::size_t nClusterRDOs) const override;

   virtual StatusCode
   makeClusters(const EventContext& ctx,
                const RDOContainer &rdo_container,
                const IPixelClusteringTool::CellContainer& cellContainer,
                unsigned int module_i,
                const InDetDD::SiDetectorElement& element,
                unsigned int icluster,
                xAOD::PixelClusterContainer& cont,
                std::any& vars) const override;
  
private:
   // Template to count cells i.e. RDOs in the rdo_collection
   // @tparam GANGED true if the pixel detector can contain ganged pixel otherwise false.
   // @param rdo_collection the pixel RDO collection.
   // @param listOfIds a list of id hashes to be considered or empty to consider all.
   // @param detector_elements the list of all detector elements of the pixel detector.
   // RDOs of ganged pixels will be counted as two.
   template <bool GANGED>
   std::pair<unsigned int, unsigned int>
   countCellsImpl(const RDOContainer& rdo_collection,
                  const std::vector<IdentifierHash> &listOfIds,
                  const InDetDD::SiDetectorElementCollection &detector_elements) const;

   using ClusterProxy = InPlaceClusterization::ClusterProxy<const IPixelClusteringTool::CellContainer>;
   using Cell = IPixelClusteringTool::CellContainer::Cell;

   std::span<IPixelClusteringTool::CellContainer::Cell>
   unpackRDOs(const RawDataCollection& RDOs,
              const InDet::SiDetectorElementStatus& pixelDetElStatus,
              const InDetDD::SiDetectorElement& element,
              IPixelClusteringTool::CellContainer &cellContainer) const;

   StatusCode makeCluster(size_t icluster,
                          const PixelClusteringTool::ClusterProxy &cluster,
                          const InDetDD::SiDetectorElement& element,
                          const InDetDD::PixelModuleDesign& design,
                          const InDetRawDataCollection<PixelRDORawData> &rdos,
                          const PixelChargeCalibCondData *calibData,
                          const PixelChargeCalibCondData::CalibrationStrategy calibStrategy,
                          const double lorentz_shift,
                          xAOD::PixelCluster::ClusterVars& clusterVars) const;

  ToolHandle< ISiLorentzAngleTool > m_pixelLorentzAngleTool {this, "PixelLorentzAngleTool", "", "Tool to retreive Lorentz angle of Pixel"};
  
  SG::ReadCondHandleKey<PixelChargeCalibCondData> m_chargeDataKey {this, "PixelChargeCalibCondData", "",
    "Pixel charge calibration data"};
  Gaudi::Property<std::string> m_idHelperName {this, "IDHelperName", "PixelID",
    "Pixel-like ID helper name to retrieve from DetectorStore"};
  
  Gaudi::Property<bool> m_addCorners {this, "AddCorners", true};
  Gaudi::Property<bool> m_useWeightedPos {this, "UseWeightedPosition", false};
  Gaudi::Property<bool> m_broadErrors {this, "UseBroadErrors", false};
  Gaudi::Property<bool> m_checkGanged {this, "CheckGanged", false};
  Gaudi::Property<bool> m_isITk {this, "isITk", true, "True if running in ITk"};
  const PixelID* m_pixelID {nullptr};
};
  
} // namespace ActsTrk 

#endif // ACTS_PIXEL_CLUSTERING_TOOL_H
