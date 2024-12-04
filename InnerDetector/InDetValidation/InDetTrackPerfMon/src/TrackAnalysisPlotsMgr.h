/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_TRACKANALYSISPLOTSMGR_H
#define INDETTRACKPERFMON_TRACKANALYSISPLOTSMGR_H

/**
 * @file    TrackAnalysisPlotsMgr.h
 * @brief   class to manage (book, fill) all the plots for the
 *          processed TrackAnalysis for tracking performance validation
 * @author  Marco Aparo <marco.aparo@cern.ch>
 * @date    19 June 2023
**/

/// xAOD includes
#include "xAODTracking/TrackParticle.h"
#include "xAODTruth/TruthParticle.h"

/// local includes
#include "InDetTrackPerfMon/ITrackAnalysisDefinitionSvc.h"
#include "PlotMgr.h"
#include "plots/TrackParametersPlots.h"
#include "plots/EfficiencyPlots.h"
#include "plots/OfflineElectronPlots.h"
#include "plots/ResolutionPlots.h"
#include "plots/FakeRatePlots.h"
#include "plots/DuplicateRatePlots.h"
#include "plots/HitsOnTracksPlots.h"
#include "plots/NtracksPlots.h"
#include "plots/VertexParametersPlots.h"

/// STD includes
#include <string>
#include <memory>
#include <vector>


namespace IDTPM {

  /// Forward-declaring internal classes
  class TrackAnalysisCollections;
  class ITrackMatchingLookup;

  class TrackAnalysisPlotsMgr : public PlotMgr {

  public :

    /// Constructor
    TrackAnalysisPlotsMgr( const std::string& dirName,
                           const std::string& anaTag,
                           const std::string& chain,
                           PlotMgr* pParent = nullptr );

    /// Destructor
    virtual ~TrackAnalysisPlotsMgr() = default;

    /// initialize
    StatusCode initialize();

    /// return members
    const std::string& anaTag() const { return m_anaTag; }
    const std::string& chain() const { return m_chain; }
    const std::string& directory() const { return m_directory; }

    /// General fill method
    StatusCode fill( TrackAnalysisCollections& trkAnaColls, float weight=1.0 );

    /// Fill all plots w.r.t. test tracks quantities for a specific
    /// collection (trigger tracks, offline tracks, truth particles)
    /// also including vertices for the test collection
    template< typename PARTICLE, typename VERTEX >
    StatusCode fillPlotsTest(
        const std::vector< const PARTICLE* >& particles,
        const ITrackMatchingLookup& matches,
        const std::vector< const VERTEX* >& vertices,
        float truthMu=0., float actualMu=0., float weight=1.0 );

    /// Fill all plots w.r.t. reference tracks quantities for a specific
    /// collection (trigger tracks, offline tracks, truth particles)
    /// also including vertices for the reference collection
    template< typename PARTICLE, typename VERTEX >
    StatusCode fillPlotsReference(
        const std::vector< const PARTICLE* >& particles,
        const ITrackMatchingLookup& matches,
        const std::vector< const VERTEX* >& vertices,
        float truthMu=0., float actualMu=0., float weight=1.0 );

    /// Fill efficiency plots w.r.t. truth (for EFTruthMatch only)
    StatusCode fillPlotsTruth(
        const std::vector< const xAOD::TrackParticle* >& testTracks,
        const std::vector< const xAOD::TrackParticle* >& refTracks,
        const std::vector< const xAOD::TruthParticle* >& truths,
        const ITrackMatchingLookup& matches,
        float truthMu=0., float actualMu=0., float weight=1.0 );

  private :

    std::string m_anaTag;
    std::string m_chain;
    std::string m_directory;

    /// TrackAnalysis definition service to "hold" the histograms configurations/flags
    SmartIF< ITrackAnalysisDefinitionSvc > m_trkAnaDefSvc;

    /// Plot categories
    /// plots w.r.t. test tracks parameters
    std::unique_ptr< TrackParametersPlots >  m_plots_trkParam_vsTest;
    std::unique_ptr< EfficiencyPlots >       m_plots_eff_vsTest;
    std::unique_ptr< EfficiencyPlots >       m_plots_tech_eff_vsTest;
    std::unique_ptr< HitsOnTracksPlots >     m_plots_hitsOnTrk_vsTest;
    std::unique_ptr< HitsOnTracksPlots >     m_plots_hitsOnMatchedTrk;
    std::unique_ptr< NtracksPlots >          m_plots_nTracks_vsTest;
    /// plots w.r.t. reference tracks parameters
    std::unique_ptr< TrackParametersPlots >  m_plots_trkParam_vsRef;
    std::unique_ptr< EfficiencyPlots >       m_plots_eff_vsRef;
    std::unique_ptr< EfficiencyPlots >       m_plots_tech_eff_vsRef;
    std::unique_ptr< HitsOnTracksPlots >     m_plots_hitsOnTrk_vsRef;
    std::unique_ptr< HitsOnTracksPlots >     m_plots_hitsOnMatchedTrk_vsRef;
    std::unique_ptr< NtracksPlots >          m_plots_nTracks_vsRef;
    /// plots w.r.t. efficiency plots w.r.t. truth (for EFTruthMatch only)
    std::unique_ptr< EfficiencyPlots >       m_plots_eff_vsTruth;
    std::unique_ptr< EfficiencyPlots >       m_plots_tech_eff_vsTruth;
    /// resolution plots
    std::unique_ptr< ResolutionPlots >       m_plots_resolution;
    /// fake rate plots (only when reference=truth)
    std::unique_ptr< FakeRatePlots >         m_plots_fakeRate;
    std::unique_ptr< FakeRatePlots >         m_plots_missingTruth;
    std::unique_ptr< HitsOnTracksPlots >     m_plots_hitsOnFakeTrk;
    std::unique_ptr< HitsOnTracksPlots >     m_plots_hitsOnUnlinkedTrk;
    /// duplicate rate plots
    std::unique_ptr< DuplicateRatePlots >    m_plots_duplRate;
    /// plots w.r.t. reference offline electron
    std::unique_ptr< OfflineElectronPlots >  m_plots_offEle;
    std::unique_ptr< OfflineElectronPlots >  m_plots_eff_vsOffEle;
    /// plots w.r.t. test vertices parameters
    std::unique_ptr< VertexParametersPlots >  m_plots_vtxParam_vsTest;
    std::unique_ptr< VertexParametersPlots >  m_plots_nVtxParam_vsTest;
    /// plots w.r.t. reference vertices parameters
    std::unique_ptr< VertexParametersPlots >  m_plots_vtxParam_vsRef;
    std::unique_ptr< VertexParametersPlots >  m_plots_nVtxParam_vsRef;

  }; // class TrackAnalysisPlotsMgr

} // namespace IDTPM

#endif // > !INDETTRACKPERFMON_TRACKANALYSISPLOTSMGR_H
