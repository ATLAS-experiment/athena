#ifndef PIXELTOTPIDDUALTOOL_PIXELTOTPIDDUALTOOL_H
#define PIXELTOTPIDDUALTOOL_PIXELTOTPIDDUALTOOL_H

#include "TrkAnalysisInterfaces/IPixelToTPIDDualTool.h"

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
#include <optional> // since no default constructor for RDataFrame
#include <cmath>
#include <memory>
#include <mutex>

/* IAN
#ifndef XAOD_STANDALONE
class AtlasDetectorID; //needed?
class Identifier; //needed?
class PixelID; //needed?
class IIBLParameterSvc; //needed?
#endif
*/ 
namespace {
  struct PrintHeaderInclude {
    PrintHeaderInclude() { 
      std::cout << "Ian: Including your PixelToTPIDDualTool/PixelToTPIDDualTool.h!" << std::endl; 
    }
  };
  static PrintHeaderInclude printHeaderInclude;
}
namespace CP {

  /// Implementation of the Pixel ToT PID tool.
  /// This is refactoring of the tool for dual use in Athena and AnalysisBase using CP Algs.

  class PixelToTPIDDualTool : public virtual IPixelToTPIDDualTool, public asg::AsgTool {
    /// Create a proper constructor for Athena
    ASG_TOOL_CLASS(PixelToTPIDDualTool, CP::IPixelToTPIDDualTool)  // depends where I put the interface...
    
  public:
    PixelToTPIDDualTool(const std::string& tool_name="PixelToTPIDDualTool");
    
    virtual ~PixelToTPIDDualTool();
    
    /// @name Function(s) implementing the asg::IAsgTool interface
    /// @{

    /// Function initialising the tool
    virtual StatusCode initialize() override;

    /// @}

    /// @name Function(s) implementing the IPixelToTPIDDualTool interface
    /// @{

#ifndef XAOD_STANDALONE
    /// Athena with ESD EDM
    virtual float dEdx(const EventContext& ctx,
                       const Trk::Track& track,
                       int& nUsedHits,
                       int& nUsedIBLOverflowHits) const override;
#endif

    /// Athena & AnalysisBase with xAOD EDM
    virtual float dEdx(const xAOD::TrackParticle& track,
                       int& nUsedHits,
                       int& nUsedIBLOverflowHits) const override;

  private:
    
    Gaudi::Property<bool> m_equalizeClusterMeasurements
    { this, "EqualizeClusterMeasurements", false, ""};

#ifndef XAOD_STANDALONE
    ServiceHandle<IIBLParameterSvc> m_IBLParameterSvc {this, "IBLParameterSvc", "IBLParameterSvc"};
    const PixelID* m_pixelid;
    SG::ReadCondHandleKey<PixelChargeCalibCondData> m_moduleDataKey
    {this, "PixelChargeCalibCondData", "PixelChargeCalibCondData", "ChargeCalibration data, for ToT overflow setting"};
#endif


#ifdef XAOD_STANDALONE
    StatusCode initSFsFromTrees();

    Gaudi::Property<std::string> m_sfDir { this, "SFDir", "share/"};
    Gaudi::Property<std::string> m_sfFileName { this, "SFFileName", "nTuple_data_lowMu_flat.root"};
    Gaudi::Property<std::string> m_sfTreeName { this, "SFTreeName", "SFs_TTree"};
    // possible override for the calibration version
    Gaudi::Property<std::string> m_sfDirLocal {this, "SFDirLocal", ""};

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

    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo{this, "EventInfoContName", "EventInfo", "event info key"}; // needed?

    Gaudi::Property<std::string> m_msosLink
    { this, "MSOSLink", "Reco_msosLink"};

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

  }; // class PixelToTPIDDualTool


} // namespace CP

#endif  // PIXELTOTPIDDUALTOOL_H
