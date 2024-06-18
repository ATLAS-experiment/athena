#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
'''
@file TileRawChannelNoiseMonitorAlgorithm.py
@brief Python configuration of TileRawChannelNoiseMonitorAlgorithm algorithm for the Run III
'''
def TileRawChannelNoiseMonitoringConfig(flags, **kwargs):

    ''' Function to configure TileRawChannelNoiseMonitorAlgorithm algorithm in the monitoring system.'''

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

    #kwargs.setdefault('TriggerChain', 'HLT_noalg_cosmiccalo_L1RD1_EMPTY') #FIXME
    kwargs.setdefault('TriggerTypes', [0x82])
    kwargs.setdefault('Gain', 1)
    kwargs.setdefault('TileRawChannelContainer', flags.Tile.RawChannelContainer)
    kwargs.setdefault('ignoreDisconnectedChannels', flags.Common.isOnline)

    # The following class will make a sequence, configure algorithms, and link
    # them to GenericMonitoringTools
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags,'TileRawChanNoiseMonitoring')

    # Adding an TileCellMonitorAlgorithm algorithm to the helper
    from AthenaConfiguration.ComponentFactory import CompFactory
    TileRawChannelNoiseMonitorAlgorithm = CompFactory.TileRawChannelNoiseMonitorAlgorithm
    tileRawChanNoiseMonAlg = helper.addAlgorithm(TileRawChannelNoiseMonitorAlgorithm, 'TileRawChanNoiseMonAlg')

    for k, v in kwargs.items():
        setattr(tileRawChanNoiseMonAlg, k, v)

    run = str(flags.Input.RunNumbers[0])

    # 1) Configure histogram with TileRawChanNoiseMonAlg algorithm execution time
    executeTimeGroup = helper.addGroup(tileRawChanNoiseMonAlg, 'TileRawChanNoiseMonExecuteTime', 'Tile/')
    executeTimeGroup.defineHistogram('TIME_execute', path = 'RawChannelNoise', type='TH1F',
                                     title = 'Time for execute TileRawChanNoiseMonAlg algorithm;time [#mus]',
                                     xbins = 100, xmin = 0, xmax = 100000)


    from TileCalibBlobObjs.Classes import TileCalibUtils as Tile

    from TileMonitoring.TileMonitoringCfgHelper import getPartitionName, getCellName, getGainName

    # 2) Configure histograms with Tile raw channel amplitude per channel
    gainName = getGainName(kwargs['Gain'])
    dimensions = [int(Tile.MAX_ROS) - 1, int(Tile.MAX_DRAWER), int(Tile.MAX_CHAN)]
    chanAmpArray = helper.addArray(dimensions, tileRawChanNoiseMonAlg,
                                   'TileRawChannelNoise', topPath = 'Tile/RawChannelNoise')
    for postfix, tool in chanAmpArray.Tools.items():
        ros, module, channel = [int(x) for x in postfix.split('_')[1:]]

        partition = getPartitionName(ros + 1)
        moduleName = Tile.getDrawerString(ros + 1, module)
        cellName = getCellName(partition, channel)

        title = 'Run %s %s: Tile cell %s / channel %s amplitude (%s);Amplitude [ADC]'
        title = title % (run, moduleName, cellName, str(channel), gainName)
        name = 'amplitude;TileRawChannelNoise_%s_%s_ch_%s_%s' % (moduleName, cellName, str(channel), gainName)
        fullPath = '{}/{}'.format(partition, moduleName)

        tool.defineHistogram(name, title = title, path = fullPath, type = 'TH1F',
                             xbins = 81, xmin = -20.25, xmax = 20.25)

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
    flags.Input.Files = ['root://eosatlas.cern.ch//eos/atlas/atlascerngroupdisk/det-tile/test/data14_cos.00239908.physics_CosmicCalo.merge.RAW._lb0004._SFO-ALL._0001.1']
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.IOVDb.GlobalTag = defaultConditionsTags.RUN2_DATA
    flags.Output.HISTFileName = 'TileRawChannelNoiseMonitorOutput.root'
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

    from TileRecUtils.TileRawChannelMakerConfig import TileRawChannelMakerCfg
    cfg.merge( TileRawChannelMakerCfg(flags) )

    cfg.merge( TileRawChannelNoiseMonitoringConfig(flags) )

    cfg.printConfig(withDetails = True, summariseProps = True)
    flags.dump()

    cfg.store( open('TileRawChannelNoiseMonitorAlgorithm.pkl','wb') )

    sc = cfg.run()

    import sys
    # Success should be 0
    sys.exit(not sc.isSuccess())
