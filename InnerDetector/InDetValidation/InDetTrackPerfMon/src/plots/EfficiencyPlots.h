/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_PLOTS_EFFICIENCYPLOTS_H
#define INDETTRACKPERFMON_PLOTS_EFFICIENCYPLOTS_H

/**
 * @file EfficiencyPlots.h
 * @author Marco Aparo <Marco.Aparo@cern.ch>
 **/

/// local includes
#include "../PlotMgr.h"


namespace IDTPM {

  class EfficiencyPlots : public PlotMgr {

  public:

    /// Constructor
    EfficiencyPlots(
        PlotMgr* pParent,
        const std::string& dirName,
        const std::string& anaTag,
        const std::string& trackType,
        bool doGlobalPlots = false,
        bool doTruthMuPlots = false );

    /// Destructor
    virtual ~EfficiencyPlots() = default;

    /// Dedicated fill method (for tracks and/or truth particles)
    template< typename PARTICLE >
    StatusCode fillPlots(
        const PARTICLE& particle,
        bool isMatched,
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

    TEfficiency* m_eff_vs_inclusive{};
    TEfficiency* m_eff_vs_pt{};
    TEfficiency* m_eff_vs_lowPt{};
    TEfficiency* m_eff_vs_eta{};
    TEfficiency* m_eff_vs_phi{};
    TEfficiency* m_eff_vs_d0{};
    TEfficiency* m_eff_vs_z0{};
    TEfficiency* m_eff_vs_prodR{};
    TEfficiency* m_eff_vs_prodZ{};
    TEfficiency* m_eff_vs_eta_vs_pt{};
    TEfficiency* m_eff_vs_eta_vs_phi{};
    TEfficiency* m_eff_vs_z0_vs_d0{};
    TEfficiency* m_eff_vs_z0sin_vs_d0{};

    /// Plots vs global quantities
    TEfficiency* m_eff_vs_truthMu{};
    TEfficiency* m_eff_vs_actualMu{};

  }; // class EfficiencyPlots

} // namespace IDTPM

#endif // > ! INDETTRACKPERFMON_PLOTS_EFFICIENCYPLOTS_H
