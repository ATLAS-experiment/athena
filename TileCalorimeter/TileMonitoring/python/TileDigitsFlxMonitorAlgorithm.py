#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

'''
@file TileDigitsFlxMonitorAlgorithm.py
@brief Python configuration of TileDigitsFlxMonitorAlgorithm algorithm for the Run III
'''

def TileDigitsFlxMonitoringConfig(flags, fragIDs=[0x201, 0x402], **kwargs):
    '''Function to configures TileDigitsFlxMonAlg algorithms in the monitoring system.'''
    
    kwargs.setdefault('TileDigitsContainerLegacy', 'TileDigitsCnt')
    kwargs.setdefault('TileDigitsContainerFlx', 'TileDigitsFlxCnt')
    kwargs.setdefault('FirstSample', 0)
    kwargs.setdefault('LastSample', 15)
    kwargs.setdefault('FelixOffset', 0)
    kwargs.setdefault('FelixScale', 4)

    felixScale = kwargs['FelixScale']

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    result.merge( TileCablingSvcCfg(flags) )

    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags, 'TileDigitsFlxMonitoring')

    from AthenaConfiguration.ComponentFactory import CompFactory
    TileDigitsFlxMonitorAlgorithm = CompFactory.TileDigitsFlxMonitorAlgorithm
    tileDigitsFlxMonAlg = helper.addAlgorithm(TileDigitsFlxMonitorAlgorithm, 'TileDigitsFlxMonAlg')
    tileDigitsFlxMonAlg.TriggerChain = ''

    for k, v in kwargs.items():
        setattr(tileDigitsFlxMonAlg, k, v)

    # Configure histogram with TileDigitsFlxMonAlg algorithm execution time
    executeTimeGroup = helper.addGroup(tileDigitsFlxMonAlg, 'TileDigitsFlxMonExecuteTime', 'Tile/Felix')
    executeTimeGroup.defineHistogram('TIME_execute', path = 'Digits', type='TH1F',
                                     title = 'Time for execute TileDigitsFlxMonAlg algorithm;time [#mus]',
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

    channelHFNGroup = helper.addGroup(tileDigitsFlxMonAlg, 'TileFlxMonHFN', 'Tile/Felix/Digits')
    for moduleName in modules:
        for gainName in ['HG', 'LG']:
            title = f'Run {runNumber} {moduleName} {gainName}: Mean RMS in event (HFN)-FELIX;Channel;HFN'
            name = f'{moduleName}_{gainName}_channel,{moduleName}_{gainName}_HFN;{moduleName}_HFN_{gainName}'
            path = moduleName
            channelHFNGroup.defineHistogram(name, title = title, path = path, type = 'TProfile',
                                            xbins = 48, xmin = -0.5, xmax = 47.5)

    channelPedGroup = helper.addGroup(tileDigitsFlxMonAlg, 'TileFlxMonPed', 'Tile/Felix/Digits')
    for moduleName in modules:
        for gainName in ['HG', 'LG']:
            title = f'Run {runNumber} {moduleName} {gainName}: Pedestal, sample[0]-FELIX;Channel;ADC'
            name = f'{moduleName}_{gainName}_channel,{moduleName}_{gainName}_Pedestal;{moduleName}_Pedestal_{gainName}'
            path = moduleName
            channelPedGroup.defineHistogram(name, title = title, path = path, type = 'TProfile',
                                            xbins = 48, xmin = -0.5, xmax = 47.5)

    channelHFNGroup = helper.addGroup(tileDigitsFlxMonAlg, 'TileLegacyMonHFN', 'Tile/Legacy/Digits')
    for moduleName in modules:
        for gainName in ['HG', 'LG']:
            title = f'Run {runNumber} {moduleName} {gainName}: Mean RMS in event (HFN);Channel;HFN'
            name = f'{moduleName}_{gainName}_channel,{moduleName}_{gainName}_HFN;{moduleName}_HFN_{gainName}'
            path = moduleName
            channelHFNGroup.defineHistogram(name, title = title, path = path, type = 'TProfile',
                                            xbins = 48, xmin = -0.5, xmax = 47.5)

    channelPedGroup = helper.addGroup(tileDigitsFlxMonAlg, 'TileLegacyMonPed', 'Tile/Legacy/Digits')
    for moduleName in modules:
        for gainName in ['HG', 'LG']:
            title = f'Run {runNumber} {moduleName} {gainName}: Pedestal, sample[0];Channel;ADC'
            name = f'{moduleName}_{gainName}_channel,{moduleName}_{gainName}_Pedestal;{moduleName}_Pedestal_{gainName}'
            path = moduleName
            channelPedGroup.defineHistogram(name, title = title, path = path, type = 'TProfile',
                                            xbins = 48, xmin = -0.5, xmax = 47.5)

    channelSamplesFlxGroup = helper.addGroup(tileDigitsFlxMonAlg, 'TileFlxMonSamples', 'Tile/Felix/Digits')
    for moduleName in modules:
        for channel in range(0, Tile.MAX_CHAN):
            for gainName in ['HG', 'LG']:
                title = (f'Run {runNumber} {moduleName} channel {channel} {gainName}: Samples-FELIX;Sample [ADC];N')
                name = f'{moduleName}_ch_{str(channel)}_{gainName}_samples'
                path = moduleName
                channelSamplesFlxGroup.defineHistogram(name, title = title, path = path, type = 'TH1F',
                                                       xbins = 4096, xmin = -0.5, xmax = 4095.5)


    channelSamplesLegacyGroup = helper.addGroup(tileDigitsFlxMonAlg, 'TileLegacyMonSamples', 'Tile/Legacy/Digits')
    for moduleName in modules:
        for channel in range(0, Tile.MAX_CHAN):
            for gainName in ['HG', 'LG']:
                title = (f'Run {runNumber} {moduleName} channel {channel} {gainName}: Samples;Sample [ADC];N')
                name = f'{moduleName}_ch_{str(channel)}_{gainName}_samples'
                path = moduleName
                channelSamplesLegacyGroup.defineHistogram(name, title = title, path = path, type = 'TH1F',
                                                          xbins = 1024, xmin = -0.5, xmax = 1023.5)

    channelAllSamplesDiffGroup = helper.addGroup(tileDigitsFlxMonAlg, 'TileChannelAllSamplesDiff', 'Tile/Compare/Digits')
    for moduleName in modules:
        for channel in range(0, Tile.MAX_CHAN):
            for gainName in ['HG', 'LG']:
                title = (f'Run {runNumber} {moduleName} channel {channel} {gainName}: Samples difference (FELIX-Legacy*{felixScale});Sample [ADC];N')
                name = f'{moduleName}_ch_{str(channel)}_{gainName}_samples_diff'
                path = moduleName
                channelAllSamplesDiffGroup.defineHistogram(name, title = title, path = path, type = 'TH1F',
                                                           xbins = 200, xmin = -100, xmax = 100)

    moduleAllSamplesGroup = helper.addGroup(tileDigitsFlxMonAlg, 'TileModuleAllSamplesDiff', 'Tile/Compare/Digits')
    for moduleName in modules:
        for gainName in ['HG', 'LG']:
            title = (f'Run {runNumber} {moduleName} {gainName}: All channels samples difference (FELIX-Legacy*{felixScale});Sample [ADC];N')
            name = f'{moduleName}_{gainName}_samples_diff;{moduleName}_{gainName}_all_channels_samples_diff'
            path = moduleName
            moduleAllSamplesGroup.defineHistogram(name, title = title, path = path, type = 'TH1F',
                                                  xbins = 200, xmin = -100, xmax = 100)

    moduleSamplesDiffGroup = helper.addGroup(tileDigitsFlxMonAlg, 'TileModuleSamplesDiff', 'Tile/Compare/Digits')
    for moduleName in modules:
        for gainName in ['HG', 'LG']:
            title = f'Run {runNumber} {moduleName} {gainName}: Samples difference (FELIX-Legacy*{felixScale});Channel;ADC'
            name = f'{moduleName}_{gainName}_channel,{moduleName}_{gainName}_samples_diff;{moduleName}_{gainName}_samples_diff'
            path = moduleName
            moduleSamplesDiffGroup.defineHistogram(name, title = title, path = path, type = 'TProfile',
                                                   xbins = 48, xmin = -0.5, xmax = 47.5)


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
    flags = initConfigFlags()
    parser = flags.getArgumentParser()
    parser.add_argument('--postExec', help='Code to execute after setup')
    parser.add_argument('--digits', default="TileDigitsCnt", help='Tile digits container to be monitored')
    parser.add_argument('--frag-ids', dest='fragIDs', nargs="*", default=['0x201','0x402'], help='Tile Frag IDs of modules to be monitored. Empty=ALL')
    args, _ = parser.parse_known_args()

    fragIDs = [int(fragID, base=16) for fragID in args.fragIDs]

    from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultTestFiles
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.Output.HISTFileName = 'TileDigitsFlxMonitorOutput.root'
    flags.DQ.useTrigger = False
    flags.DQ.enableLumiAccess = False
    flags.Exec.MaxEvents = 3
    flags.fillFromArgs(parser=parser)
    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from TileByteStream.TileByteStreamConfig import TileRawDataReadingCfg
    cfg.merge( TileRawDataReadingCfg(flags, readDigits=True, readRawChannel=False, readMuRcv=False,
                                     readDigitsFlx=(True if 'Flx' in args.digits else False)) )

    tileDigitsFlxMonitorAccumulator  = TileDigitsFlxMonitoringConfig(flags,
                                                                     fragIDs = fragIDs,
                                                                     TileDigitsContainerLegacy="TileDigitsCnt",
                                                                     TileDigitsContainerFlx=args.digits)

    cfg.merge(tileDigitsFlxMonitorAccumulator)

    # Any last things to do?
    if args.postExec:
        log.info('Executing postExec: %s', args.postExec)
        exec(args.postExec)

    cfg.printConfig(withDetails = True, summariseProps = True)
    flags.dump()

    cfg.store( open('TileDigitsFlxMonitorAlgorithm.pkl','wb') )

    sc = cfg.run()

    import sys
    # Success should be 0
    sys.exit(not sc.isSuccess())

