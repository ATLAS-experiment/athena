#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
def OverviewMonitoringConfig(inputFlags):
    '''Function to configure LVL1 Overview algorithm in the monitoring system.'''

    from AthenaConfiguration.Enums import Format

    # get the component factory - used for getting the algorithms
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()

    # use L1Calo's special MonitoringCfgHelper
    from TrigT1CaloMonitoring.LVL1CaloMonitoringConfig import L1CaloMonitorCfgHelper
    helper = L1CaloMonitorCfgHelper(inputFlags,CompFactory.OverviewMonitorAlgorithm,'OverviewMonAlg')

    # get any algorithms
    OverviewMonAlg = helper.alg

    # add any steering
    groupName = 'OverviewMonitor' # the monitoring group name is also used for the package name
    OverviewMonAlg.PackageName = groupName

    # flag for online - different duration options required
    isOnline=inputFlags.Trigger.Online.isPartition and inputFlags.Input.Format is Format.BS

    # histogram path
    histPath_exp = 'Expert/PpmTrex/Overview/'
    histPath_det = 'Expert/PpmTrex/Overview/detail'
    
    # number of processed events
    global_labels = ["Processed Events"]
    helper.defineHistogram('n_processed;l1calo_1d_NumberOfEvents',
                           fillGroup=groupName,
                           title='Number of processed events',                            
                           type='TH1I',
                           path=histPath_det,
                           hanConfig={
                               "description": "Number of processed events."
                           },
                           xbins=1,xmin=0,xmax=1, xlabels=global_labels)


    # global overview
    NumberOfGlobalErrors=15
    globalStatus_xlabels = [
        "PPMDataStatus",
        "PPMDataError",
        "SubStatus",
        "Parity",
        "LinkDown",
        "Transmission",
        "Simulation",
        "CMXSubStatus",
        "CMXParity",
        "CMXTransmission",
        "CMXSimulation",
        "RODStatus",
        "RODMissing",
        "ROBStatus",
        "Unpacking"]

    globalStatus_ylabels = []
    for crate in range(14):
        cr = crate
        if cr >= 12:
            cr -= 12
        if cr >= 8:
            cr -= 8
        type = f"PP{cr} " if (crate < 8) else f"CP{cr} " if (crate < 12) else f"JEP{cr} "
        globalStatus_ylabels.append(type)
    
    helper.defineHistogram('globalOverviewX,globalOverviewY;l1calo_2d_GlobalOverview',
                           title='L1Calo Global Error Overview;;',
                           fillGroup=groupName,
                           type='TH2I',
                           path=histPath_exp,
                           hanConfig={
                               "algorithm" : "Histogram_Empty",
                               "display" : "SetPalette(1),SetGridx,SetGridy",
                               "description" : "L1Calo Global Error Overview."
                           },
                           xbins=NumberOfGlobalErrors,xmin=0.,xmax=NumberOfGlobalErrors,
                           ybins=14,ymin=0.,ymax=14,
                           xlabels=globalStatus_xlabels, ylabels=globalStatus_ylabels,
                           opt='kAlwaysCreate')

    if isOnline:
        helper.defineHistogram('globalOverviewX,globalOverviewY;l1calo_2d_GlobalOverviewRecent',
                               title='L1Calo Global Error Overview Last 10 LumiBlocks;;',
                               fillGroup=groupName,
                               type='TH2I',
                               path=histPath_exp,
                               hanConfig={
                                   "algorithm" : "Histogram_Empty",
                                   "display" : "SetPalette(1),SetGridx,SetGridy",
                                   "description": "L1Calo Global Error Overview Last 10 LumiBlocks."
                               },
                               xbins=NumberOfGlobalErrors,xmin=0.,xmax=NumberOfGlobalErrors,
                               ybins=14,ymin=0.,ymax=14,
                               xlabels=globalStatus_xlabels, ylabels=globalStatus_ylabels,
                               opt='kLBNHistoryDepth=10,kAlwaysCreate')


    helper.defineHistogram('lb_errors;l1calo_1d_ErrorsByLumiblock',
                           title='Events with Errors by Lumiblock;Lumi Block;Number of Events;',
                           fillGroup=groupName,
                           type='TH1I',
                           path=histPath_det,
                           hanConfig={
                               "description": "Events with Errors by Lumiblock."
                           },
                           xbins=3000,xmin=0,xmax=3000,
                           weight='n_lb_errors',
                           opt='kAlwaysCreate')
    
    acc = helper.result()
    result.merge(acc)
    return result


if __name__=='__main__':
    # set input file and config options
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    import glob

    #inputs = glob.glob('/eos/atlas/atlastier0/rucio/data18_13TeV/physics_Main/00357750/data18_13TeV.00357750.physics_Main.recon.ESD.f1072/data18_13TeV.00357750.physics_Main.recon.ESD.f1072._lb0117._SFO-1._0201.1')
    inputs = glob.glob('/eos/atlas/atlastier0/rucio/data18_13TeV/physics_Main/00354311/data18_13TeV.00354311.physics_Main.recon.ESD.f1129/data18_13TeV.00354311.physics_Main.recon.ESD.f1129._lb0013._SFO-8._0001.1')

    flags = initConfigFlags()
    flags.Input.Files = inputs
    flags.Output.HISTFileName = 'ExampleMonitorOutput_LVL1.root'

    flags.lock()
    flags.dump() # print all the configs

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    OverviewMonitorCfg = OverviewMonitoringConfig(flags)
    cfg.merge(OverviewMonitorCfg)

    # message level for algorithm
    OverviewMonitorCfg.getEventAlgo('OverviewMonAlg').OutputLevel = 2 # 1/2 INFO/DEBUG
    # options - print all details of algorithms, very short summary 
    cfg.printConfig(withDetails=False, summariseProps = True)

    nevents=-1
    cfg.run(nevents)
