/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_PLOTS_VERTEXPARAMETERSPLOTS_H
#define INDETTRACKPERFMON_PLOTS_VERTEXPARAMETERSPLOTS_H

/**
 * @file    VertexParametersPlots.h
 * @author  Marco Aparo <marco.aparo@cern.ch> 
 **/

/// local includes
#include "InDetTrackPerfMon/PlotMgr.h"


namespace IDTPM {

  class VertexParametersPlots : public PlotMgr {

  public:

    /// Constructor
    VertexParametersPlots(
        PlotMgr* pParent,
        const std::string& dirName,
        const std::string& anaTag,
        const std::string& vertexType,
        bool doTrackPlots = false,
        bool doGlobalPlots = false,
        bool doTruthMuPlots = false );

    /// Destructor
    virtual ~VertexParametersPlots() = default;

    /// Book the histograms
    void initializePlots(); // needed to override PlotBase
    StatusCode bookPlots();

    /// Dedicated fill method (for reco and/or truth vertices)
    template< typename VERTEX, typename PARTICLE >
    StatusCode fillPlots(
        const VERTEX& vertex,
        const std::vector< const PARTICLE* >& associatedTracks,
        const std::vector< float >& associatedTrackWeights,
        float weight );

    /// Dedicated fill method (for vertex multiplicity vs pile-up)
    StatusCode fillPlots(
        int nVertices,
        float truthMu,
        float actualMu,
        float weight );

    /// Print out final stats on histograms
    void finalizePlots();

  private:

    std::string m_vertexType;
    bool m_doTrackPlots{};
    bool m_doGlobalPlots{};
    bool m_doTruthMuPlots{};

    /// vertex parameters plots
    TH1* m_vtx_x{};
    TH1* m_vtx_y{};
    TH1* m_vtx_z{};
    TH1* m_vtx_time{};
    TH1* m_vtx_x_err{};
    TH1* m_vtx_y_err{};
    TH1* m_vtx_z_err{};
    TH1* m_vtx_time_err{};
    TH1* m_vtx_chi2OverNdof{};
    TH1* m_vtx_type{};

    /// vertex-associated tracks plots
    TH1* m_vtx_nTracks{};
    TH1* m_vtx_track_weight{};
    TH1* m_vtx_track_pt{};
    TH1* m_vtx_track_eta{};
    TH1* m_vtx_track_nSiHits{};
    TH1* m_vtx_track_nSiHoles{};
    TH1* m_vtx_track_d0{};
    TH1* m_vtx_track_z0{};
    TH1* m_vtx_track_d0_err{};
    TH1* m_vtx_track_z0_err{};

    /// vertex multiplicity vs pileup plots
    TH2* m_nVtx_vs_truthMu_2D{};
    TH2* m_nVtx_vs_actualMu_2D{};
    TProfile* m_nVtx_vs_truthMu{};
    TProfile* m_nVtx_vs_actualMu{};

  }; // class VertexParametersPlots

} // namespace IDTPM

#endif // > ! INDETTRACKPERFMON_PLOTS_VERTEXPARAMETERSPLOTS_H
