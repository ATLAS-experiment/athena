/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_DATAPREPARATION_STRIP_CLUSTERING_TOOL_H
#define ACTSTRK_DATAPREPARATION_STRIP_CLUSTERING_TOOL_H

#include <optional>
#include <vector>

#include <Acts/Clusterization/Clusterization.hpp>

#include <AthenaBaseComps/AthAlgTool.h>
#include <ActsToolInterfaces/IStripClusteringTool.h>
#include <InDetConditionsSummaryService/IInDetConditionsTool.h>
#include <InDetIdentifier/SCT_ID.h>
#include <InDetRawData/InDetRawDataCollection.h>
#include <InDetRawData/SCT_RDORawData.h>
#include <InDetReadoutGeometry/SiDetectorElement.h>
#include <InDetReadoutGeometry/SiDetectorElementStatus.h>
#include <xAODInDetMeasurement/StripClusterContainer.h>
#include "details/CellContainer.h"
#include "details/CellContainerProxy.h"
#include "details/InPlaceClusterization.h"

namespace ActsTrk {
struct StripAuxDataCache;

class StripClusteringTool : public extends<AthAlgTool, IStripClusteringTool> {
public:
    using StripRDORawData = SCT_RDORawData;
    using StripID = SCT_ID;

    StripClusteringTool(const std::string& type,
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
               const InDet::SiDetectorElementStatus& stripDetElStatus,
               const InDetDD::SiDetectorElement& element,
               IStripClusteringTool::CellContainer &cellContainer) const override;

    virtual std::any createEventDataCache(xAOD::StripClusterContainer& cont,
                                          std::size_t nClusterRDOs) const override;

    virtual StatusCode
    makeClusters(const EventContext& ctx,
                 const RDOContainer &rdo_container,
                 const IStripClusteringTool::CellContainer& cellContainer,
                 unsigned int module_i,
                 const InDetDD::SiDetectorElement& element,
                 unsigned int icluster,
                 xAOD::StripClusterContainer& cont,
                 std::any& vars) const override;
      
private:
    using ClusterProxy = InPlaceClusterization::ClusterProxy<const IStripClusteringTool::CellContainer>;
    using Cell = IStripClusteringTool::CellContainer::Cell;

    std::span<IStripClusteringTool::CellContainer::Cell>
    unpackRDOs(const RawDataCollection& RDOs,
	       const InDet::SiDetectorElementStatus& stripDetElStatus,
	       const InDetDD::SiDetectorElement& element,
               IStripClusteringTool::CellContainer &cellContainer) const;
  
    bool passTiming(const std::bitset<3>& timePattern) const;
    
    StatusCode decodeTimeBins();

    static bool isBadStrip(const InDet::SiDetectorElementStatus *sctDetElStatus,
                           IdentifierHash waferHash,
                           std::int16_t strip);

   StatusCode makeCluster(size_t icluster,
                          xAOD::StripCluster& cl,
                          const ClusterProxy &cluster_proxy,
                          const InDetDD::SiDetectorElement& element,
                          const InDetDD::SiDetectorDesign& design,
                          const double lorentzShift,
                          Eigen::Matrix<float,1,1>& localCov,
                          StripAuxDataCache &auxDataCache) const;

    StringProperty m_timeBinStr{this, "timeBins", ""};

    ToolHandle<ISiLorentzAngleTool> m_lorentzAngleTool {this, "LorentzAngleTool", "",
      "Tool to retreive Lorentz angle of Si detector module"
    };

    // TODO this one should be removed?
    SG::ReadHandleKey<InDet::SiDetectorElementStatus> m_stripDetElStatus {this, "StripDetElStatus", "",
      "SiDetectorElementStatus for strip"};

    Gaudi::Property<bool> m_checkBadModules {this, "checkBadModules", true,
      "Check bad modules using the conditions summary tool"};

    Gaudi::Property<unsigned int> m_maxFiredStrips {this, "maxFiredStrips", 384u,
      "Threshold of number of fired strips per wafer. 0 disables the per-wafer cut."};

    SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_stripDetEleCollKey {this, "StripDetEleCollKey", "ITkStripDetectorElementCollection",
      "SiDetectorElementCollection key for strip"};

    Gaudi::Property<bool> m_isITk {this, "isITk", true,
      "True if running in ITk"};

    Gaudi::Property<unsigned int> m_errorStrategy{this, "errorStrategy", 0, "Use different error strategies for the strip clusters"};

    int m_timeBinBits[3]{-1, -1, -1};


  const StripID* m_stripID {nullptr};
};

} // namespace ActsTrk

#endif
