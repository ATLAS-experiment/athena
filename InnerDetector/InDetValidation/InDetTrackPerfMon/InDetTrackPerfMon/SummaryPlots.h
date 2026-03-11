/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_PLOTS_SUMMARYPLOTS_H
#define INDETTRACKPERFMON_PLOTS_SUMMARYPLOTS_H

/**
 * @file    SummaryPlots.h
 * @author  Marco Aparo <Marco.Aparo@cern.ch>
 **/

/// local includes
#include "InDetTrackPerfMon/PlotMgr.h"
#include "InDetTrackPerfMon/TrackAnalysisCollections.h"


namespace IDTPM {

  class SummaryPlots : public PlotMgr {

  public:

    /// Constructor
    SummaryPlots(
        PlotMgr* pParent,
        const std::string& dirName,
        const std::string& anaTag,
        bool doTrigger = false );

    /// Destructor
    virtual ~SummaryPlots() = default;

    /// Dedicated fill method
    StatusCode fillPlots(
        const std::vector< size_t >& testTrackCounts,
        const std::vector< size_t >& refTrackCounts,
        bool isNewEvent,
        float weight );

    /// Book the histograms
    void initializePlots(); // needed to override PlotBase
    StatusCode bookPlots();

    /// Print out final stats on histograms
    void finalizePlots();

  private:

    bool m_doTrigger{};

    /// Importing Counter enum
    enum Counter : size_t {
        ALL       = TrackAnalysisCollections::ALL,
        SELECTED  = TrackAnalysisCollections::SELECTED,
        INROI     = TrackAnalysisCollections::INROI,
        MATCHED   = TrackAnalysisCollections::MATCHED,
        NCOUNTERS = TrackAnalysisCollections::NCOUNTERS
    };

    enum SummaryBin : size_t {
        Nevents,
        NtotTest,
        NselTest,
        NmatchTest,
        NtotRef,
        NselRef,
        NmatchRef,
        Nbins
    };

    enum SummaryBin_trig : size_t {
        Nevents_trig,
        Nrois_trig,
        NtotTest_trig,
        NselTest_trig,
        NinRoiTest_trig,
        NmatchTest_trig,
        NtotRef_trig,
        NselRef_trig,
        NinRoiRef_trig,
        NmatchRef_trig,
        Nbins_trig
    };

    TH1*  m_summary{};
    TH1*  m_summary_trig{};

  }; // class SummaryPlots

} // namespace IDTPM

#endif // > ! INDETTRACKPERFMON_PLOTS_SUMMARYPLOTS_H
