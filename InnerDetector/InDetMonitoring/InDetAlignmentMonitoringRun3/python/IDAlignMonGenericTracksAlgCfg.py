#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

"""
@file IDAlignMonGenericTracksAlgCfg.py
@author Per Johansson
@date 2021
@brief Configuration for Run 3 based on IDAlignMonGenericTracks.cxx
"""

from math import pi as M_PI

def IDAlignMonGenericTracksAlgCfg(helper, alg, **kwargs):

    # values
    m_pTRange = 100
    m_NTracksRange = 100
    m_rangePixHits = 10
    m_rangeSCTHits = 20
    m_rangeTRTHits = 60
    m_etaRange = 2.7
    m_etaBins = 40
    m_phiBins = 80
    m_d0BsNbins = 100
    m_d0Range = 2
    m_z0Range = 70.
    m_d0BsRange = 0.05
     
    # Set a folder name from the user options
    folderName = "ExtendedTracks"
    if "TrackName" in kwargs:
        folderName = kwargs["TrackName"]
    
    # this creates a "genericTrackGroup" called "alg" which will put its histograms into the subdirectory "GenericTracks"
    genericTrackGroup = helper.addGroup(alg, 'IDA_Tracks')
    pathtrack = '/IDAlignMon/'+folderName+'/GenericTracks'

    varName = 'm_ngTracks;NTracksPerEvent'
    title = 'Number of good tracks per event; Tracks; Events'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_NTracksRange+1, xmin=-0.5, xmax=m_NTracksRange +0.5)
    
    varName = 'mu_m;mu_perEvent'
    title = '#LT#mu#GT average interactions per crossing;#LT#mu#GT;Events'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=101, xmin=-0.5, xmax= 100.5)

    varName = 'm_lb_event;LumiBlock'
    title = 'Lumiblock of the events;Lumiblock;Events'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=1024, xmin=-0.5, xmax=1023.5)

    varName = 'm_lb_track;TracksPerLumiBlock'
    title = 'Tracks Per LumiBlock;Lumiblock;Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=1024, xmin=-0.5, xmax=1023.5)

    varName = 'm_beamSpotX,m_beamSpotY;YBs_vs_XBs'
    title = 'BeamSpot Position: y vs x; x_{BS} [mm]; y_{BS} [mm]'
    genericTrackGroup.defineHistogram(varName, type='TH2F', path=pathtrack, title=title, xbins=100, xmin=-0.9, xmax=-0.1, ybins=100, ymin=-0.8, ymax=-0.1) 

    varName = 'm_beamSpotZ,m_beamSpotY;YBs_vs_ZBs'
    title = 'BeamSpot Position: y vs z; z_{BS} [mm]; y_{BS} [mm]'
    genericTrackGroup.defineHistogram(varName, type='TH2F', path=pathtrack, title=title, xbins=100, xmin= -m_z0Range, xmax= m_z0Range, ybins=100, ymin=-0.8, ymax=-0.1)

    varName = 'm_beamSpotX,m_beamSpotZ;XBs_vs_ZBs'
    title = 'BeamSpot Position: x vs z; z_{BS} [mm]; x_{BS} [mm]'
    genericTrackGroup.defineHistogram(varName, type='TH2F', path=pathtrack, title=title, xbins=100, xmin= -m_z0Range, xmax= m_z0Range, ybins=100, ymin=-0.8, ymax=-0.1)

    varName = 'm_lb,m_beamSpotY;YBs_vs_LumiBlock'
    title = 'Y BeamSpot position: y vs lumiblock; LumiBlock; y_{BS} [mm]'
    genericTrackGroup.defineHistogram(varName, type='TProfile', path=pathtrack, title=title, xbins=1024, xmin=-0.5, xmax=1023.5, ybins=100, ymin=-0.8, ymax=-0.1)

    varName = 'm_lb,m_beamSpotX;XBs_vs_LumiBlock'
    title = 'X BeamSpot position: x vs lumiblock; LumiBlock; x_{BS} [mm]'
    genericTrackGroup.defineHistogram(varName, type='TProfile', path=pathtrack, title=title, xbins=1024, xmin=-0.5, xmax=1023.5, ybins=100, ymin=-0.8, ymax=-0.1)

    varName = 'm_nhits_per_track;Nhits_per_track'
    title = 'Number of hits per track;Number of hits;Number of Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_rangePixHits + m_rangeSCTHits + m_rangeTRTHits + 1, xmin=-0.5, xmax=m_rangePixHits + m_rangeSCTHits + m_rangeTRTHits + 0.5)

    varName = 'm_npixelhits_per_track;Npixhits_per_track'
    title = 'Number of PIXEL (PIX+IBL) hits per track;Number of Pixel hits (PIX+IBL);Number of Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_rangePixHits+1, xmin=-0.5, xmax=m_rangePixHits +0.5)

    varName = 'm_npixelhits_per_track_barrel;Npixhits_per_track_barrel'
    title = 'Number of PIXEL (PIX+IBL) hits per track (Barrel);Number of Pixel hits in Barrel (PIX+IBL);Number of Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_rangePixHits+1, xmin=-0.5, xmax=m_rangePixHits +0.5)
    
    varName = 'm_npixelhits_per_track_eca;Npixhits_per_track_eca'
    title = 'Number of PIXEL (PIX+IBL) hits per track (ECA);Number of Pixel hits in EndCap A (PIX+IBL);Number of Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_rangePixHits+1, xmin=-0.5, xmax=m_rangePixHits +0.5)
    
    varName = 'm_npixelhits_per_track_ecc;Npixhits_per_track_ecc'
    title = 'Number of PIXEL (PIX+IBL) hits per track (ECC);Number of Pixel hits in EndCap C (PIX+IBL);Number of Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_rangePixHits+1, xmin=-0.5, xmax=m_rangePixHits +0.5)
    
    varName = 'm_nscthits_per_track;Nscthits_per_track'
    title = 'Number of SCT hits per track;Number of SCT hits;Number of Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_rangeSCTHits+1, xmin=-0.5, xmax=m_rangeSCTHits +0.5)

    varName = 'm_nscthits_per_track_barrel;Nscthits_per_track_barrel'
    title = 'Number of SCT hits per track (Barrel);Number of SCT hits in Barrel;Number of Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_rangeSCTHits+1, xmin=-0.5, xmax=m_rangeSCTHits +0.5)

    varName = 'm_nscthits_per_track_eca;Nscthits_per_track_eca'
    title = 'Number of SCT hits per track (ECA);Number of SCT hits in EndCap A;Number of Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_rangeSCTHits+1, xmin=-0.5, xmax=m_rangeSCTHits +0.5)
    
    varName = 'm_nscthits_per_track_ecc;Nscthits_per_track_ecc'
    title = 'Number of SCT hits per track (ECC);Number of SCT hits in EndCap C;Number of Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_rangeSCTHits+1, xmin=-0.5, xmax=m_rangeSCTHits +0.5)
    
    varName = 'm_ntrthits_per_track;Ntrthits_per_track'
    title = 'Number of TRT hits per track;Number of TRT hits;Number of Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_rangeTRTHits+1, xmin=-0.5, xmax=m_rangeTRTHits +0.5)

    varName = 'm_ntrthits_per_track_barrel;Ntrthits_per_track_barrel'
    title = 'Number of TRT hits per track (Barrel);Number of TRT hits in Barrel;Number of Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_rangeTRTHits+1, xmin=-0.5, xmax=m_rangeTRTHits +0.5)

    varName = 'm_ntrthits_per_track_eca;Ntrthits_per_track_eca'
    title = 'Number of TRT hits per track (ECA);Number of TRT hits in EndCap A;Number of Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_rangeTRTHits+1, xmin=-0.5, xmax=m_rangeTRTHits +0.5)
    
    varName = 'm_ntrthits_per_track_ecc;Ntrthits_per_track_ecc'
    title = 'Number of TRT hits per track (ECC);Number of TRT hits in EndCap C;Number of Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_rangeTRTHits+1, xmin=-0.5, xmax=m_rangeTRTHits +0.5)

    varName = 'm_eta_2fillpix,m_npixelhits_per_track_2filleta;Npixhits_vs_eta'
    title = "Number of Pixel his vs track #eta; Track #eta; Number of Pixel hits (PIX+IBL)"
    genericTrackGroup.defineHistogram(varName, type='TH2F', path=pathtrack, title=title, xbins=m_etaBins, xmin=-m_etaRange, xmax=m_etaRange, ybins=m_rangePixHits+1, ymin=-0.5, ymax=m_rangePixHits +0.5)

    varName = 'm_eta_2fillsct,m_nscthits_per_track_2filleta;Nscthits_vs_eta'
    title = "Number of SCT his vs track #eta; Track #eta; Number of SCT hits"
    genericTrackGroup.defineHistogram(varName, type='TH2F', path=pathtrack, title=title, xbins=m_etaBins, xmin=-m_etaRange, xmax=m_etaRange, ybins=m_rangeSCTHits+1, ymin=-0.5, ymax=m_rangeSCTHits +0.5)

    varName = 'm_eta_2filltrt,m_ntrthits_per_track_2filleta;Ntrthits_vs_eta'
    title = "Number of TRT his vs track #eta; Track #eta; Number of TRT hits"
    genericTrackGroup.defineHistogram(varName, type='TH2F', path=pathtrack, title=title, xbins=m_etaBins, xmin=-m_etaRange, xmax=m_etaRange, ybins=m_rangeTRTHits+1, ymin=-0.5, ymax=m_rangeTRTHits +0.5)

    varName = 'm_chi2oDoF;chi2oDoF'
    title = 'chi2oDoF;Track in #chi^{2} / NDoF;Number of Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=50, xmin=0, xmax=5.)

    varName = 'm_eta;eta'
    title = 'eta;Track #eta;Number of Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_etaBins, xmin=-m_etaRange, xmax=m_etaRange)

    varName = 'm_errEta;err_Eta'
    title = 'eta error;Track #eta error;Number of Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=50, xmin=0, xmax=0.002)

    varName = 'm_eta_pos;eta_pos'
    title = 'eta for positive tracks; #eta(#plusq);Number of Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_etaBins, xmin=-m_etaRange, xmax=m_etaRange)

    varName = 'm_eta_neg;eta_neg'
    title = 'eta for negative tracks; #eta(#minusq);Number of Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_etaBins, xmin=-m_etaRange, xmax=m_etaRange)

    varName = 'm_phi;phi'
    title = '#phi;Track #phi;Number of Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_phiBins, xmin=0, xmax= 2 * M_PI)

    varName = 'm_errPhi;err_Phi'
    title = '#phi error;Error of #phi [rad];Number of Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=50, xmin=0, xmax= 0.002)

    varName = 'm_z0;z0_origin'
    title = 'z_{0} (computed vs origin); z_{0} (origin) [mm]; Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_d0BsNbins, xmin=-m_z0Range, xmax=m_z0Range)

    varName = 'm_errZ0;err_z0'
    title = 'z_{0} error; z_{0} error [mm]; Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=50, xmin=0, xmax=0.3)

    varName = 'm_z0_bscorr;z0'
    title = 'z_{0} (corrected for beamspot);z_{0} [mm]; Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_d0BsNbins, xmin=-m_z0Range, xmax=m_z0Range)

    varName = 'm_z0sintheta;z0sintheta'
    title = 'z_{0}sin#theta; z_{0}sin#theta [mm]; Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_d0BsNbins, xmin=-m_z0Range, xmax=m_z0Range)

    varName = 'm_d0;d0_origin'
    title = 'd_{0} (computed vs origin);d_{0} (origin) [mm]; Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_d0BsNbins, xmin=-m_d0Range, xmax=m_d0Range)

    varName = 'm_errD0;err_d0'
    title = 'd_{0} error;d_{0} error [mm]; Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=60, xmin=0, xmax=0.05)

    varName = 'm_d0_bscorr;d0'
    title = 'd_{0} (corrected for beamspot);d_{0} (BS) [mm]; Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_d0BsNbins, xmin=-m_d0BsRange, xmax=m_d0BsRange)

    varName = 'm_pT;pT'
    title = 'p_{T};Signed Track p_{T} [GeV];Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=200, xmin=-m_pTRange, xmax=m_pTRange)

    varName = 'm_errPt;err_pT'
    title = 'p_{T} error;Signed Track p_{T} error [GeV];Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=50, xmin=0., xmax=3.)

    varName = 'm_pT_2fillerrPt,m_errPt_2fillpT;errpTvspT'
    title = 'p_{T} error Vs p_{T};p_{T};p_{T} error'
    genericTrackGroup.defineHistogram(varName, type='TH2F', path=pathtrack, title=title, xbins=200, xmin=-m_pTRange, xmax=m_pTRange, ybins=50, ymin=0., ymax=3.)

    varName = 'm_pT_2fillpTRes,m_pTRes_2fillpT;pTResVspT'
    title = 'p_{T} Resolution Vs p_{T};p_{T};p_{T} Resolution'
    genericTrackGroup.defineHistogram(varName, type='TH2F', path=pathtrack, title=title, xbins=200, xmin=-m_pTRange, xmax=m_pTRange, ybins=100, ymin=0, ymax=0.1)


    varName = 'm_p;P'
    title = 'Track Momentum P;Signed Track P [GeV];Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=200, xmin=-m_pTRange, xmax=m_pTRange)

    varName = 'm_pTRes;pTResolution'
    title = 'Momentum resolution;pTRes;Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=100, xmin=0, xmax=0.1)

    varName = 'm_phi_2filld0bscorr,m_d0_bscorr_2fillphi;D0VsPhi0'
    title = 'd_{0} (BS) Vs #phi_{0};#phi_{0} [rad];d_{0} (BS) [mm]'
    genericTrackGroup.defineHistogram(varName, type='TH2F', path=pathtrack, title=title, xbins=m_phiBins, xmin=0, xmax= 2 * M_PI, ybins=m_d0BsNbins, ymin=-m_d0BsRange, ymax=m_d0BsRange)
   
    varName = 'm_phi_2filld0bscorrBAR,m_d0_bscorrBAR;D0VsPhi0_Barrel'
    title = 'd_{0} (BS) Vs #phi_{0} (Barrel);#phi_{0} [rad];d_{0} (BS) [mm]'
    genericTrackGroup.defineHistogram(varName, type='TH2F', path=pathtrack, title=title, xbins=m_phiBins, xmin=0, xmax= 2 * M_PI, ybins=m_d0BsNbins, ymin=-m_d0BsRange, ymax=m_d0BsRange)
   
    varName = 'm_phi_2filld0bscorrECA,m_d0_bscorrECA;D0VsPhi0_ECA'
    title = 'd_{0} (BS) Vs #phi_{0} (ECA);#phi_{0} [rad];d_{0} (BS) [mm]'
    genericTrackGroup.defineHistogram(varName, type='TH2F', path=pathtrack, title=title, xbins=m_phiBins, xmin=0, xmax= 2 * M_PI, ybins=m_d0BsNbins, ymin=-m_d0BsRange, ymax=m_d0BsRange)
   
    varName = 'm_phi_2filld0bscorrECC,m_d0_bscorrECC;D0VsPhi0_ECC'
    title = 'd_{0} (BS) Vs #phi_{0} (ECC);#phi_{0} [rad];d_{0} (BS) [mm]'
    genericTrackGroup.defineHistogram(varName, type='TH2F', path=pathtrack, title=title, xbins=m_phiBins, xmin=0, xmax= 2 * M_PI, ybins=m_d0BsNbins, ymin=-m_d0BsRange, ymax=m_d0BsRange)
   
    # end histograms


