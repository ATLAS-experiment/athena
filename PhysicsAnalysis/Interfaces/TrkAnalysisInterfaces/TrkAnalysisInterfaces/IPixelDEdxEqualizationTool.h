//#ifndef IPIXELDEDXEQUALIZATIONTOOL_H
//#define IPIXELDEDXEQUALIZATIONTOOL_H
#ifndef PIXELDEDXEQUALIZATIONTOOL_IPIXELDEDXEQUALIZATIONTOOL_H
#define PIXELDEDXEQUALIZATIONTOOL_IPIXELDEDXEQUALIZATIONTOOL_H

// Framework include(s):
#include "AsgTools/IAsgTool.h"

#ifndef XAOD_STANDALONE
#include "TrkTrack/Track.h"
#endif
#include "xAODTracking/TrackParticle.h"

namespace CP {

  /// Interface for the Pixel ToT PID tool.
  /// This is refactoring of the tool for dual use in Athena and AnalysisBase using CP Algs.

  class IPixelDEdxEqualizationTool : public virtual asg::IAsgTool {
    /// Declare the interface that the class provides
    ASG_TOOL_INTERFACE(CP::IPixelDEdxEqualizationTool)
    
  public:

    virtual float dEdx(const xAOD::TrackParticle& track,
                       int& nUsedHits,
                       int& nUsedIBLOverflowHits) const = 0;

  }; //class IPixelDEdxEqualizationTool

} // namespace CP

#endif  // PIXELDEDXEQUALIZATIONTOOL_IPIXELDEDXEQUALIZATIONTOOL_H
