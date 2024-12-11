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
        const std::string& testType,
        const std::string& refType,
        unsigned int method = 0 );

    /// Destructor
    virtual ~ResolutionPlots() = default;

    /// Dedicated fill method (for tracks and/or truth particles)
    template< typename TEST, typename REF >
    StatusCode fillPlots(
	    const TEST& ptest,
      const REF& pref,
      float weight );

    /// Book the histograms
    void initializePlots(); // needed to override PlotBase
    StatusCode bookPlots();

    /// Finalize resolution and bias histograms
    void finalizePlots();

  private:

    std::string m_testType;
    std::string m_refType;
    unsigned int m_method{};
    
    enum Param {
      PT, ETA, D0, Z0, QOVERP, QOVERPT, THETA, PHI, Z0SIN, NPARAMS,
      NPARAMSOUT = 2 // Plot only vs pt and eta
    };

    /// get the track parameters
    template< typename PARTICLE >
    void getTrackParameters( const PARTICLE& p, float* params, float* errors );

    std::string m_paramName[ NPARAMS ] = {
      "pt", "eta", "d0", "z0", "qoverp", "ptqopt", "theta", "phi", "z0sin"
    };

    TH1* m_pull[ NPARAMS ]{};
    TH1* m_res[ NPARAMS ]{};
    TH1* m_sigma[ NPARAMS ]{};
    TH2* m_corr[ NPARAMS ]{}; // 2D correlation plots

    TH2* m_resHelper[ NPARAMS ][ NPARAMSOUT ]{};
    TH1* m_reswidth[ NPARAMS ][ NPARAMSOUT ]{};
    TH1* m_resmean[ NPARAMS ][ NPARAMSOUT ]{};

    TH2* m_pullHelper[ NPARAMS ][ NPARAMSOUT ]{};
    TH1* m_pullwidth[ NPARAMS ][ NPARAMSOUT ]{};
    TH1* m_pullmean[ NPARAMS ][ NPARAMSOUT ]{};
  };
  
}

#endif
