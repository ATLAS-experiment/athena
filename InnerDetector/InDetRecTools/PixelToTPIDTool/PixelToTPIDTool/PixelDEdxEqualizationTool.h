#ifndef PIXELTOTPIDTOOL_PIXELDEDXEQUALIZATIONTOOL_H
#define PIXELTOTPIDTOOL_PIXELDEDXEQUALIZATIONTOOL_H

#include "TrkAnalysisInterfaces/IPixelDEdxEqualizationTool.h"

#include "PixelToTPIDTool/PixelDEdxUtils.h"

#include "AsgTools/AsgTool.h"
#include "AsgTools/PropertyWrapper.h"
#include "AsgDataHandles/ReadHandle.h"
#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/WriteDecorHandle.h"
#include "AsgDataHandles/WriteDecorHandleKey.h"
#include "PathResolver/PathResolver.h"

#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackStateValidation.h"
#include "xAODTracking/TrackStateValidationContainer.h"
#include "xAODTracking/TrackMeasurementValidation.h"
#include "xAODTracking/TrackMeasurementValidationContainer.h"

// ROOT
#include "TFile.h"
#include <ROOT/RDataFrame.hxx>

// C++
#include <cmath>
#include <memory>
#include <mutex>

namespace CP {

  /// Implementation of the Pixel ToT PID tool.
  /// This is refactoring of the tool for dual use in Athena and AnalysisBase using CP Algs.

  class PixelDEdxEqualizationTool : public virtual IPixelDEdxEqualizationTool, public asg::AsgTool {
    /// Create a proper constructor for Athena
    ASG_TOOL_CLASS(PixelDEdxEqualizationTool, CP::IPixelDEdxEqualizationTool)  // depends where I put the interface...


    
  public:
    PixelDEdxEqualizationTool(const std::string& tool_name="PixelDEdxEqualizationTool");
    
    virtual ~PixelDEdxEqualizationTool();
    
    /// @name Function(s) implementing the asg::IAsgTool interface
    /// @{

    /// Function initialising the tool
    virtual StatusCode initialize() override;

    /// @}

    /// @name Function(s) implementing the IPixelDEdxEqualizationTool interface
    /// @{

    /// AnalysisBase with xAOD EDM
    virtual float dEdx(const xAOD::TrackParticle& track,
                       int& nUsedHits,
                       int& nUsedIBLOverflowHits) const;

  private:
    
    /// Common to both EDMs ///

    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo{this, "EventInfoContName", "EventInfo", "event info key"};

    /// Equalize the cluster-level dE/dx measuremented before the taking the truncated mean.
    /// For ESD EDM, always have access to pixel clusters.
    /// For xAOD EDM, requires special datasets with pixel clusters.
    Gaudi::Property<bool> m_equalizeClusterMeasurements
    { this, "EqualizeClusterMeasurements", false, "Equalize cluster dE/dx before truncated mean"};

    /// Apply tight cluster cleaning requirements (e.g. cluster size/shape cuts).
    Gaudi::Property<bool> m_tightClusterCleaning
    { this, "TightClusterCleaning", false, ""};

    StatusCode initSFsFromTrees();

    /// Equalize the track-level truncated mean instead of the individual cluster measurements.
    /// Not as good as pixel-level equalization, but does not special datasets with clusters.
    /// Nominal AOD does not have pixel clusters.
    Gaudi::Property<bool> m_equalizeTrackMeasurements
    { this, "EqualizeTrackMeasurements", false, "Equalize track-level truncated mean dE/dx"};

    Gaudi::Property<std::string> m_msosLink
    { this, "MSOSLink", "Reco_msosLink"};

    /// PathResolverFindCalibFile need the logical filename in ASG calibration area.
    Gaudi::Property<std::string> m_sfFileName { this, "SFFileName", "pixeldEdxEqualizationSFs_v0.root"}; // FIX! TBD
    /// Override version in ASG calibration area with a local file is not empty string.
    Gaudi::Property<std::string> m_sfLocalFileName {this, "SFLocalFileName", ""};
    /// Name of SF tree.
    Gaudi::Property<std::string> m_clusterSFTreeName { this, "ClusterSFTreeName", "cluster_SFs"}; // FIX! TBD
    Gaudi::Property<std::string> m_trackSFTreeName { this, "TrackSFTreeName", "track_SFs"}; // FIX! TBD

    /// dE/dx equalization scale factor dataframe read from trees.
    std::shared_ptr<ROOT::RDataFrame> m_df;
    std::shared_ptr<TFile> m_file;  // Keep the file open

    /// Map where key = run number, value is a filtered scale factor RDF (an RDF::RNode) with only the rows for that run number.
    /// So not filtering everytime in execute().
    /// Will be updated in execute, so must be mutable
    mutable std::map<unsigned int, std::shared_ptr<ROOT::RDF::RNode>> m_filteredRDFMap;
    mutable std::mutex m_mapMutex;

    /// Decorators for xAOD EDM
    /// Only one PixelClusters container shared by all track containers, so should not need to modify keys...
    /// Raw cluster dE/dx
    SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_clusterdEdxKey
      {this, "clusterdEdxKey", "PixelClusters.dEdx", "SG key for the raw pixel cluster dE/dx attribute"};
    /// Equalized cluster dE/dx:
    SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_clusterdEdxEqKey
      {this, "clusterdEdxEqKey", "PixelClusters.dEdxEq", "SG key for the equalized pixel cluster dE/dx attribute"};

  }; // class PixelDEdxEqualizationTool


} // namespace CP

#endif  // PIXELDEDXEQUALIZATIONTOOL_H
