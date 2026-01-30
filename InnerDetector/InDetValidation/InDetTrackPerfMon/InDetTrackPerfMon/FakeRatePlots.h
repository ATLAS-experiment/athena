/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_PLOTS_FAKERATEPLOTS_H
#define INDETTRACKPERFMON_PLOTS_FAKERATEPLOTS_H

/**
 * @file    FakeRatePlots.h
 * @author  Marco Aparo <Marco.Aparo@cern.ch>
 **/

/// local includes
#include "InDetTrackPerfMon/PlotMgr.h"


namespace IDTPM {

  class FakeRatePlots : public PlotMgr {

  public:

    /// Constructor
    FakeRatePlots(
        PlotMgr* pParent,
        const std::string& dirName,
        const std::string& anaTag,
        const std::string& trackType,
        bool doGlobalPlots = false,
        bool doTruthMuPlots = false );

    /// Destructor
    virtual ~FakeRatePlots() = default;

    /// Dedicated fill method (for tracks and/or truth particles)
    template< typename PARTICLE >
    StatusCode fillPlots(
        const PARTICLE& particle,
        bool isFake,
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
    bool m_doGlobalPlots{};
    bool m_doTruthMuPlots{};

    TEfficiency* m_fakerate_vs_incl{};
    TEfficiency* m_fakerate_vs_pt{};
    TEfficiency* m_fakerate_vs_logPt{};
    TEfficiency* m_fakerate_vs_eta{};
    TEfficiency* m_fakerate_vs_phi{};
    TEfficiency* m_fakerate_vs_d0{};
    TEfficiency* m_fakerate_vs_z0{};
    TEfficiency* m_fakerate_vs_truthMu{};
    TEfficiency* m_fakerate_vs_actualMu{};

  }; // class FakeRatePlots

} // namespace IDTPM

#endif // > ! INDETTRACKPERFMON_PLOTS_FAKERATEPLOTS_H
