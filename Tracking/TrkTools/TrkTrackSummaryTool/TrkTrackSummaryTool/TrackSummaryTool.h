/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRKTRACKSUMMARYTOOL_H
#define TRKTRACKSUMMARYTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrkTrack/Track.h"               //used in the included icc file
#include "TrkTrackSummary/TrackSummary.h" //used in the included icc file

#include "TRT_ElectronPidTools/ITRT_ToT_dEdx.h" //template parameter to tool handle
#include "TrkToolInterfaces/IExtendedTrackSummaryHelperTool.h" //template parameter to tool handle
#include "TrkToolInterfaces/IPixelToTPIDTool.h" //template parameter to tool handle
#include "TrkToolInterfaces/ITRT_ElectronPidTool.h" //template parameter to tool handle

#include "TrkToolInterfaces/IExtendedTrackSummaryTool.h"
#include <bitset>
#include <memory>
#include <vector>

class EventContext;
class AtlasDetectorID;
class Identifier;

namespace Trk {

/** @enum flag the methods for the probability calculation */
enum TRT_ElectronPidProbability
{
  Combined = 0,
  HighThreshold = 1,
  TimeOverThreshold = 2,
  Bremsstrahlung = 3
};

class RIO_OnTrack;
class TrackStateOnSurface;
class MeasurementBase;

class TrackSummaryTool final
  : public extends<AthAlgTool, IExtendedTrackSummaryTool>
{

public:
  TrackSummaryTool(const std::string&, const std::string&, const IInterface*);
  virtual ~TrackSummaryTool();
  virtual StatusCode initialize() override;

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
  using IExtendedTrackSummaryTool::computeAndReplaceTrackSummary;
  using IExtendedTrackSummaryTool::summary;
  using IExtendedTrackSummaryTool::summaryNoHoleSearch;
  using IExtendedTrackSummaryTool::updateTrack;

  virtual void computeAndReplaceTrackSummary(
    const EventContext& ctx,
    Track& track,
    bool suppress_hole_search = false) const override final;

  /** Same behavious as
   * IExtendedTrackSummaryTool:computeAndReplaceTrackSummary
   * but without the need to pass
   * Does hole search
   */
  virtual void updateTrack(const EventContext& ctx,
                           Track& track) const override final;

  /* Start from a copy of the existing input track summary if there,
   * otherwise start from a new one. Fill it and return it.
   * Does not modify the const track.
   */
  virtual std::unique_ptr<Trk::TrackSummary> summary(
    const EventContext& ctx,
    const Track& track) const override final;

  /* Start from a copy of the existing input track summary if there,
   * otherwise start from a new one. Fill it and return it.
   * but without doing the hole search.
   * Does not modify the const track.
   */
  virtual std::unique_ptr<Trk::TrackSummary> summaryNoHoleSearch(
    const EventContext& ctx,
    const Track& track) const override final;

  /** method which can be used to update the summary of a track
   * it, without doing hole search.
   * If a summary is present it is  modified in place.
   * Otherwise a new one is created and filled.
   */
  virtual void updateTrackSummary(const EventContext& ctx,
                                  Track& track) const override final;

  /** method which can be used to update the summary of a track
   * If a summary is present it is  modified in place.
   * Otherwise a new one is created and filled.
   */
  virtual void updateTrackSummary(
    const EventContext& ctx,
    Track& track,
    bool suppress_hole_search = false) const override final;

  /** method to update additional information (PID, dEdX), this is
   * optimised for track collection merging.
   */
  virtual void updateAdditionalInfo(Track& track) const override;

private:
  /*
   * Fill the summary info for a Track*/
  void fillSummary(const EventContext& ctx,
                   Trk::TrackSummary& ts,
                   const Trk::Track& track,
                   bool doHolesInDet,
                   bool doHolesMuon) const;

  /*
   * If a summary is there Update it with the required info.
   * If not there create a new one with the required info.
   */
  void UpdateSummary(const EventContext& ctx,
                     Track& track,
                     bool suppress_hole_search) const;

  std::unique_ptr<Trk::TrackSummary> createSummary(
    const EventContext& ctx,
    const Track& track,
    bool doHolesInDet,
    bool doHolesMuon) const;

  /** Return the correct tool, matching the passed Identifier*/
  const Trk::IExtendedTrackSummaryHelperTool* getTool(
    const Identifier& id) const;

  /**tool to decipher ID RoTs*/
  ToolHandle<IExtendedTrackSummaryHelperTool>
    m_idTool{ this, "InDetSummaryHelperTool", "", "" };
  /**tool to decipher muon RoTs*/
  ToolHandle<IExtendedTrackSummaryHelperTool>
    m_muonTool{ this, "MuonSummaryHelperTool", "", "" };

  /** controls whether holes on track in MS are produced
      Turning this on will (slightly) increase processing time.*/
  Gaudi::Property<bool> m_doHolesMuon{ this, "doHolesMuon", false, "" };
  /** controls whether holes on track in ID are produced */
  Gaudi::Property<bool> m_doHolesInDet{ this, "doHolesInDet", false, "" };

  /** controls whether the detailed summary is added for the muons */
  Gaudi::Property<bool> m_addMuonDetailedSummary{ this,
                                                  "AddDetailedMuonSummary",
                                                  true,
                                                  "" };
  /** switch to deactivate Pixel info init */
  Gaudi::Property<bool> m_pixelExists{ this, "PixelExists", true, "" };

  Gaudi::Property<bool> m_alwaysRecomputeHoles{ this,
                                                "AlwaysRecomputeHoles",
                                                false,
                                                "" };

  /**atlas id helper*/
  const AtlasDetectorID* m_detID;

  /**loops over TrackStatesOnSurface and uses this to produce the summary
     information Fills 'information', 'eProbability', and 'hitPattern'*/
  void processTrackStates(const EventContext& ctx,
                          const Track& track,
                          const Trk::TrackStates* tsos,
                          std::vector<int>& information,
                          std::bitset<numberOfDetectorTypes>& hitPattern,
                          bool doHolesInDet,
                          bool doHolesMuon) const;

  void processMeasurement(const EventContext& ctx,
                          const Track& track,
                          const Trk::MeasurementBase* meas,
                          const Trk::TrackStateOnSurface* tsos,
                          std::vector<int>& information,
                          std::bitset<numberOfDetectorTypes>& hitPattern) const;

  /** Extrapolation is performed from one hit to the next, it is checked if
     surfaces in between the extrapolation are left out. The trackParameters of
     the destination hit (instead of the trackParameters of the extrapolation
     step) are then used as starting point for the next extrapolation step. */
  void searchHolesStepWise(const Trk::Track& track,
                           std::vector<int>& information,
                           bool doHolesInDet,
                           bool doHolesMuon) const;
};

}

#include "TrkTrackSummaryTool/TrackSummaryTool.icc"
#endif
