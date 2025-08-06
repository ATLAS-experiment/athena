#!/usr/bin/env python
#
#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

'''@file ZdcMonitorAlgorithm.py
@author Y. Guo
@date 2023-08-01
@brief python configuration for ZDC monitoring under the Run III DQ framework
    will be run in the ZDC calibration stream & physics MinBias stream
    see https://acode-browser1.usatlas.bnl.gov/lxr/source/athenAControl/AthenaMonitoring/python/ExampleMonitorAlgorithm.py
    for details of structure of monitoring-configuration files
@reference https://twiki.cern.ch/twiki/bin/view/Atlas/DQRun3FrameworkTutorial
'''

import numpy as np

module_FPGA_max_ADC = 4096 

def create_log_bins(min_value, max_value, num_bins):
    # Calculate the logarithmic bin edges
    log_min = np.log10(min_value)
    log_max = np.log10(max_value)
    log_bin_edges = np.linspace(log_min, log_max, num_bins + 1)
    bin_edges = [10 ** edge for edge in log_bin_edges]

    return bin_edges

def create_vinj_bins():

    # Define min, max, and step size for each range
    min1, max1, step1 = 0.0005 - 0.000001, 0.0025 - 0.000001, 0.00005
    min2, max2, step2 = 0.0025 - 0.000001, 0.01   - 0.000001, 0.00025
    min3, max3, step3 = 0.01   - 0.000001, 0.3    - 0.000001, 0.0025
    min4, max4, step4 = 0.3    - 0.000001, 0.675  - 0.000001, 0.0125
    min5, max5, step5 = 0.675  - 0.000001, 2.500001,          0.025

    # Generate each range using the defined variables
    range1 = np.arange(min1, max1, step1)
    range2 = np.arange(min2, max2, step2)
    range3 = np.arange(min3, max3, step3)
    range4 = np.arange(min4, max4, step4)
    range5 = np.arange(min5, max5, step5)

    # Concatenate the ranges into a single array
    vinj_bins_array = np.concatenate((np.array([0.]), range1, range2, range3, range4, range5))

    return vinj_bins_array.tolist()

def create_hg_fit_amp_inj_bins():

    # Define min, max, and step size for each range
    min1, max1, step1 = 0, 280, 4 # up to the tail of 10mV 
    min2, max2, step2 = 280, module_FPGA_max_ADC+1, 8

    # Generate each range using the defined variables
    range1 = np.arange(min1, max1, step1)
    range2 = np.arange(min2, max2, step2)

    # Concatenate the ranges into a single array
    hg_amp_inj_bins_array = np.concatenate((range1, range2))

    return hg_amp_inj_bins_array.tolist()

def create_lg_fit_amp_inj_bins():

    # Define min, max, and step size for each range
    min1, max1, step1 = 0, 28, 0.4 # up to the tail of 10mV 
    min2, max2, step2 = 28, 800, 0.8 # roughly correspond to 300mV - can fine tune with ntuple
    min3, max3, step3 = 800, module_FPGA_max_ADC+1, 8


    # Generate each range using the defined variables
    range1 = np.arange(min1, max1, step1)
    range2 = np.arange(min2, max2, step2)
    range3 = np.arange(min3, max3, step3)

    # Concatenate the ranges into a single array
    lg_amp_inj_bins_array = np.concatenate((range1, range2, range3))

    return lg_amp_inj_bins_array.tolist()

def ZdcMonitoringConfig(inputFlags):

    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(inputFlags,'ZdcAthMonitorCfg')

    from AthenaConfiguration.ComponentFactory import CompFactory
    zdcMonAlg = helper.addAlgorithm(CompFactory.ZdcMonitorAlgorithm,'ZdcMonAlg')

    from ZdcRec.ZdcRecConfig import SetConfigTag
    config = SetConfigTag(inputFlags)
    print ('ZdcMonitorAlgorithm.py: Running with config tag ', config)
    
    # Edit properties of a algorithm
    zdcMonAlg.EnableZDCSingleSideTriggers = inputFlags.DQ.useTrigger and inputFlags.Input.TriggerStream == 'calibration_ZDCCalib' # added for online: enable trigger if we are running in ATLAS partition (DQ.useTrigger flag not set offline)
    zdcMonAlg.CalInfoOn = 'physics_' in inputFlags.Input.TriggerStream or inputFlags.Input.TriggerStream == 'express_express' # turn calorimeter info on if input triggerstream (autoconfigured from input file) is physics_MinBias / express_express / physics_UCC
    zdcMonAlg.EnableUCCTriggers = inputFlags.DQ.useTrigger and inputFlags.Input.TriggerStream == 'physics_UCC'
    zdcMonAlg.IsOnline = inputFlags.Common.isOnline # if running online select a subset of histograms & use coarser binnings
    zdcMonAlg.IsInjectedPulse = inputFlags.Input.TriggerStream == 'calibration_ZDCInjCalib' or inputFlags.Input.TriggerStream == 'calibration_DcmDummyProcessor'
    zdcMonAlg.IsStandalone = inputFlags.Input.TriggerStream == 'calibration_DcmDummyProcessor'
    
    zdcMonAlg.IsPEBStream = 'calibration_' in inputFlags.Input.TriggerStream
    zdcMonAlg.IsPPMode = 'pp' in config
    zdcMonAlg.IspOMode = 'pO' in config
    zdcMonAlg.IsOOMode = 'OO' in config
    zdcMonAlg.EnableOOpOTriggers = (zdcMonAlg.IspOMode or zdcMonAlg.IsOOMode) and inputFlags.DQ.useTrigger and ('physics_' in inputFlags.Input.TriggerStream or inputFlags.Input.TriggerStream == 'calibration_ZDCCalib')

    phys_stream_trig_map = {
        "physics_MinBias": ['HLT_mb_sptrk_L1TRT_FILLED', 'HLT_mb_sptrk_L1jTE10', 'HLT_mb_sptrk_L1ZDC_OR', 'HLT_mb_sptrk_L1ZDC_LOR', 'HLT_noalg_L1ZDC_OR_EMPTY', 'HLT_noalg_L1ZDC_OR_UNPAIRED_NONISO', 'HLT_noalg_L1ZDC_LOR_EMPTY', 'HLT_noalg_L1ZDC_LOR_UNPAIRED_NONISO'],
        "physics_UPC": ["L1_ZDC_XOR_jTE10_VjTE200", "L1_TRT_VjTE50", "L1_1ZDC_NZDC_jTE10_VjTE200", "L1_5ZDC_A_5ZDC_C_jTE5_VjTE200", "HLT_noalg_L1TRT_VjTE50"],
        "physics_PC": ["HLT_mb_sptrk_L1ZDC_A_C_VTE50", "L1_ZDC_A_C_VTE50", "HLT_noalg_pc_L1TE50_VTE600.0ETA49", "L1_TE50_VTE600.0ETA49"], #Run2; ["HLT_noalg_L1jTE50_VjTE600", "HLT_mb_sptrk_pc_L1ZDC_A_C_VjTE50", "L1_jTE50_VjTE600", "L1_ZDC_A_C_VjTE50"] for #Run3 
        "physics_Standby": ["HLT_noalg_L1Standby"]
    }

    OOpOTriggerChains = phys_stream_trig_map.get(inputFlags.Input.TriggerStream, [])

    OOpOCTPIDtoL1TriggerStreamMap = {
        "calibration_ZDCCalib": {
            331: "L1_ZDC_OR",
            345: "L1_ZDC_LOR",
            200: "L1_ZDC_A_C"
        },
        "physics_MinBias": {
            120: 'L1_jTE10',
        },
        "physics_Standby": {
            331: "L1_ZDC_OR",
            345: "L1_ZDC_LOR",
            1: "L1_TRT_FILLED",
            121: "L1_jTE20"
        }
    }

    OOpOCTPIDtoL1TriggerMap = OOpOCTPIDtoL1TriggerStreamMap.get(inputFlags.Input.TriggerStream, {})
    OOpOL1TriggerChainsFromCTP = list(OOpOCTPIDtoL1TriggerMap.values())
    OOpOTriggerChains += OOpOL1TriggerChainsFromCTP

    zdcMonAlg.OOpOTriggers = OOpOTriggerChains
    zdcMonAlg.OOpOL1TriggerFromCTPIDMap = OOpOCTPIDtoL1TriggerMap

    zdcMonAlg.RunNumber = inputFlags.Input.RunNumbers[0] if len(inputFlags.Input.RunNumbers) > 0 else 0
    if (len(inputFlags.Input.RunNumbers) == 0):
        print ('ZdcMonitorAlgorithm.py:  WARNING the list in the input flag Input.RunNumbers is empty - run number not set! Likely to use default pulser setting')
    elif (len(inputFlags.Input.RunNumbers) > 1):
        print ('ZdcMonitorAlgorithm.py:  WARNING the list in the input flag Input.RunNumbers has more than one element - retrieving pulser-setting configuration using the first run number! May cause misconfiguration for the other run numbers')

    zdcMonAlg.EnableZDC = inputFlags.Detector.EnableZDC_ZDC
    zdcMonAlg.EnableZDCPhysics = zdcMonAlg.EnableZDC and not zdcMonAlg.IsInjectedPulse # no physical pulse (neutrons) for injector pulse
    zdcMonAlg.EnableRPD = inputFlags.Detector.EnableZDC_RPD and not zdcMonAlg.IsInjectedPulse
    zdcMonAlg.EnableRPDAmp = zdcMonAlg.EnableRPD
    zdcMonAlg.EnableCentroid = zdcMonAlg.EnableRPD

    print ("ZdcMonitorAlgorithm.py: IsInjectedPulse? ",zdcMonAlg.IsInjectedPulse)
    print ("ZdcMonitorAlgorithm.py: IsPPMode? ",zdcMonAlg.IsPPMode)
    print ("ZdcMonitorAlgorithm.py: IspOMode? ",zdcMonAlg.IspOMode)
    print ("ZdcMonitorAlgorithm.py: IsOOMode? ",zdcMonAlg.IsOOMode)
    print ("ZdcMonitorAlgorithm.py: EnableOOpOTriggers? ",zdcMonAlg.EnableOOpOTriggers)

# --------------------------------------------------------------------------------------------------
    # Configure histograms

    # (potentially run-type dependent) range settings
    lumi_block_max = 2000
    n_lumi_block_bins_coarse = 400
    bcid_max = 3564
    n_energy_bins_default = 200
    n_fpga_bins = 204
    n_time_centroid_bins_default = 100
    n_module_amp_coarse_bins = 100
    n_module_amp_fine_bins = 200
    n_mod_fraction_bins_default = 100

    n_HG_LG_amp_ratio_bins = 120
    n_HG_LG_time_diff_bins = 50

    module_chisq_min = 0.1
    module_chisq_max = 800000
    module_chisq_nbins = 80
    module_chisq_over_amp_min = 0.01
    module_chisq_over_amp_max = 3000
    module_chisq_over_amp_nbins = 80
    module_chisq_over_amp_linear_max = 50
    module_chisq_over_amp_linear_nbins = 200


    # to ensure the logarithmic binning in C++ algorithm agrees with python
    # so that the inverse-bin-width weight calculation is correct
    zdcMonAlg.ZDCModuleChisqHistMinValue = module_chisq_min
    zdcMonAlg.ZDCModuleChisqHistMaxvalue = module_chisq_max
    zdcMonAlg.ZDCModuleChisqHistNumBins = module_chisq_nbins
    zdcMonAlg.ZDCModuleChisqOverAmpHistMinValue = module_chisq_over_amp_min
    zdcMonAlg.ZDCModuleChisqOverAmpHistMaxvalue = module_chisq_over_amp_max
    zdcMonAlg.ZDCModuleChisqOverAmpHistNumBins = module_chisq_over_amp_nbins
    
    zdcMonAlg.EnergyCutForModuleFractMonitor = 400 if zdcMonAlg.IsPPMode or zdcMonAlg.IspOMode or zdcMonAlg.IsOOMode else 13400
    zdcMonAlg.triggerSideA = "L1_ZDC_PP_A" if zdcMonAlg.IsPPMode else "L1_ZDC_A"
    zdcMonAlg.triggerSideC = "L1_ZDC_PP_C" if zdcMonAlg.IsPPMode else "L1_ZDC_C"
# --------------------------------------------------------------------------------------------------
    if (zdcMonAlg.IsInjectedPulse):
        from ZdcMonitoring.ZdcInjPulserVoltageReader import load_voltage_steps
        voltage_values_list = []
        voltage_strs_list = []
        load_voltage_steps(zdcMonAlg.RunNumber, voltage_values_list, voltage_strs_list)

        if len(voltage_values_list) == 0:
            voltage_values_list = [0.]
            voltage_strs_list = ["0.00000"]

        zdcMonAlg.InjPulseVoltageSteps = voltage_values_list
        zdcMonAlg.InjPulseVoltageStepsStr = voltage_strs_list

# --------------------------------------------------------------------------------------------------
    amp_LG_refit_max_ADC = module_FPGA_max_ADC

    fCal_single_side_min = -0.2
    fCal_single_side_max = 2.8
    fCal_sum_min = -0.5
    fCal_sum_max = 5.5
    fCal_single_side_nbins = 240
    fCal_sum_nbins = 240

    total_Et_sum_min = -0.5
    total_Et_sum_max = 5.5
    total_Et_sum_nbins = 240

    uncalib_sum_zoomin_nbins = 100
    energy_sum_zoomin_nbins = 200
    energy_sum_1n_nbins = 350
    energy_sum_1n_xmin = 1000.
    energy_sum_1n_xmax = 4500.
    
    ########### DEFAULT SETTINGS FOR PBPB ###########
    if "PbPb" in config or "Injector" in config:
        print ("looking at pbpb / injected-pulse data")
        energy_sum_xmax = 200000.0
        energy_sum_two_sides_xmax_TeV = 400.0
        energy_sum_single_side_xmax_TeV = 200.0
        energy_sum_zoomin_xmax = 13000.0
        uncalib_amp_sum_zoomin_xmax = 7200.0
        time_in_data_buffer = 75. #75 ns (3 BCID's) in buffer
        x_centroid_min = -20 #small amplitude sum --> large range for x, y position
        x_centroid_max = 20
        y_centroid_min = -20
        y_centroid_max = 60
        zdc_amp_sum_xmax = 163840.0
        rpd_channel_amp_min = - 2000. 
        rpd_amp_sum_xmax = 245760.0 #not the full range but a reasonable value
        rpd_max_adc_sum_xmax = 40960.0
        rpd_sum_adc_max = 25000.
        lg_gain_factor = 10
        module_calib_amp_xmax = 100000.0 #about the full dynamic range: 160 N / 4 * 2.5TeV
        module_amp_1Nmonitor_xmax = 1250.0 #about 5N / 4 * 2.7TeV
        module_calib_amp_1Nmonitor_xmax = 3400.0 #about 5N / 4 * 2.7TeV

    elif config == "LHCf2022":  ########### overwrite for LHCf ###########
        print ("looking at 2022 lhcf data")
        energy_sum_xmax = 3000
        energy_sum_two_sides_xmax_TeV = 10.0
        energy_sum_single_side_xmax_TeV = 5.0
        energy_sum_zoomin_xmax = 3000
        uncalib_amp_sum_zoomin_xmax = module_FPGA_max_ADC
        time_in_data_buffer = 75. #75 ns (3 BCID's) in buffer
        x_centroid_min = -500 #small amplitude sum --> large range for x, y position
        x_centroid_max = 500
        y_centroid_min = -50
        y_centroid_max = 750
        zdc_amp_sum_xmax = 3000
        rpd_channel_amp_min = - 200. 
        rpd_amp_sum_xmax = 3000
        rpd_max_adc_sum_xmax = 3000
        rpd_sum_adc_max = 5000
        lg_gain_factor = 1
        module_calib_amp_xmax = 5000
        module_amp_1Nmonitor_xmax = 2000 #about 5N / 4 * 2.7TeV
        module_calib_amp_1Nmonitor_xmax = 5000 #about 5N / 4 * 2.7TeV

    elif zdcMonAlg.IsPPMode:  ########### overwrite for pp mode ###########
        print ("looking at pp data")
        energy_sum_xmax = 5000
        energy_sum_two_sides_xmax_TeV = 10.0
        energy_sum_single_side_xmax_TeV = 5.0
        energy_sum_zoomin_xmax = 5000
        uncalib_amp_sum_zoomin_xmax = module_FPGA_max_ADC
        time_in_data_buffer = 75. #75 ns (3 BCID's) in buffer
        x_centroid_min = -20 #small amplitude sum --> large range for x, y position
        x_centroid_max = 20
        y_centroid_min = -20
        y_centroid_max = 120
        zdc_amp_sum_xmax = 5000
        rpd_channel_amp_min = - 200. 
        rpd_amp_sum_xmax = 5000
        rpd_max_adc_sum_xmax = 5000
        rpd_sum_adc_max = 5000.
        lg_gain_factor = 1
        module_calib_amp_xmax = 5000
        module_amp_1Nmonitor_xmax = 2000 #about 5N / 4 * 2.7TeV
        module_calib_amp_1Nmonitor_xmax = 5000 #about 5N / 4 * 2.7TeV
    elif zdcMonAlg.IsOOMode:  ########### overwrite for OO mode ###########
        print ("looking at OO data")
        energy_sum_xmax = 10000.
        energy_sum_two_sides_xmax_TeV = 40.
        energy_sum_single_side_xmax_TeV = 20.0
        energy_sum_zoomin_xmax = 13000.0 #4.85N (tail of 4N)
        uncalib_amp_sum_zoomin_xmax = 7200.0 #tail of 4N
        time_in_data_buffer = 75. #75 ns (3 BCID's) in buffer
        x_centroid_min = -20 #small amplitude sum --> large range for x, y position
        x_centroid_max = 20
        y_centroid_min = -20
        y_centroid_max = 60
        zdc_amp_sum_xmax = 50000.0
        rpd_channel_amp_min = - 2000. 
        rpd_amp_sum_xmax = 245760.0
        rpd_max_adc_sum_xmax = 40960.0
        rpd_sum_adc_max = 25000.
        lg_gain_factor = 4
        module_calib_amp_xmax = 13400.0 #20N / 4 * 2680.
        module_amp_1Nmonitor_xmax = 1250.0 #about 5N / 4 * 2.7TeV
        module_calib_amp_1Nmonitor_xmax = 3400.0 #about 5N / 4 * 2.7TeV
        fCal_single_side_min = -0.02 # overwrite FCal binning
        fCal_single_side_max = 0.43
        fCal_sum_min = -0.03
        fCal_sum_max = 0.72
        fCal_single_side_nbins = 90
        fCal_sum_nbins = 150
        total_Et_sum_min = -0.1 # overwrite total ET binning 
        total_Et_sum_max = 1.5
        total_Et_sum_nbins = 160

    elif zdcMonAlg.IspOMode:  ########### overwrite for OO mode ###########
        print ("looking at pO data")
        energy_sum_xmax = 30000.
        energy_sum_two_sides_xmax_TeV = 40. #22.4N per side
        energy_sum_single_side_xmax_TeV = 20.0
        energy_sum_zoomin_xmax = 13000.0 #4.85N (tail of 4N)
        uncalib_amp_sum_zoomin_xmax = 7200.0 #tail of 4N
        time_in_data_buffer = 75. #75 ns (3 BCID's) in buffer
        x_centroid_min = -20 #small amplitude sum --> large range for x, y position
        x_centroid_max = 20
        y_centroid_min = -20
        y_centroid_max = 60
        zdc_amp_sum_xmax = 50000.0
        rpd_channel_amp_min = - 2000. 
        rpd_amp_sum_xmax = 245760.0
        rpd_max_adc_sum_xmax = 40960.0
        rpd_sum_adc_max = 25000.
        lg_gain_factor = 4
        module_calib_amp_xmax = 13400.0 #20N / 4 * 2680.
        module_amp_1Nmonitor_xmax = 1250.0 #about 5N / 4 * 2.7TeV
        module_calib_amp_1Nmonitor_xmax = 3400.0 #about 5N / 4 * 2.7TeV
        fCal_single_side_min = -0.05 # overwrite FCal binning
        fCal_single_side_max = 0.2
        fCal_sum_min = -0.05
        fCal_sum_max = 0.35
        fCal_single_side_nbins = 100
        fCal_sum_nbins = 100
        total_Et_sum_min = -0.1 # overwrite total ET binning 
        total_Et_sum_max = 0.5
        total_Et_sum_nbins = 120
    else:
        import sys
        print("ZdcMonitorAlgorithm      ERROR: Invalid configuration! Config tag is", config)
        sys.exit(1)


    lg_max_ADC = module_FPGA_max_ADC * lg_gain_factor
    module_amp_xmax = lg_max_ADC

    hg_lg_amp_ratio_min_nominal = 0.6
    hg_lg_amp_ratio_min_tight = 0.9
    hg_lg_amp_ratio_max_nominal = 1.4
    hg_lg_amp_ratio_max_tight = 1.2

    # #bins for RPD channel amplitude, including negative values - determined by the ratio between the negative amplitude range & positive amplitude range
    rpd_sub_amp_min = - module_amp_xmax / 4.
    rpd_sub_amp_max = module_amp_xmax / 2.
    n_rpd_amp_bins_full_range = int((abs(rpd_channel_amp_min) + rpd_sum_adc_max) / rpd_sum_adc_max * n_energy_bins_default)
    n_rpd_sub_amp_bins = int((abs(rpd_sub_amp_min) + rpd_sub_amp_max) / rpd_sub_amp_max * n_energy_bins_default)

# --------------------------------------------------------------------------------------------------
# ----------------------------------- General ZDC observables ----------------------------------- 
# --------- Event-level observables / decoding error monitoring / A-C side correlations ---------
# --------------------------------------------------------------------------------------------------

    genZdcMonTool = helper.addGroup(zdcMonAlg, 'genZdcMonTool', topPath = 'ZDC')

    nDecodingErrorBits = 3

    genZdcMonTool.defineHistogram('decodingErrorBits',title=';;Events',
                            path='/EXPERT/Global/DecodingErrors',
                            opt='kVec',
                            xbins=nDecodingErrorBits,xmin=0.0,xmax=nDecodingErrorBits,
                            xlabels=['No Decoding Error', 'ZDC Decoding Error', 'RPD Decoding Error'])
    
    nUCCTrigBits = 7

    genZdcMonTool.defineHistogram('uccTrigBits',title=';;Events',
                            path='/EXPERT/Global/UCCTrigs',
                            opt='kVec',
                            xbins=nUCCTrigBits,xmin=0.0,xmax=nUCCTrigBits,
                            xlabels=['UCC Trig Enabled', 'Pass HELT50', 'Pass HELT35', 'Pass HELT25', 'Pass HELT20', 'Pass HELT15', 'UCC Trig Disabled'])

    if zdcMonAlg.EnableOOpOTriggers:
        OOpOTriggerLabels = ['OOpO Trig Enabled'] + OOpOTriggerChains + ['OOpO Trig Disabled']
        
        nOOpOTrigBits = len(OOpOTriggerLabels)

        genZdcMonTool.defineHistogram('OOpOTrigBits',title=';;Events',
                                path='/EXPERT/Global/OOpOTrigs',
                                opt='kVec',
                                xbins=nOOpOTrigBits,xmin=0.0,xmax=nOOpOTrigBits,
                                xlabels=OOpOTriggerLabels)

    if (zdcMonAlg.EnableCentroid):
        genZdcMonTool.defineHistogram('rpdCosDeltaReactionPlaneAngle', title=';Cos (#Delta #phi_{AorC});Events',
                                path='/EXPERT/Global/ReactionPlane',
                                cutmask='bothHasCentroid', # only require both sides to have centroid
                                xbins=n_time_centroid_bins_default,xmin=-1,xmax=1)
        if (not zdcMonAlg.IsOnline):
            genZdcMonTool.defineHistogram('rpdCosDeltaReactionPlaneAngle;rpdCosDeltaReactionPlaneAngle_requireValid', title=';Cos (#Delta #phi_{AorC});Events',
                                    path='/EXPERT/Global/ReactionPlane',
                                    cutmask='bothReactionPlaneAngleValid', # require centroid calculation on both sides to be valid
                                    xbins=n_time_centroid_bins_default,xmin=-1,xmax=1)

    if (zdcMonAlg.EnableZDCPhysics):
        genZdcMonTool.defineHistogram('zdcEnergySumA, zdcEnergySumC', type='TH2F', title=';E_{ZDC,A} [GeV];E_{ZDC,C} [GeV]',
                                path='/EXPERT/Global/SideACCorr',
                                xbins=n_energy_bins_default,xmin=0.0,xmax=energy_sum_xmax,
                                ybins=n_energy_bins_default,ymin=0.0,ymax=energy_sum_xmax)
        if (zdcMonAlg.IspOMode or zdcMonAlg.IsOOMode): #OO mode --> add zoomed-in AC-correlation
            genZdcMonTool.defineHistogram('zdcEnergySumA, zdcEnergySumC;zdcEnergySumC_vs_zdcEnergySumA_zoomin', type='TH2F', title=';E_{ZDC,A} [GeV];E_{ZDC,C} [GeV]',
                                    path='/EXPERT/Global/SideACCorr',
                                    xbins=energy_sum_zoomin_nbins,xmin=0.0,xmax=energy_sum_zoomin_xmax,
                                    ybins=energy_sum_zoomin_nbins,ymin=0.0,ymax=energy_sum_zoomin_xmax)
            genZdcMonTool.defineHistogram('zdcUncalibSumA, zdcUncalibSumC;zdcUncalibSumC_vs_zdcUncalibSumA_zoomin', type='TH2F', title=';Amp Sum SideA [ADC];Amp Sum SideC [ADC]',
                                    path='/EXPERT/Global/SideACCorr',
                                    xbins=uncalib_sum_zoomin_nbins,xmin=0.0,xmax=uncalib_amp_sum_zoomin_xmax,
                                    ybins=uncalib_sum_zoomin_nbins,ymin=0.0,ymax=uncalib_amp_sum_zoomin_xmax)

            if (zdcMonAlg.EnableOOpOTriggers):
                for oo_po_trig in OOpOTriggerChains:
                    genZdcMonTool.defineHistogram('zdcEnergySumA, zdcEnergySumC;zdcEnergySumC_vs_zdcEnergySumA_pass'+oo_po_trig, type='TH2F', title=';E_{ZDC,A} [GeV];E_{ZDC,C} [GeV]',
                                        path='/EXPERT/Global/SideACCorr',
                                        cutmask='pass'+oo_po_trig,
                                        xbins=n_energy_bins_default,xmin=0.0,xmax=energy_sum_xmax,
                                        ybins=n_energy_bins_default,ymin=0.0,ymax=energy_sum_xmax)
                    genZdcMonTool.defineHistogram('zdcEnergySumA, zdcEnergySumC;zdcEnergySumC_vs_zdcEnergySumA_zoomin_pass'+oo_po_trig, type='TH2F', title=';E_{ZDC,A} [GeV];E_{ZDC,C} [GeV]',
                                        path='/EXPERT/Global/SideACCorr',
                                        cutmask='pass'+oo_po_trig,
                                        xbins=energy_sum_zoomin_nbins,xmin=0.0,xmax=energy_sum_zoomin_xmax,
                                        ybins=energy_sum_zoomin_nbins,ymin=0.0,ymax=energy_sum_zoomin_xmax)
                    genZdcMonTool.defineHistogram('zdcUncalibSumA, zdcUncalibSumC;zdcUncalibSumC_vs_zdcUncalibSumA_zoomin_pass'+oo_po_trig, type='TH2F', title=';Amp Sum SideA [ADC];Amp Sum SideC [ADC]',
                                        path='/EXPERT/Global/SideACCorr',
                                        cutmask='pass'+oo_po_trig,
                                        xbins=uncalib_sum_zoomin_nbins,xmin=0.0,xmax=uncalib_amp_sum_zoomin_xmax,
                                        ybins=uncalib_sum_zoomin_nbins,ymin=0.0,ymax=uncalib_amp_sum_zoomin_xmax)



        # FCal E_T vs ZDC E_T
        # to be run on min bias stream
        if (zdcMonAlg.CalInfoOn):
            genZdcMonTool.defineHistogram('fcalEtA, fcalEtC', type='TH2F', title=';E_{FCal, A} [GeV];E_{FCal, C} [GeV]',
                                path='/EXPERT/Global/SideACCorr',
                                xbins=fCal_single_side_nbins,xmin=fCal_single_side_min,xmax=fCal_single_side_max,
                                ybins=fCal_single_side_nbins,ymin=fCal_single_side_min,ymax=fCal_single_side_max)

            genZdcMonTool.defineHistogram('fcalEtSumTwoSides, zdcEnergySumTwoSidesTeV;zdcEnergySum_vs_fCalEt', type='TH2F', title=';FCal Energy [TeV];ZDC Energy [TeV]',
                                path = '/EXPERT/Global/ZDCFcalCorr',
                                opt='kAlwaysCreate',
                                xbins=fCal_sum_nbins,xmin=fCal_sum_min,xmax=fCal_sum_max,
                                ybins=n_energy_bins_default,ymin=0.0,ymax=energy_sum_two_sides_xmax_TeV)

            genZdcMonTool.defineHistogram('fcalEtSumTwoSides, zdcHadronicEnergySumTwoSidesTeV;zdcHadronicEnergySum_vs_fCalEt', type='TH2F', title=';FCal Energy [TeV];ZDC Hadronic Energy [TeV]',
                                path = '/EXPERT/Global/ZDCHEFcalCorr',
                                opt='kAlwaysCreate',
                                xbins=fCal_sum_nbins,xmin=fCal_sum_min,xmax=fCal_sum_max,
                                ybins=n_energy_bins_default,ymin=0.0,ymax=energy_sum_two_sides_xmax_TeV)

            genZdcMonTool.defineHistogram('totalEt24, zdcEnergySumTwoSidesTeV;zdcEnergySum_vs_totalEt', type='TH2F', title=';Total Et |#eta| < 2.4 [TeV];ZDC Energy [TeV]',
                                path = '/EXPERT/Global/ZDCTotalEtCorr',
                                opt='kAlwaysCreate',
                                xbins=total_Et_sum_nbins,xmin=total_Et_sum_min,xmax=total_Et_sum_max,
                                ybins=n_energy_bins_default,ymin=0.0,ymax=energy_sum_two_sides_xmax_TeV)


            if (zdcMonAlg.EnableOOpOTriggers):
                for oo_po_trig in OOpOTriggerChains:
                    genZdcMonTool.defineHistogram('fcalEtSumTwoSides, zdcEnergySumTwoSidesTeV;zdcEnergySum_vs_fCalEt_pass'+oo_po_trig, type='TH2F', title=';FCal Energy [TeV];ZDC Energy [TeV]',
                                        path = '/EXPERT/Global/ZDCFcalCorr',
                                        cutmask='pass'+oo_po_trig,
                                        xbins=fCal_sum_nbins,xmin=fCal_sum_min,xmax=fCal_sum_max,
                                        ybins=n_energy_bins_default,ymin=0.0,ymax=energy_sum_two_sides_xmax_TeV)
                    genZdcMonTool.defineHistogram('totalEt24, zdcEnergySumTwoSidesTeV;zdcEnergySum_vs_totalEt_pass'+oo_po_trig, type='TH2F', title=';Total Et |#eta| < 2.4 [TeV];ZDC Energy [TeV]',
                                        path = '/EXPERT/Global/ZDCTotalEtCorr',
                                        cutmask='pass'+oo_po_trig,
                                        xbins=total_Et_sum_nbins,xmin=total_Et_sum_min,xmax=total_Et_sum_max,
                                        ybins=n_energy_bins_default,ymin=0.0,ymax=energy_sum_two_sides_xmax_TeV)


            if (zdcMonAlg.EnableUCCTriggers):
                genZdcMonTool.defineHistogram('fcalEtSumTwoSides, zdcEnergySumTwoSidesTeV;zdcEnergySum_vs_fCalEt_passUCCTrig_HELT15', type='TH2F', title=';FCal Energy [TeV];ZDC Energy [TeV]',
                                    path = '/EXPERT/Global/ZDCFcalCorr',
                                    cutmask = 'passUCCTrig_HELT15',
                                    opt='kAlwaysCreate',
                                    xbins=fCal_sum_nbins,xmin=fCal_sum_min,xmax=fCal_sum_max,
                                    ybins=n_energy_bins_default,ymin=0.0,ymax=energy_sum_two_sides_xmax_TeV)
                genZdcMonTool.defineHistogram('fcalEtSumTwoSides, zdcEnergySumTwoSidesTeV;zdcEnergySum_vs_fCalEt_passUCCTrig_HELT20', type='TH2F', title=';FCal Energy [TeV];ZDC Energy [TeV]',
                                    path = '/EXPERT/Global/ZDCFcalCorr',
                                    cutmask = 'passUCCTrig_HELT20',
                                    opt='kAlwaysCreate',
                                    xbins=fCal_sum_nbins,xmin=fCal_sum_min,xmax=fCal_sum_max,
                                    ybins=n_energy_bins_default,ymin=0.0,ymax=energy_sum_two_sides_xmax_TeV)
                genZdcMonTool.defineHistogram('fcalEtSumTwoSides, zdcEnergySumTwoSidesTeV;zdcEnergySum_vs_fCalEt_passUCCTrig_HELT25', type='TH2F', title=';FCal Energy [TeV];ZDC Energy [TeV]',
                                    path = '/EXPERT/Global/ZDCFcalCorr',
                                    cutmask = 'passUCCTrig_HELT25',
                                    opt='kAlwaysCreate',
                                    xbins=fCal_sum_nbins,xmin=fCal_sum_min,xmax=fCal_sum_max,
                                    ybins=n_energy_bins_default,ymin=0.0,ymax=energy_sum_two_sides_xmax_TeV)
                genZdcMonTool.defineHistogram('fcalEtSumTwoSides, zdcEnergySumTwoSidesTeV;zdcEnergySum_vs_fCalEt_passUCCTrig_HELT35', type='TH2F', title=';FCal Energy [TeV];ZDC Energy [TeV]',
                                    path = '/EXPERT/Global/ZDCFcalCorr',
                                    cutmask = 'passUCCTrig_HELT35',
                                    opt='kAlwaysCreate',
                                    xbins=fCal_sum_nbins,xmin=fCal_sum_min,xmax=fCal_sum_max,
                                    ybins=n_energy_bins_default,ymin=0.0,ymax=energy_sum_two_sides_xmax_TeV)
                genZdcMonTool.defineHistogram('fcalEtSumTwoSides, zdcEnergySumTwoSidesTeV;zdcEnergySum_vs_fCalEt_passUCCTrig_HELT50', type='TH2F', title=';FCal Energy [TeV];ZDC Energy [TeV]',
                                    path = '/EXPERT/Global/ZDCFcalCorr',
                                    cutmask = 'passUCCTrig_HELT50',
                                    opt='kAlwaysCreate',
                                    xbins=fCal_sum_nbins,xmin=fCal_sum_min,xmax=fCal_sum_max,
                                    ybins=n_energy_bins_default,ymin=0.0,ymax=energy_sum_two_sides_xmax_TeV)

                genZdcMonTool.defineHistogram('fcalEtSumTwoSides, zdcHadronicEnergySumTwoSidesTeV;zdcHadronicEnergySum_vs_fCalEt_passUCCTrig_HELT15', type='TH2F', title=';FCal Energy [TeV];ZDC Hadronic Energy [TeV]',
                                    path = '/EXPERT/Global/ZDCHEFcalCorr',
                                    cutmask = 'passUCCTrig_HELT15',
                                    opt='kAlwaysCreate',
                                    xbins=fCal_sum_nbins,xmin=fCal_sum_min,xmax=fCal_sum_max,
                                    ybins=n_energy_bins_default,ymin=0.0,ymax=energy_sum_two_sides_xmax_TeV)
                genZdcMonTool.defineHistogram('fcalEtSumTwoSides, zdcHadronicEnergySumTwoSidesTeV;zdcHadronicEnergySum_vs_fCalEt_passUCCTrig_HELT20', type='TH2F', title=';FCal Energy [TeV];ZDC Hadronic Energy [TeV]',
                                    path = '/EXPERT/Global/ZDCHEFcalCorr',
                                    cutmask = 'passUCCTrig_HELT20',
                                    opt='kAlwaysCreate',
                                    xbins=fCal_sum_nbins,xmin=fCal_sum_min,xmax=fCal_sum_max,
                                    ybins=n_energy_bins_default,ymin=0.0,ymax=energy_sum_two_sides_xmax_TeV)
                genZdcMonTool.defineHistogram('fcalEtSumTwoSides, zdcHadronicEnergySumTwoSidesTeV;zdcHadronicEnergySum_vs_fCalEt_passUCCTrig_HELT25', type='TH2F', title=';FCal Energy [TeV];ZDC Hadronic Energy [TeV]',
                                    path = '/EXPERT/Global/ZDCHEFcalCorr',
                                    cutmask = 'passUCCTrig_HELT25',
                                    opt='kAlwaysCreate',
                                    xbins=fCal_sum_nbins,xmin=fCal_sum_min,xmax=fCal_sum_max,
                                    ybins=n_energy_bins_default,ymin=0.0,ymax=energy_sum_two_sides_xmax_TeV)
                genZdcMonTool.defineHistogram('fcalEtSumTwoSides, zdcHadronicEnergySumTwoSidesTeV;zdcHadronicEnergySum_vs_fCalEt_passUCCTrig_HELT35', type='TH2F', title=';FCal Energy [TeV];ZDC Hadronic Energy [TeV]',
                                    path = '/EXPERT/Global/ZDCHEFcalCorr',
                                    cutmask = 'passUCCTrig_HELT35',
                                    opt='kAlwaysCreate',
                                    xbins=fCal_sum_nbins,xmin=fCal_sum_min,xmax=fCal_sum_max,
                                    ybins=n_energy_bins_default,ymin=0.0,ymax=energy_sum_two_sides_xmax_TeV)
                genZdcMonTool.defineHistogram('fcalEtSumTwoSides, zdcHadronicEnergySumTwoSidesTeV;zdcHadronicEnergySum_vs_fCalEt_passUCCTrig_HELT50', type='TH2F', title=';FCal Energy [TeV];ZDC Hadronic Energy [TeV]',
                                    path = '/EXPERT/Global/ZDCHEFcalCorr',
                                    cutmask = 'passUCCTrig_HELT50',
                                    opt='kAlwaysCreate',
                                    xbins=fCal_sum_nbins,xmin=fCal_sum_min,xmax=fCal_sum_max,
                                    ybins=n_energy_bins_default,ymin=0.0,ymax=energy_sum_two_sides_xmax_TeV)

# --------------------------------------------------------------------------------------------------
    sides = ["C","A"]
    modules = ["0","1","2","3"]
    channels = ["0","1","2","3","4","5","6","7","8","9","10","11","12","13","14","15"]

    nZdcStatusBits = 18
    nRpdStatusBits = 15
    nRpdCentroidStatusBits = 21

# --------------------------------------------------------------------------------------------------
# ---------------------------------- Per-ZDC-arm/side observables ---------------------------------- 
# --------------------------------------------------------------------------------------------------

    if (zdcMonAlg.EnableZDCPhysics or zdcMonAlg.EnableRPDAmp or zdcMonAlg.EnableCentroid):

        zdcSideMonToolArr = helper.addArray([sides],zdcMonAlg,'ZdcSideMonitor', topPath = 'ZDC')


    # ---------------------------- Calorimeter per-arm variables ---------------------------- 
    # ---------------------------- Calorimeter energy/amplitude sum ----------------------------
    if (zdcMonAlg.EnableZDCPhysics):
        zdcSideMonToolArr.defineHistogram('zdcEnergySum',title='ZDC Side {0} Energy Sum;Side {0} Energy [GeV];Events',
                                path = '/EXPERT/ZDC/PerArm/Energy',
                                xbins=n_energy_bins_default,xmin=0.0,xmax=energy_sum_xmax) # 2.5TeV * 80 neutrons
        
        # --------------------- calibrated energy sum in the 1-to-4n-range ---------------------
        zdcSideMonToolArr.defineHistogram('zdcEnergySum;zdcEnergySum_zoomin_noTrigSelec',title='ZDC Side {0} Energy Sum (1-to-4n, no trigger selection);Side {0} Energy[GeV];Events',
                                path = '/SHIFT/ZDC/PerArm/Energy',
                                opt='kAlwaysCreate', # always create for shift-needed histograms
                                xbins=energy_sum_zoomin_nbins,xmin=0.0,xmax=energy_sum_zoomin_xmax) # up to the "far end" of 4-neutron peak
        if (zdcMonAlg.EnableOOpOTriggers):
            for oo_po_trig in OOpOTriggerChains:
                zdcSideMonToolArr.defineHistogram('zdcEnergySum;zdcEnergySum_zoomin_pass'+oo_po_trig,title='ZDC Side {0} Energy Sum (1-to-4n, no trigger selection);Side {0} Energy[GeV];Events',
                                        path = '/SHIFT/ZDC/PerArm/Energy',
                                        cutmask='pass'+oo_po_trig,
                                        opt='kAlwaysCreate', # always create for shift-needed histograms
                                        xbins=energy_sum_zoomin_nbins,xmin=0.0,xmax=energy_sum_zoomin_xmax) # up to the "far end" of 4-neutron peak

        if (zdcMonAlg.EnableZDCSingleSideTriggers):
            zdcSideMonToolArr.defineHistogram('zdcEnergySum;zdcEnergySum_zoomin_wTrigSelec',title='ZDC Side {0} Energy Sum (1-to-4n, require opposite-side trigger);Side {0} Energy[GeV];Events',
                                    path = '/SHIFT/ZDC/PerArm/Energy',
                                    opt='kAlwaysCreate', # always create for shift-needed histograms
                                    cutmask = 'passTrigOppSide',
                                    xbins=energy_sum_zoomin_nbins,xmin=0.0,xmax=energy_sum_zoomin_xmax) # up to the "far end" of 4-neutron peak
        
        # --------------------- uncalibrated amplitude sum in the 1-to-4n-range ---------------------
        zdcSideMonToolArr.defineHistogram('zdcUncalibSum;zdcUncalibSum_zoomin_noTrigSelec',title='ZDC Side {0} Uncalibrated Amplitude Sum (1-to-4n, no trigger selection);Side {0} Amp Sum [ADC];Events',
                                path = '/EXPERT/ZDC/PerArm/UncalibAmp',
                                xbins=energy_sum_zoomin_nbins,xmin=0.0,xmax=uncalib_amp_sum_zoomin_xmax) # up to the "far end" of 4-neutron peak

        if (zdcMonAlg.EnableZDCSingleSideTriggers):
            zdcSideMonToolArr.defineHistogram('zdcUncalibSum;zdcUncalibSum_zoomin_wTrigSelec',title='ZDC Side {0} Uncalibrated Amplitude Sum (1-to-4n, require opposite-side trigger);Side {0} Amp Sum [ADC];Events',
                                    path = '/EXPERT/ZDC/PerArm/UncalibAmp',
                                    cutmask = 'passTrigOppSide',
                                    xbins=energy_sum_zoomin_nbins,xmin=0.0,xmax=uncalib_amp_sum_zoomin_xmax) # up to the "far end" of 4-neutron peak
        
        # --------------------- uncalibrated amplitude sum in the full range ---------------------
        zdcSideMonToolArr.defineHistogram('zdcUncalibSum',title='ZDC Side {0} Uncalibrated Sum;[ADC];Events',
                                path = '/EXPERT/ZDC/PerArm/UncalibAmp',
                                xbins=n_energy_bins_default,xmin=0.0,xmax=zdc_amp_sum_xmax)

    # ---------------------------- Calorimeter energy/amplitude sum: LB dependence ----------------------------
        if (not zdcMonAlg.IsOnline): # offline - use fine LB binnings
            zdcSideMonToolArr.defineHistogram('lumiBlock, zdcEnergySum;zdcEnergySum_vs_lb_noTrig', type='TH2F', title=';lumi block;Side {0} Energy [GeV]',
                                    path = '/EXPERT/ZDC/PerArm/Energy',
                                    xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                                    ybins=energy_sum_zoomin_nbins,ymin=0.0,ymax=energy_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks
            if (zdcMonAlg.EnableZDCSingleSideTriggers):
                zdcSideMonToolArr.defineHistogram('lumiBlock, zdcEnergySum;zdcEnergySum_vs_lb_wTrig', type='TH2F', title=';lumi block;Side {0} Energy [GeV]',
                                        path = '/EXPERT/ZDC/PerArm/Energy',
                                        cutmask = 'passTrigOppSide',
                                        xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max, 
                                        ybins=energy_sum_zoomin_nbins,ymin=0.0,ymax=energy_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks
        else: # online - use coarse LB binnings
            zdcSideMonToolArr.defineHistogram('zdcEnergySum;zdcEnergySum_1n_noTrigSelec',title='ZDC Side {0} Energy Sum (1n range, no trigger selection);Side {0} Energy[GeV];Events',
                                    path = '/SHIFT/ZDC/PerArm/Energy',
                                    opt='kAlwaysCreate', # always create for shift-needed histograms
                                    xbins=energy_sum_1n_nbins,xmin=energy_sum_1n_xmin,xmax=energy_sum_1n_xmax)

            zdcSideMonToolArr.defineHistogram('lumiBlock, zdcEnergySum;zdcEnergySum_1n_vs_lb_noTrig', type='TH2F', title=';lumi block;Side {0} Energy (1n range) [GeV]',
                                    path = '/EXPERT/ZDC/PerArm/Energy',
                                    xbins=n_lumi_block_bins_coarse,xmin=0.0,xmax=lumi_block_max,
                                    ybins=energy_sum_1n_nbins,ymin=energy_sum_1n_xmin,ymax=energy_sum_1n_xmax)

        if (not zdcMonAlg.IsOnline): #only offline
            zdcSideMonToolArr.defineHistogram('lumiBlock, zdcUncalibSum;zdcUncalibSum_vs_lb_noTrig', type='TH2F', title=';lumi block;ZDC Side {0} Uncalibrated Sum [ADC]',
                                    path = '/EXPERT/ZDC/PerArm/UncalibAmp',
                                    xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                                    ybins=n_energy_bins_default,ymin=0.0,ymax=uncalib_amp_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks
            if (zdcMonAlg.EnableZDCSingleSideTriggers):
                zdcSideMonToolArr.defineHistogram('lumiBlock, zdcUncalibSum;zdcUncalibSum_vs_lb_wTrig', type='TH2F', title=';lumi block;ZDC Side {0} Uncalibrated Sum [ADC]',
                                        path = '/EXPERT/ZDC/PerArm/UncalibAmp',
                                        cutmask = 'passTrigOppSide',
                                        xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max, 
                                        ybins=n_energy_bins_default,ymin=0.0,ymax=uncalib_amp_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks

    # ---------------------------- Calorimeter energy/amplitude sum: BCID dependence ----------------------------
        if (not zdcMonAlg.IsOnline): #only offline
            zdcSideMonToolArr.defineHistogram('bcid, zdcEnergySum;zdcEnergySum_vs_bcid_noTrig', type='TH2F', title=';BCID;Side {0} Energy [GeV]',
                                    path = '/EXPERT/ZDC/PerArm/Energy',
                                    xbins=bcid_max,xmin=0.0,xmax=bcid_max,
                                    ybins=n_energy_bins_default,ymin=0.0,ymax=energy_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks
            if (zdcMonAlg.EnableZDCSingleSideTriggers):
                zdcSideMonToolArr.defineHistogram('bcid, zdcEnergySum;zdcEnergySum_vs_bcid_wTrig', type='TH2F', title=';BCID;Side {0} Energy [GeV]',
                                        path = '/EXPERT/ZDC/PerArm/Energy',
                                        cutmask = 'passTrigOppSide',
                                        xbins=bcid_max,xmin=0.0,xmax=bcid_max, 
                                        ybins=n_energy_bins_default,ymin=0.0,ymax=energy_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks
        
        if (zdcMonAlg.CalInfoOn):
            zdcSideMonToolArr.defineHistogram('fCalEt, zdcEnergySumTeV;zdcEnergySum_vs_fCalEt_single_side', type='TH2F', title=';Side {0} FCal Energy [TeV];Side {0} ZDC Energy [TeV]',
                                path = '/EXPERT/ZDC/PerArm/ZDCFcalCorr',
                                opt='kAlwaysCreate',
                                xbins=fCal_single_side_nbins,xmin=fCal_single_side_min,xmax=fCal_single_side_max,
                                ybins=n_energy_bins_default,ymin=0.0,ymax=energy_sum_single_side_xmax_TeV)


    # ---------------------------- Calorimeter average time & LB dependence ---------------------------- 

        zdcSideMonToolArr.defineHistogram('zdcAvgTime',title='ZDC Side Average Time;t[ns];Events',
                                path = '/EXPERT/ZDC/PerArm/AvgTime',
                                xbins=n_time_centroid_bins_default,xmin=-10.0,xmax=10.0)

        if (not zdcMonAlg.IsOnline): #only offline
            zdcSideMonToolArr.defineHistogram('lumiBlock, zdcAvgTime;zdcAvgTime_vs_lb', type='TH2F', title=';ZDC Side Average Time versus Lumi block;lumi block;t[ns]',
                                    path = '/EXPERT/ZDC/PerArm/AvgTime',
                                    xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                                    ybins=n_time_centroid_bins_default,ymin=-10.0,ymax=10.0)

    # ---------------------------- RPD (centroid-related) per-arm variables ---------------------------- 
    # ---------------------------- centroid status ---------------------------- 
        zdcSideMonToolArr.defineHistogram('centroidStatusBits',title=';;Events',
                                path='/EXPERT/RPD/PerArm/Centroid',
                                xbins=nRpdCentroidStatusBits,xmin=0.0,xmax=nRpdCentroidStatusBits,
                                opt='kVec',
                                xlabels=['ValidBit', 'HasCentroidBit', 'ZDCInvalidBit', 'InsufficientZDCEnergyBit', 'ExcessiveZDCEnergyBit', 'EMInvalidBit', 'InsufficientEMEnergyBit', 'ExcessiveEMEnergyBit', 'RPDInvalidBit', 'PileupBit', 'ExcessivePileupBit', 'ZeroSumBit', 'ExcessiveSubtrUnderflowBit', 'Row0ValidBit', 'Row1ValidBit', 'Row2ValidBit', 'Row3ValidBit', 'Col0ValidBit', 'Col1ValidBit', 'Col2ValidBit', 'Col3ValidBit'])

        zdcSideMonToolArr.defineHistogram('centroidValidBitFloat;centroidValidBit_RequireMinZDCEnergy',title='Centroid valid bit;;Events',
                                path='/SHIFT/RPD/PerArm/Centroid',
                                opt='kAlwaysCreate', # always create for shift-needed histograms
                                cutmask='passMinZDCEnergyCutForCentroidValidEvaluation',
                                xbins=2,xmin=0,xmax=2,
                                xlabels=['Valid','Invalid'])

    # ---------------------------- x, y centroid & reaction plane angle requiring centroid ValidBit ---------------------------- 
        zdcSideMonToolArr.defineHistogram('xCentroid',title=';Centroid x position [mm];Events',
                                path='/SHIFT/RPD/PerArm/Centroid',
                                opt='kAlwaysCreate', # always create for shift-needed histograms
                                cutmask='centroidValid',
                                xbins=n_time_centroid_bins_default,xmin=x_centroid_min,xmax=x_centroid_max)

        zdcSideMonToolArr.defineHistogram('yCentroid',title=';Centroid y position [mm];Events',
                                path='/SHIFT/RPD/PerArm/Centroid',
                                opt='kAlwaysCreate', # always create for shift-needed histograms
                                cutmask='centroidValid',
                                xbins=n_time_centroid_bins_default*2,xmin=y_centroid_min,xmax=y_centroid_max)

        zdcSideMonToolArr.defineHistogram('xCentroid, yCentroid',type='TH2F',title=';Centroid x position [mm];Centroid y position [mm]',
                                path='/EXPERT/RPD/PerArm/Centroid',
                                cutmask='centroidValid',
                                xbins=n_time_centroid_bins_default,xmin=x_centroid_min,xmax=x_centroid_max,
                                ybins=n_time_centroid_bins_default*2,ymin=y_centroid_min,ymax=y_centroid_max)
    
        zdcSideMonToolArr.defineHistogram('ReactionPlaneAngle',title=';Reaction Plane Angle;Events',
                                path='/EXPERT/RPD/PerArm/ReactionPlane',
                                cutmask='centroidValid',
                                xbins=64,xmin=-3.141593,xmax=3.141593)
    
    # ---------------------------- x, y centroid & reaction plane angle requiring only HasCentroidBit ---------------------------- 
        
        if (not zdcMonAlg.IsOnline): #only offline
            zdcSideMonToolArr.defineHistogram('xCentroid, yCentroid;yCentroid_vs_xCentroid_requireOnlyHasCentroidBit',type='TH2F',title=';Centroid x position [mm];Centroid y position [mm]',
                                    path='/EXPERT/RPD/PerArm/Centroid',
                                    xbins=n_time_centroid_bins_default,xmin=x_centroid_min,xmax=x_centroid_max,
                                    ybins=n_time_centroid_bins_default,ymin=y_centroid_min,ymax=y_centroid_max)
            zdcSideMonToolArr.defineHistogram('ReactionPlaneAngle;ReactionPlaneAngle_requireOnlyHasCentroidBit',title=';Reaction Plane Angle;Events',
                                        path='/EXPERT/RPD/PerArm/ReactionPlane',
                                        xbins=64,xmin=-3.141593,xmax=3.141593)


    # ---------------------------- Centroid LB dependence ---------------------------- 
        if (not zdcMonAlg.IsOnline): #only offline        
            zdcSideMonToolArr.defineHistogram('lumiBlock, xCentroid;xCentroid_vs_lb_requireOnlyHasCentroidBit', type='TH2F', title=';lumi block;Centroid x position [mm]',
                                    path='/EXPERT/RPD/PerArm/CentroidLBdep',
                                    xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                                    ybins=n_time_centroid_bins_default,ymin=x_centroid_min,ymax=x_centroid_max)
            zdcSideMonToolArr.defineHistogram('lumiBlock, yCentroid;yCentroid_vs_lb_requireOnlyHasCentroidBit', type='TH2F', title=';lumi block;Centroid y position [mm]',
                                    path='/EXPERT/RPD/PerArm/CentroidLBdep',
                                    xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                                    ybins=n_time_centroid_bins_default,ymin=y_centroid_min,ymax=y_centroid_max)

            zdcSideMonToolArr.defineHistogram('lumiBlock, xCentroid;xCentroid_vs_lb', type='TH2F', title=';lumi block;Centroid x position [mm]',
                                    path='/EXPERT/RPD/PerArm/CentroidLBdep',
                                    cutmask='centroidValid',
                                    xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                                    ybins=n_time_centroid_bins_default,ymin=x_centroid_min,ymax=x_centroid_max)
            zdcSideMonToolArr.defineHistogram('lumiBlock, yCentroid;yCentroid_vs_lb', type='TH2F', title=';lumi block;Centroid y position [mm]',
                                    path='/EXPERT/RPD/PerArm/CentroidLBdep',
                                    cutmask='centroidValid',
                                    xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                                    ybins=n_time_centroid_bins_default,ymin=y_centroid_min,ymax=y_centroid_max)

    # ---------------------------- ZDC-RPD correlations ---------------------------- 
        zdcSideMonToolArr.defineHistogram('zdcEnergySum, rpdMaxADCSum', type='TH2F', title=';E ZDC side [TeV];RPD Max ADC Sum (AorC) [ADC counts]',
                                path='/EXPERT/ZdcRpdPerSideCorr',
                                cutmask='RPDSideValid',
                                xbins=n_energy_bins_default,xmin=0.0,xmax=energy_sum_xmax,
                                ybins=n_energy_bins_default,ymin=0.0,ymax=rpd_max_adc_sum_xmax) # try a value for now
        zdcSideMonToolArr.defineHistogram('zdcEnergySum, rpdAmplitudeCalibSum', type='TH2F', title=';E ZDC side [GeV];RPD Calib Amp Sum (AorC) [ADC counts]',
                                path='/EXPERT/ZdcRpdPerSideCorr',
                                cutmask='RPDSideValid',
                                xbins=n_energy_bins_default,xmin=0.0,xmax=energy_sum_xmax,
                                ybins=n_energy_bins_default,ymin=0.0,ymax=rpd_amp_sum_xmax) # try a value for now
        zdcSideMonToolArr.defineHistogram('zdcEMModuleEnergy, rpdAmplitudeCalibSum', type='TH2F', title=';E EM module AorC [GeV];RPD Calib Amp Sum (AorC) [ADC counts]',
                                path='/EXPERT/ZdcRpdPerSideCorr',
                                cutmask='RPDSideValid',
                                xbins=n_energy_bins_default,xmin=0.0,xmax=module_calib_amp_xmax / 2., # divide by 2 to make a more zoomed-in plot (not full range)
                                ybins=n_energy_bins_default,ymin=0.0,ymax=rpd_amp_sum_xmax) # try a value for now

# --------------------------------------------------------------------------------------------------
# ------------------------------------- ZDC-module observables ------------------------------------- 
# --------------------------------------------------------------------------------------------------


    zdcModuleMonToolArr = helper.addArray([sides,modules],zdcMonAlg,'ZdcModuleMonitor', topPath = 'ZDC')

    # ---------------------------- ZDC-module status ---------------------------- 

    zdcModuleMonToolArr.defineHistogram('zdcStatusBits',title=';;Events',
                            path='/SHIFT/ZDC/ZdcModule/ModuleStatusBits',
                            opt='kAlwaysCreate kVec', # always create for shift-needed histograms
                            xbins=nZdcStatusBits,xmin=0.0,xmax=nZdcStatusBits,
                            xlabels=['PulseBit', 'LowGainBit', 'FailBit', 'HGOverflowBit', 'HGUnderflowBit', 'PSHGOverUnderflowBit', 'LGOverflowBit', 'LGUnderflowBit', 'PrePulseBit', 'PostPulseBit', 'FitFailedBit', 'BadChisqBit', 'BadT0Bit', 'ExcludeEarlyLGBit', 'ExcludeLateLGBit', 'preExpTailBit', 'FitMinAmpBit', 'RepassPulseBit'])

    # ---------------------------- ZDC-module amplitudes & amplitude fractions ---------------------------- 

    zdcModuleMonToolArr.defineHistogram('zdcModuleAmp',title=';Module Amplitude [ADC Counts];Events',
                            path='/SHIFT/ZDC/ZdcModule/ModuleAmp',
                            opt='kAlwaysCreate', # always create for shift-needed histograms
                            xbins=n_fpga_bins * 2,xmin=0.0,xmax=module_amp_xmax)

    zdcModuleMonToolArr.defineHistogram('zdcModuleMaxADC',title=';Module Max ADC;Events',
                            path='/EXPERT/ZDC/ZdcModule/ModuleMaxADC',
                            xbins=n_fpga_bins,xmin=0.0,xmax=module_amp_xmax)


    if (not zdcMonAlg.IsPPMode): # for PP mode data, LG never filled
        zdcModuleMonToolArr.defineHistogram('zdcModuleAmp;zdcModuleAmp_HG',title=';Module Amplitude HG [ADC Counts];Events',
                                path='/EXPERT/ZDC/ZdcModule/ModuleAmp', 
                                cutmask='zdcModuleHG',
                                xbins=n_fpga_bins,xmin=0.0,xmax=module_FPGA_max_ADC)

        zdcModuleMonToolArr.defineHistogram('zdcModuleAmp;zdcModuleAmp_LG',title=';Module Amplitude LG [ADC Counts];Events',
                                path='/EXPERT/ZDC/ZdcModule/ModuleAmp',
                                cutmask='zdcModuleLG', # require to use LG
                                xbins=n_fpga_bins,xmin=0.0,xmax=module_amp_xmax)

    if (zdcMonAlg.IsInjectedPulse):
        zdcModuleMonToolArr.defineHistogram('zdcModuleAmpLGRefit',title=';LG-Refit Amplitude [ADC Counts];Events',
                                path='/EXPERT/ZDC/ZdcModule/ModuleAmp',
                                cutmask='zdcModuleHG',
                                xbins=n_fpga_bins,xmin=0.0,xmax=amp_LG_refit_max_ADC)
        
    if (not zdcMonAlg.IsOnline and not zdcMonAlg.IsInjectedPulse): # only offline
        zdcModuleMonToolArr.defineHistogram('zdcModuleAmp;zdcModuleAmp_halfrange',title=';Module Amplitude [ADC Counts];Events',
                                path='/EXPERT/ZDC/ZdcModule/ModuleAmp',
                                xbins=n_fpga_bins,xmin=0.0,xmax=module_amp_xmax / 2.)
    

    # ---------------------------- ZDC-module amplitude to max ADC ratio (debug purpose) ---------------------------- 
    if (not zdcMonAlg.IsOnline): # 2D (memory consuming): offline only for calib stream
        zdcModuleMonToolArr.defineHistogram('zdcModuleAmp,zdcModuleAmpToMaxADCRatio;zdcModuleAmpToMaxADCRatio_vs_zdcModuleMaxADC_HG',type='TH2F',title=';Module Max ADC HG [ADC];Avg Amp/Max ADC',
                                path='/EXPERT/ZDC/ZdcModule/ModuleAmpToMaxADCRatio', 
                                cutmask='zdcModuleHG',
                                xbins=n_fpga_bins,xmin=0.0,xmax=module_FPGA_max_ADC,
                                ybins=100,ymin=0.0,ymax=2.)

        zdcModuleMonToolArr.defineHistogram('zdcModuleAmp,zdcModuleAmpToMaxADCRatio;zdcModuleAmpToMaxADCRatio_vs_zdcModuleMaxADC_LG',type='TH2F',title=';Module Max ADC LG [ADC];Avg Amp/Max ADC',
                                path='/EXPERT/ZDC/ZdcModule/ModuleAmpToMaxADCRatio', 
                                cutmask='zdcModuleLG',
                                xbins=n_fpga_bins,xmin=0.0,xmax=module_amp_xmax, #max ADC has no LG gain factor
                                ybins=100,ymin=0.0,ymax=2.)
    
    # ---------------------------- ZDC-module amplitude fractions & correlations with energy deposits ---------------------------- 
    
    if (zdcMonAlg.IsInjectedPulse): # no real energy deposit --> do not require minimum ZDC energy
        zdcModuleMonToolArr.defineHistogram('zdcModuleFract',title=';Module Amplitude Fraction;Events',
                                path='/SHIFT/ZDC/ZdcModule/ModuleFraction',
                                opt='kAlwaysCreate', # always create for shift-needed histograms
                                xbins=n_mod_fraction_bins_default,xmin=0.0,xmax=1.)
    else:
        zdcModuleMonToolArr.defineHistogram('zdcModuleFract;zdcModuleFract_above_cut',title=';Module Amplitude Fraction;Events',
                                path='/SHIFT/ZDC/ZdcModule/ModuleFraction',
                                opt='kAlwaysCreate', # always create for shift-needed histograms
                                cutmask='zdcEnergyAboveModuleFractCut',
                                xbins=n_mod_fraction_bins_default,xmin=0.0,xmax=1.)
        zdcModuleMonToolArr.defineHistogram('zdcEnergySumCurrentSide, zdcModuleFract;zdcModuleFract_vs_zdcEnergySum_fullrange', type='TH2F', title=';ZDC Energy Sum Current Side [GeV];Module Amplitude Fraction',
                                path='/EXPERT/ZDC/ZdcModule/ModuleFractionVsEnergy',
                                xbins=n_energy_bins_default,xmin=0.0,xmax=energy_sum_xmax,
                                ybins=n_mod_fraction_bins_default,ymin=0.0,ymax=1.)
        zdcModuleMonToolArr.defineHistogram('zdcEnergySumCurrentSide, zdcModuleFract;zdcModuleFract_vs_zdcEnergySum_profile', type='TProfile', title=';ZDC Energy Sum Current Side [GeV];Module Amplitude Fraction',
                                path='/EXPERT/ZDC/ZdcModule/ModuleFractionVsEnergy',
                                cutmask='zdcModuleFractionValid',
                                xbins=n_energy_bins_default,xmin=0.0,xmax=energy_sum_xmax)
        zdcModuleMonToolArr.defineHistogram('zdcEnergySumCurrentSide, zdcModuleFract;zdcModuleFract_vs_zdcEnergySum_zoomedin', type='TH2F', title=';Amplitude Sum Current Side [ADC Counts];Module Amplitude Fraction',
                                path='/EXPERT/ZDC/ZdcModule/ModuleFractionVsEnergy',
                                xbins=n_energy_bins_default,xmin=0.0,xmax=10000,
                                ybins=n_mod_fraction_bins_default,ymin=0.0,ymax=1.)
        if (not zdcMonAlg.IsOnline): # only offline
            zdcModuleMonToolArr.defineHistogram('zdcModuleFract',title=';Module Amplitude Fraction;Events',
                                    path='/EXPERT/ZDC/ZdcModule/ModuleFraction',
                                    xbins=n_mod_fraction_bins_default,xmin=0.0,xmax=1.)
            zdcModuleMonToolArr.defineHistogram('zdcModuleFract;zdcModuleFract_above20N',title=';Module Amplitude Fraction;Events',
                                    path='/EXPERT/ZDC/ZdcModule/ModuleFraction',
                                    cutmask='zdcAbove20NCurrentSide',
                                    xbins=n_mod_fraction_bins_default,xmin=0.0,xmax=1.)

    if (not zdcMonAlg.IsInjectedPulse):
        zdcModuleMonToolArr.defineHistogram('zdcModuleCalibAmp',title=';Module Calibrated Amplitude [GeV];Events',
                                path='/EXPERT/ZDC/ZdcModule/ModuleCalibAmp',
                                xbins=2*n_energy_bins_default,xmin=0.0,xmax=module_calib_amp_xmax) # 2.5TeV * 40
        
    if (not zdcMonAlg.IsOnline and not zdcMonAlg.IsInjectedPulse):
        zdcModuleMonToolArr.defineHistogram('zdcModuleCalibAmp;zdcModuleCalibAmp_halfrange',title=';Module Calibrated Amplitude [GeV];Events',
                                path='/EXPERT/ZDC/ZdcModule/ModuleCalibAmp',
                                xbins=2*n_energy_bins_default,xmin=0.0,xmax=module_calib_amp_xmax / 2.) # 2.5TeV * 40

    # ---------------------------- ZDC-module reco amplitude versus input voltage ---------------------------- 
    if (zdcMonAlg.IsInjectedPulse):
        # ---------------------------- HG response ----------------------------
        zdcModuleMonToolArr.defineHistogram('injectedPulseInputVoltage,zdcModuleFitAmp;zdcModuleAmpHG_vs_injectedPulseInputVoltage', type='TH2F', title=';Pulse amp [V];Signal Fit Amp [ADC Counts]',
                                cutmask='zdcHGInjPulseValid',
                                path='/EXPERT/ZDC/ZdcModule/ModuleAmpHGVsInputVoltage',
                                xbins=create_vinj_bins(),
                                ybins=create_hg_fit_amp_inj_bins())

        if (zdcMonAlg.IsOnline): # also plot profile online in case 2D-hist scale is wrong by a large factor (e.g, due to attenuator setting)
            zdcModuleMonToolArr.defineHistogram('injectedPulseInputVoltage,zdcModuleFitAmp;zdcModuleAmpHG_vs_injectedPulseInputVoltage_profile', type='TProfile', title=';Pulse amp [V];Signal Fit Amp [ADC Counts]',
                                    cutmask='zdcHGInjPulseValid',
                                    path='/EXPERT/ZDC/ZdcModule/ModuleAmpHGVsInputVoltage',
                                    xbins=create_vinj_bins())
        else: # ---------------------------- HG response max ADC (offline only) ----------------------------
            zdcModuleMonToolArr.defineHistogram('injectedPulseInputVoltage,zdcModuleMaxADCHG', type='TH2F', title=';Pulse amp [V];Max ADC HG',
                                    cutmask='zdcHGInjPulseValid',
                                    path='/EXPERT/ZDC/ZdcModule/ModuleMaxADCHGVsInputVoltage',
                                    xbins=create_vinj_bins(),
                                    ybins=create_hg_fit_amp_inj_bins())

        # ---------------------------- LG response ----------------------------
        zdcModuleMonToolArr.defineHistogram('injectedPulseInputVoltage,zdcModuleLGFitAmp;zdcModuleAmpLG_vs_injectedPulseInputVoltage', type='TH2F', title=';Pulse amp [V];Signal Fit Amp [ADC Counts]',
                                cutmask='zdcLGInjPulseValid',
                                path='/EXPERT/ZDC/ZdcModule/ModuleAmpLGVsInputVoltage',
                                xbins=create_vinj_bins(),
                                ybins=create_lg_fit_amp_inj_bins())

        if (zdcMonAlg.IsOnline): # also plot profile online in case 2D-hist scale is wrong by a large factor (e.g, due to attenuator setting)
            zdcModuleMonToolArr.defineHistogram('injectedPulseInputVoltage,zdcModuleLGFitAmp;zdcModuleAmpLG_vs_injectedPulseInputVoltage_profile', type='TProfile', title=';Pulse amp [V];Signal Fit Amp [ADC Counts]',
                                    cutmask='zdcLGInjPulseValid',
                                    path='/EXPERT/ZDC/ZdcModule/ModuleAmpLGVsInputVoltage',
                                    xbins=create_vinj_bins())
        else: # ---------------------------- LG response max ADC (offline only) ----------------------------
            zdcModuleMonToolArr.defineHistogram('injectedPulseInputVoltage,zdcModuleMaxADCLG;zdcModuleMaxADCLG_vs_injectedPulseInputVoltage', type='TH2F', title=';Pulse amp [V];Max ADC LG',
                                    cutmask='zdcLGInjPulseValid',
                                    path='/EXPERT/ZDC/ZdcModule/ModuleMaxADCLGVsInputVoltage',
                                    xbins=create_vinj_bins(),
                                    ybins=create_hg_fit_amp_inj_bins()) # maxADC has no LG gain factor multiplied

        # ---------------------------- HG & LG response binned by the voltage strings ----------------------------
        if (not zdcMonAlg.IsOnline):
            zdcModuleMonToolArr.defineHistogram('VoltageIndex, zdcModuleFitAmp', type='TH2F', title=';;Signal Fit Amp [ADC Counts]',
                            path='/EXPERT/ZDC/ZdcModule/ModuleAmpHGVsInputVoltageStr',
                            cutmask='zdcHGInjPulseValid',
                            xbins=len(voltage_strs_list),xmin=0.0,xmax=len(voltage_strs_list),
                            ybins=create_hg_fit_amp_inj_bins(),
                            xlabels=voltage_strs_list)

            zdcModuleMonToolArr.defineHistogram('VoltageIndex, zdcModuleLGFitAmp', type='TH2F', title=';;Signal Fit Amp [ADC Counts]',
                            path='/EXPERT/ZDC/ZdcModule/ModuleAmpLGVsInputVoltageStr',
                            cutmask='zdcLGInjPulseValid',
                            xbins=len(voltage_strs_list),xmin=0.0,xmax=len(voltage_strs_list),
                            ybins=create_lg_fit_amp_inj_bins(),
                            xlabels=voltage_strs_list)

        # ---------------------------- HG & LG response 1D histograms ----------------------------
        # ---------------------------- only offline ----------------------------

        if (not zdcMonAlg.IsOnline):
            zdcModuleSingleVoltageResponseArr = helper.addArray([sides,modules,voltage_strs_list],zdcMonAlg,'LucrodResponseSingleVoltageMonitor', topPath = 'ZDC/EXPERT/ZDC/ZdcModule/LucrodResponseSingleVoltage')
            zdcModuleSingleVoltageResponseArr.defineHistogram('zdcModuleFitAmp;zdcModuleAmpHG_fixed_vInj', type='TH1F', title=';Signal Fit Amp [ADC Counts];Events',
                                    cutmask='zdcHGInjPulseValid',
                                    xbins=create_hg_fit_amp_inj_bins())

            zdcModuleSingleVoltageResponseArr.defineHistogram('zdcModuleLGFitAmp;zdcModuleAmpLG_fixed_vInj', type='TH1F', title=';Signal Fit Amp [ADC Counts];Events',
                                    cutmask='zdcLGInjPulseValid',
                                    xbins=create_lg_fit_amp_inj_bins())

    # ---------------------------- ZDC-module times ---------------------------- 

    zdcModuleMonToolArr.defineHistogram('zdcModuleTime',title=';Module Time [ns];Events',
                            path='/SHIFT/ZDC/ZdcModule/ModuleTime',
                            opt='kAlwaysCreate', # always create for shift-needed histograms
                            xbins=n_time_centroid_bins_default,xmin=-10.0,xmax=10.0)

    if (not zdcMonAlg.IsPPMode): # for PP mode, LG never filled
        zdcModuleMonToolArr.defineHistogram('zdcModuleTime;zdcModuleTime_LG',title=';Module Time [ns];Events',
                                path='/SHIFT/ZDC/ZdcModule/ModuleTime',
                                opt='kAlwaysCreate', # always create for shift-needed histograms
                                cutmask='zdcModuleLG',
                                xbins=n_time_centroid_bins_default,xmin=-10.0,xmax=10.0)

    zdcModuleMonToolArr.defineHistogram('zdcModuleTime;zdcModuleTime_HG',title=';Module Time [ns];Events',
                            path='/SHIFT/ZDC/ZdcModule/ModuleTime',
                            opt='kAlwaysCreate', # always create for shift-needed histograms
                            cutmask='zdcModuleHG',
                            xbins=n_time_centroid_bins_default,xmin=-10.0,xmax=10.0)

    zdcModuleMonToolArr.defineHistogram('zdcModuleFitT0',title=';Module FitT0 [ns];Events',
                            path='/EXPERT/ZDC/ZdcModule/ModuleFitT0',
                            xbins=200,xmin=0.0,xmax=time_in_data_buffer)
    
    if (not zdcMonAlg.IsPPMode): # for PP mode, LG never filled
        zdcModuleMonToolArr.defineHistogram('zdcModuleFitT0;zdcModuleFitT0_LG',title=';Module FitT0 LG [ns];Events',
                                path='/EXPERT/ZDC/ZdcModule/ModuleFitT0',
                                cutmask='zdcModuleLG',                            
                                xbins=n_time_centroid_bins_default,xmin=0.0,xmax=time_in_data_buffer)

        zdcModuleMonToolArr.defineHistogram('zdcModuleFitT0;zdcModuleFitT0_HG',title=';Module FitT0 HG [ns];Events',
                                path='/EXPERT/ZDC/ZdcModule/ModuleFitT0',
                                cutmask='zdcModuleHG',
                                xbins=n_time_centroid_bins_default,xmin=0.0,xmax=time_in_data_buffer)

    
    if (not zdcMonAlg.IsOnline and not zdcMonAlg.IsInjectedPulse):
        zdcModuleMonToolArr.defineHistogram('zdcModuleCalibTime',title=';Module Calibrated Time [ns];Events',
                                path='/EXPERT/ZDC/ZdcModule/ModuleCalibTime',
                                xbins=n_time_centroid_bins_default,xmin=-10.0,xmax=10.0)
    
    # ---------------------------- ZDC-module pulse-fitting chi squares (pulse fitting goodness) ---------------------------- 

    zdcModuleMonToolArr.defineHistogram('zdcModuleChisq',title=';Module Chi-square;Events',
                            path='/EXPERT/ZDC/ZdcModule/ModuleChisq',
                            weight='zdcModuleChisqEventWeight',
                            xbins=create_log_bins(module_chisq_min, module_chisq_max, module_chisq_nbins))
    zdcModuleMonToolArr.defineHistogram('zdcModuleChisqOverAmp',title=';Module Chi-square / Amplitude;Events',
                            path='/SHIFT/ZDC/ZdcModule/ModuleChisq',
                            opt='kAlwaysCreate', # always create for shift-needed histograms
                            weight='zdcModuleChisqOverAmpEventWeight',
                            xbins=create_log_bins(module_chisq_over_amp_min, module_chisq_over_amp_max, module_chisq_over_amp_nbins))
    zdcModuleMonToolArr.defineHistogram('zdcModuleChisqOverAmp;zdcModuleChisqOverAmp_linear',title=';Module Chi-square / Amplitude;Events',
                            path='/SHIFT/ZDC/ZdcModule/ModuleChisq',
                            opt='kAlwaysCreate', # always create for shift-needed histograms
                            xbins=module_chisq_over_amp_linear_nbins,xmin=0.,xmax=module_chisq_over_amp_linear_max)

    if (not zdcMonAlg.IsOnline):
        zdcModuleMonToolArr.defineHistogram('zdcModuleAmp, zdcModuleChisqOverAmp',type='TH2F',title=';Module Amplitude [ADC Counts];Module Chi-square / Amplitude',
                                path='/EXPERT/ZDC/ZdcModule/ModuleChisq',
                                weight='zdcModuleChisqOverAmpEventWeight',
                                xbins=n_energy_bins_default,xmin=0.0,xmax=module_amp_xmax / 2.,
                                ybins=create_log_bins(module_chisq_over_amp_min, module_chisq_over_amp_max, module_chisq_over_amp_nbins))


    # ---------------------------- LG & HG comparisons ---------------------------- 
    zdcModuleMonToolArr.defineHistogram('zdcModuleHGtoLGAmpRatio',title=';HG-to-LG Amplitude Raio;Events',
                            path='/SHIFT/ZDC/ZdcModule/ModuleHGLGCompr',
                            opt='kAlwaysCreate', # always create for shift-needed histograms
                            cutmask='zdcModuleHG',
                            xbins=n_HG_LG_amp_ratio_bins,xmin=hg_lg_amp_ratio_min_nominal,xmax=hg_lg_amp_ratio_max_nominal)

    if (not zdcMonAlg.IsInjectedPulse):
        zdcModuleMonToolArr.defineHistogram('zdcModuleHGtoLGAmpRatioNoNonlinCorr',title=';HG-to-LG Amplitude Raio;Events',
                                path='/SHIFT/ZDC/ZdcModule/ModuleHGLGComprNoNonlinCorr',
                                opt='kAlwaysCreate', # always create for shift-needed histograms
                                cutmask='zdcModuleHG',
                                xbins=n_HG_LG_amp_ratio_bins,xmin=hg_lg_amp_ratio_min_nominal,xmax=hg_lg_amp_ratio_max_nominal)

    zdcModuleMonToolArr.defineHistogram('zdcModuleAmp, zdcModuleHGtoLGAmpRatio', type='TH2F', title=';ZDC HG Amplitude [ADC Counts];HG-to-LG Amplitude Raio;Events',
                            path='/EXPERT/ZDC/ZdcModule/ModuleHGLGCompr',
                            cutmask='zdcModuleHG',
                            xbins=n_module_amp_fine_bins, xmin=0.0, xmax=module_FPGA_max_ADC,
                            ybins=n_HG_LG_amp_ratio_bins,ymin=hg_lg_amp_ratio_min_nominal,ymax=hg_lg_amp_ratio_max_nominal)

    if (not zdcMonAlg.IsInjectedPulse):
        zdcModuleMonToolArr.defineHistogram('zdcModuleAmp, zdcModuleHGtoLGAmpRatioNoNonlinCorr', type='TH2F', title=';ZDC HG Amplitude [ADC Counts];HG-to-LG Amplitude Raio;Events',
                                path='/EXPERT/ZDC/ZdcModule/ModuleHGLGComprNoNonlinCorr',
                                cutmask='zdcModuleHG',
                                xbins=n_module_amp_fine_bins, xmin=0.0, xmax=module_FPGA_max_ADC,
                                ybins=n_HG_LG_amp_ratio_bins,ymin=hg_lg_amp_ratio_min_nominal,ymax=hg_lg_amp_ratio_max_nominal)

    zdcModuleMonToolArr.defineHistogram('zdcModuleAmp, zdcModuleHGtoLGAmpRatio;zdcModuleHGtoLGAmpRatio_vs_zdcModuleAmp_profile', type='TProfile', title=';ZDC HG Amplitude [ADC Counts];Average HG-to-LG Amplitude Raio;Events',
                            path='/EXPERT/ZDC/ZdcModule/ModuleHGLGCompr',
                            cutmask='zdcModuleHG',
                            xbins=n_module_amp_fine_bins, xmin=0.0, xmax=module_FPGA_max_ADC)

    if (not zdcMonAlg.IsOnline):
        zdcModuleMonToolArr.defineHistogram('zdcModuleAmpLGRefit, zdcModuleHGtoLGAmpRatio', type='TH2F', title=';ZDC LG-Refit Amplitude [ADC Counts];HG-to-LG Amplitude Raio;Events',
                                path='/EXPERT/ZDC/ZdcModule/ModuleHGLGCompr',
                                cutmask='zdcModuleHG',
                                xbins=n_module_amp_fine_bins, xmin=0.0, xmax=amp_LG_refit_max_ADC,
                                ybins=n_HG_LG_amp_ratio_bins,ymin=hg_lg_amp_ratio_min_nominal,ymax=hg_lg_amp_ratio_max_nominal)
        zdcModuleMonToolArr.defineHistogram('zdcModuleAmpLGRefit, zdcModuleHGtoLGAmpRatio;zdcModuleHGtoLGAmpRatio_vs_zdcModuleAmpLGRefit_profile', type='TProfile', title=';ZDC LG-Refit Amplitude [ADC Counts];Average HG-to-LG Amplitude Raio;Events',
                                path='/EXPERT/ZDC/ZdcModule/ModuleHGLGCompr',
                                cutmask='zdcModuleHG',
                                xbins=n_module_amp_fine_bins, xmin=0.0, xmax=amp_LG_refit_max_ADC)

        zdcModuleMonToolArr.defineHistogram('zdcModuleHGtoLGT0Diff',title=';HG-LG T0 Difference [ns];Events',
                                path='/EXPERT/ZDC/ZdcModule/ModuleHGLGCompr',
                                cutmask='zdcModuleHG',
                                xbins=n_HG_LG_time_diff_bins,xmin=-10.0,xmax=10.0)

        zdcModuleMonToolArr.defineHistogram('zdcModuleHGtoLGT0Diff, zdcModuleHGtoLGAmpRatio', type='TH2F', title=';HG-LG T0 Difference [ns];HG-to-LG Amplitude Raio;Events',
                                path='/EXPERT/ZDC/ZdcModule/ModuleHGLGCompr',
                                cutmask='zdcModuleHG',
                                xbins=n_HG_LG_time_diff_bins,xmin=2.0,xmax=4.0, # zoomed in to see potential correlations
                                ybins=n_HG_LG_amp_ratio_bins,ymin=hg_lg_amp_ratio_min_tight,ymax=hg_lg_amp_ratio_max_tight)


    # ---------------------------- LB and BCID-dep ZDC-module-level observables ---------------------------- 
    # ---------------------------- ZDC-module amplitudes ---------------------------- 

    if (not zdcMonAlg.IsInjectedPulse):
        zdcModuleMonToolArr.defineHistogram('lumiBlock, zdcModuleCalibAmp;zdcModuleCalibAmp_vs_lb', type='TH2F', title=';lumi block;Module Calib Amplitude',
                                path='/EXPERT/ZDC/ZdcModule/ModuleCalibAmpLBdep',
                                xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                                ybins=n_module_amp_coarse_bins, ymin=0.0, ymax=module_calib_amp_1Nmonitor_xmax)
    if (not zdcMonAlg.IsOnline):
        zdcModuleMonToolArr.defineHistogram('lumiBlock, zdcModuleAmp;zdcModuleAmp_vs_lb', type='TH2F', title=';lumi block;Module Amplitude [ADC counts]',
                                path='/EXPERT/ZDC/ZdcModule/ModuleAmpLBdep',
                                xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                                ybins=n_module_amp_coarse_bins, ymin=0.0, ymax=module_amp_1Nmonitor_xmax)
        zdcModuleMonToolArr.defineHistogram('lumiBlock, zdcModuleFract;zdcModuleFract_above20N_vs_lb', type='TH2F',title=';lumi block;Module Amplitude Fraction',
                                path='/EXPERT/ZDC/ZdcModule/ModuleFractionLBdep',
                                cutmask='zdcAbove20NCurrentSide',
                                xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                                ybins=n_mod_fraction_bins_default,ymin=0.0,ymax=1.)

    if (not zdcMonAlg.IsOnline and not zdcMonAlg.IsInjectedPulse):
        zdcModuleMonToolArr.defineHistogram('bcid, zdcModuleCalibAmp', type='TH2F', title=';BCID;Module Calib Amplitude',
                                path='/EXPERT/ZDC/ZdcModule/ModuleCalibAmpBCIDdep',
                                xbins=bcid_max,xmin=0.0,xmax=bcid_max,
                                ybins=n_module_amp_coarse_bins, ymin=0.0, ymax=module_calib_amp_1Nmonitor_xmax)

    # ---------------------------- ZDC-module times ---------------------------- 
    if (not zdcMonAlg.IsOnline): #offline - fine binnings
        zdcModuleMonToolArr.defineHistogram('lumiBlock, zdcModuleTime;zdcModuleTime_vs_lb', type='TH2F', title=';lumi block;Module Time [ns]',
                                path='/EXPERT/ZDC/ZdcModule/ModuleTimeLBdep',
                                xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                                ybins=n_time_centroid_bins_default, ymin=-10.0, ymax=10.0)
        zdcModuleMonToolArr.defineHistogram('lumiBlock, zdcModuleTime;zdcModuleTime_LG_vs_lb', type='TH2F', title=';lumi block;Module Time [ns]',
                                path='/EXPERT/ZDC/ZdcModule/ModuleTimeLBdep',
                                cutmask='zdcModuleLG',
                                xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                                ybins=n_time_centroid_bins_default, ymin=-10.0, ymax=10.0)
        zdcModuleMonToolArr.defineHistogram('lumiBlock, zdcModuleTime;zdcModuleTime_HG_vs_lb', type='TH2F', title=';lumi block;Module Time [ns]',
                                path='/EXPERT/ZDC/ZdcModule/ModuleTimeLBdep',
                                cutmask='zdcModuleHG',
                                xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                                ybins=n_time_centroid_bins_default, ymin=-10.0, ymax=10.0)
        zdcModuleMonToolArr.defineHistogram('lumiBlock, zdcModuleCalibTime;zdcModuleCalibTime_LG_vs_lb', type='TH2F', title=';lumi block;Module Time [ns]',
                                path='/EXPERT/ZDC/ZdcModule/ModuleTimeLBdep',
                                cutmask='zdcModuleLG',
                                xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                                ybins=n_time_centroid_bins_default, ymin=-10.0, ymax=10.0)
        zdcModuleMonToolArr.defineHistogram('lumiBlock, zdcModuleCalibTime;zdcModuleCalibTime_HG_vs_lb', type='TH2F', title=';lumi block;Module Time [ns]',
                                path='/EXPERT/ZDC/ZdcModule/ModuleTimeLBdep',
                                cutmask='zdcModuleHG',
                                xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                                ybins=n_time_centroid_bins_default, ymin=-10.0, ymax=10.0)
    else: #online - coarse binnings
        zdcModuleMonToolArr.defineHistogram('lumiBlock, zdcModuleTime;zdcModuleTime_LG_vs_lb', type='TH2F', title=';lumi block;Module Time [ns]',
                                path='/EXPERT/ZDC/ZdcModule/ModuleTimeLBdep',
                                cutmask='zdcModuleLG',
                                xbins=n_lumi_block_bins_coarse,xmin=0.0,xmax=lumi_block_max,
                                ybins=n_time_centroid_bins_default, ymin=-10.0, ymax=10.0)
        zdcModuleMonToolArr.defineHistogram('lumiBlock, zdcModuleTime;zdcModuleTime_HG_vs_lb', type='TH2F', title=';lumi block;Module Time [ns]',
                                path='/EXPERT/ZDC/ZdcModule/ModuleTimeLBdep',
                                cutmask='zdcModuleHG',
                                xbins=n_lumi_block_bins_coarse,xmin=0.0,xmax=lumi_block_max,
                                ybins=n_time_centroid_bins_default, ymin=-10.0, ymax=10.0)

    zdcModuleMonToolArr.defineHistogram('lumiBlock, zdcModuleTime;zdcModuleTime_LG_vs_lb_profile', type='TProfile', title=';lumi block;Module Time [ns]',
                            path='/EXPERT/ZDC/ZdcModule/ModuleTimeLBdep',
                            cutmask='zdcModuleLGTimeValid',
                            xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max)
    zdcModuleMonToolArr.defineHistogram('lumiBlock, zdcModuleTime;zdcModuleTime_HG_vs_lb_profile', type='TProfile', title=';lumi block;Module Time [ns]',
                            path='/EXPERT/ZDC/ZdcModule/ModuleTimeLBdep',
                            cutmask='zdcModuleHGTimeValid',
                            xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max)

# --------------------------------------------------------------------------------------------------
# ------------------------------------ RPD-channel observables ------------------------------------- 
# --------------------------------------------------------------------------------------------------

    if (zdcMonAlg.EnableRPDAmp):

        rpdChannelMonToolArr = helper.addArray([sides,channels],zdcMonAlg,'RpdChannelMonitor', topPath = 'ZDC')

    # ---------------------------- amplitudes ---------------------------- 
        rpdChannelMonToolArr.defineHistogram('RPDChannelAmplitudeCalib', title=';RPD Channel Calibrated Amplitude;Events',
                                path='/SHIFT/RPD/RPDChannel/CalibAmp',
                                opt='kAlwaysCreate', # always create for shift-needed histograms
                                cutmask='RPDChannelValid',
                                xbins=n_rpd_amp_bins_full_range,xmin=rpd_channel_amp_min,xmax=rpd_sum_adc_max) # NOT energy calibration - calibration factor is 1 for now
        rpdChannelMonToolArr.defineHistogram('RPDChannelMaxADC', title=';Max ADC [ADC Counts];Events',
                                path='/EXPERT/RPD/RPDChannel/MaxADC',
                                cutmask='RPDChannelValid',
                                xbins=n_energy_bins_default,xmin=0.0,xmax=module_FPGA_max_ADC)

        if (not zdcMonAlg.IsOnline):
            rpdChannelMonToolArr.defineHistogram('RPDChannelSubAmp', title=';RPD Channel Subtracted Amplitude;Events',
                                    path='/EXPERT/RPD/RPDChannel/SubAmp',
                                    cutmask='RPDChannelCentroidValid',
                                    xbins=n_rpd_sub_amp_bins,xmin=rpd_sub_amp_min,xmax=rpd_sub_amp_max) # NOT energy calibration - calibration factor is 1 for now

        # max ADC versus sum ADC
        if (not zdcMonAlg.IsOnline):
            rpdChannelMonToolArr.defineHistogram('RPDChannelAmplitudeCalib,RPDChannelMaxADC', type='TH2F', title=';Sum ADC [ADC Counts];Max ADC [ADC Counts]',
                                    path='/EXPERT/RPD/RPDChannel/MaxADCVsSumADC',
                                    cutmask='RPDChannelValid',
                                    xbins=n_rpd_amp_bins_full_range,xmin=rpd_channel_amp_min,xmax=rpd_sum_adc_max, #change to xmax=20000 for zoomed in
                                    ybins=n_energy_bins_default,ymin=0.0,ymax=module_FPGA_max_ADC) # change to ymax=3000.0 for zoomed in
    # ---------------------------- timing (max sample) ---------------------------- 
        rpdChannelMonToolArr.defineHistogram('RPDChannelMaxSample', title=';Max Sample;Events',
                                path='/EXPERT/RPD/RPDChannel/MaxSample',
                                cutmask='RPDChannelValid',
                                xbins=24,xmin=0.0,xmax=24.)


    # ---------------------------- status bits ---------------------------- 
        rpdChannelMonToolArr.defineHistogram('RPDStatusBits',title=';;Events',
                                path='/EXPERT/RPD/RPDChannel/StatusBits',
                                xbins=nRpdStatusBits,xmin=0,xmax=nRpdStatusBits,
                                opt='kVec',
                                xlabels=['ValidBit', 'OutOfTimePileupBit', 'OverflowBit', 'PrePulseBit', 'PostPulseBit', 'NoPulseBit', 'BadAvgBaselineSubtrBit', 'InsufficientPileupFitPointsBit', 'PileupStretchedExpFitFailBit', 'PileupStretchedExpGrowthBit', 'PileupBadStretchedExpSubtrBit', 'PileupExpFitFailBit', 'PileupExpGrowthBit', 'PileupBadExpSubtrBit', 'PileupStretchedExpPulseLike'])

        rpdChannelMonToolArr.defineHistogram('RPDChannelValidBitFloat;RPDChannelValidBit',title='RPD Channel valid bit;;Events',
                                path='/SHIFT/RPD/RPDChannel/StatusBits',
                                opt='kAlwaysCreate', # always create for shift-needed histograms
                                xbins=2,xmin=0,xmax=2,
                                xlabels=['Valid','Invalid'])

    # ---------------------------- LB dependence ---------------------------- 
        if (not zdcMonAlg.IsOnline): #offline
            rpdChannelMonToolArr.defineHistogram('lumiBlock, RPDChannelAmplitudeCalib;RPDChannelAmplitudeCalib_vs_lb', type='TH2F', title=';lumi block;RPD Channel Calibrated Amplitude',
                                    path='/EXPERT/RPD/RPDChannel/CalibAmpLBdep',
                                    cutmask='RPDChannelValid',
                                    xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                                    ybins=n_rpd_amp_bins_full_range,ymin=rpd_channel_amp_min,ymax=rpd_sum_adc_max) # NOT energy calibration - calibration factor is 1 for now
            rpdChannelMonToolArr.defineHistogram('lumiBlock, RPDChannelMaxADC;RPDChannelMaxADC_vs_lb', type='TH2F', title=';lumi block;Max ADC [ADC Counts]',
                                    path='/EXPERT/RPD/RPDChannel/MaxADCLBdep',
                                    cutmask='RPDChannelValid',
                                    xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                                    ybins=n_energy_bins_default,ymin=0.0,ymax=module_FPGA_max_ADC)
        else: #online
            rpdChannelMonToolArr.defineHistogram('lumiBlock, RPDChannelAmplitudeCalib;RPDChannelAmplitudeCalib_vs_lb', type='TH2F', title=';lumi block;RPD Channel Calibrated Amplitude',
                                    path='/EXPERT/RPD/RPDChannel/CalibAmpLBdep',
                                    cutmask='RPDChannelValid',
                                    xbins=n_lumi_block_bins_coarse,xmin=0.0,xmax=lumi_block_max,
                                    ybins=n_rpd_amp_bins_full_range,ymin=rpd_channel_amp_min,ymax=rpd_sum_adc_max) # NOT energy calibration - calibration factor is 1 for now
            rpdChannelMonToolArr.defineHistogram('lumiBlock, RPDChannelMaxADC;RPDChannelMaxADC_vs_lb', type='TH2F', title=';lumi block;Max ADC [ADC Counts]',
                                    path='/EXPERT/RPD/RPDChannel/MaxADCLBdep',
                                    cutmask='RPDChannelValid',
                                    xbins=n_lumi_block_bins_coarse,xmin=0.0,xmax=lumi_block_max,
                                    ybins=n_energy_bins_default,ymin=0.0,ymax=module_FPGA_max_ADC)


    ### STEP 6 ###
    # Finalize. The return value should be a tuple of the ComponentAccumulator
    # and the sequence containing the created algorithms. If we haven't called
    # any configuration other than the AthMonitorCfgHelper here, then we can 
    # just return directly (and not create "result" above)
    return helper.result()
    
    # # Otherwise, merge with result object and return
    # acc = helper.result()
    # result.merge(acc)
    # return result

if __name__=='__main__':
    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import WARNING
    log.setLevel(WARNING)

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    directory = ''
    inputfile = 'AOD.pool.root'
    flags.Input.Files = [directory+inputfile]
    # flags.Input.isMC = False
    parser = flags.getArgumentParser()
    parser.add_argument('--datasetTag',default="HI2023",help="dataset tag")
    parser.add_argument('--runNumber',default=None,help="specify to select a run number")
    parser.add_argument('--streamTag',default="ZDCCalib",help="ZDCCalib or MinBias")
    parser.add_argument('--outputHISTFile',default=None,help="specify output HIST file name")
    args = flags.fillFromArgs(parser=parser)

    flags.DQ.useTrigger = False if flags.Input.isMC else True # isMC is autoconfigured from the input file; if MC: turn trigger off
    if args.runNumber is not None: # streamTag has default but runNumber doesn't
        flags.Output.HISTFileName = f'ZdcMonitorOutput_{args.datasetTag}_{args.streamTag}_{args.runNumber}.root'
    else:
        flags.Output.HISTFileName = f'ZdcMonitorOutput_{args.datasetTag}_{args.streamTag}.root'    
    
    if args.outputHISTFile is not None: # overwrite the output HIST file name to be match the name set in the grid job
        flags.Output.HISTFileName = f'{args.outputHISTFile}'
    flags.lock()

    print('Output', flags.Output.HISTFileName)
    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    zdcMonitorAcc = ZdcMonitoringConfig(flags)
    cfg.merge(zdcMonitorAcc)

    # If you want to turn on more detailed messages ...
    # zdcMonitorAcc.getEventAlgo('ZdcMonAlg').OutputLevel = 2 # DEBUG
    # If you want fewer messages ...
    # zdcMonitorAcc.getEventAlgo('ZdcMonAlg').OutputLevel = 4 # WARNING
    cfg.printConfig(withDetails=False) # set True for exhaustive info

    cfg.run() #use cfg.run(20) to only run on first 20 events
