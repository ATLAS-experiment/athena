#
#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#

"""
@file ITkGlobalTrackMonAlgCfg.py
@author Leonid Serkin and Per Johansson
@date April 2020
@brief Configuration for Run 3 based on InDetGlobalTrackMonTool.cxx
"""

from math import pi as M_PI
from AthenaConfiguration.Enums import BeamType
from AthenaConfiguration.ComponentFactory import CompFactory

def HistoITkGlobalTrackMonAlgCfg(helper, alg, flags=None):

    # ITk extends tracking acceptance from |eta|<2.5 (Run 1-3 ID) to
    # |eta|<4.0; phase-II pile-up (mu=200) also drives much higher
    # baseline track multiplicities per event.  Widen the relevant
    # axes when the ITk geometry is active.
    isITk = bool(flags) and flags.Detector.GeometryITk

    # values
    m_nBinsEta = 80 if isITk else 50
    m_nBinsPhi = 50
    m_trackBin = 300 if isITk else 150
    m_c_etaRange = 4.0 if isITk else 2.5
    m_c_range_LB = 3000
    m_trackMax = 3000 if isITk else 150

    # this creates a "trackGroup" called "alg" which will put its histograms into the subdirectory "Track"
    trackGroup = helper.addGroup(alg, 'Track')
    pathtrack = '/ITkGlobal/Track'
    pathhits = '/ITkGlobal/Hits'
    pathTIDE = '/ITkGlobal/TIDE'


    varName = 'm_nBase;nCOMBtrks' #done
    title = 'Track multiplicity (baseline tracks);Track multiplicity;Events'
    trackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_trackBin, xmin=0.5, xmax=m_trackMax + 0.5)

    # Eta-phi maps
    varName = 'm_eta_perigee,m_phi_perigee;Trk_Base_eta_phi' #done
    title = 'Distribution of eta vs phi for combined tracks passing Loose Primary selection;#eta;#phi_{0}'
    trackGroup.defineHistogram(varName, type='TH2F', path=pathtrack, title=title, xbins=m_nBinsEta, xmin=-m_c_etaRange, xmax=m_c_etaRange, ybins=m_nBinsPhi, ymin=-M_PI, ymax=M_PI)

    varName = 'm_eta_perigee_loose,m_phi_perigee_loose;Trk_Loose_eta_phi' #done
    title = 'Distribution of eta vs phi for combined tracks passing Loose selection;#eta;#phi_{0}'
    trackGroup.defineHistogram(varName, type='TH2F', path=pathtrack, title=title, xbins=m_nBinsEta, xmin=-m_c_etaRange, xmax=m_c_etaRange, ybins=m_nBinsPhi, ymin=-M_PI, ymax=M_PI)

    varName = 'm_eta_perigee,m_phi_perigee,m_track_pass_tight;Trk_Tight_eta_phi_ratio' #done
    title = 'Distribution of eta vs phi for combined tracks passing Tight selection;#eta;#phi_{0}'
    trackGroup.defineHistogram(varName, type='TProfile2D', path=pathtrack, title=title, xbins=m_nBinsEta, xmin=-m_c_etaRange, xmax=m_c_etaRange, ybins=m_nBinsPhi, ymin=-M_PI, ymax=M_PI)

    varName = 'm_eta_perigee,m_phi_perigee,m_NextToInnermostPixelLayerHit;Trk_noBLhit_eta_phi_ratio' #done
    title = 'Eta-phi of tracks with no innermost pixel layer hit but a hit is expected, ratio to total tracks;#eta;#phi_{0}'
    trackGroup.defineHistogram(varName, type='TProfile2D', path=pathtrack, title=title, xbins=m_nBinsEta, xmin=-m_c_etaRange, xmax=m_c_etaRange, ybins=m_nBinsPhi, ymin=-M_PI, ymax=M_PI)

    varName = 'm_eta_perigee,m_phi_perigee,m_pixHits;Trk_nPIXhits_eta_phi' #done
    title = 'Number of PIX hits per track, eta-phi profile;#eta;#phi'
    trackGroup.defineHistogram(varName, type='TProfile2D', path=pathhits, title=title, xbins=m_nBinsEta, xmin=-m_c_etaRange, xmax=m_c_etaRange, ybins=m_nBinsPhi, ymin=-M_PI, ymax=M_PI)

    varName = 'm_eta_perigee,m_phi_perigee,m_stripHits;Trk_nStriphits_eta_phi' #done
    title = 'Number of Strip hits per track, eta-phi profile;#eta;#phi'
    trackGroup.defineHistogram(varName, type='TProfile2D', path=pathhits, title=title, xbins=m_nBinsEta, xmin=-m_c_etaRange, xmax=m_c_etaRange, ybins=m_nBinsPhi, ymin=-M_PI, ymax=M_PI)

    varName = 'm_lb,m_pixHits;trk_nPIXhits_LB' #done
    title = 'Average number of PIX hits by LB;LB;Average number of hits in LB'
    trackGroup.defineHistogram(varName, type='TProfile', path=pathtrack, title=title, xbins=m_c_range_LB, xmin=0, xmax=m_c_range_LB)

    varName = 'm_lb,m_stripHits;trk_nStriphits_LB' #done
    title = 'Average number of Strip hits by LB;LB;Average number of hits in LB'
    trackGroup.defineHistogram(varName, type='TProfile', path=pathtrack, title=title, xbins=m_c_range_LB, xmin=0, xmax=m_c_range_LB)

    varName = 'm_eta_perigee,m_phi_perigee,m_numberOfPixelDeadSensors;Trk_nPIXdisabled_eta_phi' # done
    title = 'Number of PIX disabled detector elements per track, eta-phi profile;#eta;#phi'
    trackGroup.defineHistogram(varName, type='TProfile2D', path=pathhits, title=title, xbins=m_nBinsEta, xmin=-m_c_etaRange, xmax=m_c_etaRange, ybins=m_nBinsPhi, ymin=-M_PI, ymax=M_PI)

    varName = 'm_eta_perigee,m_phi_perigee,m_numberOfStripDeadSensors;Trk_nStripdisabled_eta_phi' #done
    title = 'Number of Strip disabled detector elements per track, eta-phi profile;#eta;#phi'
    trackGroup.defineHistogram(varName, type='TProfile2D', path=pathhits, title=title, xbins=m_nBinsEta, xmin=-m_c_etaRange, xmax=m_c_etaRange, ybins=m_nBinsPhi, ymin=-M_PI, ymax=M_PI)

    varName = 'm_eta_perigee,m_phi_perigee,m_numberOfPixelSharedHits;Trk_nPixShared_eta_phi' #done
    title = 'Number of Pixel shared hits per track, eta-phi profile;#eta;#phi'
    trackGroup.defineHistogram(varName, type='TProfile2D', path=pathhits, title=title, xbins=m_nBinsEta, xmin=-m_c_etaRange, xmax=m_c_etaRange, ybins=m_nBinsPhi, ymin=-M_PI, ymax=M_PI)

    varName = 'm_eta_perigee,m_phi_perigee,m_numberOfPixelSplitHits;Trk_nPixSplit_eta_phi' #done
    title = 'Number of Pixel split hits per track, eta-phi profile;#eta;#phi'
    trackGroup.defineHistogram(varName, type='TProfile2D', path=pathhits, title=title, xbins=m_nBinsEta, xmin=-m_c_etaRange, xmax=m_c_etaRange, ybins=m_nBinsPhi, ymin=-M_PI, ymax=M_PI)

    varName = 'm_eta_perigee,m_phi_perigee,m_numberOfStripSharedHits;Trk_nStripShared_eta_phi' #done
    title = 'Number of Strip shared hits per track, eta-phi profile;#eta;#phi'
    trackGroup.defineHistogram(varName, type='TProfile2D', path=pathhits, title=title, xbins=m_nBinsEta, xmin=-m_c_etaRange, xmax=m_c_etaRange, ybins=m_nBinsPhi, ymin=-M_PI, ymax=M_PI)

    varName = 'm_eta_perigee,m_phi_perigee,m_numberOfPixelHoles;Trk_nPixHoles_eta_phi' #done
    title = 'Number of Pixel holes per track, eta-phi profile;#eta;#phi'
    trackGroup.defineHistogram(varName, type='TProfile2D', path=pathhits, title=title, xbins=m_nBinsEta, xmin=-m_c_etaRange, xmax=m_c_etaRange, ybins=m_nBinsPhi, ymin=-M_PI, ymax=M_PI)

    varName = 'm_eta_perigee,m_phi_perigee,m_numberOfStripHoles;Trk_nStripHoles_eta_phi' #done
    title = 'Number of Strip holes per track, eta-phi profile;#eta;#phi'
    trackGroup.defineHistogram(varName, type='TProfile2D', path=pathhits, title=title, xbins=m_nBinsEta, xmin=-m_c_etaRange, xmax=m_c_etaRange, ybins=m_nBinsPhi, ymin=-M_PI, ymax=M_PI)

    varName = 'm_lb,m_nBase_LB;Trk_nBase_LB' #done
    title = 'Average number of baseline tracks per event in LB;LB number;Average number of loose primary tracks per event in LB'
    trackGroup.defineHistogram(varName, type='TProfile', path=pathtrack, title=title, xbins=m_c_range_LB, xmin=0, xmax=m_c_range_LB)

    varName = 'm_lb,m_nTight_LB;Trk_nTight_LB' #done
    title = 'Average number of tight tracks per event in LB;LB number;Average number of tight tracks per event in LB'
    trackGroup.defineHistogram(varName, type='TProfile', path=pathtrack, title=title, xbins=m_c_range_LB, xmin=0, xmax=m_c_range_LB)

    varName = 'm_lb,m_nNoBL_LB;Trk_noBLhits_LB' #done
    title = 'Average number of tracks with missing innermost pixel layer hit per event in LB;LB number;Average number of tracks with missing innermost pixel layer hit per event in LB'
    trackGroup.defineHistogram(varName, type='TProfile', path=pathtrack, title=title, xbins=m_c_range_LB, xmin=0, xmax=m_c_range_LB)

    varName = 'm_lb,m_NoBL_LB;Trk_noBLhits_frac_LB' #done
    title = 'Fraction of tracks with missing innermost pixel layer hit per event in LB;LB number;Fraction of tracks with missing innermost pixel layer hit per event in LB'
    trackGroup.defineHistogram(varName, type='TProfile', path=pathtrack, title=title, xbins=m_c_range_LB, xmin=0, xmax=m_c_range_LB)

# TIDE Histogram Group
    varName = 'm_jetassocdR,m_jetassocd0Reso;Trk_jetassoc_d0_dr' 
    title = 'IP resolution per ghost associated track vs #DeltaR of track and jet;#DeltaR of track and jet;IP resolution per ghost associated track'
    trackGroup.defineHistogram(varName, type='TProfile', path=pathTIDE, title=title, xbins=20, xmin=0, xmax=0.4)

    varName = 'm_jetassocdR,m_jetassocz0Reso;Trk_jetassoc_z0_dr'
    title = 'IP resolution per ghost associated track vs #DeltaR of track and jet;#DeltaR of track and jet;IP resolution per ghost associated track'
    trackGroup.defineHistogram(varName, type='TProfile', path=pathTIDE, title=title, xbins=20, xmin=0, xmax=0.4)

    varName = 'm_lb,m_jetassocIPReso;Trk_jetassoc_ip_reso_lb'
    title = 'IP resolution per ghost associated track vs LB;LB;IP resolution per ghost associated track'
    trackGroup.defineHistogram(varName, type='TProfile', path=pathTIDE, title=title, xbins=m_c_range_LB, xmin=0, xmax=m_c_range_LB)

    varName = 'm_pixSplitdR,m_pixSplitFrac;Trk_jetassoc_split_pix_dr'
    title = 'Fraction of split Pixel hits per ghost associated track vs #DeltaR of track and jet;#DeltaR;Fraction'
    trackGroup.defineHistogram(varName, type='TProfile', path=pathTIDE, title=title, xbins=20, xmin=0, xmax=0.4)

    varName = 'm_lb,m_pixSplitFrac;Trk_jetassoc_split_pix_lb'
    title = 'Fraction of split Pixel hits vs LB;LB;Fraction'
    trackGroup.defineHistogram(varName, type='TProfile', path=pathTIDE, title=title, xbins=m_c_range_LB, xmin=0, xmax=m_c_range_LB)

    varName = 'm_pixShareddR,m_pixSharedFrac;Trk_jetassoc_shared_pix_dr'
    title = 'Fraction of shared Pixel hits per ghost associated track vs #DeltaR of track and jet;#DeltaR;Fraction'
    trackGroup.defineHistogram(varName, type='TProfile', path=pathTIDE, title=title, xbins=20, xmin=0, xmax=0.4)

    varName = 'm_lb,m_pixSharedFrac;Trk_jetassoc_shared_pix_lb'
    title = 'Fraction of shared Pixel hits vs LB;LB;Fraction'
    trackGroup.defineHistogram(varName, type='TProfile', path=pathTIDE, title=title, xbins=m_c_range_LB, xmin=0, xmax=m_c_range_LB)

# end histograms


def ITkGlobalTrackMonAlgCfg(helper, acc,
                              flags, name="ITkGlobalTrackMonAlg", **kwargs):

    from InDetTrackSelectionTool.InDetTrackSelectionToolConfig import (
        InDetTrackSelectionTool_TightPrimary_TrackTools_Cfg,
        InDetTrackSelectionTool_Loose_Cfg)

    if "TrackSelectionTool" not in kwargs:
        if (flags.Beam.Type is BeamType.Cosmics or flags.Beam.Energy < 500000 or
            flags.Reco.EnableHI):
            kwargs.setdefault("TrackSelectionTool", acc.popToolsAndMerge(
                InDetTrackSelectionTool_Loose_Cfg(flags, name='LowECMTrackSelectionTool',
                                                  minPt = 500)))
        else:
            kwargs.setdefault("TrackSelectionTool", acc.popToolsAndMerge(
                InDetTrackSelectionTool_TightPrimary_TrackTools_Cfg(
                    flags,
                    maxNPixelHoles = 1, # Default for TightPrimary is 0
                    minPt = 5000)))

    if "Tight_TrackSelectionTool" not in kwargs:
        kwargs.setdefault("Tight_TrackSelectionTool", acc.popToolsAndMerge(
            InDetTrackSelectionTool_TightPrimary_TrackTools_Cfg(
                flags,
                minPt = 5000)))

    if "Loose_TrackSelectionTool" not in kwargs:
        kwargs.setdefault("Loose_TrackSelectionTool", acc.popToolsAndMerge(
            InDetTrackSelectionTool_Loose_Cfg(
                flags,
                minPt = 1000)))

    if "TrackToVertexIPEstimator" not in kwargs:
        from TrkConfig.TrkVertexFitterUtilsConfig import TrackToVertexIPEstimatorCfg
        kwargs.setdefault("TrackToVertexIPEstimator", acc.popToolsAndMerge(
            TrackToVertexIPEstimatorCfg(flags)))

    from AthenaMonitoring.FilledBunchFilterToolConfig import FilledBunchFilterToolCfg
    from AthenaMonitoring.AtlasReadyFilterConfig import AtlasReadyFilterCfg

        # TIDE relies on Pixel split-hit info via ghost-associated tracks
        # in jets; keep enabled (Phase II reco does provide pixel splits).

    monAlg = helper.addAlgorithm(
        CompFactory.ITkGlobalTrackMonAlg, name,
        addFilterTools = [FilledBunchFilterToolCfg(flags), AtlasReadyFilterCfg(flags)],
        **kwargs)

    HistoITkGlobalTrackMonAlgCfg(helper, monAlg, flags)
    return
