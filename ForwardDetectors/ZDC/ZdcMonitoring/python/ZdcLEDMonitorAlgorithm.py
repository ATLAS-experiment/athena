#!/usr/bin/env python
#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

'''@file ZdcLEDMonitorAlgorithm.py
@author Y. Guo
@author S. Mohapatra
@date 2023-08-01
@brief python configuration for ZDC LED monitoring under the Run III DQ framework
       will be run in the ZDC LED calibration stream
       see ExampleMonitorAlgorithm.py in AthenaMonitoring package for detailed step explanations
'''
def ZdcLEDMonitoringConfig(inputFlags, run_type):

#---------------------------------------------------------------------------------------
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(inputFlags,'ZdcAthMonitorCfg')

    from AthenaConfiguration.ComponentFactory import CompFactory
    zdcLEDMonAlg = helper.addAlgorithm(CompFactory.ZdcLEDMonitorAlgorithm,'ZdcLEDMonAlg')

# --------------------------------------------------------------------------------------------------

    nLEDs = 3
    nSides = 2
    nModules = 4
    nChannels = 16

    n_energy_bins_default = 200
    lumi_block_max = 2000
    bcid_max = 3564
    adc_sum_max = 8192.0
    max_adc_max = 4096.0

# --------------------------------------------------------------------------------------------------

    zdcModLEDMonToolArr = helper.addArray([nLEDs,nSides,nModules],zdcLEDMonAlg,'ZdcModLEDMonitor', topPath='ZDC/ZDCLED/')
    rpdChanLEDMonToolArr = helper.addArray([nLEDs,nSides,nChannels],zdcLEDMonAlg,'RPDChanLEDMonitor', topPath='ZDC/RPDLED/')

 
    zdcModLEDMonToolArr.defineHistogram('zdcLEDADCSum', title='LED ADC Sum [ADC Counts];Events',
                            path='zdcLEDADCSum',
                            xbins=n_energy_bins_default,xmin=0.0,xmax=adc_sum_max)
    zdcModLEDMonToolArr.defineHistogram('zdcLEDMaxADC', title='LED Max ADC [ADC Counts];Events',
                            path='zdcLEDMaxADC',
                            xbins=n_energy_bins_default,xmin=0.0,xmax=max_adc_max)
    zdcModLEDMonToolArr.defineHistogram('zdcLEDMaxSample', title='LED Max Sample [ADC Counts];Events',
                            path='zdcLEDMaxSample',
                            xbins=25,xmin=0.0,xmax=25)
    zdcModLEDMonToolArr.defineHistogram('zdcLEDAvgTime', title='LED Average Time [ns];Events',
                            path='zdcLEDAvgTime',
                            xbins=150,xmin=0.0,xmax=75.)
    rpdChanLEDMonToolArr.defineHistogram('rpdLEDADCSum', title='LED ADC Sum [ADC Counts];Events',
                            path='rpdLEDADCSum',
                            xbins=n_energy_bins_default,xmin=0.0,xmax=adc_sum_max)
    rpdChanLEDMonToolArr.defineHistogram('rpdLEDMaxADC', title='LED Max ADC [ADC Counts];Events',
                            path='rpdLEDMaxADC',
                            xbins=n_energy_bins_default,xmin=0.0,xmax=max_adc_max)
    rpdChanLEDMonToolArr.defineHistogram('rpdLEDMaxSample', title='LED Max Sample [ADC Counts];Events',
                            path='rpdLEDMaxSample',
                            xbins=n_energy_bins_default,xmin=0.0,xmax=25)
    rpdChanLEDMonToolArr.defineHistogram('rpdLEDAvgTime', title='LED Average Time [ns];Events',
                            path='rpdLEDAvgTime',
                            xbins=n_energy_bins_default,xmin=0.0,xmax=75.)

# -------------------------------------------- lumi block dependence ------------------------------------------------------

    zdcModLEDMonToolArr.defineHistogram('lumiBlock, zdcLEDADCSum;zdcLEDADCSum_vs_lb', type='TH2F', title=';lumi block;LED ADC Sum [ADC Counts]',
                            path='zdcLEDADCSumLBdep',
                            xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                            ybins=n_energy_bins_default,ymin=0.0,ymax=adc_sum_max)
    zdcModLEDMonToolArr.defineHistogram('lumiBlock, zdcLEDMaxADC;zdcLEDMaxADC_vs_lb', type='TH2F', title=';lumi block;LED Max ADC [ADC Counts]',
                            path='zdcLEDMaxADCLBdep',
                            xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                            ybins=n_energy_bins_default,ymin=0.0,ymax=max_adc_max)
    zdcModLEDMonToolArr.defineHistogram('lumiBlock, zdcLEDMaxSample;zdcLEDMaxSample_vs_lb', type='TH2F', title=';lumi block;LED Max Sample [ADC Counts]',
                            path='zdcLEDMaxSampleLBdep',
                            xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                            ybins=25,ymin=0.0,ymax=25)
    zdcModLEDMonToolArr.defineHistogram('lumiBlock, zdcLEDAvgTime;zdcLEDAvgTime_vs_lb', type='TH2F', title=';lumi block;LED Average Time [ns]',
                            path='zdcLEDAvgTimeLBdep',
                            xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                            ybins=150,ymin=0.0,ymax=75.)
    rpdChanLEDMonToolArr.defineHistogram('lumiBlock, rpdLEDADCSum;rpdLEDADCSum_vs_lb', type='TH2F', title=';lumi block;LED ADC Sum [ADC Counts]',
                            path='rpdLEDADCSumLBdep',
                            xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                            ybins=n_energy_bins_default,ymin=0.0,ymax=adc_sum_max)
    rpdChanLEDMonToolArr.defineHistogram('lumiBlock, rpdLEDMaxADC;rpdLEDMaxADC_vs_lb', type='TH2F', title=';lumi block;LED Max ADC [ADC Counts]',
                            path='rpdLEDMaxADCLBdep',
                            xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                            ybins=n_energy_bins_default,ymin=0.0,ymax=max_adc_max)
    rpdChanLEDMonToolArr.defineHistogram('lumiBlock, rpdLEDMaxSample;rpdLEDMaxSample_vs_lb', type='TH2F', title=';lumi block;LED Max Sample [ADC Counts]',
                            path='rpdLEDMaxSampleLBdep',
                            xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                            ybins=n_energy_bins_default,ymin=0.0,ymax=25)
    rpdChanLEDMonToolArr.defineHistogram('lumiBlock, rpdLEDAvgTime;rpdLEDAvgTime_vs_lb', type='TH2F', title=';lumi block;LED Average Time [ns]',
                            path='rpdLEDAvgTimeLBdep',
                            xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                            ybins=n_energy_bins_default,ymin=0.0,ymax=75.)


# -------------------------------------------- BCID dependence ------------------------------------------------------

    zdcModLEDMonToolArr.defineHistogram('bcid, zdcLEDADCSum', type='TH2F', title=';BCID;LED ADC Sum [ADC Counts]',
                            path='zdcLEDADCSumBCIDdep',
                            xbins=bcid_max,xmin=0.0,xmax=bcid_max,
                            ybins=n_energy_bins_default,ymin=0.0,ymax=adc_sum_max)
    zdcModLEDMonToolArr.defineHistogram('bcid, zdcLEDMaxADC', type='TH2F', title=';BCID;LED Max ADC [ADC Counts]',
                            path='zdcLEDMaxADCBCIDdep',
                            xbins=bcid_max,xmin=0.0,xmax=bcid_max,
                            ybins=n_energy_bins_default,ymin=0.0,ymax=max_adc_max)
    zdcModLEDMonToolArr.defineHistogram('bcid, zdcLEDMaxSample', type='TH2F', title=';BCID;LED Max Sample [ADC Counts]',
                            path='zdcLEDMaxSampleBCIDdep',
                            xbins=bcid_max,xmin=0.0,xmax=bcid_max,
                            ybins=25,ymin=0.0,ymax=25)
    zdcModLEDMonToolArr.defineHistogram('bcid, zdcLEDAvgTime', type='TH2F', title=';BCID;LED Average Time [ns]',
                            path='zdcLEDAvgTimeBCIDdep',
                            xbins=bcid_max,xmin=0.0,xmax=bcid_max,
                            ybins=150,ymin=0.0,ymax=75.)
    rpdChanLEDMonToolArr.defineHistogram('bcid, rpdLEDADCSum', type='TH2F', title=';BCID;LED ADC Sum [ADC Counts]',
                            path='rpdLEDADCSumBCIDdep',
                            xbins=bcid_max,xmin=0.0,xmax=bcid_max,
                            ybins=n_energy_bins_default,ymin=0.0,ymax=adc_sum_max)
    rpdChanLEDMonToolArr.defineHistogram('bcid, rpdLEDMaxADC', type='TH2F', title=';BCID;LED Max ADC [ADC Counts]',
                            path='rpdLEDMaxADCBCIDdep',
                            xbins=bcid_max,xmin=0.0,xmax=bcid_max,
                            ybins=n_energy_bins_default,ymin=0.0,ymax=max_adc_max)
    rpdChanLEDMonToolArr.defineHistogram('bcid, rpdLEDMaxSample', type='TH2F', title=';BCID;LED Max Sample [ADC Counts]',
                            path='rpdLEDMaxSampleBCIDdep',
                            xbins=bcid_max,xmin=0.0,xmax=bcid_max,
                            ybins=n_energy_bins_default,ymin=0.0,ymax=25)
    rpdChanLEDMonToolArr.defineHistogram('bcid, rpdLEDAvgTime', type='TH2F', title=';BCID;LED Average Time [ns]',
                            path='rpdLEDAvgTimeBCIDdep',
                            xbins=bcid_max,xmin=0.0,xmax=bcid_max,
                            ybins=n_energy_bins_default,ymin=0.0,ymax=75.)


    return helper.result()
    

if __name__=='__main__':
    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import WARNING
    log.setLevel(WARNING)

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    directory = ''
    inputfile = 'myLEDAOD.pool.root'
    flags.Input.Files = [directory+inputfile]
    flags.Input.isMC = False
    flags.Output.HISTFileName = 'ZdcLEDMonitorOutput_2018PbPb.root'
    parser = flags.getArgumentParser()
    parser.add_argument('--runNumber',default=None,help="specify to select a run number")
    parser.add_argument('--outputHISTFile',default=None,help="specify output HIST file name")
    args = flags.fillFromArgs(parser=parser)
    if args.runNumber is not None: # streamTag has default but runNumber doesn't
        flags.Output.HISTFileName = f'ZdcLEDMonitorOutput_HI2023_{args.runNumber}.root'
    else:
        flags.Output.HISTFileName = 'ZdcLEDMonitorOutput_HI2023.root'    

    if args.outputHISTFile is not None: # overwrite the output HIST file name to be match the name set in the grid job
        flags.Output.HISTFileName = f'{args.outputHISTFile}'
    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    run_type = "pbpb"

    zdcLEDMonitorAcc = ZdcLEDMonitoringConfig(flags, run_type)
    cfg.merge(zdcLEDMonitorAcc)

    # If you want to turn on more detailed messages ...
    zdcLEDMonitorAcc.getEventAlgo('ZdcLEDMonAlg').OutputLevel = 2 # DEBUG
    # If you want fewer messages ...
   
    cfg.printConfig(withDetails=False) # set True for exhaustive info

    cfg.run(20) #use cfg.run(20) to only run on first 20 events

