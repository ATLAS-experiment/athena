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

  /// Interface for the Pixel ToT PID tool.
  /// This is refactoring of the tool for dual use in Athena and AnalysisBase using CP Algs.

  class IPixelDEdxEqualizationTool : public virtual asg::IAsgTool {
    /// Declare the interface that the class provides
    ASG_TOOL_INTERFACE(CP::IPixelDEdxEqualizationTool)
    
  public:

    virtual std::shared_ptr<ROOT::RDF::RNode> getFilteredSFDF(const int runNumber) const = 0;
    virtual double getTrackdEdxSF(const xAOD::TrackParticle& track, const int runNumber) const = 0;
    virtual double getClusterdEdxSF(const PixelDEdx::PixelClusterStruct&, const int runNumber) const = 0;

  }; //class IPixelDEdxEqualizationTool

} // namespace CP

#endif  // PIXELDEDXEQUALIZATIONTOOL_IPIXELDEDXEQUALIZATIONTOOL_H
