//#ifndef IPIXELTOTPIDTOOL_H
//#define IPIXELTOTPIDTOOL_H
#ifndef PIXELTOTPIDTOOL_IPIXELTOTPIDTOOL_H
#define PIXELTOTPIDTOOL_IPIXELTOTPIDTOOL_H

// Framework include(s):
#include "AsgTools/IAsgTool.h"

#ifndef XAOD_STANDALONE
#include "TrkTrack/Track.h"
#endif
#include "xAODTracking/TrackParticle.h"

namespace CP {

  /// Interface for the Pixel ToT PID tool.
  /// This is refactoring of the tool for dual use in Athena and AnalysisBase using CP Algs.

  class IPixelToTPIDTool : public virtual asg::IAsgTool {
    /// Declare the interface that the class provides
    ASG_TOOL_INTERFACE(CP::IPixelToTPIDTool)
    
  public:

    /// Athena
#ifndef XAOD_STANDALONE
    virtual float dEdx(const EventContext& ctx,
                       const Trk::Track& track,
                       int& nUsedHits,
                       int& nUsedIBLOverflowHits) const = 0;
#endif

    /// AnalysisBase
#ifdef XAOD_STANDALONE
    virtual float dEdx(const xAOD::TrackParticle& track,
                       int& nUsedHits,
                       int& nUsedIBLOverflowHits) const = 0;
#endif

  }; //class IPixelToTPIDTool

} // namespace CP

#endif  // PIXELTOTPIDTOOL_IPIXELTOTPIDTOOL_H
