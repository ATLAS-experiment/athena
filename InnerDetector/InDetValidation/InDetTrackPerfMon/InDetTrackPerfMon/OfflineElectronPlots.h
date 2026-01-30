/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_PLOTS_OFFLINEELECTRONPLOTS_H
#define INDETTRACKPERFMON_PLOTS_OFFLINEELECTRONPLOTS_H

/**
 * @file    OfflineElectronPlots.h
 * @author  Marco Aparo <marco.aparo@cern.ch>
 **/

/// local includes
#include "InDetTrackPerfMon/PlotMgr.h"

/// xAOD includes
#include "xAODTracking/TrackParticle.h"
#include "xAODTruth/TruthParticle.h"


namespace IDTPM {

  class OfflineElectronPlots : public PlotMgr {

  public:

    /// Constructor
    OfflineElectronPlots(
        PlotMgr* pParent,
        const std::string& dirName,
        const std::string& anaTag,
        bool doEfficiency = false );

    /// Destructor
    virtual ~OfflineElectronPlots() = default;

    /// Book the histograms
    void initializePlots();
    StatusCode bookPlots();

    /// Dedicated fill methods 
    StatusCode fillPlots(
        const xAOD::TrackParticle& track,
        bool isMatched, float weight );
    StatusCode fillPlots(
        const xAOD::TruthParticle&, bool, float );

    /// Print out final stats on histograms
    void finalizePlots();

  private:

    bool m_doEfficiency{};

    TH1* m_Et{};
    TH1* m_EtOverPt{};
    /// TODO - include more plots

    TEfficiency* m_eff_vs_Et{};
    TEfficiency* m_eff_vs_EtOverPt{};

  }; // class OfflineElectronPlots

} // namespace IDTPM

#endif // > ! INDETTRACKPERFMON_PLOTS_OFFLINEELECTRONPLOTS_H
