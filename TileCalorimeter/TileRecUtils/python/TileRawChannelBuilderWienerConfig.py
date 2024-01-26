# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

"""Define method to construct configured Tile Wiener raw channel builder tool"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from TileConfiguration.TileConfigFlags import TileRunType

def TileRawChannelBuilderWienerCfg(flags, **kwargs):
    """Return component accumulator with configured private Tile Wiener raw channel builder tool

    Arguments:
        flags  -- Athena configuration flags
    """

    name = kwargs.pop('name', 'TileRawChannelBuilderWiener')
    kwargs.setdefault('TileRawChannelContainer', 'TileRawChannelWiener')

    from LumiBlockComps.BunchCrossingCondAlgConfig import BunchCrossingCondAlgCfg
    acc = BunchCrossingCondAlgCfg(flags)

    if 'TileCondToolNoiseSample' not in kwargs:
        from TileConditions.TileSampleNoiseConfig import TileCondToolNoiseSampleCfg
        sampleNoiseTool = acc.popToolsAndMerge( TileCondToolNoiseSampleCfg(flags) )
        kwargs['TileCondToolNoiseSample'] = sampleNoiseTool

    kwargs.setdefault('correctTime', flags.Tile.correctTime)
    kwargs.setdefault('MC', flags.Input.isMC)
    kwargs.setdefault('BestPhase', False)
    kwargs.setdefault('MaxIterations', 5) # iterative mode on
    kwargs.setdefault('Minus1Iteration', True)
    kwargs.setdefault('PedestalMode', 1)
    kwargs.setdefault('AmplitudeCorrection', False) # don't need correction after iterations
    kwargs.setdefault('TimeCorrection', False)      # don't need correction after iterations

    if flags.Tile.correctTime and 'TileCondToolTiming' not in kwargs:
        from TileConditions.TileTimingConfig import TileCondToolTimingCfg
        timingTool = acc.popToolsAndMerge( TileCondToolTimingCfg(flags) )
        kwargs['TileCondToolTiming'] = timingTool

    TileRawChannelBuilderWiener=CompFactory.TileRawChannelBuilderWienerFilter
    from TileRecUtils.TileRawChannelBuilderConfig import TileRawChannelBuilderCfg
    rawChanBuilder = acc.popToolsAndMerge( TileRawChannelBuilderCfg(flags, name, TileRawChannelBuilderWiener, **kwargs) )
    acc.setPrivateTools(rawChanBuilder)

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
    flags.Tile.NoiseFilter = 1
    flags.lock()

    flags.dump()

    acc = ComponentAccumulator()

    #printing Configurables isn't reliable with GaudiConfig2
    acc.popToolsAndMerge( TileRawChannelBuilderWienerCfg(flags) )

    acc.printConfig(withDetails = True, summariseProps = True)
    acc.store( open('TileRawChannelBuilderWiener.pkl','wb') )
