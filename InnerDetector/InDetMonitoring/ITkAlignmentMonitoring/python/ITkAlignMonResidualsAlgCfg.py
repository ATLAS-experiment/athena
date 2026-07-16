#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

'''@file ITkAlignMonResidualsAlgCfg.py
@brief Configuration of the ITk alignment residual monitoring,
based on the Run 3 IDAlignMonResidualsAlgCfg.py
'''
def ITkAlignMonResidualsAlgCfg(helper, alg, flags=None, **kwargs):
    '''Function to configures some algorithms in the monitoring system.'''

    #Histogram range values
    m_nBinsMuRange        = 101
    m_muRangeMin          = -0.5
    m_muRangeMax          = 100.5
    m_minSiResFillRange   = -0.05
    m_maxSiResFillRange   = 0.05
    m_minStripResFillRange   = -0.07
    m_maxStripResFillRange   = 0.07
    m_RangeOfPullHistos   = 4
    m_minPIXResYFillRange = -0.3
    m_maxPIXResYFillRange = 0.3
    m_FinerBinningFactor = 1

    # ITk numerology (ATLAS-P2-RUN4-03): 5 pixel barrel layers,
    # 9 pixel endcap layer/disk groups, 4 strip barrel layers,
    # 6 strip endcap disks per side.  These constants drive the
    # histogram booking below and are pushed to the C++ algorithm
    # properties so that booking and filling always stay consistent.
    layersPix = ['0', '1', '2', '3', '4']
    layersNamePix = ['L0', 'L1', 'L2', 'L3', 'L4']
    m_EtaModulesPix = [25, 13, 19, 19, 19]
    m_EtaModulesMinPix = [-12.5, -6.5, -9.5, -9.5, -9.5]
    m_EtaModulesMaxPix = [12.5, 6.5, 9.5, 9.5, 9.5]
    m_PhiModules = [12, 20, 32, 44, 56]
    m_PixBModEtaShift = [12, 33, 51, 72, 93]
    m_PixBModPhiShift = [0, 16, 40, 76, 124]
    m_EtaModulesShift_barrel = 104
    m_PhiModulesShift_barrel = 184
    layersECPix = ['0', '1', '2', '3', '4', '5', '6', '7', '8']
    m_PhiModulesPerRing = 56    # largest disk
    m_PixECModPhiShift = [0, 23, 58, 83, 120, 157, 206, 255, 316]
    m_PhiModulesShift_ec = 372
    layersStripB = ['0', '1', '2', '3']
    m_PhiModulesStrip = [28, 40, 56, 72]
    m_EtaModulesStrip = 113
    m_EtaModulesMinStrip = -56.5
    m_EtaModulesMaxStrip = 56.5
    m_StripBModEtaShift = [56, 171, 258, 317]
    m_StripBModPhiShift = [0, 32, 76, 136]
    m_EtaModulesShift_strip_barrel = 348
    m_PhiModulesShift_strip_barrel = 212
    layersECstrip = ['0', '1', '2', '3', '4', '5']
    m_EtaModulesStripEC = 18      # rings per disk (max)
    m_PhiModulesPerRingStrip = 64
    m_StripECNmods = 64
    m_StripECGap = 10
    m_PhiModulesShift_strip_ec = 444 #64 mod/disk x 6 disks + 10*6 gaps
    # Stacked barrel/endcap layer axes of the common silicon summary plots
    m_siliconBarrelLayers = len(layersPix) + 2 * len(layersStripB)
    m_siliconECLayers     = len(layersECPix) + 2 * len(layersECstrip)

    # Push the numerology to the C++ algorithm so that the histogram
    # tool maps and fill-time indexing match the booking below.
    alg.NPixelBarrelLayers = len(layersPix)
    alg.NStripBarrelLayers = len(layersStripB)
    alg.NPixelEndcapLayers = len(layersECPix)
    alg.NStripEndcapLayers = len(layersECstrip)
    alg.PixelBarrelModEtaShift = m_PixBModEtaShift
    alg.PixelBarrelModPhiShift = m_PixBModPhiShift
    alg.PixelEndcapModPhiShift = m_PixECModPhiShift
    alg.StripBarrelModEtaShift = m_StripBModEtaShift
    alg.StripBarrelModPhiShift = m_StripBModPhiShift
    alg.StripEndcapNmods = m_StripECNmods
    alg.StripEndcapModGap = m_StripECGap

    # Set a folder name from the user options
    folderName = "ExtendedTracks"
    if "TrackName" in kwargs:
        folderName = kwargs["TrackName"]
    
    # this creates a "residualGroup" called "alg" which will put its histograms into the subdirectory "Residuals"
    residualGroup = helper.addGroup(alg, 'Residuals')
    pathResiduals = '/ITkAlignMon/'+folderName+'/Residuals'

    # Histograms for the Alignment Residual monitoring:    
    varName = 'm_mu;mu_perEvent'
    title = 'mu_perEvent;#LT#mu#GT per event;Events'
    residualGroup.defineHistogram(varName, type='TH1F', path=pathResiduals, title=title, xbins=m_nBinsMuRange, xmin=m_muRangeMin, xmax=m_muRangeMax)

    varName = 'm_detType;sirescalcfailure'
    title = 'Hits with ResidualPullCalculator problem;Events;DetType'
    residualGroup.defineHistogram(varName, type='TH1F', path=pathResiduals, title=title, xbins=2, xmin=0, xmax=2)

    
    #Silicon Plots
    #Common for Pixel and Strip, barrel and Endcaps
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

    residualXArray = helper.addArray([len(layersPix)], alg, 'PixResidualX', topPath = pathResiduals)
    for postfix, tool in residualXArray.Tools.items():
        layer = layersPix[int( postfix.split('_')[1] )]
        title = ('UnBiased X Residual Pixel Barrel %s' % layersNamePix[int(layer)])
        name = 'm_pix_residualsx;pix_b' + layer + '_residualx'
        tool.defineHistogram(name, title = title, type = 'TH1F',
                             xbins = 100 * m_FinerBinningFactor, xmin = m_minSiResFillRange, xmax = m_maxSiResFillRange)
 
    residualYArray = helper.addArray([len(layersPix)], alg, 'PixResidualY', topPath = pathResiduals)
    for postfix, tool in residualYArray.Tools.items():
        layer = layersPix[int( postfix.split('_')[1] )]
        title = ('UnBiased Y Residual Pixel Barrel %s' % layersNamePix[int(layer)]) 
        name = 'm_pix_residualsy;pix_b' + layer + '_residualy'
        tool.defineHistogram(name, title = title, type = 'TH1F',
                             xbins = 100 * m_FinerBinningFactor, xmin = m_minPIXResYFillRange, xmax = m_maxPIXResYFillRange)

   # Define local X,Y 3D histograms (not really 3D histograms but TProfile 2D)
    residualX2DProfArray = helper.addArray([len(layersPix)], alg, 'PixResidualX_2DProf', topPath = pathResiduals)
    for postfix, tool in residualX2DProfArray.Tools.items():
        layer = layersPix[int( postfix.split('_')[1] )]
        title = ('Local X Residual vs Module Eta-Phi-ID Pixel Barrel layer %s; Mod Eta; Mod Phi; Local X Residual [mm]' % layersNamePix[int(layer)]) 
        name = 'm_modEta,m_modPhi,m_pix_residualsx;pix_b' + layer + '_xresvsmodetaphi_2dprof'
        tool.defineHistogram(name, title = title, type = 'TProfile2D', xbins = m_EtaModulesPix[int(layer)], xmin = m_EtaModulesMinPix[int(layer)], xmax = m_EtaModulesMaxPix[int(layer)],
                                                  ybins = m_PhiModules[int(layer)], ymin = -0.5, ymax = m_PhiModules[int(layer)] - 0.5,
                                                  zmin = m_minSiResFillRange, zmax = m_maxSiResFillRange)

    residualY2DProfArray = helper.addArray([len(layersPix)], alg, 'PixResidualY_2DProf', topPath = pathResiduals)
    for postfix, tool in residualY2DProfArray.Tools.items():
        layer = layersPix[int( postfix.split('_')[1] )]
        title = ('Local Y Residual vs Module Eta-Phi-ID Pixel Barrel layer %s; Mod Eta; Mod Phi; Local Y Residual [mm]' % layersNamePix[int(layer)]) 
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
        tool.defineHistogram(name, title = title, type = 'TProfile2D', xbins = len(layersECPix), xmin = - 0.5, xmax = len(layersECPix) - 0.5,
                                                  ybins = m_PhiModulesPerRing, ymin = -0.5, ymax = m_PhiModulesPerRing - 0.5,
                                                  zmin = m_minSiResFillRange, zmax = m_maxSiResFillRange)

    residualECY2DProfArray = helper.addArray([len(endcapsPix)], alg, 'PixResidualYEC_2DProf', topPath = pathResiduals)
    for postfix, tool in residualECY2DProfArray.Tools.items():
        layer = endcapsPix[int( postfix.split('_')[1])]
        title = ('Local Y Residual vs Module Eta-Phi-ID Pixel Endcap %s; Disk; Mod Phi; Local Y Residual [mm]' % layer.upper()) 
        name = 'm_layerDisk,m_modPhi,m_pix_ec_residualy;pix_ec' + layer + '_yresvsmodetaphi_2dprof'
        tool.defineHistogram(name, title = title, type = 'TProfile2D', xbins = len(layersECPix), xmin = - 0.5, xmax = len(layersECPix) - 0.5,
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

    #Strip Barrel Plots
    varName = 'm_strip_b_residualx;strip_b_residualx'
    title = 'UnBiased X Residual Strip Barrel;Residual [mm]'
    residualGroup.defineHistogram(varName, type='TH1F', path=pathResiduals, title=title, xbins=100, xmin=m_minStripResFillRange, xmax=m_maxStripResFillRange)

    varName = 'm_strip_b_biased_residualx;strip_b_biasedresidualx'
    title = 'Biased X Residual Strip Barrel;Residual [mm]'
    residualGroup.defineHistogram(varName, type='TH1F', path=pathResiduals, title=title, xbins=100, xmin=m_minStripResFillRange, xmax=m_maxStripResFillRange)

    residualStripXArray = helper.addArray([len(layersStripB)], alg, 'StripResidualX', topPath = pathResiduals)
    for postfix, tool in residualStripXArray.Tools.items():
        layer = layersStripB[int( postfix.split('_')[1] )]
        title = ('UnBiased X Residual Strip Barrel %s' % layer) 
        name = 'm_strip_residualsx;strip_b' + layer + '_residualx'
        tool.defineHistogram(name, title = title, type = 'TH1F',
                             xbins = 100 * m_FinerBinningFactor, xmin = m_minStripResFillRange, xmax = m_maxStripResFillRange)

    # Define local X 3D histograms (not really 3D histograms but TProfile 2D)
    residualStripX2DProfArray = helper.addArray([len(layersStripB)], alg, 'StripResidualX_2DProf', topPath = pathResiduals)
    for postfix, tool in residualStripX2DProfArray.Tools.items():
        layer = layersStripB[int( postfix.split('_')[1] )]
        title = ('Local X Residual vs Module Eta-Phi-ID Strip Barrel layer %s; Mod Eta; Mod Phi; Local X Residual [mm]' % layer) 
        name = 'm_modEta,m_modPhi,m_strip_residualsx;strip_b' + layer + '_xresvsmodetaphi_2dprof'
        tool.defineHistogram(name, title = title, type = 'TProfile2D', xbins = m_EtaModulesStrip, xmin = m_EtaModulesMinStrip, xmax = m_EtaModulesMaxStrip,
                                                  ybins = m_PhiModulesStrip[int(layer)], ymin = -0.5, ymax = m_PhiModulesStrip[int(layer)] - 0.5,
                                                  zmin = m_minStripResFillRange, zmax = m_maxStripResFillRange)

    residualStripXS02DProfArray = helper.addArray([len(layersStripB)], alg, 'Strip_s0_ResidualX_2DProf', topPath = pathResiduals)
    for postfix, tool in residualStripXS02DProfArray.Tools.items():
        layer = layersStripB[int( postfix.split('_')[1] )]
        title = ('Local X Residual vs Module Eta-Phi-ID Strip Barrel layer %s side 0; Mod Eta; Mod Phi; Local X Residual side 0 [mm]' % layer) 
        name = 'm_modEta,m_modPhi,m_strip_residualsx;strip_b' + layer + '_s0_xresvsmodetaphi_2dprof'
        tool.defineHistogram(name, title = title, type = 'TProfile2D', xbins = m_EtaModulesStrip, xmin = m_EtaModulesMinStrip, xmax = m_EtaModulesMaxStrip,
                                                  ybins = m_PhiModulesStrip[int(layer)], ymin = -0.5, ymax = m_PhiModulesStrip[int(layer)] - 0.5,
                                                  zmin = m_minStripResFillRange, zmax = m_maxStripResFillRange)

    residualStripXS12DProfArray = helper.addArray([len(layersStripB)], alg, 'Strip_s1_ResidualX_2DProf', topPath = pathResiduals)
    for postfix, tool in residualStripXS12DProfArray.Tools.items():
        layer = layersStripB[int( postfix.split('_')[1] )]
        title = ('Local X Residual vs Module Eta-Phi-ID Strip Barrel layer %s side 1; Mod Eta; Mod Phi; Local X Residual side 1 [mm]' % layer) 
        name = 'm_modEta,m_modPhi,m_strip_residualsx;strip_b' + layer + '_s1_xresvsmodetaphi_2dprof'
        tool.defineHistogram(name, title = title, type = 'TProfile2D', xbins = m_EtaModulesStrip, xmin = m_EtaModulesMinStrip, xmax = m_EtaModulesMaxStrip,
                                                   ybins = m_PhiModulesStrip[int(layer)], ymin = -0.5, ymax = m_PhiModulesStrip[int(layer)] - 0.5,
                                                   zmin = m_minStripResFillRange, zmax = m_maxStripResFillRange)

    pullStripXArray = helper.addArray([len(layersStripB)], alg, 'StripPullX', topPath = pathResiduals)
    for postfix, tool in pullStripXArray.Tools.items():
        layer = layersStripB[int( postfix.split('_')[1] )]
        title = ('UnBiased X Pull Strip Barrel %s' % layer) + ';Pull'
        name = 'm_strip_pullsx;strip_b' + layer + '_pullx'
        tool.defineHistogram(name, title = title, type = 'TH1F',
                             xbins = 100 * m_FinerBinningFactor, xmin = -m_RangeOfPullHistos, xmax = m_RangeOfPullHistos)

    resXvsEtaStripArray = helper.addArray([len(layersStripB)], alg, 'StripResidualXvsEta', topPath = pathResiduals)
    for postfix, tool in resXvsEtaStripArray.Tools.items():
        layer = layersStripB[int( postfix.split('_')[1] )]
        layerInd = int(layer)
        title = ('X Residual Distribution vs Module Eta-ID Strip Barrel %s' % layer) + ';Mod Eta;Residual [mm]'
        name = 'm_modEta,m_residualX;strip_b' + layer + '_xresidualvseta_2d'
        tool.defineHistogram(name, title = title, type = 'TH2F',
                             xbins = m_EtaModulesStrip, xmin = m_EtaModulesMinStrip, xmax = m_EtaModulesMaxStrip,
                             ybins = 50 * m_FinerBinningFactor, ymin = m_minStripResFillRange, ymax = m_maxStripResFillRange)

    resXvsPhiStripArray = helper.addArray([len(layersStripB)], alg, 'StripResidualXvsPhi', topPath = pathResiduals)
    for postfix, tool in resXvsPhiStripArray.Tools.items():
        layer = layersStripB[int( postfix.split('_')[1] )]
        layerInd = int(layer)
        PhiModules = m_PhiModulesStrip[layerInd]
        title = ('X Residual Distribution vs Module Phi-ID Strip Barrel %s' % layer) + ';Mod Phi;Residual [mm]'
        name = 'm_modPhi,m_residualX;strip_b' + layer + '_xresidualvsphi_2d'
        tool.defineHistogram(name, title = title, type = 'TH2F',
                             xbins = PhiModules, xmin = - 0.5, xmax = PhiModules - 0.5,
                             ybins = 50 * m_FinerBinningFactor, ymin = m_minStripResFillRange, ymax = m_maxStripResFillRange)

    varName = 'm_modPhiShift_strip_barrel,m_residualX_strip_barrel;strip_b_xresvsmodphi_profile'
    title = 'X Residual Mean vs (Modified) Module Phi-ID Strip Barrel;(Modified) Phi-ID;Residual [mm]'
    residualGroup.defineHistogram(varName, title = title, type = 'TProfile', path=pathResiduals,
                                  xbins = m_PhiModulesShift_strip_barrel, xmin = - 0.5, xmax = m_PhiModulesShift_strip_barrel - 0.5)

    varName = 'm_modEtaShift_strip_barrel,m_residualX_strip_barrel;strip_b_xresvsmodeta_profile'
    title = 'X Residual Mean vs (Modified) Module Eta-ID Strip Barrel;(Modified) Eta-ID;Residual [mm]'
    residualGroup.defineHistogram(varName, title = title, type = 'TProfile', path=pathResiduals,
                                  xbins = m_EtaModulesShift_strip_barrel, xmin = - 0.5, xmax = m_EtaModulesShift_strip_barrel - 0.5)

    #Strip EndCap A plots
    varName = 'm_strip_eca_residualx;strip_eca_residualx'
    title = 'UnBiased X Residual Strip EndCap A;Residual [mm]'
    residualGroup.defineHistogram(varName, type='TH1F', path=pathResiduals, title=title, xbins=100, xmin=m_minStripResFillRange, xmax=m_maxStripResFillRange)

    # Define local X 3D histograms (not really 3D histograms but TProfile 2D)
    residualStripECAX2DProfArray = helper.addArray([len(layersECstrip)], alg, 'StripECAResidualX_2DProf', topPath = pathResiduals)
    for postfix, tool in residualStripECAX2DProfArray.Tools.items():
        layer = layersECstrip[int( postfix.split('_')[1] )]
        title = ('Local X Residual vs Module Eta-Phi-ID Strip Endcap A Disk %s; Mod Eta; Mod Phi; Local X Residual [mm]' % layer) 
        name = 'm_modEta,m_modPhi,m_strip_eca_residualx;strip_eca' + layer + '_xresvsmodetaphi_2dprof'
        tool.defineHistogram(name, title = title, type = 'TProfile2D', xbins = m_EtaModulesStripEC, xmin = -0.5, xmax = m_EtaModulesStripEC - 0.5,
                                                  ybins = m_PhiModulesPerRingStrip, ymin = - 0.5, ymax = m_PhiModulesPerRingStrip - 0.5,
                                                  zmin=m_minStripResFillRange, zmax=m_maxStripResFillRange)

    residualStripECAXS02DProfArray = helper.addArray([len(layersECstrip)], alg, 'StripECA_s0_ResidualX_2DProf', topPath = pathResiduals)
    for postfix, tool in residualStripECAXS02DProfArray.Tools.items():
        layer = layersECstrip[int( postfix.split('_')[1] )]
        title = ('Local X Residual vs Module Eta-Phi-ID Strip Endcap A Disk %s side 0; Mod Eta; Mod Phi; Local X Residual side 0 [mm]' % layer) 
        name = 'm_modEta,m_modPhi,m_strip_eca_residualx;strip_eca' + layer + '_s0_xresvsmodetaphi_2dprof'
        tool.defineHistogram(name, title = title, type = 'TProfile2D', xbins = m_EtaModulesStripEC, xmin = -0.5, xmax = m_EtaModulesStripEC - 0.5,
                                                  ybins = m_PhiModulesPerRingStrip, ymin = - 0.5, ymax = m_PhiModulesPerRingStrip - 0.5,
                                                  zmin=m_minStripResFillRange, zmax=m_maxStripResFillRange)

    residualStripECAXS12DProfArray = helper.addArray([len(layersECstrip)], alg, 'StripECA_s1_ResidualX_2DProf', topPath = pathResiduals)
    for postfix, tool in residualStripECAXS12DProfArray.Tools.items():
        layer = layersECstrip[int( postfix.split('_')[1] )]
        title = ('Local X Residual vs Module Eta-Phi-ID Strip Endcap A Disk %s side 1; Mod Eta; Mod Phi; Local X Residual side 1 [mm]' % layer) 
        name = 'm_modEta,m_modPhi,m_strip_eca_residualx;strip_eca' + layer + '_s1_xresvsmodetaphi_2dprof'
        tool.defineHistogram(name, title = title, type = 'TProfile2D', xbins = m_EtaModulesStripEC, xmin = -0.5, xmax = m_EtaModulesStripEC - 0.5,
                                                  ybins = m_PhiModulesPerRingStrip, ymin = - 0.5, ymax = m_PhiModulesPerRingStrip - 0.5,
                                                  zmin=m_minStripResFillRange, zmax=m_maxStripResFillRange)
        
    varName = 'm_strip_eca_pullx;strip_eca_pulllx'
    title = 'UnBiased X Pull Strip EndCap A;Pull'
    residualGroup.defineHistogram(varName, type='TH1F', path=pathResiduals, title=title, xbins=100, xmin=-m_RangeOfPullHistos, xmax=m_RangeOfPullHistos)

    varName = 'm_modPhiShift_strip_eca,m_residualX_strip_eca;strip_eca_xresvsmodphi_profile'
    title = 'X Residual Mean vs (Modified) Module Phi-ID Strip ECA;(Modified) Phi-ID;Residual [mm]'
    residualGroup.defineHistogram(varName, title = title, type = 'TProfile', path=pathResiduals,
                                  xbins = m_PhiModulesShift_strip_ec, xmin = - 0.5, xmax = m_PhiModulesShift_strip_ec - 0.5)

    #Strip EndCap C plots
    varName = 'm_strip_ecc_residualx;strip_ecc_residualx'
    title = 'UnBiased X Residual Strip EndCap C;Residual [mm]'
    residualGroup.defineHistogram(varName, type='TH1F', path=pathResiduals, title=title, xbins=100, xmin=m_minStripResFillRange, xmax=m_maxStripResFillRange)

    # Define local X 3D histograms (not really 3D histograms but TProfile 2D)
    residualStripECCX2DProfArray = helper.addArray([len(layersECstrip)], alg, 'StripECCResidualX_2DProf', topPath = pathResiduals)
    for postfix, tool in residualStripECCX2DProfArray.Tools.items():
        layer = layersECstrip[int( postfix.split('_')[1] )]
        title = ('Local X Residual vs Module Eta-Phi-ID Strip Endcap C Disk %s; Mod Eta; Mod Phi; Local X Residual [mm]' % layer) 
        name = 'm_modEta,m_modPhi,m_strip_ecc_residualx;strip_ecc' + layer + '_xresvsmodetaphi_2dprof'
        tool.defineHistogram(name, title = title, type = 'TProfile2D', xbins = m_EtaModulesStripEC, xmin = -0.5, xmax = m_EtaModulesStripEC - 0.5,
                                                  ybins = m_PhiModulesPerRingStrip, ymin = -0.5, ymax = m_PhiModulesPerRingStrip - 0.5,
                                                  zmin=m_minStripResFillRange, zmax=m_maxStripResFillRange)

    residualStripECCXS02DProfArray = helper.addArray([len(layersECstrip)], alg, 'StripECC_s0_ResidualX_2DProf', topPath = pathResiduals)
    for postfix, tool in residualStripECCXS02DProfArray.Tools.items():
        layer = layersECstrip[int( postfix.split('_')[1] )]
        title = ('Local X Residual vs Module Eta-Phi-ID Strip Endcap C Disk %s side 0; Mod Eta; Mod Phi; Local X Residual side 0 [mm]' % layer) 
        name = 'm_modEta,m_modPhi,m_strip_ecc_residualx;strip_ecc' + layer + '_s0_xresvsmodetaphi_2dprof'
        tool.defineHistogram(name, title = title, type = 'TProfile2D', xbins = m_EtaModulesStripEC, xmin = -0.5, xmax = m_EtaModulesStripEC - 0.5,
                                                  ybins = m_PhiModulesPerRingStrip, ymin = - 0.5, ymax = m_PhiModulesPerRingStrip - 0.5,
                                                  zmin=m_minStripResFillRange, zmax=m_maxStripResFillRange)

    residualStripECCXS12DProfArray = helper.addArray([len(layersECstrip)], alg, 'StripECC_s1_ResidualX_2DProf', topPath = pathResiduals)
    for postfix, tool in residualStripECCXS12DProfArray.Tools.items():
        layer = layersECstrip[int( postfix.split('_')[1] )]
        title = ('Local X Residual vs Module Eta-Phi-ID Strip Endcap C Disk %s side 1; Mod Eta; Mod Phi; Local X Residual side 1 [mm]' % layer) 
        name = 'm_modEta,m_modPhi,m_strip_ecc_residualx;strip_ecc' + layer + '_s1_xresvsmodetaphi_2dprof'
        tool.defineHistogram(name, title = title, type = 'TProfile2D', xbins = m_EtaModulesStripEC, xmin = -0.5, xmax = m_EtaModulesStripEC - 0.5,
                                                  ybins = m_PhiModulesPerRingStrip, ymin = - 0.5, ymax = m_PhiModulesPerRingStrip - 0.5,
                                                  zmin=m_minStripResFillRange, zmax=m_maxStripResFillRange)
    
    varName = 'm_strip_ecc_pullx;strip_ecc_pulllx'
    title = 'UnBiased X Pull Strip EndCap C;Pull'
    residualGroup.defineHistogram(varName, type='TH1F', path=pathResiduals, title=title, xbins=100, xmin=-m_RangeOfPullHistos, xmax=m_RangeOfPullHistos)

    varName = 'm_modPhiShift_strip_ecc,m_residualX_strip_ecc;strip_ecc_xresvsmodphi_profile'
    title = 'X Residual Mean vs (Modified) Module Phi-ID Strip ECC;(Modified) Phi-ID;Residual [mm]'
    residualGroup.defineHistogram(varName, title = title, type = 'TProfile', path=pathResiduals,
                                  xbins = m_PhiModulesShift_strip_ec, xmin = - 0.5, xmax = m_PhiModulesShift_strip_ec - 0.5)

    # end histograms
    #
