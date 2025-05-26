#ifndef PIXELDEDXEQUALIZATIONTOOL_IPIXELDEDXEQUALIZATIONTOOL_H
#define PIXELDEDXEQUALIZATIONTOOL_IPIXELDEDXEQUALIZATIONTOOL_H

// Framework include(s):
#include "AsgTools/IAsgTool.h"

#include "xAODTracking/TrackParticle.h"

#include "ROOT/RDataFrame.hxx"

/// Forward declare
namespace PixelDEdx {
  struct PixelClusterStruct;
}

namespace CP {

  // Full struct definition needed here.

  struct TrackSFRecord {
    double etaLow;
    double etaHigh;
    double SF_IBLOFYes;
    double SF_IBLOFNo;
  };
  
  struct ClusterSFRecord {
    int bec;
    int layerID;
    int etaM;
    double SF;
    double SFerr;
  };
  
  /// Interface for the Pixel ToT PID tool.
  /// This is refactoring of the tool for dual use in Athena and AnalysisBase using CP Algs.

  class IPixelDEdxEqualizationTool : public virtual asg::IAsgTool {
    /// Declare the interface that the class provides
    ASG_TOOL_INTERFACE(CP::IPixelDEdxEqualizationTool)
    
  public:

    virtual std::shared_ptr<std::vector<TrackSFRecord>> getRunTrackSFs(const int runNumber) const = 0;
    virtual std::shared_ptr<std::vector<ClusterSFRecord>> getRunClusterSFs(const int runNumber) const = 0;
    virtual double getTrackdEdxSF(const xAOD::TrackParticle& track, const int runNumber) const = 0;
    virtual double getClusterdEdxSF(const PixelDEdx::PixelClusterStruct&, const int runNumber) const = 0;

  }; //class IPixelDEdxEqualizationTool

} // namespace CP

#endif  // PIXELDEDXEQUALIZATIONTOOL_IPIXELDEDXEQUALIZATIONTOOL_H
