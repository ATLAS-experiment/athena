#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

"""
@file ITkAlignMonGenericTracksAlgCfg.py
@author Per Johansson
@date 2021
@brief Configuration for Run 3 based on IDAlignMonGenericTracks.cxx
"""

from math import pi as M_PI

def ITkAlignMonGenericTracksAlgCfg(helper, alg, flags=None, **kwargs):

    # values (ITk: |eta| < 4.0, high pile-up track multiplicity)
    m_pTRange = 100
    m_NTracksRange = 5000
    m_rangePixHits = 20
    m_rangeStripHits = 20
    m_etaRange = 4.0
    m_etaBins = 80
    m_phiBins = 80
    m_d0BsNbins = 100
    m_d0Range = 2
    m_z0Range = 90.
    m_d0BsRange = 0.05

    # ITk numerology (ATLAS-P2-RUN4-03)
    m_EtaModulesPix = [25, 13, 19, 19, 19]
    m_EtaModulesMinPix = [-12.5, -6.5, -9.5, -9.5, -9.5]
    m_EtaModulesMaxPix = [12.5, 6.5, 9.5, 9.5, 9.5]
    m_PhiModules = [12, 20, 32, 44, 56]
    m_PhiModulesStrip = [28, 40, 56, 72]
    m_PhiModulesPerRing = 56
    m_EtaModulesStripEC = 18
    m_PhiModulesStripEC = 64
    m_EtaModulesStrip = 113
    m_EtaModulesMinStrip = -56.5
    m_EtaModulesMaxStrip = 56.5   
     
    # Set a folder name from the user options
    folderName = "ExtendedTracks"
    if "TrackName" in kwargs:
        folderName = kwargs["TrackName"]
    
    # this creates a "genericTrackGroup" called "alg" which will put its histograms into the subdirectory "GenericTracks"
    genericTrackGroup = helper.addGroup(alg, 'IDA_Tracks')
    pathtrack = '/ITkAlignMon/'+folderName+'/GenericTracks'

    # BeamSpot position histos
    varName = 'm_beamSpotX,m_beamSpotY;YBs_vs_XBs'
    title = 'BeamSpot Position: y vs x; x_{BS} [mm]; y_{BS} [mm]'
    genericTrackGroup.defineHistogram(varName, type='TH2F', path=pathtrack, title=title, xbins=100, xmin=-0.5, xmax=0.5, ybins=100, ymin=-0.5, ymax=0.5) 

    varName = 'm_beamSpotZ,m_beamSpotY;YBs_vs_ZBs'
    title = 'BeamSpot Position: y vs z; z_{BS} [mm]; y_{BS} [mm]'
    genericTrackGroup.defineHistogram(varName, type='TH2F', path=pathtrack, title=title, xbins=100, xmin= -m_z0Range, xmax= m_z0Range, ybins=100, ymin=-0.5, ymax=0.5)

    varName = 'm_beamSpotX,m_beamSpotZ;XBs_vs_ZBs'
    title = 'BeamSpot Position: x vs z; z_{BS} [mm]; x_{BS} [mm]'
    genericTrackGroup.defineHistogram(varName, type='TH2F', path=pathtrack, title=title, xbins=100, xmin= -m_z0Range, xmax= m_z0Range, ybins=100, ymin=-0.5, ymax=0.5)

    varName = 'm_lb,m_beamSpotY;YBs_vs_LumiBlock'
    title = 'Y BeamSpot position: y vs lumiblock; LumiBlock; y_{BS} [mm]'
    genericTrackGroup.defineHistogram(varName, type='TProfile', path=pathtrack, title=title, xbins=1024, xmin=-0.5, xmax=1023.5, ybins=100, ymin=-0.5, ymax=0.5)

    varName = 'm_lb,m_beamSpotX;XBs_vs_LumiBlock'
    title = 'X BeamSpot position: x vs lumiblock; LumiBlock; x_{BS} [mm]'
    genericTrackGroup.defineHistogram(varName, type='TProfile', path=pathtrack, title=title, xbins=1024, xmin=-0.5, xmax=1023.5, ybins=100, ymin=-0.5, ymax=0.5)

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
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_rangePixHits + m_rangeStripHits + 1, xmin=-0.5, xmax=m_rangePixHits + m_rangeStripHits + 0.5)

    varName = 'm_npixelhits_per_track;Npixhits_per_track'
    title = 'Number of pixel hits per track;Pixel hits;Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_rangePixHits+1, xmin=-0.5, xmax=m_rangePixHits +0.5)

    varName = 'm_npixelhits_per_track_barrel;Npixhits_per_track_barrel'
    title = 'Number of pixel hits per track (Barrel);Pixel hits in Barrel;Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_rangePixHits+1, xmin=-0.5, xmax=m_rangePixHits +0.5)
    
    varName = 'm_npixelhits_per_track_eca;Npixhits_per_track_eca'
    title = 'Number of pixel hits per track (ECA);Pixel hits in EndCap A;Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_rangePixHits+1, xmin=-0.5, xmax=m_rangePixHits +0.5)
    
    varName = 'm_npixelhits_per_track_ecc;Npixhits_per_track_ecc'
    title = 'Number of pixel hits per track (ECC);Pixel hits in EndCap C;Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_rangePixHits+1, xmin=-0.5, xmax=m_rangePixHits +0.5)
    
    varName = 'm_nstriphits_per_track;Nstriphits_per_track'
    title = 'Number of Strip hits per track;Strip hits;Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_rangeStripHits+1, xmin=-0.5, xmax=m_rangeStripHits +0.5)

    varName = 'm_nstriphits_per_track_barrel;Nstriphits_per_track_barrel'
    title = 'Number of Strip hits per track (Barrel);Strip hits in Barrel;Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_rangeStripHits+1, xmin=-0.5, xmax=m_rangeStripHits +0.5)

    varName = 'm_nstriphits_per_track_eca;Nstriphits_per_track_eca'
    title = 'Number of Strip hits per track (ECA);Strip hits in EndCap A;Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_rangeStripHits+1, xmin=-0.5, xmax=m_rangeStripHits +0.5)
    
    varName = 'm_nstriphits_per_track_ecc;Nstriphits_per_track_ecc'
    title = 'Number of Strip hits per track (ECC);Strip hits in EndCap C;Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_rangeStripHits+1, xmin=-0.5, xmax=m_rangeStripHits +0.5)
    
    
   
    ## Eta vs Nhits
    varName = 'm_eta,m_npixelhits_per_track;Npixhits_vs_eta'
    title = "Number of Pixel hits vs track #eta; Track #eta;Pixel hits"
    genericTrackGroup.defineHistogram(varName, type='TH2F', path=pathtrack, title=title, xbins=m_etaBins, xmin=-m_etaRange, xmax=m_etaRange, ybins=m_rangePixHits+1, ymin=-0.5, ymax=m_rangePixHits +0.5)

    varName = 'm_eta,m_nstriphits_per_track;Nstriphits_vs_eta'
    title = "Number of Strip hits vs track #eta; Track #eta;Strip hits"
    genericTrackGroup.defineHistogram(varName, type='TH2F', path=pathtrack, title=title, xbins=m_etaBins, xmin=-m_etaRange, xmax=m_etaRange, ybins=m_rangeStripHits+1, ymin=-0.5, ymax=m_rangeStripHits +0.5)

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
    title = 'z_{0} error; z_{0} error [mm];Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=50, xmin=0, xmax=0.3)

    varName = 'm_z0_bscorr;z0'
    title = 'z_{0} (corrected for beamspot);z_{0} (BS) [mm]; Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_d0BsNbins, xmin=-m_z0Range, xmax=m_z0Range)

    varName = 'm_z0sintheta;z0sintheta'
    title = 'z_{0}sin#theta; z_{0}sin#theta [mm];Tracks'
    genericTrackGroup.defineHistogram(varName, type='TH1F', path=pathtrack, title=title, xbins=m_d0BsNbins, xmin=-m_z0Range, xmax=m_z0Range)

    varName = 'm_d0;d0_origin'
    title = 'd_{0} (computed vs origin);d_{0} (origin) [mm];Tracks'
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
   
    # d0_origin 
    varName = 'm_pT,m_d0;D0orVsPt'
    title = 'd_{0} (origin) Vs p_{T};Signed track p_{T} [GeV];d_{0} (origin) [mm]'
    genericTrackGroup.defineHistogram( varName, type='TH2F', path=pathtrack, title=title, xbins=200, xmin=-m_pTRange, xmax=m_pTRange, ybins=m_d0BsNbins, ymin=-m_d0BsRange, ymax=m_d0BsRange )

    ## Eta-ID vs Phi-ID vs hits 
    ### Pixel barrel and endcap
    layersPix = ['0', '1', '2', '3', '4']
    layersName = ['L0','L1','L2','L3','L4']
    pixBhitmeasArray = helper.addArray([len(layersPix)], alg, 'measurements_vs_Eta_Phi_pix_b', topPath=pathtrack)
    for postfix, tool in pixBhitmeasArray.Tools.items():
        layer = layersPix[int( postfix.split('_')[1] )]
        title = ('Number of hits vs Module Eta-Phi-ID Pixel Barrel layer %s; Mod Eta; Mod Phi; Pixel hits' % layersName[int(layer)]) 
        name = 'm_modEta,m_modPhi;measurements_vs_Eta_Phi_pix_b' + layer
        tool.defineHistogram(name, title = title, type = 'TH2F', xbins = m_EtaModulesPix[int(layer)], xmin = m_EtaModulesMinPix[int(layer)], xmax = m_EtaModulesMaxPix[int(layer)],
                                                  ybins = m_PhiModules[int(layer)], ymin = -0.5, ymax = m_PhiModules[int(layer)] - 0.5)
     
    endcapsPix = ['a', 'c']
    pixEChitmeAsrray = helper.addArray([len(endcapsPix)], alg, 'measurements_vs_Eta_Phi_pix_ec', topPath = pathtrack)
    for postfix, tool in pixEChitmeAsrray.Tools.items():
        layer = endcapsPix[int(postfix.split('_')[1])]
        title = ('Number of hits vs Module Eta-Phi-ID Pixel EndCap %s; Disk; Mod Phi; Pixel hits' %  layer.upper()) 
        name = 'm_layerDisk,m_modPhi;measurements_vs_Eta_Phi_pix_ec' + layer
        tool.defineHistogram(name, title = title, type = 'TH2F', xbins = 9, xmin = - 0.5, xmax = 9 - 0.5,
                                                  ybins = m_PhiModulesPerRing, ymin = -0.5, ymax = m_PhiModulesPerRing - 0.5)
    
    ### Strip barrel and endcap
    layersStripB = ['0', '1', '2', '3']
    stripBhitmeasArray = helper.addArray([len(layersStripB)], alg, 'measurements_vs_Eta_Phi_strip_b_s0', topPath = pathtrack)
    for postfix, tool in stripBhitmeasArray.Tools.items():
        layer = layersStripB[int( postfix.split('_')[1] )]
        title = ('Number of hits vs Module Eta-Phi-ID Strip Barrel layer %s side 0; Mod Eta; Mod Phi; Strip hits ' % layer) 
        name = 'm_modEta,m_modPhi;measurements_vs_Eta_Phi_strip_b' + layer + '_s0'
        tool.defineHistogram(name, title = title, type = 'TH2F', xbins = m_EtaModulesStrip, xmin = m_EtaModulesMinStrip, xmax = m_EtaModulesMaxStrip,
                                                   ybins = m_PhiModulesStrip[int(layer)], ymin = -0.5, ymax = m_PhiModulesStrip[int(layer)] - 0.5)

    stripBhitmeasArray = helper.addArray([len(layersStripB)], alg, 'measurements_vs_Eta_Phi_strip_b_s1', topPath = pathtrack)
    for postfix, tool in stripBhitmeasArray.Tools.items():
        layer = layersStripB[int( postfix.split('_')[1] )]
        title = ('Number of hits vs Module Eta-Phi-ID Strip Barrel layer %s side 1; Mod Eta; Mod Phi; Strip hits ' % layer) 
        name = 'm_modEta,m_modPhi;measurements_vs_Eta_Phi_strip_b' + layer + '_s1'
        tool.defineHistogram(name, title = title, type = 'TH2F', xbins = m_EtaModulesStrip, xmin = m_EtaModulesMinStrip, xmax = m_EtaModulesMaxStrip,
                                                   ybins = m_PhiModulesStrip[int(layer)], ymin = -0.5, ymax = m_PhiModulesStrip[int(layer)] - 0.5)

    layersStripEC = ['0', '1', '2', '3', '4', '5']
    stripECAs0hitmeasArray = helper.addArray([len(layersStripEC)], alg, 'measurements_vs_Eta_Phi_strip_eca_s0', topPath = pathtrack)
    for postfix, tool in stripECAs0hitmeasArray.Tools.items():
        layer = layersStripEC[int( postfix.split('_')[1] )]
        title = ('Number of hits vs Module Eta-Phi-ID Strip EndCap A Disk %s side 0; Mod Eta; Mod Phi; Strip hits EndCap A s0 ' % layer)
        name = 'm_modEta,m_modPhi;measurements_vs_Eta_Phi_strip_eca' + layer + '_s0'
        tool.defineHistogram(name, title = title, type = 'TH2F', xbins = m_EtaModulesStripEC, xmin = -0.5, xmax = m_EtaModulesStripEC - 0.5,
                                                   ybins = m_PhiModulesStripEC, ymin = -0.5, ymax = m_PhiModulesStripEC - 0.5)

    stripECAs1hitmeasArray = helper.addArray([len(layersStripEC)], alg, 'measurements_vs_Eta_Phi_strip_eca_s1', topPath = pathtrack)
    for postfix, tool in stripECAs1hitmeasArray.Tools.items():
        layer = layersStripEC[int( postfix.split('_')[1] )]
        title = ('Number of hits vs Module Eta-Phi-ID Strip EndCap A Disk %s side 1; Mod Eta; Mod Phi; Strip hits EndCap A s1 ' % layer)
        name = 'm_modEta,m_modPhi;measurements_vs_Eta_Phi_strip_eca' + layer + '_s1'
        tool.defineHistogram(name, title = title, type = 'TH2F', xbins = m_EtaModulesStripEC, xmin = -0.5, xmax = m_EtaModulesStripEC - 0.5,
                                                   ybins = m_PhiModulesStripEC, ymin = -0.5, ymax = m_PhiModulesStripEC - 0.5)

    stripECCs0hitmeasArray = helper.addArray([len(layersStripEC)], alg, 'measurements_vs_Eta_Phi_strip_ecc_s0', topPath = pathtrack)
    for postfix, tool in stripECCs0hitmeasArray.Tools.items():
        layer = layersStripEC[int( postfix.split('_')[1] )]
        title = ('Number of hits vs Module Eta-Phi-ID Strip EndCap C Disk %s side 0; Mod Eta; Mod Phi; Strip hits EndCap C s0 ' % layer)
        name = 'm_modEta,m_modPhi;measurements_vs_Eta_Phi_strip_ecc' + layer + '_s0'
        tool.defineHistogram(name, title = title, type = 'TH2F', xbins = m_EtaModulesStripEC, xmin = -0.5, xmax = m_EtaModulesStripEC - 0.5,
                                                   ybins = m_PhiModulesStripEC, ymin = -0.5, ymax = m_PhiModulesStripEC - 0.5)

    stripECCs1hitmeasArray = helper.addArray([len(layersStripEC)], alg, 'measurements_vs_Eta_Phi_strip_ecc_s1', topPath = pathtrack)
    for postfix, tool in stripECCs1hitmeasArray.Tools.items():
        layer = layersStripEC[int( postfix.split('_')[1] )]
        title = ('Number of hits vs Module Eta-Phi-ID Strip EndCap C Disk %s side 1; Mod Eta; Mod Phi; Strip hits EndCap C s1 ' % layer)
        name = 'm_modEta,m_modPhi;measurements_vs_Eta_Phi_strip_ecc' + layer + '_s1'
        tool.defineHistogram(name, title = title, type = 'TH2F', xbins = m_EtaModulesStripEC, xmin = -0.5, xmax = m_EtaModulesStripEC - 0.5,
                                                   ybins = m_PhiModulesStripEC, ymin = -0.5, ymax = m_PhiModulesStripEC - 0.5)
    
    # end histograms
    #


