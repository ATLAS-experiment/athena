//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//

#ifndef TRACKINGANALYSISALGORITHMS_PIXELDEDXEQUALIZATIONTOOL_H
#define TRACKINGANALYSISALGORITHMS_PIXELDEDXEQUALIZATIONTOOL_H

#include "TrkAnalysisInterfaces/IPixelDEdxEqualizationTool.h"

#include "TrackingAnalysisAlgorithms/PixelDEdxUtils.h"


#include "AsgTools/AsgTool.h"
#include "AsgTools/PropertyWrapper.h"
#include "PathResolver/PathResolver.h"

#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"
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
#include <shared_mutex>

namespace CP {

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

    virtual double getTrackdEdxSF(const xAOD::TrackParticle& track, const int runNumber) const override;
    virtual double getClusterdEdxSF(const PixelDEdx::PixelClusterStruct& cluster, const int runNumber) const override;

  private:
    
    const std::vector<TrackSFRecord>& getRunTrackSFs(const int runNumber) const;
    const std::vector<ClusterSFRecord>& getRunClusterSFs(const int runNumber) const;

    template<typename RecordType>
    const std::vector<RecordType>& getRunSFs(
                                             const int runNumber,
                                             std::map<int, std::vector<RecordType>>& cache,
                                             const std::string& treeName) const;

    /// Flags
    Gaudi::Property<bool> m_equalizeTrackMeasurements
    { this, "EqualizeTrackMeasurements", false, "Equalize track-level truncated mean dE/dx"};
    Gaudi::Property<bool> m_equalizeClusterMeasurements
    { this, "EqualizeClusterMeasurements", false, "Equalize cluster dE/dx before truncated mean"};

    /// PathResolverFindCalibFile needs the logical filename in ASG calibration area.
    Gaudi::Property<std::string> m_sfFileName { this, "SFFileName", "PixelDEdxCalib/pixeldEdxEqualizationSFs_v1p1.root"};
    /// Override version in ASG calibration area with a local file is not empty string.
    Gaudi::Property<std::string> m_sfLocalFileName {this, "SFLocalFileName", ""};
    std::string m_filename;
    
    /// Name of SF tree.
    Gaudi::Property<std::string> m_clusterSFTreeName { this, "ClusterSFTreeName", "cluster_SFs"};
    Gaudi::Property<std::string> m_trackSFTreeName { this, "TrackSFTreeName", "track_SFs"};

    /// Cache map: runNumber -> vector<SFRecord>
    mutable std::map<int, std::vector<ClusterSFRecord>> m_cachedClusterSFData ATLAS_THREAD_SAFE;
    mutable std::map<int, std::vector<TrackSFRecord>> m_cachedTrackSFData ATLAS_THREAD_SAFE;
    mutable std::shared_mutex m_cacheMutex ATLAS_THREAD_SAFE;

    /// Highest eta bin for which track-based equalization SFs are define.
    /// If track has higher eta, use SF from highest bin.
    double m_maxEta;

  }; // class PixelDEdxEqualizationTool

} // namespace CP

#endif  // PIXELDEDXEQUALIZATIONTOOL_H
