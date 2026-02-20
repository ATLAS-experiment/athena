#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

'''@file InDetAlignMonResidualsAlgCfg.py
@author PerJohansson
@date 2021-2022
@brief Configuration for Run 3 based on InDetAlignMonResiduals.cxx
'''
def IDAlignMonResidualsAlgCfg(helper, alg, **kwargs):
    '''Function to configures some algorithms in the monitoring system.'''

    #Values
    m_nBinsMuRange        = 101
    m_muRangeMin          = -0.5
    m_muRangeMax          = 100.5
    m_minSiResFillRange   = -0.05
    m_maxSiResFillRange   = 0.05
    m_minSCTResFillRange   = -0.07
    m_maxSCTResFillRange   = 0.07
    m_siliconBarrelLayers = 12
    m_siliconECLayers     = 24
    m_RangeOfPullHistos   = 4
    m_minPIXResYFillRange = -0.3
    m_maxPIXResYFillRange = 0.3
    m_FinerBinningFactor = 1
    strawRadius = 2.5
    m_EtaModulesPix = [20, 13, 13, 13]
    m_EtaModulesMinPix = [-10.5, -6.5, -6.5, -6.5]
    m_EtaModulesMaxPix = [9.5, 6.5, 6.5, 6.5]
    m_PhiModules = [14, 22, 38, 52]
    m_PhiModulesShift_barrel = 158
    m_EtaModulesShift_barrel = 96
    m_PhiModulesPerRing = 48
    m_PhiModulesShift_ec = 152
    m_EtaModulesSCT = 13
    m_EtaModulesSCTEC = 3
    m_EtaModulesMinSCT = -6.5
    m_EtaModulesMaxSCT = 6.5
    m_PhiModulesSCT = [32, 40, 48, 56]
    m_PhiModulesPerRingSCT = 52
    m_PhiModulesShift_sct_barrel = 208
    m_EtaModulesShift_sct_barrel = 90
    m_PhiModulesShift_sct_ec = 548 #52 mod/disk x 9 disk + 10*8 gaps
    m_minTRTResWindow = -0.6
    m_maxTRTResWindow = 0.6
    m_TRTB_nSectorBins = 32
    m_TRTEC_nPhiBins = 24
     
    # Set a folder name from the user options
    folderName = "ExtendedTracks"
    if "TrackName" in kwargs:
        folderName = kwargs["TrackName"]
    
    # this creates a "residualGroup" called "alg" which will put its histograms into the subdirectory "Residuals"
    residualGroup = helper.addGroup(alg, 'Residuals')
    pathResiduals = '/IDAlignMon/'+folderName+'/Residuals'

    # Histograms for the Alignment Residual monitoring:    
    varName = 'm_mu;mu_perEvent'
    title = 'mu_perEvent;#LT#mu#GT per event;Events'
    residualGroup.defineHistogram(varName, type='TH1F', path=pathResiduals, title=title, xbins=m_nBinsMuRange, xmin=m_muRangeMin, xmax=m_muRangeMax)

    varName = 'm_detType;sirescalcfailure'
    title = 'Hits with ResidualPullCalculator problem;Events;DetType'
    residualGroup.defineHistogram(varName, type='TH1F', path=pathResiduals, title=title, xbins=2, xmin=0, xmax=2)

    
    #TRT Barrel Plots
    layersTRTB = ['', 'Side_A', 'Side_C']
    predictedRBArray = helper.addArray([len(layersTRTB)], alg, 'TRTPredictedRB', topPath = pathResiduals)
    for postfix, tool in predictedRBArray.Tools.items():
        layer = layersTRTB[int( postfix.split('_')[1] )]
        title = ('Predicted drift radius from TRT Barrel %s' % layer) + ';Predicted Drift Radius [mm];Entries' 
        name = 'm_trt_b_PredictedR;trt_b_PredictedR' + layer
        tool.defineHistogram(name, title = title, type = 'TH1F', xbins = 100 , xmin = -strawRadius  , xmax = strawRadius)

    measuredRBArray = helper.addArray([len(layersTRTB)], alg, 'TRTMeasuredRB', topPath = pathResiduals)
    for postfix, tool in measuredRBArray.Tools.items():
        layer = layersTRTB[int( postfix.split('_')[1] )]
        title = ('Measured at drift radius for TRT Barrel %s' % layer) + ';Measured Drift Radius [mm];Entries' 
        name = 'm_trt_b_MeasuredR;trt_b_MeasuredR' + layer
        tool.defineHistogram(name, title = title, type = 'TH1F', xbins = 100 , xmin = -strawRadius , xmax = strawRadius)

    residualRBArray = helper.addArray([len(layersTRTB)], alg, 'TRTResidualRB', topPath = pathResiduals)
    for postfix, tool in residualRBArray.Tools.items():
        layer = layersTRTB[int( postfix.split('_')[1] )]
        title = ('UnBiased Residual for the TRT %s' % layer) + ';Residual [mm];Entries' 
        name = 'm_trt_b_residualR;trt_b_residualR' + layer
        tool.defineHistogram(name, title = title, type = 'TH1F', xbins = 100 * m_FinerBinningFactor, xmin = m_minTRTResWindow , xmax = m_maxTRTResWindow)
 
    pullRBArray = helper.addArray([len(layersTRTB)], alg, 'TRTPullRB', topPath = pathResiduals)
    for postfix, tool in pullRBArray.Tools.items():
        layer = layersTRTB[int( postfix.split('_')[1] )]
        title = ('Pull for the TRT %s' % layer) + ';Pull;Entries' 
        name = 'm_trt_b_pullR;trt_b_pullR' + layer
        tool.defineHistogram(name, title = title, type = 'TH1F', xbins = 100,  xmin = -m_RangeOfPullHistos , xmax = m_RangeOfPullHistos)

    residualRNoTubeBArray = helper.addArray([len(layersTRTB)], alg, 'TRTResidualRNoTubeB', topPath = pathResiduals)
    for postfix, tool in residualRNoTubeBArray.Tools.items():
        layer = layersTRTB[int( postfix.split('_')[1] )]
        title = ('UnBiased Residual (notube) for the TRT %s' % layer) + ';Residual [mm];Entries' 
        name = 'm_trt_b_residualR_notube;trt_b_residualR' + layer + '_notube'
        tool.defineHistogram(name, title = title, type = 'TH1F', xbins = 100 * m_FinerBinningFactor, xmin = m_minTRTResWindow , xmax = m_maxTRTResWindow)

    pullRNoTubeBArray = helper.addArray([len(layersTRTB)], alg, 'TRTPullRNoTubeB', topPath = pathResiduals)
    for postfix, tool in pullRNoTubeBArray.Tools.items():
        layer = layersTRTB[int( postfix.split('_')[1] )]
        title = ('Pull (notube) for the TRT %s' % layer) + ';Pull;Entries' 
        name = 'm_trt_b_pullR_notube;trt_b_pullR_notube' + layer
        tool.defineHistogram(name, title = title, type = 'TH1F', xbins = 100,  xmin = -m_RangeOfPullHistos , xmax = m_RangeOfPullHistos)

    lrBArray = helper.addArray([len(layersTRTB)], alg, 'TRTLRB', topPath = pathResiduals)
    for postfix, tool in lrBArray.Tools.items():
        layer = layersTRTB[int( postfix.split('_')[1] )]
        title = ('|0= LRcor !isTube | 1= LRcor isTube| 2= !LRcor !isTube | 3= !LRcor isTube %s' % layer) + ';;Entries' 
        name = 'm_trt_b_lr;trt_b_lr' + layer
        tool.defineHistogram(name, title = title, type = 'TH1F', xbins = 4,  xmin = 0 , xmax = 4)

    #Plots for A+C,A,C over Barrel layer 0,1,2
    layTRTB = ['0', '1', '2']
    resvsEtaBArray = helper.addArray([len(layersTRTB),len(layTRTB)], alg, 'TRTResVsEtaB', topPath = pathResiduals)
    for postfix, tool in resvsEtaBArray.Tools.items():
        layer = layersTRTB[int( postfix.split('_')[1] )]
        lay = layTRTB[int( postfix.split('_')[2] )]
        title = ('Average Residual vs Track Eta for TRT Barrel module layer %s %s' % (lay,layer)) + ';Track Eta;Average Residual [mm]' 
        name = 'm_trt_b_aveResVsTrackEta, m_trt_b_residualR;trt_b_aveResVsTrackEta_l' + lay + layer
        tool.defineHistogram(name, title = title, type = 'TProfile', xbins = 20,  xmin = -2.5 , xmax = 2.5, ymin = -1.0 , ymax = 1.0)

    resVsPhiSecBArray = helper.addArray([len(layersTRTB),len(layTRTB)], alg, 'TRTResVsPhiSecB', topPath = pathResiduals)
    for postfix, tool in resVsPhiSecBArray.Tools.items():
        layer = layersTRTB[int( postfix.split('_')[1] )]
        lay = layTRTB[int( postfix.split('_')[2] )]
        title = ('Average Residual vs PhiSec for TRT Barrel Layer %s %s' % (lay,layer)) + ';Phi Sector;Average Residual [mm]' 
        name = 'm_trt_b_PhiSec,m_trt_b_residualR;trt_b_aveRes_l' + lay +layer
        tool.defineHistogram(name, title = title, type = 'TProfile', xbins = m_TRTB_nSectorBins, xmin = -0.5 , xmax = 31.5, ymin = -1.0 , ymax = 1.0)
    
    lrVsPhiSecBArray = helper.addArray([len(layersTRTB),len(layTRTB)], alg, 'TRTLRVsPhiSecB', topPath = pathResiduals)
    for postfix, tool in lrVsPhiSecBArray.Tools.items():
        layer = layersTRTB[int( postfix.split('_')[1] )]
        lay = layTRTB[int( postfix.split('_')[2] )]
        title = ('LR assignment vs Phi Sector for TRT Barrel Layer %s %s' % (lay,layer)) + ';Phi Sector;fraction of LR assignment correct' 
        name = 'm_trt_b_PhiSec,m_trt_b_lrVsPhiSec;trt_b_lr_l' + lay + layer
        tool.defineHistogram(name, title = title, type = 'TProfile', xbins = m_TRTB_nSectorBins, xmin = -0.5 , xmax = 31.5)

 
    #TRT EndCap Plots
    layersTRTEC = ['Endcap_A', 'Endcap_C']
    predictedRECArray = helper.addArray([len(layersTRTEC)], alg, 'TRTPredictedREC', topPath = pathResiduals)
    for postfix, tool in predictedRECArray.Tools.items():
        layer = layersTRTEC[int( postfix.split('_')[1] )]
        title = ('Predicted drift radius for TRT %s' % layer) + ';Predicted Drift Radius [mm];Entries' 
        name = 'm_trt_ec_PredictedR;trt_ec_PredictedR' + layer
        tool.defineHistogram(name, title = title, type = 'TH1F', xbins = 100 , xmin = -strawRadius , xmax = strawRadius)
    
    measuredRECArray = helper.addArray([len(layersTRTEC)], alg, 'TRTMeasuredREC', topPath = pathResiduals)
    for postfix, tool in measuredRECArray.Tools.items():
        layer = layersTRTEC[int( postfix.split('_')[1] )]
        title = ('Measured at line drift radius from TRT %s' % layer) + ';Measured Drift Radius [mm];Entries' 
        name = 'm_trt_ec_MeasuredR;trt_ec_MeasuredR' + layer
        tool.defineHistogram(name, title = title, type = 'TH1F', xbins = 100 , xmin = -strawRadius , xmax = strawRadius)

    residualRECArray = helper.addArray([len(layersTRTEC)], alg, 'TRTResidualREC', topPath = pathResiduals)
    for postfix, tool in residualRECArray.Tools.items():
        layer = layersTRTEC[int( postfix.split('_')[1] )]
        title = ('UnBiased Residual for TRT %s' % layer) + ';Residual [mm];Entries' 
        name = 'm_trt_ec_residualR;trt_ec_residualR' + layer
        tool.defineHistogram(name, title = title, type = 'TH1F', xbins = 200, xmin = m_minTRTResWindow , xmax = m_maxTRTResWindow)

    pullRECArray = helper.addArray([len(layersTRTEC)], alg, 'TRTPullREC', topPath = pathResiduals)
    for postfix, tool in pullRECArray.Tools.items():
        layer = layersTRTEC[int( postfix.split('_')[1] )]
        title = ('Pull for TRT %s' % layer) + ';Pull;Entries' 
        name = 'm_trt_ec_pullR;trt_ec_pullR_' + layer
        tool.defineHistogram(name, title = title, type = 'TH1F', xbins = 100, xmin = -m_RangeOfPullHistos , xmax = m_RangeOfPullHistos)

    pullRNoTubeECArray = helper.addArray([len(layersTRTEC)], alg, 'TRTPullRNoTubeEC', topPath = pathResiduals)
    for postfix, tool in pullRNoTubeECArray.Tools.items():
        layer = layersTRTEC[int( postfix.split('_')[1] )]
        title = ('Pull (notube) for TRT %s' % layer) + ';Pull;Entries' 
        name = 'm_trt_ec_pullR_notube;trt_ec_pullR_notube_' + layer
        tool.defineHistogram(name, title = title, type = 'TH1F', xbins = 100, xmin = -m_RangeOfPullHistos , xmax = m_RangeOfPullHistos)

    residualRECArray = helper.addArray([len(layersTRTEC)], alg, 'TRTResidualRNoTubeEC', topPath = pathResiduals)
    for postfix, tool in residualRECArray.Tools.items():
        layer = layersTRTEC[int( postfix.split('_')[1] )]
        title = ('UnBiased Residual (notube) for TRT %s' % layer) + ';Residual [mm];Entries' 
        name = 'm_trt_ec_residualR_notube;trt_ec_residualR_notube' + layer
        tool.defineHistogram(name, title = title, type = 'TH1F', xbins = 200, xmin = m_minTRTResWindow , xmax = m_maxTRTResWindow)

    lrECArray = helper.addArray([len(layersTRTEC)], alg, 'TRTLREC', topPath = pathResiduals)
    for postfix, tool in lrECArray.Tools.items():
        layer = layersTRTEC[int( postfix.split('_')[1] )]
        title = ('|0= LRcor !isTube | 1= LRcor isTube| 2= !LRcor !isTube | 3= !LRcor isTube %s' % layer) + ';;Entries' 
        name = 'm_trt_ec_lr;trt_ec_lr' + layer
        tool.defineHistogram(name, title = title, type = 'TH1F', xbins = 4, xmin = 0 , xmax = 4)

    resvsEtaECArray = helper.addArray([len(layersTRTEC)], alg, 'TRTResVsEtaEC', topPath = pathResiduals)
    for postfix, tool in resvsEtaECArray.Tools.items():
        layer = layersTRTEC[int( postfix.split('_')[1] )]
        title = ('Average Residual vs Eta for TRT %s' % layer) + ';Track Eta;Residual [mm]' 
        name = 'm_trt_ec_aveResVsTrackEta, m_trt_ec_residualR;trt_ec_aveResVsTrackEta_' + layer
        tool.defineHistogram(name, title = title, type = 'TProfile', xbins = 20,  xmin = -2.5 , xmax = 2.5, ymin = -1.0, ymax = 1.0)

    resVsPhiSecECArray = helper.addArray([len(layersTRTEC)], alg, 'TRTResVsPhiEC', topPath = pathResiduals)
    for postfix, tool in resVsPhiSecECArray.Tools.items():
        layer = layersTRTEC[int( postfix.split('_')[1] )]
        title = ('Average Residual vs PhiSec for TRT %s' % layer) + ';Phi Sector;Average Residual [mm]' 
        name = 'm_trt_ec_phi,m_trt_ec_residualR;trt_ec_aveResVsPhiSec_' + layer
        tool.defineHistogram(name, title = title, type = 'TProfile', xbins = m_TRTEC_nPhiBins, xmin = -0.5 , xmax = m_TRTEC_nPhiBins - 0.5, ymin = -1.0, ymax = 1.0)

    lrVsPhiSecECArray = helper.addArray([len(layersTRTEC)], alg, 'TRTLRVsPhiEC', topPath = pathResiduals)
    for postfix, tool in lrVsPhiSecECArray.Tools.items():
        layer = layersTRTEC[int( postfix.split('_')[1] )]
        title = ('LR assignment vs Phi Sector for TRT %s' % layer) + ';Phi Sector;fraction of LR assignment correct' 
        name = 'm_trt_ec_phi,m_trt_ec_lrVsPhiSec;trt_ec_lrVsPhiSec_' + layer
        tool.defineHistogram(name, title = title, type = 'TProfile', xbins = m_TRTEC_nPhiBins, xmin = -0.5 , xmax = m_TRTEC_nPhiBins - 0.5)

    # TRT residuals vs pt-wheel for end-caps
    residualECTRT2DProfArray = helper.addArray([len(layersTRTEC)], alg, 'TRTResECvspT_2DProf', topPath = pathResiduals)
    for postfix, tool in residualECTRT2DProfArray.Tools.items():
        layer = layersTRTEC[int(postfix.split('_')[1])]
        title = ('Residual vs pT-Wheel TRT %s; Wheel; Momentum p_{T}; Residual [mm]' % layer.upper()) 
        name = 'm_layer_or_wheel,m_pT,m_trt_ec_residualR;trt_ec_resVsqPtWheel_' + layer
        tool.defineHistogram(name, title = title, type = 'TProfile2D', xbins = 20, xmin = - 0.5, xmax = 20 - 0.5,
                                                  ybins = 80, ymin = -40, ymax = 40,
                                                  zmin = m_minTRTResWindow, zmax = m_maxTRTResWindow)

    #Silicon Plots
    #Common for Pixel and SCT, barrel and Endcaps
    varName = 'm_si_residualx;si_residualx'
    title = 'Silicon UnBiased X Residual;Residual [mm];Events'
    residualGroup.defineHistogram(varName, type='TH1F', path=pathResiduals, title=title, xbins=100, xmin=m_minSiResFillRange, xmax=m_maxSiResFillRange)

    varName = 'm_si_b_residualx;si_b_residualx'
    title = 'Silicon Barrel Only UnBiased X Residual;Residual [mm];Events'
    residualGroup.defineHistogram(varName, type='TH1F', path=pathResiduals, title=title, xbins=100, xmin=m_minSiResFillRange, xmax=m_maxSiResFillRange)

    varName = 'm_layerDisk_si, m_si_barrel_resX;si_barrel_resX'
    title = 'Residual X vs Silicon Barrel Layer;Layer;Residual [mm]'
    residualGroup.defineHistogram(varName, type='TH2F', path=pathResiduals, title=title, xbins=m_siliconBarrelLayers, xmin=-0.5, xmax=m_siliconBarrelLayers-0.5, ybins=100 * m_FinerBinningFactor, ymin=m_minSiResFillRange, ymax=m_maxSiResFillRange)

    varName = 'm_layerDisk_si, m_si_barrel_resY;si_barrel_resY'
    title = 'Residual Y vs Silicon Barrel Layer;Layer;Residual [mm]'
    residualGroup.defineHistogram(varName, type='TH2F', path=pathResiduals, title=title, xbins=m_siliconBarrelLayers, xmin=-0.5, xmax=m_siliconBarrelLayers-0.5, ybins=100 * m_FinerBinningFactor, ymin=m_minPIXResYFillRange, ymax=m_maxPIXResYFillRange)

    varName = 'm_layerDisk_si, m_si_barrel_pullX;si_barrel_pullX'
    title = 'Pull X vs Silicon Barrel Layer;Layer;Pull'
    residualGroup.defineHistogram(varName, type='TH2F', path=pathResiduals, title=title, xbins=m_siliconBarrelLayers, xmin=-0.5, xmax=m_siliconBarrelLayers-0.5, ybins=100, ymin=-m_RangeOfPullHistos, ymax=m_RangeOfPullHistos)

    varName = 'm_layerDisk_si, m_si_barrel_pullY;si_barrel_pullY'
    title = 'Pull Y vs Silicon Barrel Layer;Layer;Pull'
    residualGroup.defineHistogram(varName, type='TH2F', path=pathResiduals, title=title, xbins=m_siliconBarrelLayers, xmin=-0.5, xmax=m_siliconBarrelLayers-0.5, ybins=100, ymin=-m_RangeOfPullHistos, ymax=m_RangeOfPullHistos)

    varName = 'm_layerDisk_si, m_si_eca_resX;si_eca_resX'
    title = 'Residual X vs Silicon ECA Layer;Residual [mm]'
    residualGroup.defineHistogram(varName, type='TH2F', path=pathResiduals, title=title, xbins=m_siliconECLayers, xmin=-0.5, xmax=m_siliconECLayers-0.5, ybins=100 * m_FinerBinningFactor, ymin=m_minSiResFillRange, ymax=m_maxSiResFillRange)

    varName = 'm_layerDisk_si, m_si_eca_resY;si_eca_resY'
    title = 'Residual Y vs Silicon ECA Layer;Layer;Residual [mm]'
    residualGroup.defineHistogram(varName, type='TH2F', path=pathResiduals, title=title, xbins=m_siliconECLayers, xmin=-0.5, xmax=m_siliconECLayers-0.5, ybins=100 * m_FinerBinningFactor, ymin=m_minPIXResYFillRange, ymax=m_maxPIXResYFillRange)

    varName = 'm_layerDisk_si, m_si_eca_pullX;si_eca_pullX'
    title = 'Pull X vs Silicon ECA Layer;Layer;Pull'
    residualGroup.defineHistogram(varName, type='TH2F', path=pathResiduals, title=title, xbins=m_siliconECLayers, xmin=-0.5, xmax=m_siliconECLayers-0.5, ybins=100, ymin=-m_RangeOfPullHistos, ymax=m_RangeOfPullHistos)

    varName = 'm_layerDisk_si, m_si_eca_pullY;si_eca_pullY'
    title = 'Pull Y vs Silicon ECA Layer;Layer;Pull'
    residualGroup.defineHistogram(varName, type='TH2F', path=pathResiduals, title=title, xbins=m_siliconECLayers, xmin=-0.5, xmax=m_siliconECLayers-0.5, ybins=100, ymin=-m_RangeOfPullHistos, ymax=m_RangeOfPullHistos)

    varName = 'm_layerDisk_si, m_si_ecc_resX;si_ecc_resX'
    title = 'Residual X vs Silicon ECC Layer;Layer;Residual [mm]'
    residualGroup.defineHistogram(varName, type='TH2F', path=pathResiduals, title=title, xbins=m_siliconECLayers, xmin=-0.5, xmax=m_siliconECLayers-0.5, ybins=100 * m_FinerBinningFactor, ymin=m_minSiResFillRange, ymax=m_maxSiResFillRange)

    varName = 'm_layerDisk_si, m_si_ecc_resY;si_ecc_resY'
    title = 'Residual Y vs Silicon ECC Layer;Layer;Residual [mm]'
    residualGroup.defineHistogram(varName, type='TH2F', path=pathResiduals, title=title, xbins=m_siliconECLayers, xmin=-0.5, xmax=m_siliconECLayers-0.5, ybins=100 * m_FinerBinningFactor, ymin=m_minPIXResYFillRange, ymax=m_maxPIXResYFillRange)

    varName = 'm_layerDisk_si, m_si_ecc_pullX;si_ecc_pullX'
    title = 'Pull X vs Silicon ECA Layer;Layer;Pull'
    residualGroup.defineHistogram(varName, type='TH2F', path=pathResiduals, title=title, xbins=m_siliconECLayers, xmin=-0.5, xmax=m_siliconECLayers-0.5, ybins=100, ymin=-m_RangeOfPullHistos, ymax=m_RangeOfPullHistos)

    varName = 'm_layerDisk_si, m_si_ecc_pullY;si_ecc_pullY'
    title = 'Pull Y vs Silicon ECC Layer;Layer;Pull'
    residualGroup.defineHistogram(varName, type='TH2F', path=pathResiduals, title=title, xbins=m_siliconECLayers, xmin=-0.5, xmax=m_siliconECLayers-0.5, ybins=100, ymin=-m_RangeOfPullHistos, ymax=m_RangeOfPullHistos)

       
    #Pixel Barrel Plots
    varName = 'm_pix_b_residualx;pix_b_residualx'
    title = 'UnBiased X Residual Pixel Barrel;Residual [mm];Events'
    residualGroup.defineHistogram(varName, type='TH1F', path=pathResiduals, title=title, xbins=100, xmin=m_minSiResFillRange, xmax=m_maxSiResFillRange)

    varName = 'm_pix_b_biased_residualx;pix_b_biasedresidualx'
    title = 'Biased X Residual Pixel Barrel;Residual [mm];Events'
    residualGroup.defineHistogram(varName, type='TH1F', path=pathResiduals, title=title, xbins=100, xmin=m_minSiResFillRange, xmax=m_maxSiResFillRange)

    varName = 'm_pix_b_residualy;pix_b_residualy'
    title = 'UnBiased Y Residual Pixel Barrel;Residual [mm];Events'
    residualGroup.defineHistogram(varName, type='TH1F', path=pathResiduals, title=title, xbins=100, xmin=m_minPIXResYFillRange, xmax=m_maxPIXResYFillRange)

    varName = 'm_pix_b_biased_residualy;pix_b_biasedresidualy'
    title = 'Biased Y Residual Pixel Barrel;Residual [mm];Events'
    residualGroup.defineHistogram(varName, type='TH1F', path=pathResiduals, title=title, xbins=100, xmin=m_minPIXResYFillRange, xmax=m_maxPIXResYFillRange)

    layersPix = ['0', '1', '2', '3']
    layersName = ['IBL','B-layer','1','2']
    residualXArray = helper.addArray([len(layersPix)], alg, 'PixResidualX', topPath = pathResiduals)
    for postfix, tool in residualXArray.Tools.items():
        layer = layersPix[int( postfix.split('_')[1] )]
        title = ('UnBiased X Residual Pixel Barrel %s' % layersName[int(layer)]) 
        name = 'm_pix_residualsx;pix_b' + layer + '_residualx'
        tool.defineHistogram(name, title = title, type = 'TH1F',
                             xbins = 100 * m_FinerBinningFactor, xmin = m_minSiResFillRange, xmax = m_maxSiResFillRange)
 
    residualYArray = helper.addArray([len(layersPix)], alg, 'PixResidualY', topPath = pathResiduals)
    for postfix, tool in residualYArray.Tools.items():
        layer = layersPix[int( postfix.split('_')[1] )]
        title = ('UnBiased Y Residual Pixel Barrel %s' % layersName[int(layer)]) 
        name = 'm_pix_residualsy;pix_b' + layer + '_residualy'
        tool.defineHistogram(name, title = title, type = 'TH1F',
                             xbins = 100 * m_FinerBinningFactor, xmin = m_minPIXResYFillRange, xmax = m_maxPIXResYFillRange)

   # Define local X,Y 3D histograms (not really 3D histograms but TProfile 2D)
    residualX2DProfArray = helper.addArray([len(layersPix)], alg, 'PixResidualX_2DProf', topPath = pathResiduals)
    for postfix, tool in residualX2DProfArray.Tools.items():
        layer = layersPix[int( postfix.split('_')[1] )]
        title = ('Local X Residual vs Module Eta-Phi-ID Pixel Barrel layer %s; Mod Eta; Mod Phi; Local X Residual [mm]' % layersName[int(layer)]) 
        name = 'm_modEta,m_modPhi,m_pix_residualsx;pix_b' + layer + '_xresvsmodetaphi_2dprof'
        tool.defineHistogram(name, title = title, type = 'TProfile2D', xbins = m_EtaModulesPix[int(layer)], xmin = m_EtaModulesMinPix[int(layer)], xmax = m_EtaModulesMaxPix[int(layer)],
                                                  ybins = m_PhiModules[int(layer)], ymin = -0.5, ymax = m_PhiModules[int(layer)] - 0.5,
                                                  zmin = m_minSiResFillRange, zmax = m_maxSiResFillRange)

    residualY2DProfArray = helper.addArray([len(layersPix)], alg, 'PixResidualY_2DProf', topPath = pathResiduals)
    for postfix, tool in residualY2DProfArray.Tools.items():
        layer = layersPix[int( postfix.split('_')[1] )]
        title = ('Local Y Residual vs Module Eta-Phi-ID Pixel Barrel layer %s; Mod Eta; Mod Phi; Local Y Residual [mm]' % layersName[int(layer)]) 
        name = 'm_modEta,m_modPhi,m_pix_residualsy;pix_b' + layer + '_yresvsmodetaphi_2dprof'
        tool.defineHistogram(name, title = title, type = 'TProfile2D', xbins = m_EtaModulesPix[int(layer)], xmin = m_EtaModulesMinPix[int(layer)], xmax = m_EtaModulesMaxPix[int(layer)],
                                                  ybins = m_PhiModules[int(layer)], ymin = -0.5, ymax = m_PhiModules[int(layer)] - 0.5,
                                                  zmin = m_minPIXResYFillRange, zmax = m_maxPIXResYFillRange)
        
    pullXArray = helper.addArray([len(layersPix)], alg, 'PixPullX', topPath = pathResiduals)
    for postfix, tool in pullXArray.Tools.items():
        layer = layersPix[int( postfix.split('_')[1] )]
        title = ('UnBiased X Pull Pixel Barrel %s' % layer) + ';Pull'
        name = 'm_pix_pullsx;pix_b' + layer + '_pullx'
        tool.defineHistogram(name, title = title, type = 'TH1F',
                             xbins = 100 * m_FinerBinningFactor, xmin = -m_RangeOfPullHistos, xmax = m_RangeOfPullHistos)

    pullYArray = helper.addArray([len(layersPix)], alg, 'PixPullY', topPath = pathResiduals)
    for postfix, tool in pullYArray.Tools.items():
        layer = layersPix[int( postfix.split('_')[1] )]
        title = ('UnBiased Y Pull Pixel Barrel %s' % layer) + ';Pull'
        name = 'm_pix_pullsy;pix_b' + layer + '_pully'
        tool.defineHistogram(name, title = title, type = 'TH1F',
                             xbins = 100 * m_FinerBinningFactor, xmin = -m_RangeOfPullHistos, xmax = m_RangeOfPullHistos)

    resXvsEtaArray = helper.addArray([len(layersPix)], alg, 'PixResidualXvsEta', topPath = pathResiduals)
    for postfix, tool in resXvsEtaArray.Tools.items():
        layer = layersPix[int( postfix.split('_')[1] )]
        layerInd = int(layer)
        EtaModules = m_EtaModulesPix[layerInd]
        EtaModulesMin = m_EtaModulesMinPix[layerInd]
        EtaModulesMax = m_EtaModulesMaxPix[layerInd]
        title = ('X Residual Distribution vs Module Eta-ID Pixel Barrel %s' % layer) + ';Mod Eta;Residual [mm]'
        name = 'm_modEta,m_residualX;pix_b' + layer + '_xresidualvseta_2d'
        tool.defineHistogram(name, title = title, type = 'TH2F',
                             xbins = EtaModules, xmin = EtaModulesMin, xmax = EtaModulesMax,
                             ybins = 50 * m_FinerBinningFactor, ymin = m_minSiResFillRange, ymax = m_maxSiResFillRange)

    resYvsEtaArray = helper.addArray([len(layersPix)], alg, 'PixResidualYvsEta', topPath = pathResiduals)
    for postfix, tool in resYvsEtaArray.Tools.items():
        layer = layersPix[int( postfix.split('_')[1] )]
        layerInd = int(layer)
        EtaModules = m_EtaModulesPix[layerInd]
        EtaModulesMin = m_EtaModulesMinPix[layerInd]
        EtaModulesMax = m_EtaModulesMaxPix[layerInd]
        title = ('Y Residual Distribution vs Module Eta-ID Pixel Barrel %s' % layer) + ';Mod Eta;Residual [mm]'
        name = 'm_modEta,m_residualY;pix_b' + layer + '_yresidualvseta_2d'
        tool.defineHistogram(name, title = title, type = 'TH2F',
                             xbins = EtaModules, xmin = EtaModulesMin, xmax = EtaModulesMax,
                             ybins = 50 * m_FinerBinningFactor, ymin = m_minPIXResYFillRange, ymax = m_maxPIXResYFillRange)

    resXvsPhiArray = helper.addArray([len(layersPix)], alg, 'PixResidualXvsPhi', topPath = pathResiduals)
    for postfix, tool in resXvsPhiArray.Tools.items():
        layer = layersPix[int( postfix.split('_')[1] )]
        layerInd = int(layer)
        PhiModules = m_PhiModules[layerInd]
        title = ('X Residual Distribution vs Module Phi-ID Pixel Barrel %s' % layer) + ';Mod Phi;Residual [mm]'
        name = 'm_modPhi,m_residualX;pix_b' + layer + '_xresidualvsphi_2d'
        tool.defineHistogram(name, title = title, type = 'TH2F',
                             xbins = PhiModules, xmin = - 0.5, xmax = PhiModules - 0.5,
                             ybins = 50 * m_FinerBinningFactor, ymin = m_minSiResFillRange, ymax = m_maxSiResFillRange)

    resYvsPhiArray = helper.addArray([len(layersPix)], alg, 'PixResidualYvsPhi', topPath = pathResiduals)
    for postfix, tool in resYvsPhiArray.Tools.items():
        layer = layersPix[int( postfix.split('_')[1] )]
        layerInd = int(layer)
        PhiModules = m_PhiModules[layerInd]
        title = ('Y Residual Distribution vs Module Phi-ID Pixel Barrel %s' % layer) + ';Mod Phi;Residual [mm]'
        name = 'm_modPhi,m_residualY;pix_b' + layer + '_yresidualvsphi_2d'
        tool.defineHistogram(name, title = title, type = 'TH2F',
                             xbins = PhiModules, xmin = - 0.5, xmax = PhiModules - 0.5,
                             ybins = 50 * m_FinerBinningFactor, ymin = m_minPIXResYFillRange, ymax = m_maxPIXResYFillRange)

    varName = 'm_modPhiShift_barrel,m_residualX_barrel;pix_b_xresvsmodphi_profile'
    title = 'X Residual Mean vs (Modified) Module Phi-ID Pixel Barrel;(Modified) Phi-ID;Residual [mm]'
    residualGroup.defineHistogram(varName, title = title, type = 'TProfile', path=pathResiduals,
                                  xbins = m_PhiModulesShift_barrel, xmin = 0, xmax = m_PhiModulesShift_barrel)
    
    varName = 'm_modPhiShift_barrel,m_residualY_barrel;pix_b_yresvsmodphi_profile'
    title = 'Y Residual Mean vs (Modified) Module Phi-ID Pixel Barrel;(Modified) Phi-ID;Residual [mm]'
    residualGroup.defineHistogram(varName, title = title, type = 'TProfile', path=pathResiduals,
                                  xbins = m_PhiModulesShift_barrel, xmin = 0, xmax = m_PhiModulesShift_barrel)
    
    varName = 'm_modEtaShift_barrel,m_residualX_barrel;pix_b_xresvsmodeta_profile'
    title = 'X Residual Mean vs (Modified) Module Eta-ID Pixel Barrel;(Modified) Eta-ID;Residual [mm]'
    residualGroup.defineHistogram(varName, title = title, type = 'TProfile', path=pathResiduals,
                                  xbins = m_EtaModulesShift_barrel, xmin = 0, xmax = m_EtaModulesShift_barrel)
    
    varName = 'm_modEtaShift_barrel,m_residualY_barrel;pix_b_yresvsmodeta_profile'
    title = 'Y Residual Mean vs (Modified) Module Eta-ID Pixel Barrel;(Modified) Eta-ID;Residual [mm]'
    residualGroup.defineHistogram(varName, title = title, type = 'TProfile', path=pathResiduals,
                                  xbins = m_EtaModulesShift_barrel, xmin = 0, xmax = m_EtaModulesShift_barrel)
       
    #Pixel EndCap A plots
    layersECPix = ['0', '1', '2']
    varName = 'm_pix_eca_residualx;pix_eca_residualx'
    title = 'UnBiased X Residual Pixel EndCap A;Residual [mm]'
    residualGroup.defineHistogram(varName, type='TH1F', path=pathResiduals, title=title, xbins=100, xmin=m_minSiResFillRange, xmax=m_maxSiResFillRange)

    varName = 'm_pix_eca_residualy;pix_eca_residualy'
    title = 'UnBiased Y Residual Pixel EndCap A;Residual [mm]'
    residualGroup.defineHistogram(varName, type='TH1F', path=pathResiduals, title=title, xbins=100, xmin=m_minPIXResYFillRange, xmax=m_maxPIXResYFillRange)

    varName = 'm_pix_eca_pullx;pix_eca_pulllx'
    title = 'UnBiased X Pull Pixel EndCap A;Pull'
    residualGroup.defineHistogram(varName, type='TH1F', path=pathResiduals, title=title, xbins=100, xmin=-m_RangeOfPullHistos, xmax=m_RangeOfPullHistos)

    varName = 'm_pix_eca_pully;pix_eca_pullly'
    title = 'UnBiased Y Pull Pixel EndCap A;Pull'
    residualGroup.defineHistogram(varName, type='TH1F', path=pathResiduals, title=title, xbins=100, xmin=-m_RangeOfPullHistos, xmax=m_RangeOfPullHistos)

    residualECAXArray = helper.addArray([len(layersECPix)], alg, 'PixResidualXECA', topPath = pathResiduals)
    for postfix, tool in residualECAXArray.Tools.items():
        layer = layersECPix[int( postfix.split('_')[1] )]
        title = ('UNBIASED X Residual Average vs Module Phi of Pixel Endcap A Disk %s' % layer) 
        name = 'm_modPhi,m_pix_eca_residualx;m_pix_eca_unbiased_xresvsmodphi_disk' + layer
        tool.defineHistogram(name, title = title, type = 'TProfile',
                             xbins = m_PhiModulesPerRing, xmin = -0.5, xmax = m_PhiModulesPerRing - 0.5)

    residualECAYArray = helper.addArray([len(layersECPix)], alg, 'PixResidualYECA', topPath = pathResiduals)
    for postfix, tool in residualECAYArray.Tools.items():
        layer = layersECPix[int( postfix.split('_')[1] )]
        title = ('UNBIASED Y Residual Average vs Module Phi of Pixel Endcap A Disk  %s' % layer) 
        name = 'm_modPhi,m_pix_eca_residualy;m_pix_eca_unbiased_yresvsmodphi_disk' + layer
        tool.defineHistogram(name, title = title, type = 'TProfile',
                             xbins = m_PhiModulesPerRing, xmin = -0.5, xmax = m_PhiModulesPerRing - 0.5)
        
    # Define local X,Y 3D histograms for endcaps A, C (not really 3D histograms but TProfile 2D)
    endcapsPix = ['a', 'c']
    residualECX2DProfArray = helper.addArray([len(endcapsPix)], alg, 'PixResidualXEC_2DProf', topPath = pathResiduals)
    for postfix, tool in residualECX2DProfArray.Tools.items():
        layer = endcapsPix[int(postfix.split('_')[1])]
        title = ('Local X Residual vs Module Disk-Phi-ID Pixel Endcap %s; Disk; Mod Phi; Local X Residual [mm]' % layer.upper()) 
        name = 'm_layerDisk,m_modPhi,m_pix_ec_residualx;pix_ec' + layer + '_xresvsmodetaphi_2dprof'
        tool.defineHistogram(name, title = title, type = 'TProfile2D', xbins = 3, xmin = - 0.5, xmax = 3 - 0.5,
                                                  ybins = m_PhiModulesPerRing, ymin = -0.5, ymax = m_PhiModulesPerRing - 0.5,
                                                  zmin = m_minSiResFillRange, zmax = m_maxSiResFillRange)

    residualECY2DProfArray = helper.addArray([len(endcapsPix)], alg, 'PixResidualYEC_2DProf', topPath = pathResiduals)
    for postfix, tool in residualECY2DProfArray.Tools.items():
        layer = endcapsPix[int( postfix.split('_')[1])]
        title = ('Local Y Residual vs Module Eta-Phi-ID Pixel Endcap %s; Disk; Mod Phi; Local Y Residual [mm]' % layer.upper()) 
        name = 'm_layerDisk,m_modPhi,m_pix_ec_residualy;pix_ec' + layer + '_yresvsmodetaphi_2dprof'
        tool.defineHistogram(name, title = title, type = 'TProfile2D', xbins = 3, xmin = - 0.5, xmax = 3 - 0.5,
                                                  ybins = m_PhiModulesPerRing, ymin = -0.5, ymax = m_PhiModulesPerRing - 0.5,
                                                  zmin = m_minPIXResYFillRange, zmax = m_maxPIXResYFillRange)

    varName = 'm_modPhiShift_eca,m_residualX_eca;pix_eca_xresvsmodphi_2d'
    title = 'X Residual Mean vs (Modified) Module Phi-ID Pixel ECA;(Modified) Phi-ID;Residual [mm]'
    residualGroup.defineHistogram(varName, title = title, type = 'TH2F', path=pathResiduals,
                                  xbins = m_PhiModulesShift_ec, xmin = 0, xmax = m_PhiModulesShift_ec,
                                  ybins = 100 * m_FinerBinningFactor, ymin = m_minSiResFillRange, ymax = m_maxSiResFillRange)

    varName = 'm_modPhiShift_eca,m_residualY_eca;pix_eca_yresvsmodphi_2d'
    title = 'Y Residual Mean vs (Modified) Module Phi-ID Pixel ECA;(Modified) Phi-ID;Residual [mm]'
    residualGroup.defineHistogram(varName, title = title, type = 'TH2F', path=pathResiduals,
                                  xbins = m_PhiModulesShift_ec, xmin = 0, xmax = m_PhiModulesShift_ec,
                                  ybins = 100 * m_FinerBinningFactor, ymin = m_minPIXResYFillRange, ymax = m_maxPIXResYFillRange)

    varName = 'm_modPhiShift_eca,m_residualX_eca;pix_eca_xresvsmodphi_profile'
    title = 'X Residual Mean vs (Modified) Module Phi-ID Pixel ECA;(Modified) Phi-ID;Residual [mm]'
    residualGroup.defineHistogram(varName, title = title, type = 'TProfile', path=pathResiduals,
                                  xbins = m_PhiModulesShift_ec, xmin = 0, xmax = m_PhiModulesShift_ec)
    
    varName = 'm_modPhiShift_eca,m_residualY_eca;pix_eca_yresvsmodphi_profile'
    title = 'Y Residual Mean vs (Modified) Module Phi-ID Pixel ECA;(Modified) Phi-ID;Residual [mm]'
    residualGroup.defineHistogram(varName, title = title, type = 'TProfile', path=pathResiduals,
                                  xbins = m_PhiModulesShift_ec, xmin = 0, xmax = m_PhiModulesShift_ec)
    
    #Pixel EndCap C plots
    varName = 'm_pix_ecc_residualx;pix_ecc_residualx'
    title = 'UnBiased X Residual Pixel EndCap C;Residual [mm]'
    residualGroup.defineHistogram(varName, type='TH1F', path=pathResiduals, title=title, xbins=100, xmin=m_minSiResFillRange, xmax=m_maxSiResFillRange)

    varName = 'm_pix_ecc_residualy;pix_ecc_residualy'
    title = 'UnBiased Y Residual Pixel EndCap C;Residual [mm]'
    residualGroup.defineHistogram(varName, type='TH1F', path=pathResiduals, title=title, xbins=100, xmin=m_minPIXResYFillRange, xmax=m_maxPIXResYFillRange)

    varName = 'm_pix_ecc_pullx;pix_ecc_pulllx'
    title = 'UnBiased X Pull Pixel EndCap C;Pull'
    residualGroup.defineHistogram(varName, type='TH1F', path=pathResiduals, title=title, xbins=100, xmin=-m_RangeOfPullHistos, xmax=m_RangeOfPullHistos)

    varName = 'm_pix_ecc_pully;pix_ecc_pullly'
    title = 'UnBiased Y Pull Pixel EndCap C;Pull'
    residualGroup.defineHistogram(varName, type='TH1F', path=pathResiduals, title=title, xbins=100, xmin=-m_RangeOfPullHistos, xmax=m_RangeOfPullHistos)

    residualECCXArray = helper.addArray([len(layersECPix)], alg, 'PixResidualXECC', topPath = pathResiduals)
    for postfix, tool in residualECCXArray.Tools.items():
        layer = layersECPix[int( postfix.split('_')[1] )]
        title = ('UNBIASED X Residual Average vs Module Phi of Pixel Endcap C Disk %s' % layer) 
        name = 'm_modPhi,m_pix_ecc_residualx;m_pix_ecc_unbiased_xresvsmodphi_disk' + layer
        tool.defineHistogram(name, title = title, type = 'TProfile',
                             xbins = m_PhiModulesPerRing, xmin = -0.5, xmax = m_PhiModulesPerRing - 0.5)

    residualECCYArray = helper.addArray([len(layersECPix)], alg, 'PixResidualYECC', topPath = pathResiduals)
    for postfix, tool in residualECCYArray.Tools.items():
        layer = layersECPix[int( postfix.split('_')[1] )]
        title = ('UNBIASED Y Residual Average vs Module Phi of Pixel Endcap C Disk  %s' % layer) 
        name = 'm_modPhi,m_pix_ecc_residualy;m_pix_ecc_unbiased_yresvsmodphi_disk' + layer
        tool.defineHistogram(name, title = title, type = 'TProfile',
                             xbins = m_PhiModulesPerRing, xmin = -0.5, xmax = m_PhiModulesPerRing - 0.5)

    varName = 'm_modPhiShift_ecc,m_residualX_ecc;pix_ecc_xresvsmodphi_2d'
    title = 'X Residual Distribution vs (Modified) Module Phi-ID Pixel ECC;(Modified) Phi-ID;Residual [mm]'
    residualGroup.defineHistogram(varName, title = title, type = 'TH2F', path=pathResiduals,
                                  xbins = m_PhiModulesShift_ec, xmin = 0, xmax = m_PhiModulesShift_ec,
                                  ybins = 100 * m_FinerBinningFactor, ymin = m_minSiResFillRange, ymax = m_maxSiResFillRange)

    varName = 'm_modPhiShift_ecc,m_residualY_ecc;pix_ecc_yresvsmodphi_2d'
    title = 'Y Residual Distribution vs (Modified) Module Phi-ID Pixel ECC;(Modified) Phi-ID;Residual [mm]'
    residualGroup.defineHistogram(varName, title = title, type = 'TH2F', path=pathResiduals,
                                  xbins = m_PhiModulesShift_ec, xmin = 0, xmax = m_PhiModulesShift_ec,
                                  ybins = 100 * m_FinerBinningFactor, ymin = m_minPIXResYFillRange, ymax = m_maxPIXResYFillRange)

    varName = 'm_modPhiShift_ecc,m_residualX_ecc;pix_ecc_xresvsmodphi_profile'
    title = 'X Residual Distribution vs (Modified) Module Phi-ID Pixel ECC;(Modified) Phi-ID;Residual [mm]'
    residualGroup.defineHistogram(varName, title = title, type = 'TProfile', path=pathResiduals,
                                  xbins = m_PhiModulesShift_ec, xmin = 0, xmax = m_PhiModulesShift_ec)

    varName = 'm_modPhiShift_ecc,m_residualY_ecc;pix_ecc_yresvsmodphi_profile'
    title = 'Y Residual Distribution vs (Modified) Module Phi-ID Pixel ECC;(Modified) Phi-ID;Residual [mm]'
    residualGroup.defineHistogram(varName, title = title, type = 'TProfile', path=pathResiduals,
                                  xbins = m_PhiModulesShift_ec, xmin = 0, xmax = m_PhiModulesShift_ec)

    #SCT Barrel Plots
    varName = 'm_sct_b_residualx;sct_b_residualx'
    title = 'UnBiased X Residual SCT Barrel;Residual [mm]'
    residualGroup.defineHistogram(varName, type='TH1F', path=pathResiduals, title=title, xbins=100, xmin=m_minSCTResFillRange, xmax=m_maxSCTResFillRange)

    varName = 'm_sct_b_biased_residualx;sct_b_biasedresidualx'
    title = 'Biased X Residual SCT Barrel;Residual [mm]'
    residualGroup.defineHistogram(varName, type='TH1F', path=pathResiduals, title=title, xbins=100, xmin=m_minSCTResFillRange, xmax=m_maxSCTResFillRange)

    layersSCTB = ['0', '1', '2', '3']
    residualSCTXArray = helper.addArray([len(layersSCTB)], alg, 'SCTResidualX', topPath = pathResiduals)
    for postfix, tool in residualSCTXArray.Tools.items():
        layer = layersSCTB[int( postfix.split('_')[1] )]
        title = ('UnBiased X Residual SCT Barrel %s' % layer) 
        name = 'm_sct_residualsx;sct_b' + layer + '_residualx'
        tool.defineHistogram(name, title = title, type = 'TH1F',
                             xbins = 100 * m_FinerBinningFactor, xmin = m_minSCTResFillRange, xmax = m_maxSCTResFillRange)

    # Define local X 3D histograms (not really 3D histograms but TProfile 2D)
    residualSCTX2DProfArray = helper.addArray([len(layersSCTB)], alg, 'SCTResidualX_2DProf', topPath = pathResiduals)
    for postfix, tool in residualSCTX2DProfArray.Tools.items():
        layer = layersSCTB[int( postfix.split('_')[1] )]
        title = ('Local X Residual vs Module Eta-Phi-ID SCT Barrel layer %s; Mod Eta; Mod Phi; Local X Residual [mm]' % layer) 
        name = 'm_modEta,m_modPhi,m_sct_residualsx;sct_b' + layer + '_xresvsmodetaphi_2dprof'
        tool.defineHistogram(name, title = title, type = 'TProfile2D', xbins = m_EtaModulesSCT, xmin = m_EtaModulesMinSCT, xmax = m_EtaModulesMaxSCT,
                                                  ybins = m_PhiModulesSCT[int(layer)], ymin = -0.5, ymax = m_PhiModulesSCT[int(layer)] - 0.5,
                                                  zmin = m_minSCTResFillRange, zmax = m_maxSCTResFillRange)

    residualSCTXS02DProfArray = helper.addArray([len(layersSCTB)], alg, 'SCT_s0_ResidualX_2DProf', topPath = pathResiduals)
    for postfix, tool in residualSCTXS02DProfArray.Tools.items():
        layer = layersSCTB[int( postfix.split('_')[1] )]
        title = ('Local X Residual vs Module Eta-Phi-ID SCT Barrel layer %s side 0; Mod Eta; Mod Phi; Local X Residual side 0 [mm]' % layer) 
        name = 'm_modEta,m_modPhi,m_sct_residualsx;sct_b' + layer + '_s0_xresvsmodetaphi_2dprof'
        tool.defineHistogram(name, title = title, type = 'TProfile2D', xbins = m_EtaModulesSCT, xmin = m_EtaModulesMinSCT, xmax = m_EtaModulesMaxSCT,
                                                  ybins = m_PhiModulesSCT[int(layer)], ymin = -0.5, ymax = m_PhiModulesSCT[int(layer)] - 0.5,
                                                  zmin = m_minSCTResFillRange, zmax = m_maxSCTResFillRange)

    residualSCTXS12DProfArray = helper.addArray([len(layersSCTB)], alg, 'SCT_s1_ResidualX_2DProf', topPath = pathResiduals)
    for postfix, tool in residualSCTXS12DProfArray.Tools.items():
        layer = layersSCTB[int( postfix.split('_')[1] )]
        title = ('Local X Residual vs Module Eta-Phi-ID SCT Barrel layer %s side 1; Mod Eta; Mod Phi; Local X Residual side 1 [mm]' % layer) 
        name = 'm_modEta,m_modPhi,m_sct_residualsx;sct_b' + layer + '_s1_xresvsmodetaphi_2dprof'
        tool.defineHistogram(name, title = title, type = 'TProfile2D', xbins = m_EtaModulesSCT, xmin = m_EtaModulesMinSCT, xmax = m_EtaModulesMaxSCT,
                                                   ybins = m_PhiModulesSCT[int(layer)], ymin = -0.5, ymax = m_PhiModulesSCT[int(layer)] - 0.5,
                                                   zmin = m_minSCTResFillRange, zmax = m_maxSCTResFillRange)

    pullSCTXArray = helper.addArray([len(layersSCTB)], alg, 'SCTPullX', topPath = pathResiduals)
    for postfix, tool in pullSCTXArray.Tools.items():
        layer = layersSCTB[int( postfix.split('_')[1] )]
        title = ('UnBiased X Pull SCT Barrel %s' % layer) + ';Pull'
        name = 'm_sct_pullsx;sct_b' + layer + '_pullx'
        tool.defineHistogram(name, title = title, type = 'TH1F',
                             xbins = 100 * m_FinerBinningFactor, xmin = -m_RangeOfPullHistos, xmax = m_RangeOfPullHistos)

    resXvsEtaSCTArray = helper.addArray([len(layersSCTB)], alg, 'SCTResidualXvsEta', topPath = pathResiduals)
    for postfix, tool in resXvsEtaSCTArray.Tools.items():
        layer = layersSCTB[int( postfix.split('_')[1] )]
        layerInd = int(layer)
        title = ('X Residual Distribution vs Module Eta-ID SCT Barrel %s' % layer) + ';Mod Eta;Residual [mm]'
        name = 'm_modEta,m_residualX;sct_b' + layer + '_xresidualvseta_2d'
        tool.defineHistogram(name, title = title, type = 'TH2F',
                             xbins = m_EtaModulesSCT, xmin = m_EtaModulesMinSCT, xmax = m_EtaModulesMaxSCT,
                             ybins = 50 * m_FinerBinningFactor, ymin = m_minSCTResFillRange, ymax = m_maxSCTResFillRange)

    resXvsPhiSCTArray = helper.addArray([len(layersPix)], alg, 'SCTResidualXvsPhi', topPath = pathResiduals)
    for postfix, tool in resXvsPhiSCTArray.Tools.items():
        layer = layersSCTB[int( postfix.split('_')[1] )]
        layerInd = int(layer)
        PhiModules = m_PhiModulesSCT[layerInd]
        title = ('X Residual Distribution vs Module Phi-ID SCT Barrel %s' % layer) + ';Mod Phi;Residual [mm]'
        name = 'm_modPhi,m_residualX;sct_b' + layer + '_xresidualvsphi_2d'
        tool.defineHistogram(name, title = title, type = 'TH2F',
                             xbins = PhiModules, xmin = - 0.5, xmax = PhiModules - 0.5,
                             ybins = 50 * m_FinerBinningFactor, ymin = m_minSCTResFillRange, ymax = m_maxSCTResFillRange)

    varName = 'm_modPhiShift_sct_barrel,m_residualX_sct_barrel;sct_b_xresvsmodphi_profile'
    title = 'X Residual Mean vs (Modified) Module Phi-ID SCT Barrel;(Modified) Phi-ID;Residual [mm]'
    residualGroup.defineHistogram(varName, title = title, type = 'TProfile', path=pathResiduals,
                                  xbins = m_PhiModulesShift_sct_barrel, xmin = - 0.5, xmax = m_PhiModulesShift_sct_barrel - 0.5)

    varName = 'm_modEtaShift_sct_barrel,m_residualX_sct_barrel;sct_b_xresvsmodeta_profile'
    title = 'X Residual Mean vs (Modified) Module Eta-ID SCT Barrel;(Modified) Eta-ID;Residual [mm]'
    residualGroup.defineHistogram(varName, title = title, type = 'TProfile', path=pathResiduals,
                                  xbins = m_EtaModulesShift_sct_barrel, xmin = - 0.5, xmax = m_EtaModulesShift_sct_barrel - 0.5)

    #SCT EndCap A plots
    varName = 'm_sct_eca_residualx;sct_eca_residualx'
    title = 'UnBiased X Residual SCT EndCap A;Residual [mm]'
    residualGroup.defineHistogram(varName, type='TH1F', path=pathResiduals, title=title, xbins=100, xmin=m_minSCTResFillRange, xmax=m_maxSCTResFillRange)

    # Define local X 3D histograms (not really 3D histograms but TProfile 2D)
    layersECsct = ['0', '1', '2', '3', '4', '5', '6', '7', '8']
    residualSCTECAX2DProfArray = helper.addArray([len(layersECsct)], alg, 'SCTECAResidualX_2DProf', topPath = pathResiduals)
    for postfix, tool in residualSCTECAX2DProfArray.Tools.items():
        layer = layersECsct[int( postfix.split('_')[1] )]
        title = ('Local X Residual vs Module Eta-Phi-ID SCT Endcap A Disk %s; Mod Eta; Mod Phi; Local X Residual [mm]' % layer) 
        name = 'm_modEta,m_modPhi,m_sct_eca_residualx;sct_eca' + layer + '_xresvsmodetaphi_2dprof'
        tool.defineHistogram(name, title = title, type = 'TProfile2D', xbins = m_EtaModulesSCTEC, xmin = -0.5, xmax = m_EtaModulesSCTEC - 0.5,
                                                  ybins = m_PhiModulesPerRingSCT, ymin = - 0.5, ymax = m_PhiModulesPerRingSCT - 0.5,
                                                  zmin=m_minSCTResFillRange, zmax=m_maxSCTResFillRange)

    residualSCTECAXS02DProfArray = helper.addArray([len(layersECsct)], alg, 'SCTECA_s0_ResidualX_2DProf', topPath = pathResiduals)
    for postfix, tool in residualSCTECAXS02DProfArray.Tools.items():
        layer = layersECsct[int( postfix.split('_')[1] )]
        title = ('Local X Residual vs Module Eta-Phi-ID SCT Endcap A Disk %s side 0; Mod Eta; Mod Phi; Local X Residual side 0 [mm]' % layer) 
        name = 'm_modEta,m_modPhi,m_sct_eca_residualx;sct_eca' + layer + '_s0_xresvsmodetaphi_2dprof'
        tool.defineHistogram(name, title = title, type = 'TProfile2D', xbins = m_EtaModulesSCTEC, xmin = -0.5, xmax = m_EtaModulesSCTEC - 0.5,
                                                  ybins = m_PhiModulesPerRingSCT, ymin = - 0.5, ymax = m_PhiModulesPerRingSCT - 0.5,
                                                  zmin=m_minSCTResFillRange, zmax=m_maxSCTResFillRange)

    residualSCTECAXS12DProfArray = helper.addArray([len(layersECsct)], alg, 'SCTECA_s1_ResidualX_2DProf', topPath = pathResiduals)
    for postfix, tool in residualSCTECAXS12DProfArray.Tools.items():
        layer = layersECsct[int( postfix.split('_')[1] )]
        title = ('Local X Residual vs Module Eta-Phi-ID SCT Endcap A Disk %s side 1; Mod Eta; Mod Phi; Local X Residual side 1 [mm]' % layer) 
        name = 'm_modEta,m_modPhi,m_sct_eca_residualx;sct_eca' + layer + '_s1_xresvsmodetaphi_2dprof'
        tool.defineHistogram(name, title = title, type = 'TProfile2D', xbins = m_EtaModulesSCTEC, xmin = -0.5, xmax = m_EtaModulesSCTEC - 0.5,
                                                  ybins = m_PhiModulesPerRingSCT, ymin = - 0.5, ymax = m_PhiModulesPerRingSCT - 0.5,
                                                  zmin=m_minSCTResFillRange, zmax=m_maxSCTResFillRange)
        
    varName = 'm_sct_eca_pullx;sct_eca_pulllx'
    title = 'UnBiased X Pull SCT EndCap A;Pull'
    residualGroup.defineHistogram(varName, type='TH1F', path=pathResiduals, title=title, xbins=100, xmin=-m_RangeOfPullHistos, xmax=m_RangeOfPullHistos)

    varName = 'm_modPhiShift_sct_eca,m_residualX_sct_eca;sct_eca_xresvsmodphi_profile'
    title = 'X Residual Mean vs (Modified) Module Phi-ID SCT ECA;(Modified) Phi-ID;Residual [mm]'
    residualGroup.defineHistogram(varName, title = title, type = 'TProfile', path=pathResiduals,
                                  xbins = m_PhiModulesShift_sct_ec, xmin = - 0.5, xmax = m_PhiModulesShift_sct_ec - 0.5)

    #SCT EndCap C plots
    varName = 'm_sct_ecc_residualx;sct_ecc_residualx'
    title = 'UnBiased X Residual SCT EndCap C;Residual [mm]'
    residualGroup.defineHistogram(varName, type='TH1F', path=pathResiduals, title=title, xbins=100, xmin=m_minSCTResFillRange, xmax=m_maxSCTResFillRange)

    # Define local X 3D histograms (not really 3D histograms but TProfile 2D)
    residualSCTECCX2DProfArray = helper.addArray([len(layersECsct)], alg, 'SCTECCResidualX_2DProf', topPath = pathResiduals)
    for postfix, tool in residualSCTECCX2DProfArray.Tools.items():
        layer = layersECsct[int( postfix.split('_')[1] )]
        title = ('Local X Residual vs Module Eta-Phi-ID SCT Endcap C Disk %s; Mod Eta; Mod Phi; Local X Residual [mm]' % layer) 
        name = 'm_modEta,m_modPhi,m_sct_ecc_residualx;sct_ecc' + layer + '_xresvsmodetaphi_2dprof'
        tool.defineHistogram(name, title = title, type = 'TProfile2D', xbins = m_EtaModulesSCTEC, xmin = -0.5, xmax = m_EtaModulesSCTEC - 0.5,
                                                  ybins = m_PhiModulesPerRingSCT, ymin = -0.5, ymax = m_PhiModulesPerRingSCT - 0.5,
                                                  zmin=m_minSCTResFillRange, zmax=m_maxSCTResFillRange)

    residualSCTECCXS02DProfArray = helper.addArray([len(layersECsct)], alg, 'SCTECC_s0_ResidualX_2DProf', topPath = pathResiduals)
    for postfix, tool in residualSCTECCXS02DProfArray.Tools.items():
        layer = layersECsct[int( postfix.split('_')[1] )]
        title = ('Local X Residual vs Module Eta-Phi-ID SCT Endcap C Disk %s side 0; Mod Eta; Mod Phi; Local X Residual side 0 [mm]' % layer) 
        name = 'm_modEta,m_modPhi,m_sct_ecc_residualx;sct_ecc' + layer + '_s0_xresvsmodetaphi_2dprof'
        tool.defineHistogram(name, title = title, type = 'TProfile2D', xbins = m_EtaModulesSCTEC, xmin = -0.5, xmax = m_EtaModulesSCTEC - 0.5,
                                                  ybins = m_PhiModulesPerRingSCT, ymin = - 0.5, ymax = m_PhiModulesPerRingSCT - 0.5,
                                                  zmin=m_minSCTResFillRange, zmax=m_maxSCTResFillRange)

    residualSCTECCXS12DProfArray = helper.addArray([len(layersECsct)], alg, 'SCTECC_s1_ResidualX_2DProf', topPath = pathResiduals)
    for postfix, tool in residualSCTECCXS12DProfArray.Tools.items():
        layer = layersECsct[int( postfix.split('_')[1] )]
        title = ('Local X Residual vs Module Eta-Phi-ID SCT Endcap C Disk %s side 1; Mod Eta; Mod Phi; Local X Residual side 1 [mm]' % layer) 
        name = 'm_modEta,m_modPhi,m_sct_ecc_residualx;sct_ecc' + layer + '_s1_xresvsmodetaphi_2dprof'
        tool.defineHistogram(name, title = title, type = 'TProfile2D', xbins = m_EtaModulesSCTEC, xmin = -0.5, xmax = m_EtaModulesSCTEC - 0.5,
                                                  ybins = m_PhiModulesPerRingSCT, ymin = - 0.5, ymax = m_PhiModulesPerRingSCT - 0.5,
                                                  zmin=m_minSCTResFillRange, zmax=m_maxSCTResFillRange)
    
    varName = 'm_sct_ecc_pullx;sct_ecc_pulllx'
    title = 'UnBiased X Pull SCT EndCap C;Pull'
    residualGroup.defineHistogram(varName, type='TH1F', path=pathResiduals, title=title, xbins=100, xmin=-m_RangeOfPullHistos, xmax=m_RangeOfPullHistos)

    varName = 'm_modPhiShift_sct_ecc,m_residualX_sct_ecc;sct_ecc_xresvsmodphi_profile'
    title = 'X Residual Mean vs (Modified) Module Phi-ID SCT ECC;(Modified) Phi-ID;Residual [mm]'
    residualGroup.defineHistogram(varName, title = title, type = 'TProfile', path=pathResiduals,
                                  xbins = m_PhiModulesShift_sct_ec, xmin = - 0.5, xmax = m_PhiModulesShift_sct_ec - 0.5)

    # end histograms
    #
