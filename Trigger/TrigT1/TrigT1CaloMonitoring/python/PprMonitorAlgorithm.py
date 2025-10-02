#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

def PprMonitoringConfig(inputFlags):
    '''Function to configure LVL1 Ppr algorithm in the monitoring system.'''

    import math 
    # get the component factory - used for getting the algorithms
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()

    # use L1Calo's special MonitoringCfgHelper
    from TrigT1CaloMonitoring.LVL1CaloMonitoringConfig import L1CaloMonitorCfgHelper
    helper = L1CaloMonitorCfgHelper(inputFlags,CompFactory.PprMonitorAlgorithm,'PprMonAlg')

    # get any algorithms
    PprMonAlg = helper.alg
    
    # add any steering
    groupName = 'PprMonitor' # the monitoring group name is also used for the package name
    PprMonAlg.PackageName = groupName

    # Steering properties 
    threshADC = 50
    PprMonAlg.TT_ADC_HitMap_Thresh = threshADC  # ADC cut for hit maps
 
    sliceNo = 15
    PprMonAlg.SliceNo = sliceNo  # Max number of timeslices in the readout
   
    threshVec = [0, 1, 3, 5, 10, 20, 30, 50] # LUT thresholds    
    PprMonAlg.LUTHitMap_ThreshVec = threshVec

    # Environment
    #isOnline = inputFlags.Trigger.Online.isPartition 
 
    # Histogram paths
    devPath = 'Developer/PprMonAlg'
    expPath = 'Expert/PpmTrex'
    
    # Trigger tower plots: eta-phi granularity
    etabins = [-4.9,-4.475,-4.050,-3.625,-3.2,-3.1,-2.9,
               -2.7,-2.5,-2.4,-2.3,-2.2,-2.1,-2.0,-1.9,
               -1.8,-1.7,-1.6,-1.5,-1.4,-1.3,-1.2,-1.1,
               -1.0,-0.9,-0.8,-0.7,-0.6,-0.5,-0.4,-0.3,
               -0.2,-0.1,0.0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,
               0.8,0.9,1.0,1.1,1.2,1.3,1.4,1.5,1.6,1.7,
               1.8,1.9,2.0,2.1,2.2,2.3,2.4,2.5,2.7,2.9,
               3.1,3.2,3.625,4.050,4.475,4.9]

    etabins_HAD_1D = [-4.9,-4.050,-3.2,-3.1,-2.9,
                      -2.7,-2.5,-2.4,-2.3,-2.2,-2.1,-2.0,-1.9,
                      -1.8,-1.7,-1.6,-1.5,-1.4,-1.3,-1.2,-1.1,
                      -1.0,-0.9,-0.8,-0.7,-0.6,-0.5,-0.4,-0.3,
                      -0.2,-0.1,0.0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,
                      0.8,0.9,1.0,1.1,1.2,1.3,1.4,1.5,1.6,1.7,
                      1.8,1.9,2.0,2.1,2.2,2.3,2.4,2.5,2.7,2.9,
                      3.1,3.2,4.050,4.9]

    phibins = 64
    phimin = 0
    phimax_2d = 64
    phimax_1d = 2.*math.pi 
    maxEnergyRange = 256
    bcn = 3564 # Max number of bunches

    #######################   
    # PPM output (LUT-CP) #
    #######################
    histPath_dev = devPath+'/Outputs'
    histPath_exp = expPath+'/Outputs'
    histPath_det = expPath+'/Outputs/detail'
    
    # LUT per BCN (Both EM & HAD together)
    helper.defineHistogram('BCID;ppm_1d_tt_lutcp_LutPerBCN',
                           fillGroup='groupLUTCP',
                           title='Number of LUT-CP > 5 GeV/2 per BC; Bunch crossing; # of LUT above limit', 
                           type='TH1F', path=histPath_det,
                           hanConfig={
                               "description": "Number of LUT-CP > 5 GeV/2 per BC."
                           },
                           xbins=bcn, xmin=0, xmax=bcn, 
                           cutmask='mask_cpET_5')
    
    # EM distributions
    helper.defineHistogram('eta_TT;ppm_em_1d_tt_lutcp_Eta',
                           fillGroup='groupLUTCP_EM',
                           title='EM LUT-CP: Distribution of peak in #eta; #eta', 
                           type='TH1F', path=histPath_dev,
                           xbins=etabins, 
                           cutmask='mask_cpET_0')
    
    helper.defineHistogram('phiTT_1D;ppm_em_1d_tt_lutcp_Phi',
                           fillGroup='groupLUTCP_EM',
                           title='EM LUT-CP: Distribution of peak in #phi; #phi', 
                           type='TH1F', path=histPath_dev,
                           xbins=phibins, xmin=phimin, xmax=phimax_1d)
 
    helper.defineHistogram('cpET_TT;ppm_em_1d_tt_lutcp_Et',
                           fillGroup='groupLUTCP_EM',
                           title='EM LUT-CP: Distribution of peak; EM LUT peak [GeV/2]', 
                           type='TH1F', path=histPath_dev,
                           xbins=maxEnergyRange-1, xmin=1, xmax=maxEnergyRange, 
                           cutmask='mask_cpET_0')

    # HAD distributions
    helper.defineHistogram('eta_TT;ppm_had_1d_tt_lutcp_Eta',
                           fillGroup='groupLUTCP_HAD',
                           title='HAD LUT-CP: Distribution of peak in #eta; #eta', 
                           type='TH1F', path=histPath_exp,
                           hanConfig={
                               "description": "HAD LUT-CP: Distribution of peak in #eta."
                           },
                           xbins=etabins_HAD_1D, 
                           cutmask='mask_cpET_0')

    helper.defineHistogram('phiTT_1D;ppm_had_1d_tt_lutcp_Phi',
                           fillGroup='groupLUTCP_HAD',
                           title='HAD LUT-CP: Distribution of peak in #phi; #phi', 
                           type='TH1F', path=histPath_exp,
                           hanConfig={
                               "description": "HAD LUT-CP: Distribution of peak in #phi."
                           },
                           xbins=phibins, xmin=phimin, xmax=phimax_1d)
 
    helper.defineHistogram('cpET_TT;ppm_had_1d_tt_lutcp_Et',
                           fillGroup='groupLUTCP_HAD',
                           title='HAD LUT-CP: Distribution of peak; HAD LUT peak [GeV/2]', 
                           type='TH1F', path=histPath_exp,
                           hanConfig={
                               "display" : "LogX,LogY",
                               "description": "HAD LUT-CP: Distribution of peak."
                           },
                           xbins=maxEnergyRange-1, xmin=1, xmax=maxEnergyRange, 
                           cutmask='mask_cpET_0')

    # Eta-phi maps
    helper.defineHistogram('etaTT_2D,phiTT_2D,cpET_TT_2D;ppm_em_2d_etaPhi_tt_lutcp_AverageEt',
                           fillGroup='groupLUTCP_EM',
                           title='EM Average LUT-CP Et for Et > 5 GeV/2', 
                           type='TProfile2D', path=histPath_dev,
                           xbins=etabins, ybins=phibins, ymin=phimin, ymax=phimax_2d)
       
    helper.defineHistogram('etaTT_2D,phiTT_2D,cpET_TT_2D;ppm_had_2d_etaPhi_tt_lutcp_AverageEt',
                           fillGroup='groupLUTCP_HAD',
                           title='HAD Average LUT-CP Et for Et > 5 GeV/2', 
                           type='TProfile2D', path=histPath_det,
                           hanConfig={
                               "display" : "SetPalette(1),LogZ",
                               "description": "HAD Average LUT-CP Et for Et > 5 GeV/2."
                           },
                           xbins=etabins, ybins=phibins, ymin=phimin, ymax=phimax_2d)

    # Eta-phi maps per threshold (low stat). 
    # Offline: fill on a per-LB basis and then merge into low-stat plots
    # Online: refresh every 10 LB

    layers = ['EM', 'HAD']
    iThresh = list(range(0, len(threshVec)))

    for layer in layers:
        histPath = histPath_dev
        for i in iThresh:
            groupname = 'groupLUTCP_{0}_{1}'.format(layer, threshVec[i])
            if layer=='HAD':
                histPath = histPath_exp if i in [0, 5] else histPath_det
            helper.defineHistogram('etaTT_2D,phiTT_2D;ppm_{0}_2d_etaPhi_tt_lutcp_Threshold0{1}'.format(layer.lower(), i),
                                   fillGroup=groupname,
                                   title='#eta - #phi map of {0} LUT-CP > {1} GeV/2'.format(layer, threshVec[i]), 
                                   type='TH2D', path=histPath,
                                   hanConfig={
                                       "display" : "SetPalette(1),LogZ",
                                       "description": '#eta - #phi map of {0} LUT-CP > {1} GeV/2'.format(layer, threshVec[i])
                                   },
                                   xbins=etabins, ybins=phibins, ymin=phimin, ymax=phimax_2d)
                 

    ########################   
    # PPM output (LUT-JEP) #
    ########################
    histPath_dev = devPath+'/Outputs'
    histPath_exp = expPath+'/Outputs'
    histPath_det = expPath+'/Outputs/detail'
   
    # LUT per BCN
    helper.defineHistogram('BCID;ppm_1d_tt_lutjep_LutPerBCN',
                           fillGroup='groupLUTJEP',
                           title='Number of LUT-JEP > 5 GeV per BC; Bunch crossing; # of LUT above limit', 
                           type='TH1F', path=histPath_det,
                           hanConfig={
                               "description": "Number of LUT-JEP > 5 GeV per BC."
                           },
                           xbins=bcn, xmin=0, xmax=bcn, 
                           cutmask='mask_jepET_5')
 
    # EM distributions
    helper.defineHistogram('eta_TT;ppm_em_1d_tt_lutjep_Eta',
                           fillGroup='groupLUTJEP_EM',
                           title='EM LUT-JEP: Distribution of peak in #eta', 
                           type='TH1F', path=histPath_dev,
                           xbins=etabins, 
                           cutmask='mask_jepET_0')

    helper.defineHistogram('phiTT_1D;ppm_em_1d_tt_lutjep_Phi',
                           fillGroup='groupLUTJEP_EM',
                           title='EM LUT-JEP: Distribution of peak in #phi; #phi', 
                           type='TH1F', path=histPath_dev,
                           xbins=phibins, xmin=phimin, xmax=phimax_1d)

    helper.defineHistogram('jepET_TT;ppm_em_1d_tt_lutjep_Et',
                           fillGroup='groupLUTJEP_EM',
                           title='EM LUT-JEP: Distribution of peak; EM LUT peak [GeV]', 
                           type='TH1F', path=histPath_dev,
                           xbins=maxEnergyRange-1, xmin=1, xmax=maxEnergyRange, 
                           cutmask='mask_jepET_0')

    # HAD distributions
    helper.defineHistogram('eta_TT;ppm_had_1d_tt_lutjep_Eta',
                           fillGroup='groupLUTJEP_HAD',
                           title='HAD LUT-JEP: Distribution of peak in #eta', 
                           type='TH1F', path=histPath_exp,
                           hanConfig={
                               "description": "HAD LUT-JEP: Distribution of peak in #eta."
                           },
                           xbins=etabins_HAD_1D, 
                           cutmask='mask_jepET_0')

    helper.defineHistogram('phiTT_1D;ppm_had_1d_tt_lutjep_Phi',
                           fillGroup='groupLUTJEP_HAD',
                           title='HAD LUT-JEP: Distribution of peak in #phi; #phi', 
                           type='TH1F', path=histPath_exp,
                           hanConfig={
                               "description": "HAD LUT-JEP: Distribution of peak in #phi."
                           },
                           xbins=phibins, xmin=phimin, xmax=phimax_1d)

    helper.defineHistogram('jepET_TT;ppm_had_1d_tt_lutjep_Et',
                           fillGroup='groupLUTJEP_HAD',
                           title='HAD LUT-JEP: Distribution of peak; HAD LUT peak [GeV]', 
                           type='TH1F', path=histPath_exp,
                           hanConfig={
                               "display" : "LogX,LogY",
                               "description": "HAD LUT-JEP: Distribution of peak."
                           },
                           xbins=maxEnergyRange-1, xmin=1, xmax=maxEnergyRange, 
                           cutmask='mask_jepET_0')   

    # Eta-phi maps
    helper.defineHistogram('etaTT_2D,phiTT_2D,jepET_TT_2D;ppm_em_2d_etaPhi_tt_lutjep_AverageEt',
                           fillGroup='groupLUTJEP_EM',
                           title='EM Average LUT-JEP Et for Et > 5 GeV', 
                           type='TProfile2D', path=histPath_dev,
                           xbins=etabins, ybins=phibins, ymin=phimin, ymax=phimax_2d)
    
    helper.defineHistogram('etaTT_2D,phiTT_2D,jepET_TT_2D;ppm_had_2d_etaPhi_tt_lutjep_AverageEt',
                           fillGroup='groupLUTJEP_HAD',
                           title='HAD Average LUT-JEP Et for Et > 5 GeV', 
                           type='TProfile2D', path=histPath_det,
                           hanConfig={
                               "display" : "SetPalette(1),LogZ",
                               "description": "HAD Average LUT-JEP Et for Et > 5 GeV."
                           },
                           xbins=etabins, ybins=phibins, ymin=phimin, ymax=phimax_2d)
    
    for layer in layers:
        histPath = histPath_dev
        for i in iThresh:
            if layer=='HAD':
                histPath = histPath_exp if i in [0, 5] else histPath_det
            groupname = 'groupLUTJEP_{0}_{1}'.format(layer, threshVec[i])
            helper.defineHistogram('etaTT_2D,phiTT_2D;ppm_{0}_2d_etaPhi_tt_lutjep_Threshold0{1}'.format(layer.lower(), i),
                                   fillGroup=groupname,
                                   title='#eta - #phi map of {0} LUT-JEP > {1} GeV'.format(layer, threshVec[i]), 
                                   type='TH2D', path=histPath,
                                   hanConfig={
                                       "display" : "SetPalette(1),LogZ",
                                       "description": '#eta - #phi map of {0} LUT-JEP > {1} GeV'.format(layer, threshVec[i]),
                                   },
                                   xbins=etabins, ybins=phibins, ymin=phimin, ymax=phimax_2d) 


    ####################
    # PPM inputs (ADC) #
    ####################
    histPath_dev = devPath+'/Inputs'
    histPath_exp = expPath+'/Inputs'
    histPath_det = expPath+'/Inputs/detail'
    
    # EM tower maps 
    helper.defineHistogram('etaTT_2D,phiTT_2D;ppm_em_2d_etaPhi_tt_adc_HitMap',
                           fillGroup='groupADC_EM',
                           title='#eta - #phi map of EM FADC > {0} for triggered timeslice; Tower #eta; Tower #phi'.format(threshADC), 
                           type='TH2F', path=histPath_dev,
                           xbins=etabins, ybins=phibins, ymin=phimin, ymax=phimax_2d) 
   
    helper.defineHistogram('etaTT_2D,phiTT_2D,adcTT;ppm_em_2d_etaPhi_tt_adc_ProfileHitMap',
                           fillGroup='groupADC_EM',
                           title='#eta - #phi profile map of EM FADC > {0} for triggered timeslice; Tower #eta; Tower #phi'.format(threshADC), 
                           type='TProfile2D', path=histPath_dev,
                           xbins=etabins, ybins=phibins, ymin=phimin, ymax=phimax_2d)
 
    # HAD tower maps 
    helper.defineHistogram('etaTT_2D,phiTT_2D;ppm_had_2d_etaPhi_tt_adc_HitMap',
                           fillGroup='groupADC_HAD',
                           title='#eta - #phi map of HAD FADC > {0} for triggered timeslice; Tower #eta; Tower #phi'.format(threshADC),
                           type='TH2F', path=histPath_exp,
                           hanConfig={
                               "display" : "SetPalette(1),LogZ",
                               "description" : "#eta - #phi map of HAD FADC > {0} for triggered timeslice."
                           },
                           xbins=etabins, ybins=phibins, ymin=phimin, ymax=phimax_2d)
 
    helper.defineHistogram('etaTT_2D,phiTT_2D,adcTT;ppm_had_2d_etaPhi_tt_adc_ProfileHitMap',
                           fillGroup='groupADC_HAD',
                           title='#eta - #phi profile map of HAD FADC > {0} for triggered timeslice; Tower #eta; Tower #phi'.format(threshADC), 
                           type='TProfile2D', path=histPath_exp,
                           hanConfig={
                               "display" : "SetPalette(1),LogZ",
                               "description": "#eta - #phi profile map of HAD FADC > {0} for triggered timeslice."
                           },
                           xbins=etabins, ybins=phibins, ymin=phimin, ymax=phimax_2d)
 
    # Triggered time-slice
    helper.defineHistogram('adcPeak;ppm_em_1d_tt_adc_TriggeredSlice',
                           fillGroup='groupTimeslice_EM',
                           title='Number of the EM triggered slice; # Slice', 
                           type='TH1F', path=histPath_dev,
                           xbins=sliceNo, xmin=0, xmax=sliceNo) 
 
    helper.defineHistogram('adcPeak;ppm_had_1d_tt_adc_TriggeredSlice',
                           fillGroup='groupTimeslice_HAD',
                           title='Number of the HAD triggered slice; # Slice', 
                           type='TH1F', path=histPath_det,
                           hanConfig={
                               "display" : "LogY",
                               "description" : "Number of the HAD triggered slice."
                           },
                           xbins=sliceNo, xmin=0, xmax=sliceNo)   
 
    helper.defineHistogram('maxADC;ppm_em_1d_tt_adc_MaxTimeslice',
                           fillGroup='groupTimeslice_EM',
                           title='EM distribution of maximum timeslice; slice', 
                           type='TH1D', path=histPath_dev,
                           xbins=sliceNo, xmin=0, xmax=sliceNo)

    helper.defineHistogram('maxADC;ppm_had_1d_tt_adc_MaxTimeslice',
                           fillGroup='groupTimeslice_HAD',
                           title='HAD distribution of maximum timeslice; slice', 
                           type='TH1D', path=histPath_exp,
                           hanConfig={
                               "display" : "SetPalette(1)",
                               "description": "HAD distribution of maximum timeslice."
                           },
                           xbins=sliceNo, xmin=0, xmax=sliceNo) 
    
    helper.defineHistogram('etaTT_2D,phiTT_2D,maxADCPlus1;ppm_em_2d_etaPhi_tt_adc_MaxTimeslice',
                           fillGroup='groupTimeslice_EM',
                           title='Average maximum timeslice for EM signal (TS:1-15); Tower #eta; Tower #phi', 
                           type='TProfile2D', path=histPath_dev,
                           xbins=etabins, ybins=phibins, ymin=phimin, ymax=phimax_2d) 

    helper.defineHistogram('etaTT_2D,phiTT_2D,maxADCPlus1;ppm_had_2d_etaPhi_tt_adc_MaxTimeslice',
                           fillGroup='groupTimeslice_HAD',
                           title='Average maximum timeslice for HAD signal (TS:1-15); Tower #eta; Tower #phi', 
                           type='TProfile2D', path=histPath_exp,
                           hanConfig={
                               "description": "Average maximum timeslice for HAD signal (TS:1-15)."
                           },
                           xbins=etabins, ybins=phibins, ymin=phimin, ymax=phimax_2d)
    
    # Bits of BCID logic word 
    bcidBitsLabels = ['none (40 MHz)', 'satBC only', 'peakF only', 'satBC & peakF', 'sat80BC & peakF', 'sat80BC & sat40BC', 'sat80BC only']
    helper.defineHistogram('bcidBits,adcBCID;ppm_2d_tt_adc_BcidBits',
                           fillGroup='groupTimeslice',
                           title='PPM: PeakADC Vs. Bits of BCID Logic Word', 
                           type='TH2I', path=histPath_det,
                           hanConfig={
                               "description": "PPM: PeakADC Vs. Bits of BCID Logic Word."
                           },
                           xbins=7, xmin=0, xmax=7, xlabels=bcidBitsLabels, ybins=1024, ymin=0, ymax=1024)
   
    # High/low threshold pass cases (Sat80 BCID)
    sat80Labels = ['no saturated ADC', 'none/none', 'none/low', 'none/high', 'low/low', 'low/high', 'high/high', 'else']
    helper.defineHistogram('sat80Word;ppm_1d_tt_adc_HLCase',
                           fillGroup='groupTimeslice',
                           title= 'PPM: Sat80 thresholds passed by ADC[n-2.5] / ADC[n-1.5]', 
                           type='TH1I', path=histPath_det,
                           hanConfig={
                               "description": "PPM: Sat80 thresholds passed by ADC[n-2.5] / ADC[n-1.5]."
                           },
                           xbins=8, xmin=0, xmax=8, xlabels=sat80Labels)

    # Signal shape profiles
    partitionsEM = ['LArFCAL1C', 'LArEMECC', 'LArOverlapC', 'LArEMBC', 'LArEMBA', 'LArOverlapA', 'LArEMECA', 'LArFCAL1A']
    partitionsHAD = [ 'LArFCAL23C', 'LArHECC', 'TileEBC', 'TileLBC', 'TileLBA', 'TileEBA', 'LArHECA', 'LArFCAL23A']

    #signalsEM = helper.addArray([partitionsEM], PprMonAlg, 'groupTimeslice_EM', topPath=mainDir)
    histPath = histPath_dev
    for p in partitionsEM:
        helper.defineHistogram('slice,wADC;ppm_em_1d_tt_adc_SignalProfile{0}'.format(p),
                               fillGroup='groupTimeslice_EM'+'_'+p,
                               title='Signal Shape Profile for {0}; Timeslice'.format(p), 
                               type='TProfile', path=histPath,
                               xbins=sliceNo, xmin=0, xmax=sliceNo)

    #signalsHAD = helper.addArray([partitionsHAD], PprMonAlg, 'groupTimeslice_HAD', topPath=mainDir)
    for p in partitionsHAD:        
        if p.startswith('Tile'):
            histPath = histPath_det
        helper.defineHistogram('slice,wADC;ppm_had_1d_tt_adc_SignalProfile{0}'.format(p),
                               fillGroup='groupTimeslice_HAD'+'_'+p,
                               title='Signal Shape Profile for {0}; Timeslice'.format(p), 
                               type='TProfile', path=histPath,
                               hanConfig={
                                   "description": "Signal Shape Profile for {0}.".format(p)
                               },
                               xbins=sliceNo, xmin=0, xmax=sliceNo)
    
    ####################
    #      Errors      #
    ####################
  
    # Note: use opt='kAlwaysCreate' for error plots so that empty plots will still be published, for sanity checks
    # MW, Feb 2025: removed kAlwaysCreate for EM which is disabled since 2025
    histPath_dev = devPath+'/Errors'
    histPath_exp = expPath+'/Errors'
    histPath_det = expPath+'/Errors/detail'

    # Pedestal correction over-/underflows (EM)
    helper.defineHistogram('etaTT;ppm_em_1d_pedOverflow_Eta',
                           fillGroup='groupErrors_EM',
                           title='EM : Overflow of pedestal correction;#eta', 
                           type='TH1F', path=histPath_dev,
                           xbins=etabins, 
                           cutmask='mask_PedCorrOverflow',
                           opt='')
   
    helper.defineHistogram('etaTT;ppm_em_1d_pedUnderflow_Eta',
                           fillGroup='groupErrors_EM',
                           title='EM : Underflow of pedestal correction;#eta', 
                           type='TH1F', path=histPath_dev,
                           xbins=etabins, 
                           cutmask='mask_PedCorrUnderflow', 
                           opt='')

    #Pedestal correction over-/underflows (HAD)
    helper.defineHistogram('etaTT;ppm_had_1d_pedOverflow_Eta',
                           fillGroup='groupErrors_HAD',
                           title='HAD : Overflow of pedestal correction;#eta', 
                           type='TH1F', path=histPath_det,
                           hanConfig={
                               "description": "Overflow of pedestal correction."
                           },
                           xbins=etabins_HAD_1D,
                           cutmask='mask_PedCorrOverflow', 
                           opt='')

    helper.defineHistogram('etaTT;ppm_had_1d_pedUnderflow_Eta',
                           fillGroup='groupErrors_HAD',
                           title='HAD : Underflow of pedestal correction;#eta', 
                           type='TH1F', path=histPath_det,
                           hanConfig={
                               "description": "Underflow of pedestal correction."
                           },
                           xbins=etabins_HAD_1D, 
                           cutmask='mask_PedCorrUnderflow', 
                           opt='')

    # MCM error block (input G-Link frame)
    globalError_xlabels_mcmerr = [
        "ChannelDisabled",
        "MCMAbsent",
        "Timeout",
        "ASICFull",
        "EventMismatch",
        "BunchMismatch",
        "FIFOCorrupt",
        "PinParity",
    ]

    # SubStatus error block (S-Link payload)
    globalError_xlabels_substat = [
        "GLinkParity",
        "GLinkProtocol",
        "BCNMismatch",
        "FIFOOverflow",
        "ModuleError",
        " ",
        "GLinkDown",
        "GLinkTimeout",
    ]

    # SubStatus Error histograms
    helper.defineHistogram('bit_2D;ppm_1d_SubStatErrorSummary',
                           fillGroup='group1DSubStatErrorSummary',
                           title='Summary of SubStatus Word Errors',
                           type='TH1F', path=histPath_exp,
                           hanConfig={
                               "algorithm" : "Histogram_Empty",
                               "description" : "Summary of SubStatus Word Errors."
                           },
                           xbins=8, xmin=0, xmax=8,
                           xlabels=globalError_xlabels_substat,
                           opt='kAlwaysCreate')
    
    helper.defineHistogram('eventMonitor,bit_2D;ppm_2d_SubStatErrorEventNumbers',
                           fillGroup='groupSubStatErrorEventNumbers',
                           title='SubStatus Word Errors Event Numbers;Events with Error/Mismatch;;',
                           type='TH2I', path=histPath_det,
                           hanConfig={
                               "description": "SubStatus Word Errors Event Numbers."
                           },
                           xbins=10, xmin=0, xmax=10, ybins=8, ymin=0, ymax=8,
                           ylabels=globalError_xlabels_substat,
                           opt='')

    helper.defineHistogram('bit_2D,y_2D;ppm_2d_SubStatError03',
                           fillGroup='groupSubStatError03',
                           title='SubStatus Word Errors (crates 0-3)',
                           type='TH2I', path=histPath_dev,
                           xbins=8, xmin=0, xmax=8, ybins=64, ymin=0, ymax=64,
                           xlabels=globalError_xlabels_substat,
                           ylabels=BinErrors('cr0cr1cr2cr3'),
                           opt='')

    helper.defineHistogram('bit_2D,y_2D;ppm_2d_SubStatError47',
                           fillGroup='groupSubStatError47',
                           title='SubStatus Word Errors (crates 4-7)',
                           type='TH2I', path=histPath_det,
                           hanConfig={
                               "display" : "SetPalette(1),SetGridx,SetGridy",
                               "description" : "SubStatus Word Errors (crates 4-7)."
                           },
                           xbins=8, xmin=0, xmax=8, ybins=64, ymin=0, ymax=64,
                           xlabels=globalError_xlabels_substat,
                           ylabels=BinErrors('cr4cr5cr6cr7'),
                           opt='')

    # MCM Error Field histograms
    helper.defineHistogram('bit_2D;ppm_1d_MCMErrorSummary',
                           fillGroup='group1DMCMErrorSummary',
                           title='Summary of MCM Errors',
                           type='TH1F', path=histPath_exp,
                           hanConfig={
                               "algorithm" : "Histogram_Empty",
                               "description" : "Summary of MCM Errors."
                           },
                           xbins=8, xmin=0, xmax=8,
                           xlabels=globalError_xlabels_mcmerr,
                           opt='kAlwaysCreate')
    
    helper.defineHistogram('eventMonitor,bit_2D;ppm_2d_MCMErrorEventNumbers',
                           fillGroup='groupMCMErrorEventNumbers',
                           title='MCM Error Field Event Numbers;Events with Error/Mismatch;;',
                           type='TH2I', path=histPath_det,
                           hanConfig={
                               "description": "MCM Error Field Event Numbers."
                           },
                           xbins=10, xmin=0, xmax=10, ybins=8, ymin=0, ymax=8,
                           ylabels=globalError_xlabels_mcmerr,
                           opt='')
       
    helper.defineHistogram('bit_2D,y_2D;ppm_2d_MCMErrorField03',
                           fillGroup='groupMCMErrorField03',
                           title='MCM Error Field (crates 0-3)',
                           type='TH2I', path=histPath_dev,
                           xbins=8, xmin=0, xmax=8, ybins=64, ymin=0, ymax=64,
                           xlabels=globalError_xlabels_mcmerr,
                           ylabels=BinErrors('cr0cr1cr2cr3'),
                           opt='')
        
    helper.defineHistogram('bit_2D,y_2D;ppm_2d_MCMErrorField47',
                           fillGroup='groupMCMErrorField47',
                           title='MCM Error Field (crates 4-7)',
                           type='TH2I', path=histPath_det,
                           hanConfig={
                               "display" : "SetPalette(1),SetGridx,SetGridy",
                               "description": "MCM Error Field (crates 4-7)."
                           },
                           xbins=8, xmin=0, xmax=8, ybins=64, ymin=0, ymax=64,
                           xlabels=globalError_xlabels_mcmerr,
                           ylabels=BinErrors('cr4cr5cr6cr7'),
                           opt='')
    

    # Finish up
    acc = helper.result()
    result.merge(acc)
    return result


def BinErrors(crs):

    cr0 = crs.split("cr")[1]
    cr1 = crs.split("cr")[2]
    cr2=  crs.split("cr")[3]
    cr3=  crs.split("cr")[4]
    
    yErrorLabels = []
    for i in range(0, 16,2):
        yErrorLabels.append(str(cr0)+'/'+str(i))
    for i in range(0, 16,2):
        yErrorLabels.append(str(cr1)+'/'+str(i))
    for i in range(0, 16,2):
        yErrorLabels.append(str(cr2)+'/'+str(i))
    for i in range(0, 16,2):
        yErrorLabels.append(str(cr3)+'/'+str(i))
        
    return yErrorLabels
        

if __name__=='__main__':
    # set input file and config options

    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    flags = initConfigFlags()
    flags.Exec.MaxEvents = 1

    flags.Input.Files = ["/eos/atlas/atlastier0/rucio/data22_13p6TeV/express_express/00423433/data22_13p6TeV.00423433.express_express.recon.ESD.x653/data22_13p6TeV.00423433.express_express.recon.ESD.x653._lb0015._SFO-ALL._0001.2"]
    flags.Output.HISTFileName = 'ExampleMonitorOutput_LVL1.root'

    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))
    

    PprMonitorCfg = PprMonitoringConfig(flags)
    cfg.merge(PprMonitorCfg)

    # message level for algorithm
    PprMonitorCfg.getEventAlgo('PprMonAlg').OutputLevel = 1 # 1/2 INFO/DEBUG
    # options - print all details of algorithms, very short summary 
    cfg.printConfig(withDetails=True, summariseProps = True)
    import sys
    sys.exit(cfg.run().isFailure())
