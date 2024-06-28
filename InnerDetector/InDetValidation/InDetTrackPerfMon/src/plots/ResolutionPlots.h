/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_PLOTS_RESOLUTIONPLOTS_H
#define INDETTRACKPERFMON_PLOTS_RESOLUTIONPLOTS_H

/**
 * @file    ResolutionPlots.h
 * @author  Thomas Strebler <thomas.strebler@cern.ch> 
 **/

/// local includes
#include "../PlotMgr.h"

namespace IDTPM {

  class ResolutionPlots : public PlotMgr {

  public:

    /// Constructor
    ResolutionPlots(
        PlotMgr* pParent,
        const std::string& dirName,
        const std::string& anaTag,
        const std::string& trackType );

    /// Destructor
    virtual ~ResolutionPlots() = default;

    /// Dedicated fill method (for tracks and/or truth particles)
    template< typename REF, typename TEST >
    StatusCode fillPlots(
      const REF& particle_ref,
	    const TEST& track_test,
      float weight );

    /// Book the histograms
    void initializePlots(); // needed to override PlotBase
    StatusCode bookPlots();

    /// Print out final stats on histograms
    void finalizePlots();

  private:

    std::string m_trackType;
    
    enum Param {
      D0, Z0, QOVERP, QOVERPT, THETA, PHI, PT, Z0SIN, NPARAMS
    };

    std::string m_paramProp[NPARAMS] = {
      "d0", "z0", "qoverp", "ptqopt", "theta", "phi", "pt", "z0sin"
    };

    TH2* m_resHelperEta[NPARAMS];
    TH1* m_reswidth_vs_eta[NPARAMS];
    TH1* m_resmean_vs_eta[NPARAMS];
    TH2* m_pullHelperEta[NPARAMS];
    TH1* m_pullwidth_vs_eta[NPARAMS];
    TH1* m_pullmean_vs_eta[NPARAMS];

    TH2* m_resHelperPt[NPARAMS];
    TH1* m_reswidth_vs_pt[NPARAMS];
    TH1* m_resmean_vs_pt[NPARAMS];
    TH2* m_pullHelperPt[NPARAMS];
    TH1* m_pullwidth_vs_pt[NPARAMS];
    TH1* m_pullmean_vs_pt[NPARAMS];

  };
  
}

#endif
    
