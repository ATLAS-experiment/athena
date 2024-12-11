/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_PLOTS_NTRACKSPLOTS_H
#define INDETTRACKPERFMON_PLOTS_NTRACKSPLOTS_H

/**
 * @file    NtracksPlots.h
 * @author  Marco Aparo <Marco.Aparo@cern.ch>
 **/

/// local includes
#include "../PlotMgr.h"
#include "../TrackAnalysisCollections.h"


namespace IDTPM {

  class NtracksPlots : public PlotMgr {

  public:

    /// Constructor
    NtracksPlots(
        PlotMgr* pParent,
        const std::string& dirName,
        const std::string& anaTag,
        const std::string& trackType,
        bool doTrigger = false,
        bool doGlobalPlots = false,
        bool doTruthMuPlots = false );

    /// Destructor
    virtual ~NtracksPlots() = default;

    /// Dedicated fill method
    StatusCode fillPlots(
        const std::vector< size_t >& trackCounts,
        const std::vector< size_t >& vertexCounts,
        float truthMu,
        float actualMu,
        float weight );

    /// Book the histograms
    void initializePlots(); // needed to override PlotBase
    StatusCode bookPlots();

    /// Print out final stats on histograms
    void finalizePlots();

  private:

    std::string m_trackType;
    bool m_doTrigger{};
    bool m_doGlobalPlots{};
    bool m_doTruthMuPlots{};

    /// Importing Counter enum
    enum Counter : size_t {
        ALL       = TrackAnalysisCollections::ALL,
        SELECTED  = TrackAnalysisCollections::SELECTED,
        INROI     = TrackAnalysisCollections::INROI,
        MATCHED   = TrackAnalysisCollections::MATCHED,
        NCOUNTERS = TrackAnalysisCollections::NCOUNTERS
    };

    std::string m_counterName[ NCOUNTERS ] = {
        "all", "selected", "selectedInRoI", "matched"
    };

    TH1*  m_nTracks[ NCOUNTERS ]{};
    TH2*  m_nTracks_vs_nVertices[ NCOUNTERS ]{};
    TH2*  m_nTracks_vs_truthMu{};
    TH2*  m_nTracks_vs_actualMu{};

  }; // class NtracksPlots

} // namespace IDTPM

#endif // > ! INDETTRACKPERFMON_PLOTS_NTRACKSPLOTS_H
