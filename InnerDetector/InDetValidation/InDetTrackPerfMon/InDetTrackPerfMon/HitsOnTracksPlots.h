/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKPERFMON_PLOTS_HITSONTRACKSPLOTS_H
#define INDETTRACKPERFMON_PLOTS_HITSONTRACKSPLOTS_H

/**
 * @file    HitsOnTracksPlots.h
 * @author  Marco Aparo <marco.aparo@cern.ch> 
 **/

/// local includes
#include "InDetTrackPerfMon/PlotMgr.h"


namespace IDTPM {

  class HitsOnTracksPlots : public PlotMgr {

  public:

    /// Constructor A
    /// for test_Hits-vs-reference plots
    /// e.g. offl_nPixelHits_vs_truth_eta
    HitsOnTracksPlots(
        PlotMgr* pParent,
        const std::string& dirName,
        const std::string& anaTag,
        const std::string& testType,
        const std::string& refType,
        bool isITk,
        bool doGlobalPlots = false,
        bool doTruthMuPlots = false,
        bool do1D = false, 
        bool doExpert = false );

    /// Constructor B
    /// for only one track type
    /// e.g. offl_nPixelHits & offl_nPixelHits_vs_offl_eta
    HitsOnTracksPlots(
        PlotMgr* pParent,
        const std::string& dirName,
        const std::string& anaTag,
        const std::string& trackType,
        bool isITk,
        bool doGlobalPlots = false,
        bool doTruthMuPlots = false,
        bool doExpert = false  );

    /// Destructor
    virtual ~HitsOnTracksPlots() = default;

    /// Book the histograms
    void initializePlots(); // needed to override PlotBase
    StatusCode bookPlots();

    /// Dedicated fill method A (for tracks and/or truth particles)
    /// for test_Hits-vs-reference plots
    template< typename PTEST, typename PREF=PTEST >
    StatusCode fillPlots(
        const PTEST& ptest, const PREF& pref,
        float truthMu, float actualMu, float weight );

    /// Dedicated fill method B (for tracks and/or truth particles)
    /// for only one track type
    template< typename PARTICLE >
    StatusCode fillPlots(
        const PARTICLE& particle,
        float truthMu, float actualMu, float weight );

    /// Print out final stats on histograms
    void finalizePlots();

  private:

    std::string m_testType;
    std::string m_refType;
    bool m_isITk{};
    bool m_doGlobalPlots{};
    bool m_doTruthMuPlots{};
    bool m_do1D{};
    bool m_doExpert{};

    enum HitParam {
        NInnerMostPixelHits,
        NInnerMostPixelEndcapHits,
        NNextToInnerMostPixelHits,
        NNextToInnerMostPixelEndcapHits,
        NInnerMostPixelSharedHits,
        NInnerMostPixelSharedEndcapHits,
        NPixelHits,
        NPixelHoles,
        NPixelSharedHits,
        PixeldEdx,
        NSCTHits,
        NSCTHoles,
        NSCTSharedHits,
        /// for greater detail level plots
        NInnerMostPixelOutliers,
        NInnerMostPixelEndcapOutliers,
        NInnerMostPixelSplitHits,
        NInnerMostPixelSplitEndcapHits,
        NExpectedInnerMostPixelHits,
        NExpectedNextToInnerMostPixelHits,
        NPixelOutliers,
        NPixelContribLayers,
        NPixelSplitHits,
        NPixelGangedHits,
        NPixelGangedHitsFlaggedFakes,
        NPixelDeadSensors,
        NSCTOutliers,
        NSCTDoubleHoles,
        NSCTDeadSensors,
        NHITPARAMSTOT,
        NHITPARAMSBASE = 13
    };
    unsigned int m_NHITPARAMS;

    std::string m_hitParamName[ NHITPARAMSTOT ] = {
        "nInnerMostPixelHits",
        "nInnerMostPixelEndcapHits",
        "nNextToInnerMostPixelHits",
        "nNextToInnerMostPixelEndcapHits",
        "nInnerMostPixelSharedHits",
        "nInnerMostPixelSharedEndcapHits",
        "nPixelHits",
        "nPixelHoles",
        "nPixelSharedHits",
        "pixeldEdx",
        "nSCTHits",
        "nSCTHoles",
        "nSCTSharedHits",
        /// for greater detail level plots
        "nInnerMostPixelOutliers",
        "nInnerMostPixelEndcapOutliers",
        "nInnerMostPixelSplitHits",
        "nInnerMostPixelSplitEndcapHits",
        "nExpectedInnerMostPixelHits",
        "nExpectedNextToInnerMostPixelHits",
        "nPixelOutliers",
        "nPixelContribLayers",
        "nPixelSplitHits",
        "nPixelGangedHits",
        "nPixelGangedHitsFlaggedFakes",
        "nPixelDeadSensors",
        "nSCTOutliers",
        "nSCTDoubleHoles",
        "nSCTDeadSensors"
    };

    enum Run3HitParam {
        NTRTHits,
        NTRTHitsXe,
        NTRTHitsAr,
        NTRTHighThresholdHits,
        NTRTHighThresholdHitsXe,
        NTRTHighThresholdHitsAr,
        /// for greater detail level plots
        NTRTOutliers,
        NTRTHighThresholdOutliers,
        NRUN3HITPARAMSTOT,
        NRUN3HITPARAMSBASE = 6 
    };
    unsigned int m_NRUN3HITPARAMS;

    std::string m_run3HitParamName[ NRUN3HITPARAMSTOT ] = {
        "nTRTHits",
        "nTRTHitsXe",
        "nTRTHitsAr",
        "nTRTHighThresholdHits",
        "nTRTHighThresholdHitsXe",
        "nTRTHighThresholdHitsAr",
        /// for greater detail level plots
        "nTRTOutliers",
        "nTRTHighThresholdOutliers"
    };

    enum Param { PT, ETA, PHI, NPARAMS };
    std::string m_paramName[ NPARAMS ] = { "pt", "eta", "phi" };

    enum ParamMu { TRUTHMU, ACTUALMU, NPARAMSMU };
    std::string m_paramMuName[ NPARAMSMU ] = { "truthMu", "actualMu" };

    /// 1D plots
    TH1* m_hits[ NHITPARAMSTOT ]{};
    TH1* m_hitsRun3[ NRUN3HITPARAMSTOT ]{};

    /// TProfile plots vs NPARAMS
    TProfile* m_hits_vs[ NHITPARAMSTOT ][ NPARAMS ]{};
    TProfile* m_hitsRun3_vs[ NRUN3HITPARAMSTOT ][ NPARAMS ]{};

    /// TProfile2D plots vs NPARAMS vs NPARAMS
    TProfile2D* m_hits_vs2D[ NHITPARAMSTOT ][ NPARAMS ][ NPARAMS ]{};
    TProfile2D* m_hitsRun3_vs2D[ NRUN3HITPARAMSTOT ][ NPARAMS ][ NPARAMS ]{};

    /// TProfile plots vs mu (truth && actual)
    TProfile* m_hits_vsMu[ NHITPARAMSTOT ][ NPARAMSMU ]{};
    TProfile* m_hitsRun3_vsMu[ NRUN3HITPARAMSTOT ][ NPARAMSMU ]{};

    /// TProfile2D plots vs mu (truth && actual) vs NPARAMS
    TProfile2D* m_hits_vsMu_vs[ NHITPARAMSTOT ][ NPARAMSMU ][ NPARAMS ]{};
    TProfile2D* m_hitsRun3_vsMu_vs[ NRUN3HITPARAMSTOT ][ NPARAMSMU ][ NPARAMS ]{};

  }; // class HitsOnTracksPlots

} // namespace IDTPM

#endif // > ! INDETTRACKPERFMON_PLOTS_HITSONTRACKSPLOTS_H
