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

    m_EtaModulesPix = [20, 13, 13, 13]
    m_EtaModulesMinPix = [-10.5, -6.5, -6.5, -6.5]
    m_EtaModulesMaxPix = [9.5, 6.5, 6.5, 6.5]
    m_PhiModules = [14, 22, 38, 52]
    m_PhiModulesPerRing = 48
    m_EtaModulesSCTEC = 3
    m_PhiModulesSCTEC = 52
    m_EtaModulesSCT = 13
    m_EtaModulesMinSCT = -6.5
    m_EtaModulesMaxSCT = 6.5   
     
    # Set a folder name from the user options
    folderName = "ExtendedTracks"
    if "TrackName" in kwargs:
        folderName = kwargs["TrackName"]
    
    # this creates a "genericTrackGroup" called "alg" which will put its histograms into the subdirectory "GenericTracks"
    genericTrackGroup = helper.addGroup(alg, 'IDA_Tracks')
    pathtrack = '/IDAlignMon/'+folderName+'/GenericTracks'

    # BeamSpot position histos
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

    # Plots per events 
    varName = 'm_ngTracks;NTracksPerEvent'
    title = 'Number of good tracks per event; Tracks; Events'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_NTracksRange+1, xmin=-0.5, xmax=m_NTracksRange +0.5)
    
    varName = 'm_mu;mu_perEvent'
    title = '#LT#mu#GT average interactions per crossing;#LT#mu#GT;Events'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=101, xmin=-0.5, xmax= 100.5)

    varName = 'm_lb;LumiBlock'
    title = 'Lumiblock of the events;Lumiblock;Events'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=1024, xmin=-0.5, xmax=1023.5)

    # Plots per lumiblock
    varName = 'm_lb_track;NTracksPerLumiBlock'
    title = 'Tracks Per LumiBlock;Lumiblock;Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=1024, xmin=-0.5, xmax=1023.5)

    # Start loop on tracks
    ## Hits per track
    varName = 'm_nhits_per_track;Nhits_per_track'
    title = 'Number of hits per track;Hits;Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_rangePixHits + m_rangeSCTHits + m_rangeTRTHits + 1, xmin=-0.5, xmax=m_rangePixHits + m_rangeSCTHits + m_rangeTRTHits + 0.5)

    varName = 'm_npixelhits_per_track;Npixhits_per_track'
    title = 'Number of PIXEL (PIX+IBL) hits per track;Pixel hits (PIX+IBL);Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_rangePixHits+1, xmin=-0.5, xmax=m_rangePixHits +0.5)

    varName = 'm_npixelhits_per_track_barrel;Npixhits_per_track_barrel'
    title = 'Number of PIXEL (PIX+IBL) hits per track (Barrel);Pixel hits in Barrel (PIX+IBL);Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_rangePixHits+1, xmin=-0.5, xmax=m_rangePixHits +0.5)
    
    varName = 'm_npixelhits_per_track_eca;Npixhits_per_track_eca'
    title = 'Number of PIXEL (PIX+IBL) hits per track (ECA);Pixel hits in EndCap A;Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_rangePixHits+1, xmin=-0.5, xmax=m_rangePixHits +0.5)
    
    varName = 'm_npixelhits_per_track_ecc;Npixhits_per_track_ecc'
    title = 'Number of PIXEL (PIX+IBL) hits per track (ECC);Pixel hits in EndCap C;Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_rangePixHits+1, xmin=-0.5, xmax=m_rangePixHits +0.5)
    
    varName = 'm_nscthits_per_track;Nscthits_per_track'
    title = 'Number of SCT hits per track;SCT hits;Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_rangeSCTHits+1, xmin=-0.5, xmax=m_rangeSCTHits +0.5)

    varName = 'm_nscthits_per_track_barrel;Nscthits_per_track_barrel'
    title = 'Number of SCT hits per track (Barrel);SCT hits in Barrel;Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_rangeSCTHits+1, xmin=-0.5, xmax=m_rangeSCTHits +0.5)

    varName = 'm_nscthits_per_track_eca;Nscthits_per_track_eca'
    title = 'Number of SCT hits per track (ECA);SCT hits in EndCap A;Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_rangeSCTHits+1, xmin=-0.5, xmax=m_rangeSCTHits +0.5)
    
    varName = 'm_nscthits_per_track_ecc;Nscthits_per_track_ecc'
    title = 'Number of SCT hits per track (ECC);SCT hits in EndCap C;Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_rangeSCTHits+1, xmin=-0.5, xmax=m_rangeSCTHits +0.5)
    
    varName = 'm_ntrthits_per_track;Ntrthits_per_track'
    title = 'Number of TRT hits per track;TRT hits;Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_rangeTRTHits+1, xmin=-0.5, xmax=m_rangeTRTHits +0.5)

    varName = 'm_ntrthits_per_track_barrel;Ntrthits_per_track_barrel'
    title = 'Number of TRT hits per track (Barrel);TRT hits in Barrel;Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_rangeTRTHits+1, xmin=-0.5, xmax=m_rangeTRTHits +0.5)

    varName = 'm_ntrthits_per_track_eca;Ntrthits_per_track_eca'
    title = 'Number of TRT hits per track (ECA);TRT hits in EndCap A;Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_rangeTRTHits+1, xmin=-0.5, xmax=m_rangeTRTHits +0.5)
    
    varName = 'm_ntrthits_per_track_ecc;Ntrthits_per_track_ecc'
    title = 'Number of TRT hits per track (ECC);TRT hits in EndCap C;Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_rangeTRTHits+1, xmin=-0.5, xmax=m_rangeTRTHits +0.5)
   
    ## Eta vs Nhits
    varName = 'm_eta,m_npixelhits_per_track;Npixhits_vs_eta'
    title = "Number of Pixel hits vs track #eta; Track #eta;Pixel hits (PIX+IBL)"
    genericTrackGroup.defineHistogram(varName, type='TH2F', path=pathtrack, title=title, xbins=m_etaBins, xmin=-m_etaRange, xmax=m_etaRange, ybins=m_rangePixHits+1, ymin=-0.5, ymax=m_rangePixHits +0.5)

    varName = 'm_eta,m_nscthits_per_track;Nscthits_vs_eta'
    title = "Number of SCT hits vs track #eta; Track #eta;SCT hits"
    genericTrackGroup.defineHistogram(varName, type='TH2F', path=pathtrack, title=title, xbins=m_etaBins, xmin=-m_etaRange, xmax=m_etaRange, ybins=m_rangeSCTHits+1, ymin=-0.5, ymax=m_rangeSCTHits +0.5)

    varName = 'm_eta,m_ntrthits_per_track;Ntrthits_vs_eta'
    title = "Number of TRT hits vs track #eta; Track #eta;TRT hits"
    genericTrackGroup.defineHistogram(varName, type='TH2F', path=pathtrack, title=title, xbins=m_etaBins, xmin=-m_etaRange, xmax=m_etaRange, ybins=m_rangeTRTHits+1, ymin=-0.5, ymax=m_rangeTRTHits +0.5)

    varName = 'm_chi2oDoF;chi2oDoF'
    title = 'chi2oDoF;#chi^{2} / NDoF;Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=50, xmin=0, xmax=5.)

    ## Track params
    varName = 'm_eta;eta'
    title = '#eta;Track #eta;Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_etaBins, xmin=-m_etaRange, xmax=m_etaRange)

    varName = 'm_errEta;err_Eta'
    title = 'Track #eta error;Track #eta error;Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=50, xmin=0, xmax=0.002)

    varName = 'm_eta;eta_pos'
    title = '#eta for positive tracks;Track #eta(#plusq);Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_etaBins, xmin=-m_etaRange, xmax=m_etaRange, cutmask='isTrkPositive')

    varName = 'm_eta;eta_neg'
    title = '#eta for negative tracks;Track #eta(#minusq);Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_etaBins, xmin=-m_etaRange, xmax=m_etaRange, cutmask='isTrkNegative')

    varName = 'm_phi;phi'
    title = 'Track #phi_{0};Track #phi_{0} [rad];Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_phiBins, xmin=0, xmax= 2 * M_PI)

    varName = 'm_errPhi;err_Phi'
    title = 'Track #phi_{0} error;Track #phi_{0} error [rad];Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=50, xmin=0, xmax= 0.002)

    varName = 'm_z0;z0_origin'
    title = 'z_{0} (computed vs origin); z_{0} (origin) [mm];Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_d0BsNbins, xmin=-m_z0Range, xmax=m_z0Range)

    varName = 'm_errZ0;err_z0'
    title = 'z_{0} error; z_{0} error [mm]; Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=50, xmin=0, xmax=0.3)

    varName = 'm_z0_bscorr;z0'
    title = 'z_{0} (corrected for beamspot);z_{0} (BS) [mm]; Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_d0BsNbins, xmin=-m_z0Range, xmax=m_z0Range)

    varName = 'm_z0sintheta;z0sintheta'
    title = 'z_{0}sin#theta; z_{0}sin#theta [mm]; Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_d0BsNbins, xmin=-m_z0Range, xmax=m_z0Range)

    varName = 'm_d0;d0_origin'
    title = 'd_{0} (computed vs origin);d_{0} (origin) [mm]; Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_d0BsNbins, xmin=-m_d0Range, xmax=m_d0Range)

    varName = 'm_errD0;errD0'
    title = 'd_{0} error;d_{0} error [mm]; Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=60, xmin=0, xmax=0.05)

    varName = 'm_d0_bscorr;d0_bscorr'
    title = 'd_{0} (corrected for beamspot);d_{0} (BS) [mm]; Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_d0BsNbins, xmin=-m_d0BsRange, xmax=m_d0BsRange)

    varName = 'm_pT;pT'
    title = 'Momentum p_{T};Signed Track p_{T} [GeV];Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=200, xmin=-m_pTRange, xmax=m_pTRange)

    varName = 'm_errPt;err_pT'
    title = 'Momentum p_{T} error;p_{T} error [GeV];Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=50, xmin=0., xmax=3.)

    varName = 'm_pT,m_errPt;errpTvspT'
    title = 'p_{T} error Vs p_{T};Signed Track p_{T} [GeV];p_{T} error [GeV]'
    genericTrackGroup.defineHistogram(varName, type='TH2F', path=pathtrack, title=title, xbins=200, xmin=-m_pTRange, xmax=m_pTRange, ybins=50, ymin=0., ymax=3.)

    varName = 'm_pT,m_pTRes;pTResVspT'
    title = 'p_{T} Resolution Vs p_{T};Signed Track p_{T} [GeV];p_{T} Resolution (#sigma(p_{T})/p_{T})'
    genericTrackGroup.defineHistogram(varName, type='TH2F', path=pathtrack, title=title, xbins=200, xmin=-m_pTRange, xmax=m_pTRange, ybins=100, ymin=0, ymax=0.1)

    varName = 'm_p;P'
    title = 'Momentum P;Signed Track P [GeV];Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=200, xmin=-m_pTRange, xmax=m_pTRange)

    varName = 'm_pTRes;pTResolution'
    title = 'Momentum p_{T} Resolution;p_{T} Resolution (#sigma(p_{T})/p_{T});Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=100, xmin=0, xmax=0.1)

    varName = 'm_eta,m_d0_bscorr;D0bsVsEta'
    title = 'd_{0} (BS) Vs #eta;Track #eta;d_{0} (BS) [mm]'
    genericTrackGroup.defineHistogram(varName, type='TH2F', path=pathtrack, title=title, xbins=m_etaBins, xmin=-m_etaRange, xmax=m_etaRange, ybins=m_d0BsNbins, ymin=-m_d0BsRange, ymax=m_d0BsRange)

    varName = 'm_pT,m_d0_bscorr;D0bsVsPt'
    title = 'd_{0} (BS) Vs p_{T};Signed track p_{T} [GeV];d_{0} (BS) [mm]'
    genericTrackGroup.defineHistogram(varName, type='TH2F', path=pathtrack, title=title, xbins=200, xmin=-m_pTRange, xmax=m_pTRange, ybins=m_d0BsNbins, ymin=-m_d0BsRange, ymax=m_d0BsRange)
   
    varName = 'm_phi,m_d0_bscorr;D0bsVsPhi0'
    title = 'd_{0} (BS) Vs #phi_{0};Track #phi_{0} [rad];d_{0} (BS) [mm]'
    genericTrackGroup.defineHistogram(varName, type='TH2F', path=pathtrack, title=title, xbins=m_phiBins, xmin=0, xmax= 2 * M_PI, ybins=m_d0BsNbins, ymin=-m_d0BsRange, ymax=m_d0BsRange)
   
    varName = 'm_phi,m_d0_bscorr;D0bsVsPhi0_Barrel'
    title = 'd_{0} (BS) Vs #phi_{0} (Barrel);Track #phi_{0} [rad];d_{0} (BS) [mm]'
    genericTrackGroup.defineHistogram(varName, type='TH2F', path=pathtrack, title=title, xbins=m_phiBins, xmin=0, xmax= 2 * M_PI, ybins=m_d0BsNbins, ymin=-m_d0BsRange, ymax=m_d0BsRange, cutmask='isTrackBarrel')
   
    varName = 'm_phi,m_d0_bscorr;D0bsVsPhi0_ECA'
    title = 'd_{0} (BS) Vs #phi_{0} (ECA);Track #phi_{0} [rad];d_{0} (BS) [mm]'
    genericTrackGroup.defineHistogram(varName, type='TH2F', path=pathtrack, title=title, xbins=m_phiBins, xmin=0, xmax= 2 * M_PI, ybins=m_d0BsNbins, ymin=-m_d0BsRange, ymax=m_d0BsRange, cutmask='isTrackECA')
   
    varName = 'm_phi,m_d0_bscorr;D0bsVsPhi0_ECC'
    title = 'd_{0} (BS) Vs #phi_{0} (ECC);Track #phi_{0} [rad];d_{0} (BS) [mm]'
    genericTrackGroup.defineHistogram(varName, type='TH2F', path=pathtrack, title=title, xbins=m_phiBins, xmin=0, xmax= 2 * M_PI, ybins=m_d0BsNbins, ymin=-m_d0BsRange, ymax=m_d0BsRange, cutmask='isTrackECC')
   
    ## Eta-ID vs Phi-ID vs hits 
    ### Pixel barrel and endcap
    layersPix = ['0', '1', '2', '3']
    layersName = ['IBL','B-layer','1','2']
    pixBhitmeasArray = helper.addArray([len(layersPix)], alg, 'measurements_vs_Eta_Phi_pix_b', topPath=pathtrack)
    for postfix, tool in pixBhitmeasArray.Tools.items():
        layer = layersPix[int( postfix.split('_')[1] )]
        title = ('Number of hits vs Module Eta-Phi-ID Pixel Barrel layer %s; Mod Eta; Mod Phi; Pixel hits (PIX+IBL)' % layersName[int(layer)]) 
        name = 'm_modEta,m_modPhi;measurements_vs_Eta_Phi_pix_b' + layer
        tool.defineHistogram(name, title = title, type = 'TH2F', xbins = m_EtaModulesPix[int(layer)], xmin = m_EtaModulesMinPix[int(layer)], xmax = m_EtaModulesMaxPix[int(layer)],
                                                  ybins = m_PhiModules[int(layer)], ymin = -0.5, ymax = m_PhiModules[int(layer)] - 0.5)
     
    endcapsPix = ['a', 'c']
    pixEChitmeAsrray = helper.addArray([len(endcapsPix)], alg, 'measurements_vs_Eta_Phi_pix_ec', topPath = pathtrack)
    for postfix, tool in pixEChitmeAsrray.Tools.items():
        layer = endcapsPix[int(postfix.split('_')[1])]
        title = ('Number of hits vs Module Eta-Phi-ID Pixel EndCap %s; Disk; Mod Phi; Pixel hits (PIX+IBL)' %  layer.upper()) 
        name = 'm_layerDisk,m_modPhi;measurements_vs_Eta_Phi_pix_ec' + layer
        tool.defineHistogram(name, title = title, type = 'TH2F', xbins = 3, xmin = - 0.5, xmax = 3 - 0.5,
                                                  ybins = m_PhiModulesPerRing, ymin = -0.5, ymax = m_PhiModulesPerRing - 0.5)
    
    ### SCT barrel and endcap
    layersSCTB = ['0', '1', '2', '3']
    sctBhitmeasArray = helper.addArray([len(layersSCTB)], alg, 'measurements_vs_Eta_Phi_sct_b_s0', topPath = pathtrack)
    for postfix, tool in sctBhitmeasArray.Tools.items():
        layer = layersSCTB[int( postfix.split('_')[1] )]
        title = ('Number of hits vs Module Eta-Phi-ID SCT Barrel layer %s side 0; Mod Eta; Mod Phi; SCT hits ' % layer) 
        name = 'm_modEta,m_modPhi;measurements_vs_Eta_Phi_sct_b' + layer + '_s0'
        tool.defineHistogram(name, title = title, type = 'TH2F', xbins = m_EtaModulesSCT, xmin = m_EtaModulesMinSCT, xmax = m_EtaModulesMaxSCT,
                                                   ybins = m_PhiModules[int(layer)], ymin = -0.5, ymax = m_PhiModules[int(layer)] - 0.5)

    sctBhitmeasArray = helper.addArray([len(layersSCTB)], alg, 'measurements_vs_Eta_Phi_sct_b_s1', topPath = pathtrack)
    for postfix, tool in sctBhitmeasArray.Tools.items():
        layer = layersSCTB[int( postfix.split('_')[1] )]
        title = ('Number of hits vs Module Eta-Phi-ID SCT Barrel layer %s side 1; Mod Eta; Mod Phi; SCT hits ' % layer) 
        name = 'm_modEta,m_modPhi;measurements_vs_Eta_Phi_sct_b' + layer + '_s1'
        tool.defineHistogram(name, title = title, type = 'TH2F', xbins = m_EtaModulesSCT, xmin = m_EtaModulesMinSCT, xmax = m_EtaModulesMaxSCT,
                                                   ybins = m_PhiModules[int(layer)], ymin = -0.5, ymax = m_PhiModules[int(layer)] - 0.5)

    layersSCTEC = ['0', '1', '2', '3', '4', '5', '6', '7', '8']
    sctECAs0hitmeasArray = helper.addArray([len(layersSCTEC)], alg, 'measurements_vs_Eta_Phi_sct_eca_s0', topPath = pathtrack)
    for postfix, tool in sctECAs0hitmeasArray.Tools.items():
        layer = layersSCTEC[int( postfix.split('_')[1] )]
        title = ('Number of hits vs Module Eta-Phi-ID SCT EndCap A Disk %s side 0; Mod Eta; Mod Phi; SCT hits EndCap A s0 ' % layer)
        name = 'm_modEta,m_modPhi;measurements_vs_Eta_Phi_sct_eca' + layer + '_s0'
        tool.defineHistogram(name, title = title, type = 'TH2F', xbins = m_EtaModulesSCTEC, xmin = -0.5, xmax = m_EtaModulesSCTEC - 0.5,
                                                   ybins = m_PhiModulesSCTEC, ymin = -0.5, ymax = m_PhiModulesSCTEC - 0.5)

    sctECAs1hitmeasArray = helper.addArray([len(layersSCTEC)], alg, 'measurements_vs_Eta_Phi_sct_eca_s1', topPath = pathtrack)
    for postfix, tool in sctECAs1hitmeasArray.Tools.items():
        layer = layersSCTEC[int( postfix.split('_')[1] )]
        title = ('Number of hits vs Module Eta-Phi-ID SCT EndCap A Disk %s side 1; Mod Eta; Mod Phi; SCT hits EndCap A s1 ' % layer)
        name = 'm_modEta,m_modPhi;measurements_vs_Eta_Phi_sct_eca' + layer + '_s1'
        tool.defineHistogram(name, title = title, type = 'TH2F', xbins = m_EtaModulesSCTEC, xmin = -0.5, xmax = m_EtaModulesSCTEC - 0.5,
                                                   ybins = m_PhiModulesSCTEC, ymin = -0.5, ymax = m_PhiModulesSCTEC - 0.5)

    sctECCs0hitmeasArray = helper.addArray([len(layersSCTEC)], alg, 'measurements_vs_Eta_Phi_sct_ecc_s0', topPath = pathtrack)
    for postfix, tool in sctECCs0hitmeasArray.Tools.items():
        layer = layersSCTEC[int( postfix.split('_')[1] )]
        title = ('Number of hits vs Module Eta-Phi-ID SCT EndCap C Disk %s side 0; Mod Eta; Mod Phi; SCT hits EndCap C s0 ' % layer)
        name = 'm_modEta,m_modPhi;measurements_vs_Eta_Phi_sct_ecc' + layer + '_s0'
        tool.defineHistogram(name, title = title, type = 'TH2F', xbins = m_EtaModulesSCTEC, xmin = -0.5, xmax = m_EtaModulesSCTEC - 0.5,
                                                   ybins = m_PhiModulesSCTEC, ymin = -0.5, ymax = m_PhiModulesSCTEC - 0.5)

    sctECCs1hitmeasArray = helper.addArray([len(layersSCTEC)], alg, 'measurements_vs_Eta_Phi_sct_ecc_s1', topPath = pathtrack)
    for postfix, tool in sctECCs1hitmeasArray.Tools.items():
        layer = layersSCTEC[int( postfix.split('_')[1] )]
        title = ('Number of hits vs Module Eta-Phi-ID SCT EndCap C Disk %s side 1; Mod Eta; Mod Phi; SCT hits EndCap C s1 ' % layer)
        name = 'm_modEta,m_modPhi;measurements_vs_Eta_Phi_sct_ecc' + layer + '_s1'
        tool.defineHistogram(name, title = title, type = 'TH2F', xbins = m_EtaModulesSCTEC, xmin = -0.5, xmax = m_EtaModulesSCTEC - 0.5,
                                                   ybins = m_PhiModulesSCTEC, ymin = -0.5, ymax = m_PhiModulesSCTEC - 0.5)
    
    # end histograms


