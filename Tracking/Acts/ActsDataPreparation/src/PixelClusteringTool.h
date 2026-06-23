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
#include "details/PixelRDOContainerTraits.h"
#include <string>


namespace ActsTrk {

template <typename T_RDOContainer>
class PixelClusteringToolImpl : public extends<AthAlgTool,typename ActsTrk::RDOContainerTraits<T_RDOContainer>::IClusteringToolType> {
public:
    using IClusteringToolType = typename ActsTrk::RDOContainerTraits<T_RDOContainer>::IClusteringToolType;
   using base_class =extends<AthAlgTool,typename ActsTrk::RDOContainerTraits<T_RDOContainer>::IClusteringToolType>::base_class;
    PixelClusteringToolImpl(const std::string& type,
                            const std::string& name,
                            const IInterface* parent);

    virtual StatusCode initialize() override;

    virtual std::pair<unsigned int, unsigned int>
    countCells(const T_RDOContainer& rdo_collection,
               const std::vector<IdentifierHash> &listOfIds,
               const InDetDD::SiDetectorElementCollection &detector_elements) const override;

   virtual StatusCode
   clusterize(const EventContext& ctx,
              const ActsTrk::RDOContainerTraits<T_RDOContainer>::PerModuleRDOs &RDOs,
              const InDet::SiDetectorElementStatus& pixelDetElStatus,
              const InDetDD::SiDetectorElement& element,
              typename IClusteringToolType::CellContainer &cellContainer) const override;

    virtual std::any createEventDataCache(xAOD::PixelClusterContainer& cont,
                                          std::size_t nClusterRDOs) const override;

   virtual StatusCode
   makeClusters(const EventContext& ctx,
                const T_RDOContainer &rdo_container,
                const typename IClusteringToolType::CellContainer& cellContainer,
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
   countCellsImpl(const T_RDOContainer& rdo_collection,
                  const std::vector<IdentifierHash> &listOfIds,
                  const InDetDD::SiDetectorElementCollection &detector_elements) const;

   using ClusterProxy = InPlaceClusterization::ClusterProxy<const typename IClusteringToolType::CellContainer>;
   using Cell = typename IClusteringToolType::CellContainer::Cell;

   std::span<typename IClusteringToolType::CellContainer::Cell>
   unpackRDOs(const ActsTrk::RDOContainerTraits<T_RDOContainer>::PerModuleRDOs &RDOs,
              const InDet::SiDetectorElementStatus& pixelDetElStatus,
              const InDetDD::SiDetectorElement& element,
              typename IClusteringToolType::CellContainer &cellContainer) const;

   StatusCode makeCluster(size_t icluster,
                          const PixelClusteringToolImpl::ClusterProxy &cluster,
                          const InDetDD::SiDetectorElement& element,
                          const InDetDD::PixelModuleDesign& design,
                          const ActsTrk::RDOContainerTraits<T_RDOContainer>::PerModuleRDOs &RDOs,
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

// to have simple component names
class PixelClusteringTool : public PixelClusteringToolImpl<PixelRDO_Container> {
public:
   using PixelClusteringToolImpl<PixelRDO_Container>::PixelClusteringToolImpl;
};
// to have simple component names
class PhaseIIPixelClusteringTool : public PixelClusteringToolImpl<PhaseIIPixelRawDataContainer> {
public:
   using PixelClusteringToolImpl<PhaseIIPixelRawDataContainer>::PixelClusteringToolImpl;
};
   
} // namespace ActsTrk 

#endif // ACTS_PIXEL_CLUSTERING_TOOL_H
