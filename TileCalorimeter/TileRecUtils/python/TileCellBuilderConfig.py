# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

"""Define method to construct configured Tile Cell builder tool"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod
from TileConfiguration.TileConfigFlags import TileRunType

def TileCellBuilderCfg(flags, mergeChannels=True, **kwargs):
    """Return component accumulator with configured private Tile Cell builder tool

    Arguments:
        flags  -- Athena configuration flags
        SkipGain - skip given gain. Defaults to -1 [use all gains]. Possible values: 0 [LG], 1 [HG].
        mergeChannels -- merge DSP results with offline reco results. Defaults to True.
    """

    acc = ComponentAccumulator()
    kwargs.setdefault('CheckDCS', flags.Tile.useDCS)
    kwargs.setdefault('TileRawChannelContainer', flags.Tile.RawChannelContainer)
    kwargs.setdefault('SkipGain', -1) # Never skip any gain by default

    kwargs.setdefault('MBTSContainer', 'MBTSContainer' if flags.GeoModel.Run in [LHCPeriod.Run1, LHCPeriod.Run2, LHCPeriod.Run3] else "")
    kwargs.setdefault('E4prContainer', 'E4prContainer' if flags.GeoModel.Run is LHCPeriod.Run2 else "")

    kwargs['mergeChannels'] = mergeChannels

    if kwargs['SkipGain'] not in [-1, 0, 1]:
        raise(Exception("Invalid Tile gain requsted to be skipped: %s" % kwargs['SkipGain']))

    from TileRecUtils.TileDQstatusConfig import TileDQstatusAlgCfg
    acc.merge( TileDQstatusAlgCfg(flags) )

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    acc.merge(LArGMCfg(flags))

    from TileGeoModel.TileGMConfig import TileGMCfg
    acc.merge(TileGMCfg(flags))

    from TileConditions.TileInfoLoaderConfig import TileInfoLoaderCfg
    acc.merge( TileInfoLoaderCfg(flags) )

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    acc.merge(TileCablingSvcCfg(flags))

    from TileConditions.TileBadChannelsConfig import TileBadChannelsCondAlgCfg
    acc.merge( TileBadChannelsCondAlgCfg(flags) )

    from TileConditions.TileEMScaleConfig import TileEMScaleCondAlgCfg
    acc.merge( TileEMScaleCondAlgCfg(flags) )

    if 'TileCondToolTiming' not in kwargs:
        from TileConditions.TileTimingConfig import TileCondToolTimingCfg
        kwargs['TileCondToolTiming'] = acc.popToolsAndMerge( TileCondToolTimingCfg(flags) )

    if kwargs['CheckDCS']:
        from TileConditions.TileDCSConfig import TileDCSCondAlgCfg
        acc.merge( TileDCSCondAlgCfg(flags) )

    if not (flags.Input.isMC or flags.Overlay.DataOverlay) and mergeChannels and 'TileDSPRawChannelContainer' not in kwargs:
        from TileRecUtils.TileRawChannelCorrectionConfig import TileRawChannelCorrectionAlgCfg
        corrAlgAcc = TileRawChannelCorrectionAlgCfg(flags)
        tileRawChannelCorrectionAlg = corrAlgAcc.getPrimary()
        tileRawChannelContainerDSP = tileRawChannelCorrectionAlg.OutputRawChannelContainer
        kwargs['TileDSPRawChannelContainer'] = tileRawChannelContainerDSP
        acc.merge( corrAlgAcc )

    TileCellBuilder=CompFactory.TileCellBuilder
    acc.setPrivateTools( TileCellBuilder(**kwargs) )

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
    flags.fillFromArgs()
    flags.lock()

    acc = ComponentAccumulator()

    print( acc.popToolsAndMerge( TileCellBuilderCfg(flags) ) )

    flags.dump()
    acc.printConfig(withDetails = True, summariseProps = True)
    acc.store( open('TileCellBuilder.pkl','wb') )
