/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITRACKSUMMARYHELPERTOOL
#define ITRACKSUMMARYHELPERTOOL


#include "GaudiKernel/IAlgTool.h"
#include <vector>
#include <bitset>
#include "TrkEventPrimitives/ParticleHypothesis.h"
#include "TrkTrackSummary/TrackSummary.h" // defines Trk::numberOfDetectorTypes used below

class Identifier;

namespace Trk {

  class Track;
  class RIO_OnTrack;
  class TrackStateOnSurface;
  class CompetingRIOsOnTrack;


/** @class ITrackSummaryHelperTool
  @brief Interface for structuring the summary creation into sub-detector
    specific tools.

    This tool is aimed to be implemented once in the Inner Detector and
    once in the Muon Spectrometer, allowing each implementation to access
    detector-specific information while a master tool can decide at run-time
    which implementation to call, depending on the type of hit on the track.

    @author Edward Moyse, Martin Siebel <http://consult.cern.ch/xwho>
*/
  class ITrackSummaryHelperTool : virtual public IAlgTool {
  public:
    DeclareInterfaceID(ITrackSummaryHelperTool, 1, 0);

  /** fill 'information' and 'hitpattern' using information from 'rot'. Should be overloaded by concrete
    TrackSummaryHelperTools. It is mandatory that the RIO_OnTrack* points to exactly the object contained
      inside the TrackStateOnSurface. This is to avoid that the RTTI from the TrackSummaryTool is done twice.
        */
    virtual void analyse(
      const EventContext& ctx,
      const Trk::Track& track,
      const RIO_OnTrack* rot,
      const TrackStateOnSurface* tsos,
      std::vector<int>& information,
      std::bitset<Trk::numberOfDetectorTypes>& hitPattern) const = 0;

    virtual void analyse(
      const EventContext& ctx,
      const Trk::Track& track,
      const CompetingRIOsOnTrack* crot,
      const TrackStateOnSurface* tsos,
      std::vector<int>& information,
      std::bitset<Trk::numberOfDetectorTypes>& hitPattern) const = 0;

    virtual void searchForHoles(
      const Trk::Track& track,
      std::vector<int>& information,
      const Trk::ParticleHypothesis partHyp = Trk::pion) const = 0;

    virtual void addDetailedTrackSummary(
      const EventContext& ctx,
      const Trk::Track& track,
      Trk::TrackSummary& summary) const = 0;

  };

}
#endif

