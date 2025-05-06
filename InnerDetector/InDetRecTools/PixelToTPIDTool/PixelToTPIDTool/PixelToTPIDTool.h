#ifndef PIXELTOTPIDTOOL_PIXELTOTPIDTOOL_H
#define PIXELTOTPIDTOOL_PIXELTOTPIDTOOL_H

#include "TrkAnalysisInterfaces/IPixelToTPIDTool.h"

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


#ifndef XAOD_STANDALONE
#pragma message("NOT compiling in XAOD_STANDALONE mode")
#include "PixelConditionsData/PixelChargeCalibCondData.h"
//#include "PixelConditionsData/PixeldEdxData.h"
//#include "StoreGate/ReadCondHandleKey.h"
#include "PixelGeoModel/IIBLParameterSvc.h"
//
#include "TrkTrack/Track.h"
#include "TrkTrack/TrackStateOnSurface.h"
#include "TrkTrack/TrackInfo.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "TrkSurfaces/Surface.h"
#include "InDetRIO_OnTrack/PixelClusterOnTrack.h"
#include "Identifier/Identifier.h" // needed?
#include "InDetIdentifier/PixelID.h"

#else
#pragma message("Compiling in XAOD_STANDALONE mode")
#endif

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

  class PixelToTPIDTool : public virtual IPixelToTPIDTool, public asg::AsgTool {
    /// Create a proper constructor for Athena
    ASG_TOOL_CLASS(PixelToTPIDTool, CP::IPixelToTPIDTool)  // depends where I put the interface...


    
  public:
    PixelToTPIDTool(const std::string& tool_name="PixelToTPIDTool");
    
    virtual ~PixelToTPIDTool();
    
    /// @name Function(s) implementing the asg::IAsgTool interface
    /// @{

    /// Function initialising the tool
    virtual StatusCode initialize() override;

    /// @}

    /// @name Function(s) implementing the IPixelToTPIDTool interface
    /// @{

    /// Athena with ESD EDM
#ifndef XAOD_STANDALONE
    virtual float dEdx(const EventContext& ctx,
                       const Trk::Track& track,
                       int& nUsedHits,
                       int& nUsedIBLOverflowHits) const override;
#endif

    /// AnalysisBase with xAOD EDM
#ifdef XAOD_STANDALONE
    virtual float dEdx(const xAOD::TrackParticle& track,
                       int& nUsedHits,
                       int& nUsedIBLOverflowHits) const override;
#endif




  private:
    
    /// Common to both EDMs ///

    /// Equalize the cluster-level dE/dx measuremented before the taking the truncated mean.
    /// For xAOD EDM, Requires special datasets with pixel clusters.
    Gaudi::Property<bool> m_equalizeClusterMeasurements
    { this, "EqualizeClusterMeasurements", false, ""};

    /// Equalize the track-level truncated mean instead of the individual cluster measurements.
    /// Not as good as pixel-level equalization, but does not require access to clusters.
    /// Nominal AOD does not have pixel clusters.
    /// Perhaps no use case during reconstruction since have access to clusters in ESD EDM.
    Gaudi::Property<bool> m_equalizeTrackMeasurements
    { this, "EqualizeTrackMeasurements", false, ""};

    /// Apply extra cluster cleaning requirements (e.g. cluster size/shape cuts).
    Gaudi::Property<bool> m_extraClusterCleaning
    { this, "ExtraClusterCleaning", false, ""};

    /// For charge -> dE/dx calc.
    double m_conversionfactor;
    float m_Pixel_sensorthickness; //250 microns Pixel Planars
    float m_IBL_3D_sensorthickness; //230 microns IBL 3D
    float m_IBL_PLANAR_sensorthickness; // 200 microns IBL Planars
    
    struct PixelCluster {  // Struct representing a pixel cluster to abstract away the two EDMs
      double locx = -99.9;
      double locy = -99.9;
      int bec = -99;
      int layer = -99;
      int eta_module = -99;
      float cosalpha = -99.9;
      float charge = -99.9;
      float dEdx = -99.9;
      float dEdxEq = -99.9;
      bool isIBL = false;
      int iblOverflow = 0;
    };

    float getClusterdEdx(const PixelCluster& cluster,
                         int& pixelhits,
                         int& nUsedIBLOverflowHits) const;
    
    float getTruncatedMean(const std::vector<PixelCluster>& clusters,
                           int& nUsedHits,
                           int pixelhits,
                           bool equalize = false) const;

    /// Athena (ESD EDM) ///
#ifndef XAOD_STANDALONE
    ServiceHandle<IIBLParameterSvc> m_IBLParameterSvc {this, "IBLParameterSvc", "IBLParameterSvc"};
    const PixelID* m_pixelid;

    SG::ReadCondHandleKey<PixelChargeCalibCondData> m_moduleDataKey
    {this, "PixelChargeCalibCondData", "PixelChargeCalibCondData", "ChargeCalibration data, for ToT overflow setting"};
#endif


    /// AnalysisBase (xAOD EDM) ///
#ifdef XAOD_STANDALONE
    StatusCode initSFsFromTrees();

    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo{this, "EventInfoContName", "EventInfo", "event info key"}; // needed?

    Gaudi::Property<std::string> m_msosLink
    { this, "MSOSLink", "Reco_msosLink"};

    /// PathResolverFindCalibFile need the logical filename in ASG calibration area.
    Gaudi::Property<std::string> m_sfFileName { this, "SFFileName", "nTuple_data_lowMu_flat.root"}; // FIX! TBD
    /// Override version in ASG calibration area with a local file is not empty string.
    Gaudi::Property<std::string> m_sfLocalFileName {this, "SFLocalFileName", ""};
    /// Name of SF tree.
    Gaudi::Property<std::string> m_clusterSFTreeName { this, "ClusterSFTreeName", "SFs_TTree"}; // FIX! TBD
    Gaudi::Property<std::string> m_trackSFTreeName { this, "TrackSFTreeName", "track_SFs_TTree"}; // FIX! TBD

    /// dE/dx equalization scale factor dataframe read from trees.
    std::shared_ptr<ROOT::RDataFrame> m_df;
    std::shared_ptr<TFile> m_file;  // Keep the file open

    /// Map where key = run number, value is a filtered scale factor RDF (an RDF::RNode) with only the rows for that run number.
    /// So not filtering everytime in execute().
    /// Will be updated in execute, so must be mutable
    mutable std::map<unsigned int, std::shared_ptr<ROOT::RDF::RNode>> m_filteredRDFMap;
    mutable std::mutex m_mapMutex;


    /// Decorators for xAOD EDM
    /// Raw track-level truncated mean dE/dx:
    ///    Returned by dEdx() if m_equalizeClusterMeasurements == false.
    ///    Already in AOD, calculated during reconstruction via this same tool using ESD EDM, stored by TrackParticleCreator.
    ///    NB: dE/dx calculated from xAOD and ESD EDMs can differ, likely due to migrations of cluster local (x,y).
    ///        We place cuts on cluster location when calculating dE/dx to avoid sensor edges.
    ///        As a result, hits used for one EDM can be excluded in calculation for the other EDM.
    /// Equalized track-level truncated mean dE/dx:
    ///    Returned by dEdx() if m_equalizeClusterMeasurements == true.
    ///    Called by PixelDEdxEqualizationAlg in TrackingAnalysisAlgorithms.  Decorate there instead.
    /// Raw cluster dE/dx:
    SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_clusterdEdxKey{this, "clusterdEdxKey", "PixelClusters.dEdx", "SG key for the raw pixel cluster dE/dx attribute"};
    /// Equalized cluster dE/dx:
    SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidationContainer> m_clusterdEdxEqKey{this, "clusterdEdxEqKey", "PixelClusters.dEdxEq", "SG key for the equalized pixel cluster dE/dx attribute"};

#endif


  }; // class PixelToTPIDTool


} // namespace CP

#endif  // PIXELTOTPIDTOOL_H
