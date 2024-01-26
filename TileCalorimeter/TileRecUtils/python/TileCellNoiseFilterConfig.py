# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

"""Define method to construct configured private Tile Cell noise filter tool"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from TileConfiguration.TileConfigFlags import TileRunType

def TileCellNoiseFilterCfg(flags, **kwargs):
    """Return component accumulator with configured private Tile cell noise filter tool

    Arguments:
        flags  -- Athena configuration flags
        UseCaloNoise -- use Calo noise conditions object. Defaults to False.
    """

    acc = ComponentAccumulator()

    useCaloNoise = kwargs.get('UseCaloNoise', False)

    from TileGeoModel.TileGMConfig import TileGMCfg
    acc.merge(TileGMCfg(flags))

    TileCellNoiseFilter=CompFactory.TileCellNoiseFilter
    tileCellNoiseFilter = TileCellNoiseFilter()

    from TileConditions.TileEMScaleConfig import TileEMScaleCondAlgCfg
    acc.merge( TileEMScaleCondAlgCfg(flags) )

    if useCaloNoise:
        from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
        acc.merge( CaloNoiseCondAlgCfg(flags, 'electronicNoise') )
        tileCellNoiseFilter.CaloNoise = 'electronicNoise'
    else:
        from TileConditions.TileSampleNoiseConfig import TileSampleNoiseCondAlgCfg
        acc.merge( TileSampleNoiseCondAlgCfg(flags) )

        from TileConditions.TileBadChannelsConfig import TileBadChanToolCfg
        badChanTool = acc.popToolsAndMerge( TileBadChanToolCfg(flags) )
        tileCellNoiseFilter.TileBadChanTool = badChanTool

    acc.setPrivateTools( tileCellNoiseFilter )

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
    flags.Tile.NoiseFilter = 111
    flags.fillFromArgs()
    flags.lock()

    acc = ComponentAccumulator()

    print( acc.popToolsAndMerge( TileCellNoiseFilterCfg(flags) ) )

    flags.dump()
    acc.printConfig(withDetails = True, summariseProps = True)
    acc.store( open('TileCellNoiseFilter.pkl','wb') )
