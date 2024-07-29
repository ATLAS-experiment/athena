/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */

///////////////////////////////////////////////////////////////////
// InDetExtensionProcessor.h, (c) ATLAS Detector Softwareop
///////////////////////////////////////////////////////////////////

#ifndef INDETEXTENSIONPROCESSOR_H
#define INDETEXTENSIONPROCESSOR_H

// Base class

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "CxxUtils/checker_macros.h"
#include "TrkTrack/TrackCollection.h"
#include "TrkTrack/TrackExtensionMap.h"
#include "TrkEventPrimitives/ParticleHypothesis.h"
#include "TrkFitterInterfaces/ITrackFitter.h"
#include "TrkToolInterfaces/ITrackScoringTool.h"
#include "TrkToolInterfaces/IExtendedTrackSummaryTool.h"

#include "GaudiKernel/ToolHandle.h"

#include <vector>
#include <array>
#include <mutex>

namespace InDet {
  /** @brief Algorithm to process a TrackExtensionMap, refit the
      extended tracks and decide if the old track is to be
      replaced by the extended one.

      @author Markus.Elsing@cern.ch
   */

  class InDetExtensionProcessor: public AthReentrantAlgorithm  {
  public:
    //! Default Algorithm constructor with parameters
    InDetExtensionProcessor(const std::string& name, ISvcLocator* pSvcLocator);

    virtual StatusCode initialize();
    virtual StatusCode execute(const EventContext& ctx) const;
    virtual StatusCode finalize();
  private:
    MsgStream &dumpStat(MsgStream &out) const;

    //! process events
    TrackCollection* createExtendedTracks(const EventContext& ctx,
                                          const TrackCollection* tracks_in,
                                          const TrackExtensionMap* track_extension_map) const;

    //! internal structuring: creates new track with original one plus extension as outliers
    Trk::Track* trackPlusExtension(const EventContext& ctx,
                                   const Trk::Track*,
                                   const std::vector<const Trk::MeasurementBase*>&) const;

    //
    // --- job options
    //
    SG::ReadHandleKey<TrackCollection> m_trackName{
      this, "TrackName", "Tracks", "Name of the input Trackcollection"};
    SG::ReadHandleKey<TrackExtensionMap> m_extensionMapName{
      this, "ExtensionMap", "TrackExtensionMap", "Name of the input extension map"};
    SG::WriteHandleKey<TrackCollection> m_newTrackName{
      this, "NewTrackName", "ExtendedTrack", "Name of the output Trackcollection"};

    PublicToolHandle<Trk::ITrackFitter> m_trackFitter{
      this, "TrackFitter", "Trk::KalmanFitter/InDetTrackFitter",
      "Toolhandle for the track fitter"};

    ToolHandle<Trk::IExtendedTrackSummaryTool> m_trackSummaryTool{
      this, "TrackSummaryTool", ""};

    PublicToolHandle<Trk::ITrackScoringTool> m_scoringTool{
      this, "ScoringTool", "Trk::TrackScoringTool",
      "Toolhandle for the track scorer"};

    BooleanProperty m_cosmics{
      this, "Cosmics", false, "switch whether we are running on cosmics"};
    Gaudi::Property<Trk::RunOutlierRemoval> m_runOutlier{
      this, "runOutlier", true, "switch whether to run outlier logics or not"};
    BooleanProperty m_keepFailedExtensionOnTrack{
      this, "keepFailedExtension", true,
      "switch whether to keep failed extension as outlier hits on the new track"};
    BooleanProperty m_refitPrds{
      this, "RefitPrds", true,
      "switch whether to do the fit with re-calibrated clusters (true) or not"};
    IntegerProperty m_matEffects{
      this, "matEffects", 3,
      "particle hypothesis to assume for material effects in track fit"};
    BooleanProperty m_suppressHoleSearch{
      this, "suppressHoleSearch", false, "suppressing hole search for comparison"};
    BooleanProperty m_tryBremFit{
      this, "tryBremFit", false, "brem recovery mode"};
    BooleanProperty m_caloSeededBrem{
      this, "caloSeededBrem", false, "calo seeded brem recovery"};
    FloatProperty m_pTminBrem{
      this, "pTminBrem", 1000., "min pT for trying a brem recovery"};
    Gaudi::Property<std::vector<float>> m_etabounds{
      this, "etaBounds", {0.8, 1.6, 2.10}, "eta intervals for internal monitoring"};
    //note: only three values!

    // -- algorithm members
    Trk::ParticleHypothesis m_particleHypothesis{Trk::undefined}; //!< nomen est omen

    //! internal monitoring: categories for counting different types of extension results
    enum StatIndex {
      iAll, iBarrel, iTransi, iEndcap, Nregions
    };

    enum StatTrkType {
      nInput, nRecognised, nExtended, nRejected, nFailed, nRecoveryBremFits, 
      nBremFits, nFits, nNotExtended, nLowScoreBremFits, nExtendedBrem, nTypes 
    };

    // -- Using atomics to be multi-thread safe
    mutable std::array< std::array<std::atomic<int>, Nregions>, nTypes > m_counters ATLAS_THREAD_SAFE;
    mutable std::atomic<int> m_Nevents ATLAS_THREAD_SAFE;

    //! monitoring and validation: does success/failure counting for each detector region
    void incrementRegionCounter(std::array<std::atomic<int>, 4>&, const Trk::Track*, bool = true) const;
  };
}

#endif //INDETEXTENSIONPROCESSOR_H
