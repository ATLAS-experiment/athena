/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_PLOTS_DUPLICATERATEPLOTS_H
#define INDETTRACKPERFMON_PLOTS_DUPLICATERATEPLOTS_H

/**
 * @file    DuplicateRatePlots.h
 * @author  Marco Aparo <Marco.Aparo@cern.ch>
 **/

/// local includes
#include "InDetTrackPerfMon/PlotMgr.h"


namespace IDTPM {

  class DuplicateRatePlots : public PlotMgr {

  public:

    /// Constructor
    DuplicateRatePlots(
        PlotMgr* pParent,
        const std::string& dirName,
        const std::string& anaTag,
        const std::string& trackType,
        bool doGlobalPlots = false,
        bool doTruthMuPlots = false );

    /// Destructor
    virtual ~DuplicateRatePlots() = default;

    /// Dedicated fill method (for tracks and/or truth particles)
    template< typename PARTICLE >
    StatusCode fillPlots(
        const PARTICLE& particle,
        unsigned int nMatched,
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
    
    TEfficiency*  m_duplrate_vs_incl{};
    TEfficiency*  m_duplrate_vs_pt{};
    TEfficiency*  m_duplrate_vs_logPt{};
    TEfficiency*  m_duplrate_vs_eta{};
    TEfficiency*  m_duplrate_vs_phi{};
    TEfficiency*  m_duplrate_vs_d0{};
    TEfficiency*  m_duplrate_vs_z0{};
    TEfficiency*  m_duplrate_vs_truthMu{};
    TEfficiency*  m_duplrate_vs_actualMu{};

    TProfile*     m_duplnum_vs_pt{};
    TProfile*     m_duplnum_vs_eta{};
    TProfile*     m_duplnum_vs_phi{};
    TProfile*     m_duplnum_vs_d0{};
    TProfile*     m_duplnum_vs_z0{};
    TProfile*     m_duplnum_vs_truthMu{};
    TProfile*     m_duplnum_vs_actualMu{};

    TProfile*     m_duplnum_nonzero_vs_pt{};
    TProfile*     m_duplnum_nonzero_vs_eta{};
    TProfile*     m_duplnum_nonzero_vs_phi{};
    TProfile*     m_duplnum_nonzero_vs_d0{};
    TProfile*     m_duplnum_nonzero_vs_z0{};
    TProfile*     m_duplnum_nonzero_vs_truthMu{};
    TProfile*     m_duplnum_nonzero_vs_actualMu{};

  }; // class DuplicateRatePlots

} // namespace IDTPM

#endif // > ! INDETTRACKPERFMON_PLOTS_DUPLICATERATEPLOTS_H
