#
#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

from TileConfiguration.TileConfigFlags import TileRunType

'''
@file TileRawChannelTimeMonitorAlgorithm.py
@brief Python configuration of TileRawChannelTimeMonitorAlgorithm algorithm for the Run III
'''
def TileRawChannelTimeMonitoringConfig(flags, amplitudeFragIDs=None, **kwargs):

    ''' Function to configure TileRawChannelTimeMonitorAlgorithm algorithm in the monitoring system.'''

    # Define one top-level monitoring algorithm. The new configuration
    # framework uses a component accumulator.
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()

    from TileRecUtils.TileDQstatusConfig import TileDQstatusAlgCfg
    result.merge( TileDQstatusAlgCfg(flags) )

    from TileGeoModel.TileGMConfig import TileGMCfg
    result.merge(TileGMCfg(flags))

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    result.merge( TileCablingSvcCfg(flags) )

    from TileConditions.TileBadChannelsConfig import TileBadChannelsCondAlgCfg
    result.merge( TileBadChannelsCondAlgCfg(flags, **kwargs) )

    from TileConditions.TileEMScaleConfig import TileEMScaleCondAlgCfg
    result.merge( TileEMScaleCondAlgCfg(flags) )

    kwargs.setdefault('CheckDCS', flags.Tile.useDCS)
    if kwargs['CheckDCS']:
        from TileConditions.TileDCSConfig import TileDCSCondAlgCfg
        result.merge( TileDCSCondAlgCfg(flags) )

    kwargs.setdefault('TriggerChain', '')
    kwargs.setdefault('TriggerTypes', [0x34]) # Laser event Trigger type - 0x34, CIS trigger type - 0x32

    # Partition pairs to monitor average time difference between partitions (ROS - 1)
    partitionPairs = [[0, 1], [0, 2], [0, 3], [1, 2], [1, 3], [2, 3]]
    kwargs.setdefault('PartitionTimeDiffferncePairs', partitionPairs)

    runType = flags.Tile.RunType
    partitionTimeCorrections = [0, 0, 0, 0]
    if flags.Input.RunNumbers[0] > 400000: # Update partition time corrections for Run 3
        if runType is TileRunType.GAPLAS:
            partitionTimeCorrections = [-28.65, -45.2, 25.24, 24.94]
        elif runType is TileRunType.GAPCIS:
            partitionTimeCorrections = [0, 0, 0, 0]
    else:
        partitionTimeCorrections = [-15.18, -15.37, 47.65, 47.42]

    kwargs.setdefault('PartitionTimeCorrections', partitionTimeCorrections)

    # The following class will make a sequence, configure algorithms, and link
    # them to GenericMonitoringTools
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags,'TileRawChannelTimeMonitoring')

    # Adding an TileCellMonitorAlgorithm algorithm to the helper
    from AthenaConfiguration.ComponentFactory import CompFactory
    TileRawChannelTimeMonitorAlgorithm = CompFactory.TileRawChannelTimeMonitorAlgorithm
    tileRawChanTimeMonAlg = helper.addAlgorithm(TileRawChannelTimeMonitorAlgorithm, 'TileRawChanTimeMonAlg')

    if amplitudeFragIDs is not None:
        tileRawChanTimeMonAlg.AmplitudeFragIDs = amplitudeFragIDs
    for k, v in kwargs.items():
        setattr(tileRawChanTimeMonAlg, k, v)

    run = str(flags.Input.RunNumbers[0])
    eventsType = 'CIS' if runType.getCommonType() is TileRunType.CIS else 'Laser'

    # 1) Configure histogram with TileRawChannelTimeMonAlg algorithm execution time
    executeTimeGroup = helper.addGroup(tileRawChanTimeMonAlg, 'TileRawChanTimeMonExecuteTime', 'Tile/')
    executeTimeGroup.defineHistogram('TIME_execute', path = 'RawChannelTime', type='TH1F',
                                     title = 'Time for execute TileRawChanTimeMonAlg algorithm;time [#mus]',
                                     xbins = 100, xmin = 0, xmax = 100000)


    from TileMonitoring.TileMonitoringCfgHelper import addTileModuleChannelMapsArray, getPartitionName

    # 2) Configure histograms with status of Tile channel time per partition
    addTileModuleChannelMapsArray(helper, tileRawChanTimeMonAlg, name = 'TileAverageTime',
                                  title = f'Tile average time with {eventsType} (partition average time is subracted)',
                                  path = 'Tile/RawChannelTime/Summary',
                                  type = 'TProfile2D', value = 'time', run = run)

    # 2.b) Configure histograms with average of Tile channel uncorrected time per partition
    addTileModuleChannelMapsArray(helper, tileRawChanTimeMonAlg, name = 'TileAverageUncorrectedTime',
                                  title = f'Tile average uncorrected time with {eventsType}',
                                  path = 'Tile/RawChannelTime/Summary',
                                  type = 'TProfile2D', value = 'time', run = run)

    # 2.c) Configure histograms with average of Tile channel amplitude per partition
    addTileModuleChannelMapsArray(helper, tileRawChanTimeMonAlg, name = 'TileAverageAmplitude',
                                  title = f'Tile average amplitude [pC] with {eventsType}',
                                  path = 'Tile/RawChannelTime/Summary',
                                  type = 'TProfile2D', value = 'amplitude', run = run)

    from TileMonitoring.TileMonitoringCfgHelper import addTile2DHistogramsArray

    # 3) Configure histograms with Tile partition average time vs luminosity block per partition
    partitionAverageTimeTitle = {}
    partitionAverageTimeTitleTemplate = f'Tile average time with {eventsType} corrected by %+0.0f [ns] vs LumiBlock;LumiBlock;t [ns]'
    for ros in range(1,5):
        partitionAverageTimeTitle[getPartitionName(ros)] = partitionAverageTimeTitleTemplate % (-partitionTimeCorrections[ros - 1])
    addTile2DHistogramsArray(helper, tileRawChanTimeMonAlg, name = 'TileAverageTimeLB',
                             xvalue = 'lumiBlock', yvalue = 'time', type='TH2D',
                             title = partitionAverageTimeTitle, opt = 'kAddBinsDynamically', merge = 'merge',
                             path = 'Tile/RawChannelTime/Summary', run = run, perPartition = True,
                             xbins = 3000, xmin = -0.5, xmax = 2999.5, ybins = 149, ymin = -74.5, ymax = 74.5)


    # 4) Configure histograms with Tile partition average time difference vs luminosity block
    partitionPairs = kwargs['PartitionTimeDiffferncePairs']
    partTimeDiffVsLBArray = helper.addArray([len(partitionPairs)], tileRawChanTimeMonAlg,
                                            'TileAverageTimeDifferenceLB', topPath = 'Tile/RawChannelTime')
    for postfix, tool in partTimeDiffVsLBArray.Tools.items():
        pairIdx = int(postfix.split('_').pop())
        partitionName1, partitionName2 = [getPartitionName(ros + 1) for ros in partitionPairs[pairIdx]]
        ros1, ros2 = partitionPairs[pairIdx]
        partitionTimeCorrection1 = partitionTimeCorrections[ros1]
        partitionTimeCorrection2 = partitionTimeCorrections[ros2]

        title = f'Run {run}: Average time with {eventsType} between {partitionName1} and {partitionName2}'
        title += ' corrected by %+0.0f [ns]' % (partitionTimeCorrection1 - partitionTimeCorrection2)
        title += ' vs luminosity block;LumiBlock;t [ns]'
        name = 'lumiBlock,time;TileAverageTimeDifferenceLB_%s-%s' % (partitionName1, partitionName2)

        tool.defineHistogram(name, title = title, path = 'Summary', type = 'TProfile',
                             xbins = 1000, xmin = -0.5, xmax = 999.5, opt = 'kAddBinsDynamically', merge = 'merge')


    from TileCalibBlobObjs.Classes import TileCalibUtils as Tile

    # 5) Configure histograms with Tile digitizer time vs luminosity block per digitizer
    maxDigitizer = 8
    digiTimeVsLBArray = helper.addArray([int(Tile.MAX_ROS - 1), int(Tile.MAX_DRAWER), maxDigitizer],
                                        tileRawChanTimeMonAlg, 'TileDigitizerTimeLB', topPath = 'Tile/RawChannelTime')
    for postfix, tool in digiTimeVsLBArray.Tools.items():
        ros, module, digitizer = [int(x) for x in postfix.split('_')[1:]]
        digitizer += 1

        moduleName = Tile.getDrawerString(ros + 1, module)
        title = 'Run ' + run + ' ' + moduleName + ' Digitizer ' + str(digitizer)
        title +=  ': Time vs LB (partition average time is subracted, fake time (-100) means no signal);LumiBlock;t [ns]'
        name = 'lumiBlock,time;TileDigitizerTimeLB_' + moduleName + '_DIGI_' + str(digitizer)
        path = getPartitionName(ros + 1) + '/' + moduleName

        tool.defineHistogram(name, title = title, path = path, type = 'TProfile',
                             xbins = 1000, xmin = -0.5, xmax = 999.5, opt = 'kAddBinsDynamically', merge = 'merge')


    modules = []
    if amplitudeFragIDs is not None:
        # Preapare module names for which monitoring histograms with amplitude vs LB should be produced
        for fragID in amplitudeFragIDs:
            ros = fragID >> 8
            drawer = fragID & 0x3F
            modules += [Tile.getDrawerString(ros, drawer)]

        ampVsLBArray = helper.addArray([modules], tileRawChanTimeMonAlg, 'TileAmplitudeVsLB', topPath='Tile/RawChannelTime')
        for postfix, tool in ampVsLBArray.Tools.items():
            # Define set of monitoring histograms with amplitude vs LB for each specified module
            moduleName = postfix[1:]
            partition = moduleName[:3]
            for channel in range(0, Tile.MAX_CHAN):
                # Define monitoring histogram with amplitude vs LB for each channel in the module
                title = f'Run {run} {moduleName} Channel {channel}: Amplitude vs luminosity block;LumiBlock;Amplitude [pC]'
                name = f'lumiBlock,amplitude_{channel};TileAmplitudeVsLB_{moduleName}_ch_{channel}'
                path = f'{partition}/{moduleName}'

                tool.defineHistogram(name, title=title, path=path, type='TProfile',
                                     xbins=1000, xmin=-0.5, xmax=999.5, opt='kAddBinsDynamically', merge='merge')


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
    from AthenaConfiguration.TestDefaults import defaultConditionsTags, defaultGeometryTags
    flags = initConfigFlags()
    inputDirectory = '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TileByteStream/TileByteStream-02-00-00'
    inputFile = 'data18_tilecomm.00363899.calibration_tile.daq.RAW._lb0000._TileREB-ROS._0005-200ev.data'
    flags.Input.Files = [inputDirectory + '/' + inputFile]
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.IOVDb.GlobalTag = defaultConditionsTags.RUN2_DATA
    flags.Output.HISTFileName = 'TileRawChannelTimeMonitorOutput.root'
    flags.DQ.useTrigger = False
    flags.DQ.enableLumiAccess = False

    flags.Tile.RunType = TileRunType.GAPLAS
    flags.Tile.doFit = True
    flags.Tile.correctTime = True
    flags.Tile.doOverflowFit = False
    flags.Tile.BestPhaseFromCOOL = True
    flags.Tile.NoiseFilter = 1
    flags.Exec.MaxEvents = 3
    flags.fillFromArgs()

    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from TileByteStream.TileByteStreamConfig import TileRawDataReadingCfg
    cfg.merge( TileRawDataReadingCfg(flags, readMuRcv=False, readBeamElem=True) )

    from TileRecUtils.TileRawChannelMakerConfig import TileRawChannelMakerCfg
    cfg.merge( TileRawChannelMakerCfg(flags) )

    cfg.merge( TileRawChannelTimeMonitoringConfig(flags) )

    cfg.printConfig(withDetails = True, summariseProps = True)
    flags.dump()

    cfg.store( open('TileRawChannelTimeMonitorAlgorithm.pkl','wb') )

    sc = cfg.run()

    import sys
    # Success should be 0
    sys.exit(not sc.isSuccess())
