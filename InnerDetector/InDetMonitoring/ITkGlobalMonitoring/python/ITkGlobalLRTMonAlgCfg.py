#
#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#

"""
@file ITkGlobalLRTMonAlgCfg.py
@author Leonid Serkin and Per Johansson
@date July 2021
@brief Configuration for Run 3 based on InDetGlobalLRTMonTool.cxx
"""

from math import pi as M_PI
from AthenaConfiguration.ComponentFactory import CompFactory

def HistoITkGlobalLRTMonAlgCfg(helper, alg, flags=None):

    # ITk values: tracking acceptance |eta| < 4.0, Phase-II pile-up
    # (mu=200) track multiplicities.

    # values
    m_nBinsEta = 80
    m_nBinsPhi = 50
    m_trackBin = 200
    m_c_etaRange = 4.0
    m_c_range_LB = 3000

    # this creates a "lrtGroup" called "alg" which will put its histograms into the subdirectory "Track"
    lrtGroup = helper.addGroup(alg, 'LRT')
    pathtrack = '/ITkGlobal/LRTTrack'
    pathhits = '/ITkGlobal/LRTHits'


    varName = 'm_nBase;nCOMBtrks'
    title = 'Track multiplicity (baseline tracks);Track multiplicity;Events'
    lrtGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_trackBin, xmin=0, xmax=200)

    varName = 'm_d0_perigee;trkD' 
    title = 'd_{0} ;d_{0} (mm)'
    lrtGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=20, xmin=-325, xmax=325)

    varName = 'm_z0_perigee;trkZ' 
    title = 'z_{0} ;z_{0} (mm);'
    lrtGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=20, xmin=-600, xmax=600)


    varName = 'm_radius_perigee;trkR'
    title = 'Radius of first hit ; R (m);'
    lrtGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=20, xmin=-600, xmax=600)


    varName = 'm_eta_perigee;trkEta'
    title = '#eta of all tracks;#eta;'
    lrtGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_nBinsEta, xmin=-m_c_etaRange, xmax=m_c_etaRange)


    varName = 'm_phi_perigee;trkPhi'
    title = ' #varphi of all tracks; #varphi (rad);'
    lrtGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_nBinsPhi, xmin=-3.2, xmax=3.2)

    varName = 'm_trkPt;trkPt'
    title = 'Track Pt;p_{T} (GeV);'
    lrtGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=100, xmin=-1, xmax=200)


    # Eta-phi maps
    varName = 'm_eta_perigee,m_phi_perigee;Trk_Base_eta_phi' 
    title = 'Distribution of eta vs phi for LRT tracks;#eta,#phi_{0}'
    lrtGroup.defineHistogram(varName, type='TH2F', path=pathtrack, title=title, xbins=m_nBinsEta, xmin=-m_c_etaRange, xmax=m_c_etaRange, ybins=m_nBinsPhi, ymin=-M_PI, ymax=M_PI)

    varName = 'm_eta_perigee,m_phi_perigee,m_track_pass_tight;Trk_Tight_eta_phi_ratio'
    title = 'Distribution of eta vs phi for combined tracks passing Tight selection;#eta;#phi_{0}'
    lrtGroup.defineHistogram(varName, type='TProfile2D', path=pathtrack, title=title, xbins=m_nBinsEta, xmin=-m_c_etaRange, xmax=m_c_etaRange, ybins=m_nBinsPhi, ymin=-M_PI, ymax=M_PI)

    varName = 'm_eta_perigee,m_phi_perigee,m_NextToInnermostPixelLayerHit;Trk_noBLhit_eta_phi_ratio'
    title = 'Eta-phi of tracks with no innermost pixel layer hit but a hit is expected, ratio to total tracks;#eta;#phi_{0}'
    lrtGroup.defineHistogram(varName, type='TProfile2D', path=pathtrack, title=title, xbins=m_nBinsEta, xmin=-m_c_etaRange, xmax=m_c_etaRange, ybins=m_nBinsPhi, ymin=-M_PI, ymax=M_PI)

    varName = 'm_eta_perigee,m_phi_perigee,m_pixHits;Trk_nPIXhits_eta_phi'
    title = 'Number of PIX hits per track, eta-phi profile;#eta;#phi'
    lrtGroup.defineHistogram(varName, type='TProfile2D', path=pathhits, title=title, xbins=m_nBinsEta, xmin=-m_c_etaRange, xmax=m_c_etaRange, ybins=m_nBinsPhi, ymin=-M_PI, ymax=M_PI)

    varName = 'm_eta_perigee,m_phi_perigee,m_stripHits;Trk_nStriphits_eta_phi'
    title = 'Number of Strip hits per track, eta-phi profile;#eta;#phi'
    lrtGroup.defineHistogram(varName, type='TProfile2D', path=pathhits, title=title, xbins=m_nBinsEta, xmin=-m_c_etaRange, xmax=m_c_etaRange, ybins=m_nBinsPhi, ymin=-M_PI, ymax=M_PI)


    varName = 'm_lb,m_d0_perigee;trk_d0_LB'
    title = 'D0'
    lrtGroup.defineHistogram(varName, type='TProfile', path=pathtrack, title=title, xbins=m_c_range_LB, xmin=0, xmax=m_c_range_LB)

    varName = 'm_lb,m_pixHits;trk_nPIXhits_LB'
    title = 'Average number of PIX hits by LB;LB;Average number of hits in LB'
    lrtGroup.defineHistogram(varName, type='TProfile', path=pathtrack, title=title, xbins=m_c_range_LB, xmin=0, xmax=m_c_range_LB)

    varName = 'm_lb,m_stripHits;trk_nStriphits_LB'
    title = 'Average number of Strip hits by LB;LB;Average number of hits in LB'
    lrtGroup.defineHistogram(varName, type='TProfile', path=pathtrack, title=title, xbins=m_c_range_LB, xmin=0, xmax=m_c_range_LB)

    varName = 'm_eta_perigee,m_phi_perigee,m_numberOfPixelDeadSensors;Trk_nPIXdisabled_eta_phi' # done
    title = 'Number of PIX disabled detector elements per track, eta-phi profile;#eta;#phi'
    lrtGroup.defineHistogram(varName, type='TProfile2D', path=pathhits, title=title, xbins=m_nBinsEta, xmin=-m_c_etaRange, xmax=m_c_etaRange, ybins=m_nBinsPhi, ymin=-M_PI, ymax=M_PI)

    varName = 'm_eta_perigee,m_phi_perigee,m_numberOfStripDeadSensors;Trk_nStripdisabled_eta_phi'
    title = 'Number of Strip disabled detector elements per track, eta-phi profile;#eta;#phi'
    lrtGroup.defineHistogram(varName, type='TProfile2D', path=pathhits, title=title, xbins=m_nBinsEta, xmin=-m_c_etaRange, xmax=m_c_etaRange, ybins=m_nBinsPhi, ymin=-M_PI, ymax=M_PI)

    varName = 'm_eta_perigee,m_phi_perigee,m_numberOfPixelSharedHits;Trk_nPixShared_eta_phi'
    title = 'Number of Pixel shared hits per track, eta-phi profile;#eta;#phi'
    lrtGroup.defineHistogram(varName, type='TProfile2D', path=pathhits, title=title, xbins=m_nBinsEta, xmin=-m_c_etaRange, xmax=m_c_etaRange, ybins=m_nBinsPhi, ymin=-M_PI, ymax=M_PI)

    varName = 'm_eta_perigee,m_phi_perigee,m_numberOfPixelSplitHits;Trk_nPixSplit_eta_phi'
    title = 'Number of Pixel split hits per track, eta-phi profile;#eta;#phi'
    lrtGroup.defineHistogram(varName, type='TProfile2D', path=pathhits, title=title, xbins=m_nBinsEta, xmin=-m_c_etaRange, xmax=m_c_etaRange, ybins=m_nBinsPhi, ymin=-M_PI, ymax=M_PI)

    varName = 'm_eta_perigee,m_phi_perigee,m_numberOfStripSharedHits;Trk_nStripShared_eta_phi'
    title = 'Number of Strip shared hits per track, eta-phi profile;#eta;#phi'
    lrtGroup.defineHistogram(varName, type='TProfile2D', path=pathhits, title=title, xbins=m_nBinsEta, xmin=-m_c_etaRange, xmax=m_c_etaRange, ybins=m_nBinsPhi, ymin=-M_PI, ymax=M_PI)

    varName = 'm_eta_perigee,m_phi_perigee,m_numberOfPixelHoles;Trk_nPixHoles_eta_phi'
    title = 'Number of Pixel holes per track, eta-phi profile;#eta;#phi'
    lrtGroup.defineHistogram(varName, type='TProfile2D', path=pathhits, title=title, xbins=m_nBinsEta, xmin=-m_c_etaRange, xmax=m_c_etaRange, ybins=m_nBinsPhi, ymin=-M_PI, ymax=M_PI)

    varName = 'm_eta_perigee,m_phi_perigee,m_numberOfStripHoles;Trk_nStripHoles_eta_phi'
    title = 'Number of Strip holes per track, eta-phi profile;#eta;#phi'
    lrtGroup.defineHistogram(varName, type='TProfile2D', path=pathhits, title=title, xbins=m_nBinsEta, xmin=-m_c_etaRange, xmax=m_c_etaRange, ybins=m_nBinsPhi, ymin=-M_PI, ymax=M_PI)

    varName = 'm_lb,m_nBase_LB;Trk_nBase_LB' 
    title = 'Average number of baseline tracks per event in LB;LB number;Average number of LRT tracks per event in LB'
    lrtGroup.defineHistogram(varName, type='TProfile', path=pathtrack, title=title, xbins=m_c_range_LB, xmin=0, xmax=m_c_range_LB)



    varName = 'm_lumiPerBCID,m_nBase_Lumi;Trk_nBase_Lumi' 
    title = 'Average number of baseline tracks per event in Pileup;#mu;Average number of loose primary tracks per event in Pileup'
    lrtGroup.defineHistogram(varName, type='TProfile', path=pathtrack, title=title, xbins=40, xmin=0, xmax=80)


    varName = 'm_lb,m_nTight_LB;Trk_nTight_LB'
    title = 'Average number of tight tracks per event in LB;LB number;Average number of tight tracks per event in LB'
    lrtGroup.defineHistogram(varName, type='TProfile', path=pathtrack, title=title, xbins=m_c_range_LB, xmin=0, xmax=m_c_range_LB)

    varName = 'm_lb,m_nNoBL_LB;Trk_noBLhits_LB'
    title = 'Average number of tracks with missing innermost pixel layer hit per event in LB;LB number;Average number of tracks with missing innermost pixel layer hit per event in LB'
    lrtGroup.defineHistogram(varName, type='TProfile', path=pathtrack, title=title, xbins=m_c_range_LB, xmin=0, xmax=m_c_range_LB)

    varName = 'm_lb,m_NoBL_LB;Trk_noBLhits_frac_LB'
    title = 'Fraction of tracks with missing innermost pixel layer hit per event in LB;LB number;Fraction of tracks with missing innermost pixel layer hit per event in LB'
    lrtGroup.defineHistogram(varName, type='TProfile', path=pathtrack, title=title, xbins=m_c_range_LB, xmin=0, xmax=m_c_range_LB)

# end histograms


def ITkGlobalLRTMonAlgCfg(helper, acc,
                            flags, name="ITkGlobalLRTMonAlg", **kwargs):

    if "TrackSelectionTool" not in kwargs:
        from InDetTrackSelectionTool.InDetTrackSelectionToolConfig import (
            ITkGlobalLRTMonAlg_TrackSelectionToolCfg)
        kwargs.setdefault("TrackSelectionTool", acc.popToolsAndMerge(
            ITkGlobalLRTMonAlg_TrackSelectionToolCfg(flags)))

    from AthenaMonitoring.FilledBunchFilterToolConfig import FilledBunchFilterToolCfg
    from AthenaMonitoring.AtlasReadyFilterConfig import AtlasReadyFilterCfg


    monAlg = helper.addAlgorithm(
        CompFactory.ITkGlobalLRTMonAlg, name,
        addFilterTools = [FilledBunchFilterToolCfg(flags), AtlasReadyFilterCfg(flags)],
        **kwargs)

    HistoITkGlobalLRTMonAlgCfg(helper, monAlg, flags)
    return
