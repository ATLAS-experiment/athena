#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

'''
@file TileDigitsFlxMonitorAlgorithm.py
@brief Python configuration of TileDigitsFlxMonitorAlgorithm algorithm for the Run III
'''

def TileRawChannelFlxMonitoringConfig(flags, fragIDs=[0x201, 0x402], topPath='TestBeam', **kwargs):
    '''Function to configures TileRawChannelFlxMonAlg algorithms in the monitoring system.'''

    kwargs.setdefault('TileRawChannelContainerLegacy', 'TileRawChannelFit')
    kwargs.setdefault('TileRawChannelContainerFlx', 'TileRawChannelFlxFit')
    kwargs.setdefault('FelixScale', 4)

    felixScale = kwargs['FelixScale']
    
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    result.merge( TileCablingSvcCfg(flags) )

    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags, 'TileRawChannelFlxMonitoring')

    from AthenaConfiguration.ComponentFactory import CompFactory
    TileRawChannelFlxMonitorAlgorithm = CompFactory.TileRawChannelFlxMonitorAlgorithm
    tileRawChannelFlxMonAlg = helper.addAlgorithm(TileRawChannelFlxMonitorAlgorithm, 'TileRawChannelFlxMonAlg')
    tileRawChannelFlxMonAlg.TriggerChain = ''

    for k, v in kwargs.items():
        setattr(tileRawChannelFlxMonAlg, k, v)

    # Configure histogram with TileRawChannelFlxMonAlg algorithm execution time
    executeTimeGroup = helper.addGroup(tileRawChannelFlxMonAlg, 'TileRawChannelFlxMonExecuteTime', f'{topPath}/Felix')
    executeTimeGroup.defineHistogram('TIME_execute', path = 'RawChannel', type='TH1F',
                                     title = 'Time for execute TileRawChannelFlxMonAlg algorithm;time [#mus]',
                                     xbins = 300, xmin = 0, xmax = 300000)

    runNumber = flags.Input.RunNumbers[0]
    from TileCalibBlobObjs.Classes import TileCalibUtils as Tile

    modules = []
    if fragIDs:
        for fragID in fragIDs:
            ros = fragID >> 8
            drawer = fragID & 0x3F
            modules += [Tile.getDrawerString(ros, drawer)]
    else:
        for ros in range(1, Tile.MAX_ROS):
            for drawer in range(0, Tile.MAX_DRAWER):
                modules += [Tile.getDrawerString(ros, drawer)]

    channelLegacyGroup = helper.addGroup(tileRawChannelFlxMonAlg, 'TileRawChannelAmpLegacy', f'{topPath}/Legacy/RawChannel')
    for moduleName in modules:
        for gainName in ['HG', 'LG']:
            title = f'Run {str(runNumber)} {moduleName} {gainName}: Amplitude (Legacy);Channel;Amplitude'
            name = f'{moduleName}_{gainName}_channel,{moduleName}_{gainName}_amplitude;{moduleName}_{gainName}_amplitude_legacy'
            path = moduleName
            channelLegacyGroup.defineHistogram(name, title = title, path = path, type = 'TProfile',
                                               xbins = 48, xmin = -0.5, xmax = 47.5)

    channelFelixGroup = helper.addGroup(tileRawChannelFlxMonAlg, 'TileRawChannelAmpFlx', f'{topPath}/Felix/RawChannel')
    for moduleName in modules:
        for gainName in ['HG', 'LG']:
            title = f'Run {str(runNumber)} {moduleName} {gainName}: Amplitude (FELIX);Channel;Amplitude'
            name = f'{moduleName}_{gainName}_channel,{moduleName}_{gainName}_amplitude;{moduleName}_{gainName}_amplitude_felix'
            path = moduleName
            channelFelixGroup.defineHistogram(name, title = title, path = path, type = 'TProfile',
                                              xbins = 48, xmin = -0.5, xmax = 47.5)

    channelCompareGroup = helper.addGroup(tileRawChannelFlxMonAlg, 'TileRawChannelAmpDiff', f'{topPath}/Compare/RawChannel')
    for moduleName in modules:
        for gainName in ['HG', 'LG']:
            title = f'Run {str(runNumber)} {moduleName} {gainName}: Amplitude difference (FELIX-Legacy*{felixScale});Channel;Amplitude difference [ADC]'
            name = f'{moduleName}_{gainName}_channel,{moduleName}_{gainName}_amplitude_diff;{moduleName}_{gainName}_amplitude_diff'
            path = moduleName
            channelCompareGroup.defineHistogram(name, title = title, path = path, type = 'TProfile',
                                                xbins = 48, xmin = -0.5, xmax = 47.5)
    
    channelCompareVsLegacyGroup = helper.addGroup(tileRawChannelFlxMonAlg, 'TileRawChannelAmpDiffVsLegacy', f'{topPath}/Compare/RawChannel')
    for moduleName in modules:
        for gainName in ['HG', 'LG']:
            title = f'Run {str(runNumber)} {moduleName} {gainName}: Amplitude difference (FELIX-Legacy*{felixScale});Amplitude (Legacy) [ADC];Amplitude difference [ADC]'
            name = f'{moduleName}_{gainName}_amplitude,{moduleName}_{gainName}_amplitude_diff;{moduleName}_{gainName}_amplitude_diff_vs_legacy'
            path = moduleName
            channelCompareVsLegacyGroup.defineHistogram(name, title = title, path = path, type = 'TProfile',
                                                        xbins = 1024, xmin = -0.5, xmax = 1023.5)

    accumalator = helper.result()
    result.merge(accumalator)
    return result



if __name__=='__main__':

    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO
    log.setLevel(INFO)

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.Output.HISTFileName = 'TileRawChannelFlxMonitorOutput.root'
    flags.DQ.useTrigger = False
    flags.DQ.enableLumiAccess = False
    flags.Exec.MaxEvents = 3
    flags.fillFromArgs()
    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from TileByteStream.TileByteStreamConfig import TileRawDataReadingCfg
    cfg.merge( TileRawDataReadingCfg(flags, readMuRcv=False) )

    cfg.merge(TileRawChannelFlxMonitoringConfig(flags,
                                                TileRawChannelContainerLegacy='TileRawChannelFit',
                                                TileRawChannelContainerFlx='TileRawChannelFlxFit',
                                                fillHistogramsForDSP=True))

    flags.dump()

    cfg.store( open('TileRawChannelMonitorAlgorithm.pkl','wb') )

    sc = cfg.run()

    import sys
    # Success should be 0
    sys.exit(not sc.isSuccess())

