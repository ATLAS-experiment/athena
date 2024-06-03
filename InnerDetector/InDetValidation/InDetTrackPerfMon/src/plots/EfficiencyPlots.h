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
        const std::string& trackType );

    /// Destructor
    virtual ~EfficiencyPlots() = default;

    /// Dedicated fill method (for tracks and/or truth particles)
    template< typename PARTICLE >
    StatusCode fillPlots(
        const PARTICLE& particle,
        bool isMatched, float weight );

    /// Book the histograms
    void initializePlots(); // needed to override PlotBase
    StatusCode bookPlots();

    /// Print out final stats on histograms
    void finalizePlots();

  private:

    std::string m_trackType;

    TEfficiency* m_eff_vs_pt;
    TEfficiency* m_eff_vs_eta;
    /// TODO - include more plots

  }; // class EfficiencyPlots

} // namespace IDTPM

#endif // > ! INDETTRACKPERFMON_PLOTS_EFFICIENCYPLOTS_H
