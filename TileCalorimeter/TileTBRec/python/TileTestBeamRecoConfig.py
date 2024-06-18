#!/usr/bin/env python
#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
'''
@file TileTestBeamRecoConfig.py
'''

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

import sys


def TileTestBeamRawChannelMakerCfg(flags, nsamples, useFELIX=False, **kwargs):

    ''' Function to configure reconstruction of Tile raw channels from digits.'''

    acc = ComponentAccumulator()

    suffix = "Flx" if useFELIX else ""
    kwargs.setdefault('name', f'TileRCh{suffix}Maker')
    kwargs.setdefault('TileDigitsContainer', f'TileDigits{suffix}Cnt')
    kwargs.setdefault('Cardinality', flags.Concurrency.NumThreads)
    kwargs.setdefault('TileInfoName', f'TileInfo{suffix}')

    from TileRecUtils.TileRawChannelMakerConfig import TileRawChannelMakerCfg
    rawChMaker = acc.getPrimaryAndMerge(TileRawChannelMakerCfg(flags, **kwargs))
    if flags.Tile.doFit:
        rawChannelBuilderFitFilter = rawChMaker.TileRawChannelBuilder['TileRawChannelBuilderFitFilter']
        rawChannelBuilderFitFilter.FrameLength = nsamples
        rawChannelBuilderFitFilter.SaturatedSample = 4095 if useFELIX else -1
    for tool in rawChMaker.TileRawChannelBuilder:
        tool.TileRawChannelContainer = str(tool.TileRawChannelContainer).replace('TileRawChannel', f'TileRawChannel{suffix}')
        tool.TileInfoName = f'TileInfo{suffix}'

    # Configure TileInfoLoader to set up number of samples
    from TileConditions.TileInfoLoaderConfig import TileInfoLoaderCfg
    acc.merge(TileInfoLoaderCfg(flags, name=f'TileInfoLoader{suffix}', TileInfo=f'TileInfo{suffix}',
                                NSamples=nsamples, TrigSample=((nsamples-1)//2) ))

    return acc


def TileTestBeamRecoCfg(flags, useDemoCabling, nsamples, useFELIX=False, filterDigits=False, filterChannels=True):

    ''' Function to configure reconstruction of Tile TestBeam data.'''

    acc = ComponentAccumulator()

    suffix = "Flx" if useFELIX else ""
    digitsContainer = f'TileDigits{suffix}Cnt'
    rawChannelContainer = flags.Tile.RawChannelContainer.replace('TileRawChannel', f'TileRawChannel{suffix}')

    # =====> For FELIX configure the algorithm to select one gain
    if useFELIX and filterDigits:
            digitsContainer = 'TileDigitsFlxFiltered'
            TileDigitsGainFilter = CompFactory.TileDigitsGainFilter
            acc.addEventAlgo( TileDigitsGainFilter(HighGainThreshold=4095,
                                                   InputDigitsContainer='TileDigitsFlxCnt',
                                                   OutputDigitsContainer=digitsContainer) )

    #  =====> Configure reconstruction of Tile raw channels from digits
    acc.merge(TileTestBeamRawChannelMakerCfg(flags, nsamples, useFELIX, TileDigitsContainer=digitsContainer))

    if useFELIX and filterChannels:
        rawChannelContainerFiltered = f'{rawChannelContainer}Filtered'
        TileRawChannelGainFilter = CompFactory.TileRawChannelGainFilter
        acc.addEventAlgo( TileRawChannelGainFilter(InputRawChannelContainer=rawChannelContainer,
                                                   OutputRawChannelContainer=rawChannelContainerFiltered) )
        rawChannelContainer = rawChannelContainerFiltered

    #  =====> Configure reconstruction of Tile cells from raw channels
    from TileRecUtils.TileCellMakerConfig import TileCellMakerCfg
    skipGains = [0, 1] if flags.Tile.RunType.isBiGain() else [-1]
    gainName = {-1 : "", 0 : "HG", 1 : "LG"} # Skip gain to actually used gain name mapping
    for skipGain in skipGains:
        cellMaker = acc.getPrimaryAndMerge(TileCellMakerCfg(flags, name=f'TileCell{suffix}Maker{gainName[skipGain]}',
                                                            CaloCellsOutputName=f'AllCalo{suffix}{gainName[skipGain]}',
                                                            mergeChannels=False, SkipGain=skipGain))
        cellBuilder = cellMaker.CaloCellMakerToolNames['TileCellBuilder']
        cellBuilder.TileRawChannelContainer = rawChannelContainer
        cellBuilder.UseDemoCabling = useDemoCabling
        cellBuilder.maskBadChannels = False
        cellBuilder.MBTSContainer = ""
        cellBuilder.E4prContainer = ""


    # Configure TileInfoLoader to set up number of samples
    ADCmax = 4095 if useFELIX else 1023
    ADCmaskValue = 4800 if useFELIX else 2047
    from TileConditions.TileInfoLoaderConfig import TileInfoLoaderCfg
    acc.merge(TileInfoLoaderCfg(flags, name=f'TileInfoLoader{suffix}', TileInfo=f'TileInfo{suffix}',
                                NSamples=nsamples, TrigSample=((nsamples-1)//2),
                                ADCmax=ADCmax, ADCmaskValue=ADCmaskValue))

    return acc


if __name__ == '__main__':

    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO
    log.setLevel(INFO)

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultConditionsTags, defaultGeometryTags, defaultTestFiles

    flags = initConfigFlags()
    parser = flags.getArgumentParser()
    parser.add_argument('--demo-cabling', dest='demoCabling', type=int, default=2018, help='Time Demonatrator cabling to be used')
    parser.add_argument('--nsamples', type=int, default=15, help='Number of samples')
    args, _ = parser.parse_known_args()

    flags.Exec.MaxEvents = 3
    flags.Common.isOnline = True
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN3
    flags.Input.Files = defaultTestFiles.RAW_RUN3
    flags.IOVDb.GlobalTag = defaultConditionsTags.RUN3_DATA

    flags.Tile.doFit = True
    flags.Tile.useDCS = False
    flags.Tile.NoiseFilter = 0
    flags.Tile.correctTime = False
    flags.Tile.correctTimeJumps = False
    flags.Tile.BestPhaseFromCOOL = False
    flags.Tile.doOverflowFit = False

    flags.fillFromArgs(parser=parser)
    flags.lock()

    flags.dump()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from TileByteStream.TileByteStreamConfig import TileRawDataReadingCfg
    cfg.merge( TileRawDataReadingCfg(flags, readMuRcv=False,
                                     readDigits=True,
                                     readRawChannel=True,
                                     readDigitsFlx=True,
                                     readBeamElem=True) )

    cfg.merge( TileTestBeamRecoCfg(flags, useDemoCabling=args.demoCabling, nsamples=args.nsamples) )
    cfg.merge( TileTestBeamRecoCfg(flags, useDemoCabling=args.demoCabling, nsamples=16, useFELIX=True) )

    # Scan first event for all fragments to create proper ROD to ROB map
    cfg.getCondAlgo('TileHid2RESrcIDCondAlg').RODStatusProxy = None

    cfg.printConfig(withDetails=True, summariseProps=True, printDefaults=True)

    sc = cfg.run()
    # Success should be 0
    sys.exit(not sc.isSuccess())
