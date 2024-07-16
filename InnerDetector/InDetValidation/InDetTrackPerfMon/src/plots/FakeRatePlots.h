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
#include "../PlotMgr.h"


namespace IDTPM {

  class FakeRatePlots : public PlotMgr {

  public:

    /// Constructor
    FakeRatePlots(
        PlotMgr* pParent,
        const std::string& dirName,
        const std::string& anaTag,
        const std::string& trackType );

    /// Destructor
    virtual ~FakeRatePlots() = default;

    /// Dedicated fill method (for tracks and/or truth particles)
    template< typename PARTICLE >
    StatusCode fillPlots(
        const PARTICLE& particle,
        bool isFake, float weight );

    /// Book the histograms
    void initializePlots(); // needed to override PlotBase
    StatusCode bookPlots();

    /// Print out final stats on histograms
    void finalizePlots();

  private:

    std::string m_trackType;

    TEfficiency* m_fakerate_vs_pt;
    TEfficiency* m_fakerate_vs_eta;
    TEfficiency* m_fakerate_vs_phi;
    TEfficiency* m_fakerate_vs_d0;
    TEfficiency* m_fakerate_vs_z0;
    /// TODO - include more plots

  }; // class FakeRatePlots

} // namespace IDTPM

#endif // > ! INDETTRACKPERFMON_PLOTS_FAKERATEPLOTS_H
