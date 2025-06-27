# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

"""Define method to construct configured Tile raw channel maker algorithm"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep
from TileConfiguration.TileConfigFlags import TileRunType

def TileRawChannelMakerCfg(flags, **kwargs):
    """Return component accumulator with configured Tile raw channel maker algorithm

    Arguments:
        flags  -- Athena configuration flags
    """

    acc = ComponentAccumulator()

    from TileConditions.TileInfoLoaderConfig import TileInfoLoaderCfg
    acc.merge( TileInfoLoaderCfg(flags) )

    kwargs.setdefault('name', 'TileRChMaker')
    name = kwargs['name']

    if flags.Common.ProductionStep in [ProductionStep.PileUpPresampling, ProductionStep.PileUpPretracking]:
        kwargs.setdefault('TileDigitsContainer', flags.Overlay.BkgPrefix + 'TileDigitsCnt')
    else:
        kwargs.setdefault('TileDigitsContainer', 'TileDigitsCnt')

    from AthenaCommon.Logging import logging
    mlog = logging.getLogger( 'TileRawChannelMakerCfg' )

    if flags.Tile.doOverflowFit:
        kwargs.setdefault('FitOverflow', True)
        from TileRecUtils.TileRawChannelBuilderFitConfig import TileRawChannelBuilderFitOverflowCfg
        tileRawChannelBuilderFitOverflow = acc.popToolsAndMerge( TileRawChannelBuilderFitOverflowCfg(flags) )
        kwargs.setdefault('TileRawChannelBuilderFitOverflow', tileRawChannelBuilderFitOverflow)
    else:
        kwargs.setdefault('FitOverflow', False)

    tileRawChannelBuilder = []

    if flags.Tile.doFit:
        from TileRecUtils.TileRawChannelBuilderFitConfig import TileRawChannelBuilderFitFilterCfg
        tileRawChannelBuilderFitFilter = acc.popToolsAndMerge( TileRawChannelBuilderFitFilterCfg(flags) )
        tileRawChannelBuilder += [tileRawChannelBuilderFitFilter]
        mlog.info(" adding now TileRawChannelBuilderFitFilter with name %s to the algorithm: %s",
                  tileRawChannelBuilderFitFilter.name, name)

    if flags.Tile.doMF:
        from TileRecUtils.TileRawChannelBuilderMFConfig import TileRawChannelBuilderMFCfg
        tileRawChannelBuilderMF = acc.popToolsAndMerge( TileRawChannelBuilderMFCfg(flags) )
        tileRawChannelBuilder += [tileRawChannelBuilderMF]
        mlog.info(" adding now TileRawChannelBuilderMF with name %s to the algorithm: %s",
                  tileRawChannelBuilderMF.name, name)

    if flags.Tile.doOF1:
        from TileRecUtils.TileRawChannelBuilderOptConfig import TileRawChannelBuilderOF1Cfg
        tileRawChannelBuilderOF1 = acc.popToolsAndMerge( TileRawChannelBuilderOF1Cfg(flags) )
        tileRawChannelBuilder += [tileRawChannelBuilderOF1]
        mlog.info(" adding now TileRawChannelBuilderOpt2Filter with name %s to the algorithm: %s",
                  tileRawChannelBuilderOF1.name, name)

    if flags.Tile.doWiener:
        from TileRecUtils.TileRawChannelBuilderWienerConfig import TileRawChannelBuilderWienerCfg
        tileRawChannelBuilderWiener = acc.popToolsAndMerge( TileRawChannelBuilderWienerCfg(flags) )
        tileRawChannelBuilder += [tileRawChannelBuilderWiener]
        mlog.info(" adding now TileRawChannelBuilderWienerFilter with name %s to the algorithm: %s",
                  tileRawChannelBuilderWiener.name, name)

    if flags.Tile.doOpt2:
        from TileRecUtils.TileRawChannelBuilderOptConfig import TileRawChannelBuilderOpt2Cfg
        tileRawChannelBuilderOpt2 = acc.popToolsAndMerge( TileRawChannelBuilderOpt2Cfg(flags) )
        tileRawChannelBuilder += [tileRawChannelBuilderOpt2]
        mlog.info(" adding now TileRawChannelBuilderOpt2Filter with name %s to the algorithm: %s",
                  tileRawChannelBuilderOpt2.name, name)

    if flags.Tile.doOptATLAS:
        from TileRecUtils.TileRawChannelBuilderOptConfig import TileRawChannelBuilderOptATLASCfg
        tileRawChannelBuilderOptATLAS = acc.popToolsAndMerge( TileRawChannelBuilderOptATLASCfg(flags) )
        tileRawChannelBuilder += [tileRawChannelBuilderOptATLAS]
        mlog.info(" adding now TileRawChannelBuilderOpt2Filter with name %s to the algorithm: %s",
                  tileRawChannelBuilderOptATLAS.name, name)

    kwargs.setdefault('TileRawChannelBuilder', tileRawChannelBuilder)

    if flags.Common.isOverlay and flags.Concurrency.NumThreads > 0:
        kwargs.setdefault('Cardinality', flags.Concurrency.NumThreads)

    TileRawChannelMaker=CompFactory.TileRawChannelMaker
    acc.addEventAlgo(TileRawChannelMaker(**kwargs), primary = True)

    return acc


def TileRawChannelMakerDigiHSTruthCfg(flags, **kwargs):
    """Return component accumulator with configured Tile raw channel maker algorithm for HS

    Arguments:
        flags  -- Athena configuration flags
    """

    kwargs.setdefault('name', 'TileRChMaker_DigiHSTruth')
    kwargs.setdefault('TileDigitsContainer', 'TileDigitsCnt_DigiHSTruth')

    acc = TileRawChannelMakerCfg(flags, **kwargs)
    rawChannelMaker = acc.getPrimary()

    rawChannelbuilders = rawChannelMaker.TileRawChannelBuilder

    for rawChannelBuilder in rawChannelbuilders:
        rawChannelBuilder.TileRawChannelContainer = f'{rawChannelBuilder.TileRawChannelContainer}_DigiHSTruth'

    return acc


def TileRawChannelOutputCfg(flags, tileRawChannelMaker, streamName):
    """Return component accumulator with configured Output stream for Tile raw channel maker algorithm

    Arguments:
        flags  -- Athena configuration flags
        tileRawChannelMaker -- Tile raw channel maker algorithm
        streamName -- name of output stream.
    """

    outputItemList = []
    rawChannelbuilders = tileRawChannelMaker.TileRawChannelBuilder

    for rawChannelBuilder in rawChannelbuilders:
        outputItemList += [f'TileRawChannelContainer#{rawChannelBuilder.TileRawChannelContainer}']

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    acc = OutputStreamCfg(flags, streamName, ItemList = outputItemList)

    return acc


def TileRawChannelMakerOutputCfg(flags, streamName = 'ESD', **kwargs):
    """Return component accumulator with configured Tile raw channel maker algorithm and Output stream

    Arguments:
        flags  -- Athena configuration flags
        streamName -- name of output stream. Defaults to ESD.
    """

    acc = TileRawChannelMakerCfg(flags, **kwargs)
    acc.merge( TileRawChannelOutputCfg(flags, acc.getPrimary(), streamName) )

    return acc


def TileRawChannelMakerDigiHSTruthOutputCfg(flags, streamName = 'ESD', **kwargs):
    """Return component accumulator with configured Tile raw channel maker algorithm and Output stream

    Arguments:
        flags  -- Athena configuration flags
        streamName -- name of output stream. Defaults to ESD.
    """

    acc = TileRawChannelMakerDigiHSTruthCfg(flags, **kwargs)
    acc.merge( TileRawChannelOutputCfg(flags, acc.getPrimary(), streamName) )

    return acc


if __name__ == "__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultTestFiles
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG

    # Test setup
    log.setLevel(DEBUG)

    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.Tile.RunType = TileRunType.PHY
    flags.Tile.doFit = True
    flags.Tile.doOF1 = True
    flags.Tile.doWiener = True
    flags.Tile.doOpt2 = True
    flags.Tile.doOptATLAS = True
    flags.Tile.correctTimeJumps = True
    flags.Tile.NoiseFilter = 1
    flags.Output.ESDFileName = "myESD.pool.root"
    flags.Exec.MaxEvents=3
    flags.fillFromArgs()

    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from TileByteStream.TileByteStreamConfig import TileRawDataReadingCfg
    acc.merge( TileRawDataReadingCfg(flags, readMuRcv=False) )

    acc.merge( TileRawChannelMakerOutputCfg(flags) )

    flags.dump()
    acc.printConfig(withDetails = True, summariseProps = True)
    acc.store( open('TileRawChannelMaker.pkl','wb') )

    sc = acc.run()

    import sys
    # Success should be 0
    sys.exit(not sc.isSuccess())

