/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRKIEXTENDEDTRACKSUMMARYTOOL_H
#define TRKIEXTENDEDTRACKSUMMARYTOOL_H

#include "ITrackSummaryTool.h"

namespace Trk {

/** @class IExtendedTrackSummaryTool
    @brief Interface for condensing Trk::Track properties and associated
           hits to a (non-fittable) foot print, the Trk::TrackSummary object.

    @author Edward Moyse, Martin Siebel <http://consult.cern.ch/xwho>
*/
class IExtendedTrackSummaryTool : virtual public ITrackSummaryTool
{
public:
  DeclareInterfaceID(Trk::IExtendedTrackSummaryTool, 1, 0);

  using ITrackSummaryTool::summary;
  using ITrackSummaryTool::summaryNoHoleSearch;
  using ITrackSummaryTool::updateAdditionalInfo;
  using ITrackSummaryTool::updateTrackSummary;

  /** Compute track summary and replace the summary in given track.
   * @param track the track whose track summary is replaced with a newly
   * computed one
   * @param suppress_hole_search do not perform the hole search independent of
   * the settings of the ID and muon hole search properties. Will recompute the
   * track summary for the given track, delete the old track summary of the
   * track if there is already one and set the new one. The hole
   * search is performed according to the settings of the ID and muon hole
   * search properties unless the suppress_hole_search argument is true.
   */
  virtual void computeAndReplaceTrackSummary(
    const EventContext& ctx,
    Track& track,
    bool suppress_hole_search = false) const = 0;

  /** Start from a copy of the existing input track summary if there,
   * otherwise start from a new one. Fill it and return it.
   * Does not modify the const track.
   */
  virtual std::unique_ptr<Trk::TrackSummary> summary(
    const EventContext& ctx,
    const Track& track) const = 0;

  /** method which can be used to update the summary of a track.
   * If a summary is present is modified in place
   * otherwise a new one is created.
   */
  virtual void updateTrackSummary(const EventContext& ctx,
                                  Track& track,
                                  bool suppress_hole_search = false) const = 0;

  /** Start from a copy of the existing input track summary if there,
   * otherwise start from a new one. Fill it and return it.
   * but without doing the hole search.
   * Does not modify the const track.
   */
  virtual std::unique_ptr<Trk::TrackSummary> summaryNoHoleSearch(
    const EventContext& ctx,
    const Track& track) const = 0;

  virtual void updateAdditionalInfo(Track& track) const = 0;
};
}

#endif
