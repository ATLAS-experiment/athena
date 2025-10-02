/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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

namespace ActsTrk {


class StripClusteringTool : public extends<AthAlgTool, IStripClusteringTool> {
public:
    using StripRDORawData = SCT_RDORawData;
    using StripID = SCT_ID;

    StripClusteringTool(const std::string& type,
			const std::string& name,
			const IInterface* parent);

    virtual StatusCode initialize() override;

    virtual StatusCode
    clusterize(const EventContext& ctx,
	       const InDetRawDataCollection<StripRDORawData>& RDOs,
	       const InDet::SiDetectorElementStatus& stripDetElStatus,
	       const InDetDD::SiDetectorElement& element,
         Acts::Ccl::ClusteringData& data,
	       std::vector<typename IStripClusteringTool::ClusterCollection>& collection) const override;

    virtual StatusCode
    makeClusters(const EventContext& ctx,
		 typename IStripClusteringTool::ClusterCollection& cluster,
		 const InDetDD::SiDetectorElement& element,
		 typename ClusterContainer::iterator itrContainer) const override;
      
private:
    std::optional<std::pair<typename IStripClusteringTool::CellCollection, bool>>
    unpackRDOs(const EventContext& ctx,
	       const RawDataCollection& RDOs,
	       const InDet::SiDetectorElementStatus& stripDetElStatus,
	       const InDetDD::SiDetectorElement& element) const;
  
    bool passTiming(const std::bitset<3>& timePattern) const;
    
    StatusCode decodeTimeBins();

    bool isBadStrip(const EventContext& ctx,
        const InDet::SiDetectorElementStatus *sctDetElStatus,
		    const StripID& idHelper,
		    IdentifierHash waferHash,
		    Identifier stripId) const;

    // N.B. the cluster is added to the container
    StatusCode makeCluster(StripClusteringTool::Cluster &cluster,
			   double LorentzShift,
			   Eigen::Matrix<float,1,1>& localCov,
			   const StripID& stripID,
			   const InDetDD::SiDetectorElement& element,
			   const InDetDD::SiDetectorDesign& design,
			   xAOD::StripCluster& container) const;

    StringProperty m_timeBinStr{this, "timeBins", ""};

    ToolHandle<ISiLorentzAngleTool> m_lorentzAngleTool {this, "LorentzAngleTool", "",
      "Tool to retreive Lorentz angle of Si detector module"
    };

    // TODO this one should be removed?
    SG::ReadHandleKey<InDet::SiDetectorElementStatus> m_stripDetElStatus {this, "StripDetElStatus", "",
      "SiDetectorElementStatus for strip"};

    ToolHandle<IInDetConditionsTool> m_conditionsTool {this, "conditionsTool", "",
      "Conditions summary tool"};

    Gaudi::Property<bool> m_checkBadModules {this, "checkBadModules", true,
      "Check bad modules using the conditions summary tool"};

    Gaudi::Property<unsigned int> m_maxFiredStrips {this, "maxFiredStrips", 384u,
      "Threshold of number of fired strips per wafer. 0 disables the per-wafer cut."};

    SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_stripDetEleCollKey {this, "StripDetEleCollKey", "ITkStripDetectorElementCollection",
      "SiDetectorElementCollection key for strip"};

    Gaudi::Property<bool> m_isITk {this, "isITk", true,
      "True if running in ITk"};

    int m_timeBinBits[3]{-1, -1, -1};


  const StripID* m_stripID {nullptr};
};

} // namespace ActsTrk

#endif
