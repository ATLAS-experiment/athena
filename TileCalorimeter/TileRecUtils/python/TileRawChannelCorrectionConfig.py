# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

"""Define method to construct configured Tile correction tools and algorithm"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from TileConfiguration.TileConfigFlags import TileRunType

def TileRawChannelOF1CorrectorCfg(flags, **kwargs):
    """Return component accumulator with configured private Tile OF1 raw channel correction tool

    Arguments:
        flags  -- Athena configuration flags
    """

    acc = ComponentAccumulator()

    kwargs.setdefault('CorrectPedestalDifference', flags.Tile.correctPedestalDifference)
    kwargs.setdefault('ZeroAmplitudeWithoutDigits', flags.Tile.zeroAmplitudeWithoutDigits)
    kwargs.setdefault('TileDigitsContainer', 'TileDigitsCnt')

    if kwargs['CorrectPedestalDifference']:
        from TileConditions.TileSampleNoiseConfig import TileSampleNoiseCondAlgCfg
        acc.merge( TileSampleNoiseCondAlgCfg(flags, TileSampleNoise="TileSampleNoise") )
        acc.merge( TileSampleNoiseCondAlgCfg(flags, ForceOnline=True, TileSampleNoise="TileOnlineSampleNoise") )

        if 'TileCondToolTiming' not in kwargs:
            from TileConditions.TileTimingConfig import TileCondToolOnlineTimingCfg
            kwargs['TileCondToolTiming'] = acc.popToolsAndMerge( TileCondToolOnlineTimingCfg(flags) )

        if 'TileCondToolOfc' not in kwargs:
            from TileConditions.TileOFCConfig import TileCondToolOfcCoolCfg
            kwargs['TileCondToolOfc'] = acc.popToolsAndMerge( TileCondToolOfcCoolCfg(flags, OfcType = 'OF1') )

    if kwargs['ZeroAmplitudeWithoutDigits']:

        if 'TileCondToolDspThreshold' not in kwargs:
            from TileConditions.TileDSPThresholdConfig import TileCondToolDspThresholdCfg
            kwargs['TileCondToolDspThreshold'] = acc.popToolsAndMerge( TileCondToolDspThresholdCfg(flags) )

    if kwargs['CorrectPedestalDifference'] or kwargs['ZeroAmplitudeWithoutDigits']:
        from TileConditions.TileEMScaleConfig import TileEMScaleCondAlgCfg
        acc.merge( TileEMScaleCondAlgCfg(flags) )

    TileRawChannelOF1Corrector=CompFactory.TileRawChannelOF1Corrector
    acc.setPrivateTools( TileRawChannelOF1Corrector(**kwargs) )

    return acc



def TileRawChannelNoiseFilterCfg(flags, **kwargs):
    """Return component accumulator with configured private Tile raw channel noise filter tool

    Arguments:
        flags  -- Athena configuration flags (ConfigFlags)
    """

    acc = ComponentAccumulator()

    from TileRecUtils.TileDQstatusConfig import TileDQstatusAlgCfg
    acc.merge( TileDQstatusAlgCfg(flags) )

    from TileConditions.TileInfoLoaderConfig import TileInfoLoaderCfg
    acc.merge( TileInfoLoaderCfg(flags) )

    from TileConditions.TileEMScaleConfig import TileEMScaleCondAlgCfg
    acc.merge( TileEMScaleCondAlgCfg(flags) )

    from TileConditions.TileSampleNoiseConfig import TileSampleNoiseCondAlgCfg
    acc.merge( TileSampleNoiseCondAlgCfg(flags) )

    from TileConditions.TileBadChannelsConfig import TileBadChannelsCondAlgCfg
    acc.merge( TileBadChannelsCondAlgCfg(flags) )

    TileRawChannelNoiseFilter=CompFactory.TileRawChannelNoiseFilter
    acc.setPrivateTools( TileRawChannelNoiseFilter(**kwargs) )

    return acc



def TileTimeBCOffsetFilterCfg(flags, **kwargs):
    """Return component accumulator with configured private Tile raw channel timing jump correction tool

    Arguments:
        flags  -- Athena configuration flags (ConfigFlags)
        EneThreshold3 - energy threshold on 3 channels in one DMU (in MeV)
        EneThreshold1 - energy threshold on 1 channel (in MeV)
        TimeThreshold - threshold on time difference (in ns)
    """

    acc = ComponentAccumulator()
    kwargs.setdefault('CheckDCS', flags.Tile.useDCS)
    kwargs.setdefault('EneThreshold3', 1000)
    kwargs.setdefault('EneThreshold1', 3000)
    kwargs.setdefault('TimeThreshold', 15)
    kwargs.setdefault('AverTimeEneThreshold', 500)
    kwargs.setdefault('RefTimeThreshold', 10)
    kwargs.setdefault('SampleDiffMaxMin_HG', 15)
    kwargs.setdefault('SampleDiffMaxMin_LG', -1)
    
    from TileRecUtils.TileDQstatusConfig import TileDQstatusAlgCfg
    acc.merge( TileDQstatusAlgCfg(flags) )

    from TileConditions.TileCablingSvcConfig import TileCablingSvcCfg
    acc.merge(TileCablingSvcCfg(flags))

    from TileConditions.TileEMScaleConfig import TileEMScaleCondAlgCfg
    acc.merge( TileEMScaleCondAlgCfg(flags) )

    from TileConditions.TileBadChannelsConfig import TileBadChannelsCondAlgCfg
    acc.merge( TileBadChannelsCondAlgCfg(flags) )

    if kwargs['CheckDCS']:
        from TileConditions.TileDCSConfig import TileDCSCondAlgCfg
        acc.merge( TileDCSCondAlgCfg(flags) )

    TileTimeBCOffsetFilter=CompFactory.TileTimeBCOffsetFilter
    acc.setPrivateTools( TileTimeBCOffsetFilter(**kwargs) )

    return acc



def TileRawChannelCorrectionToolsCfg(flags, **kwargs):
    """Return component accumulator with configured private Tile raw channel correction tools

    Arguments:
        flags  -- Athena configuration flags (ConfigFlags)
    """

    acc = ComponentAccumulator()

    noiseFilterTools = []

    if flags.Tile.correctPedestalDifference or flags.Tile.zeroAmplitudeWithoutDigits:
        noiseFilterTools += [ acc.popToolsAndMerge( TileRawChannelOF1CorrectorCfg(flags) ) ]

    if flags.Tile.NoiseFilter == 1:
        noiseFilterTools += [ acc.popToolsAndMerge( TileRawChannelNoiseFilterCfg(flags) ) ]

    if flags.Tile.correctTimeJumps:
        noiseFilterTools += [ acc.popToolsAndMerge( TileTimeBCOffsetFilterCfg(flags) ) ]

    acc.setPrivateTools( noiseFilterTools )

    return acc


def TileRawChannelCorrectionAlgCfg(flags, **kwargs):
    """Return component accumulator with configured Tile raw channel correction algorithm

    Arguments:
        flags  -- Athena configuration flags (ConfigFlags)

    Keyword arguments:
        InputRawChannelContainer -- input Tile raw channel container. Defaults to TileRawChannelCnt.
        OutputRawChannelContainer -- output Tile raw channel container. Defaults to TileRawChannelCntCorrected.
    """

    acc = ComponentAccumulator()

    kwargs.setdefault('InputRawChannelContainer', 'TileRawChannelCnt')
    kwargs.setdefault('OutputRawChannelContainer', 'TileRawChannelCntCorrected')

    if 'NoiseFilterTools' not in kwargs:
        kwargs['NoiseFilterTools'] = acc.popToolsAndMerge( TileRawChannelCorrectionToolsCfg(flags) )

    TileRawChannelCorrectionAlg=CompFactory.TileRawChannelCorrectionAlg
    acc.addEventAlgo(TileRawChannelCorrectionAlg(**kwargs), primary = True)

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
    flags.Tile.correctPedestalDifference = True
    flags.Tile.zeroAmplitudeWithoutDigits = True
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    from TileByteStream.TileByteStreamConfig import TileRawDataReadingCfg
    acc.merge( TileRawDataReadingCfg(flags, readMuRcv=False) )

    acc.merge( TileRawChannelCorrectionAlgCfg(flags) )

    flags.dump()
    acc.printConfig(withDetails = True, summariseProps = True)
    acc.store( open('TileRawChannelCorrection.pkl','wb') )

    sc = acc.run(maxEvents = 3)

    import sys
    # Success should be 0
    sys.exit(not sc.isSuccess())

