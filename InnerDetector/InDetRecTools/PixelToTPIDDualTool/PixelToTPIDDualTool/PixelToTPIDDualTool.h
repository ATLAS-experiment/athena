#ifndef PIXELTOTPIDDUALTOOL_PIXELTOTPIDDUALTOOL_H
#define PIXELTOTPIDDUALTOOL_PIXELTOTPIDDUALTOOL_H

#include "TrkAnalysisInterfaces/IPixelToTPIDDualTool.h"
#include "AsgTools/AsgTool.h"
#include "AsgDataHandles/ReadHandle.h"
#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgDataHandles/WriteDecorHandle.h"
#include "AsgDataHandles/WriteDecorHandleKey.h"

#include "xAODTracking/TrackStateValidation.h"
#include "xAODTracking/TrackMeasurementValidation.h"

#include "AsgTools/PropertyWrapper.h"
#include "xAODEventInfo/EventInfo.h"
#include "PathResolver/PathResolver.h"
#include "TFile.h"
#include <ROOT/RDataFrame.hxx>

#include <optional> // since no default constructor for RDataFrame

#ifndef XAOD_STANDALONE
#pragma message("NOT compiling in XAOD_STANDALONE mode")
#include "PixelConditionsData/PixelChargeCalibCondData.h"
#include "PixelConditionsData/PixeldEdxData.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "PixelGeoModel/IIBLParameterSvc.h"
#else
#pragma message("Compiling in XAOD_STANDALONE mode")
#endif

#ifndef XAOD_STANDALONE
class AtlasDetectorID;
class Identifier;
class PixelID;
class IIBLParameterSvc;
#endif

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
    StatusCode initSFsFromTrees();
    

#ifndef XAOD_STANDALONE
    ServiceHandle<IIBLParameterSvc> m_IBLParameterSvc {this, "IBLParameterSvc", "IBLParameterSvc"};
    const PixelID* m_pixelid;
    SG::ReadCondHandleKey<PixelChargeCalibCondData> m_moduleDataKey
    {this, "PixelChargeCalibCondData", "PixelChargeCalibCondData", "ChargeCalibration data, for ToT overflow setting"};
#endif

    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo{this, "EventInfoContName", "EventInfo", "event info key"}; // needed?

    Gaudi::Property<bool> m_equalizeClusterMeasurements
    { this, "EqualizeClusterMeasurements", false, ""};

    Gaudi::Property<std::string> m_msosLink
    { this, "MSOSLink", "Reco_msosLink"};

    Gaudi::Property<std::string> m_sfDir { this, "SFDir", "share/"};
    Gaudi::Property<std::string> m_sfFileName { this, "SFFileName", "nTuple_data_lowMu_flat.root"};
    Gaudi::Property<std::string> m_sfTreeName { this, "SFTreeName", "SFs_TTree"};
    // possible override for the calibration version
    Gaudi::Property<std::string> m_sfDirLocal {this, "SFDirLocal", ""};

    /// dE/dx equalization scale factor dataframe read from trees.
    std::optional<ROOT::RDataFrame> m_df; 
    std::unique_ptr<TFile> m_file;  // Keep the file open

    /// Decorators
    /// Start with equalized dE/dx measurement.  Safe to hardcode PixelCluster container?  Track container -> MSOS container is 1-to-1. Only 1 PixelCluster container...
    /// Also include raw dE/dx?  Or the SF?  Or the SF error?
    SG::WriteDecorHandleKey<xAOD::TrackMeasurementValidation> m_clusterdEdxKey{this, "clusterdEdxKey", "PixelClusters.dEdxEq", "SG key for the equalized pixel cluster dE/dx attribute"};

    /// For charge -> dE/dx calc.
    double m_conversionfactor;
    float m_Pixel_sensorthickness; //250 microns Pixel Planars
    float m_IBL_3D_sensorthickness; //230 microns IBL 3D
    float m_IBL_PLANAR_sensorthickness; // 200 microns IBL Planars
    
    struct PixelCluster {  // Struct representing a pixel cluster to unify the two EDMs
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
