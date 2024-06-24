# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

"""Define methods to construct configured Tile L2 builder tool and algorithm"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep


def TileL2BuilderCfg(flags, **kwargs):
    """Return component accumulator with configured private Tile L2 builder tool

    Arguments:
        flags  -- Athena configuration flags
    """

    kwargs.setdefault('name', 'TileL2Builder')

    rawChannelContainer = 'TileRawChannelCnt'
    if (flags.Input.isMC or flags.Overlay.DataOverlay):
        rawChannelContainer = flags.Tile.RawChannelContainer
    kwargs.setdefault('TileRawChannelContainer', rawChannelContainer)

    acc = ComponentAccumulator()

    from TileGeoModel.TileGMConfig import TileGMCfg
    acc.merge( TileGMCfg(flags) )

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    acc.merge( TileCablingSvcCfg(flags) )

    from TileConditions.TileBadChannelsConfig import TileBadChannelsCondAlgCfg
    acc.merge( TileBadChannelsCondAlgCfg(flags) )

    from TileConditions.TileEMScaleConfig import TileEMScaleCondAlgCfg
    acc.merge( TileEMScaleCondAlgCfg(flags) )

    TileL2Builder=CompFactory.TileL2Builder
    acc.setPrivateTools( TileL2Builder(**kwargs) )

    return acc



def TileRawChannelToL2Cfg(flags, **kwargs):
    """Return component accumulator with configured Tile raw channels to L2 algorithm

    Arguments:
        flags  -- Athena configuration flags
    """

    kwargs.setdefault('name', 'TileRawChannelToL2')

    if flags.Common.ProductionStep == ProductionStep.PileUpPresampling:
        kwargs.setdefault('TileL2Container', flags.Overlay.BkgPrefix + 'TileL2Cnt')
    else:
        kwargs.setdefault('TileL2Container', 'TileL2Cnt')

    acc = ComponentAccumulator()

    if 'TileL2Builder' not in kwargs:
        l2Builder = acc.popToolsAndMerge( TileL2BuilderCfg(flags) )
        kwargs['TileL2Builder'] = l2Builder


    TileRawChannelToL2=CompFactory.TileRawChannelToL2
    acc.addEventAlgo( TileRawChannelToL2(**kwargs), primary = True )

    return acc


def TileRawChannelToL2OutputCfg(flags, streamName = 'RDO', **kwargs):
    """Return component accumulator with configured Tile raw channels to L2 algorithm with Output stream

    Arguments:
        flags  -- Athena configuration flags
        streamName -- name of output stream. Defaults to RDO.
    """

    acc = TileRawChannelToL2Cfg(flags, **kwargs)
    tileRawChanToL2Alg = acc.getPrimary()

    if 'TileL2Container' in tileRawChanToL2Alg._properties:
        tileL2Container = tileRawChanToL2Alg._properties['TileL2Container']
    else:
        tileL2Container = tileRawChanToL2Alg._descriptors['TileL2Container'].default

    if flags.Output.doWriteRDO:
        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
        acc.merge( OutputStreamCfg(flags, streamName, [f'TileL2Container#{tileL2Container}']) )

    return acc


if __name__ == "__main__":

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultConditionsTags, defaultGeometryTags, defaultTestFiles
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG

    # Test setup
    log.setLevel(DEBUG)

    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2
    flags.GeoModel.AtlasVersion = defaultGeometryTags.RUN2
    flags.IOVDb.GlobalTag = defaultConditionsTags.RUN2_DATA
    flags.fillFromArgs()
    flags.Output.ESDFileName = "myESD.pool.root"
    flags.lock()

    # Construct our accumulator to run
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from TileByteStream.TileByteStreamConfig import TileRawDataReadingCfg
    acc.merge( TileRawDataReadingCfg(flags, readDigits=False, readMuRcv=False) )

    acc.merge( TileRawChannelToL2OutputCfg(flags, streamName = 'ESD') )
    acc.getService('StoreGateSvc').Dump = True

    flags.dump()
    acc.printConfig(withDetails = True, summariseProps = True)
    acc.store( open('TileL2.pkl','wb') )


    sc = acc.run(maxEvents=3)

    # Success should be 0
    import sys
    sys.exit(not sc.isSuccess())
