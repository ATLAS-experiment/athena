/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_PLOTS_TRACKPARAMETERSPLOTS_H
#define INDETTRACKPERFMON_PLOTS_TRACKPARAMETERSPLOTS_H

/**
 * @file    TrackParametersPlots.h
 * @author  Marco Aparo <marco.aparo@cern.ch> 
 **/

/// local includes
#include "InDetTrackPerfMon/PlotMgr.h"


namespace IDTPM {

  class TrackParametersPlots : public PlotMgr {

  public:

    /// Constructor
    TrackParametersPlots(
        PlotMgr* pParent,
        const std::string& dirName,
        const std::string& anaTag,
        const std::string& trackType,
        bool plotErrors = false,
        bool recomputeIP = false );

    /// Destructor
    virtual ~TrackParametersPlots() = default;

    /// Book the histograms
    void initializePlots(); // needed to override PlotBase
    StatusCode bookPlots();

    /// Dedicated fill method (for tracks and/or truth particles)
    template< typename PARTICLE >
    StatusCode fillPlots( const PARTICLE& particle, float weight );

    /// Print out final stats on histograms
    void finalizePlots();

  private:

    std::string m_trackType;
    bool m_plotErrors{};
    bool m_recomputeIP{};

    TH1* m_pt{};
    TH1* m_eta{};
    TH1* m_phi{};
    TH1* m_d0{};
    TH1* m_z0{};
    TH1* m_z0sin{};
    TH1* m_theta{};
    TH1* m_qoverp{};
    TH1* m_prodR{};
    TH1* m_prodZ{};
    TH1* m_nSiHits{};
    TH2* m_nSiHits_vs_eta{};
    TH1* m_chi2{};
    TH1* m_ndof{};
    TH1* m_chi2OverNdof{};
    TH1* m_author{};
    TH1* m_time{};
    TEfficiency* m_hasValidTime_eff_vs_eta{};
    TH2* m_eta_vs_pt{};
    TH2* m_eta_vs_phi{};
    TH2* m_z0_vs_d0{};
    TH2* m_z0sin_vs_d0{};

    /// sigma plots
    TH1* m_sigma_pt{};
    TH1* m_sigma_eta{};
    TH1* m_sigma_phi{};
    TH1* m_sigma_d0{};
    TH1* m_sigma_z0{};
    TH1* m_sigma_z0sin{};
    TH1* m_sigma_theta{};

    /// significance plots
    TH1* m_significance_pt{};
    TH1* m_significance_eta{};
    TH1* m_significance_phi{};
    TH1* m_significance_d0{};
    TH1* m_significance_z0{};
    TH1* m_significance_z0sin{};
    TH1* m_significance_theta{};

  }; // class TrackParametersPlots

} // namespace IDTPM

#endif // > ! INDETTRACKPERFMON_PLOTS_TRACKPARAMETERSPLOTS_H
